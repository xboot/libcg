#include "sw_ft_raster.h"
#include "sw_ft_math.h"

#define SW_FT_ERR_XCAT(x, y) x##y
#define SW_FT_ERR_CAT(x, y) SW_FT_ERR_XCAT(x, y)
#define SW_FT_BEGIN_STMNT do {
#define SW_FT_END_STMNT \
    }                   \
    while (0)
#define SW_FT_UINT_MAX UINT_MAX
#define SW_FT_INT_MAX INT_MAX
#define SW_FT_ULONG_MAX ULONG_MAX
#define SW_FT_CHAR_BIT CHAR_BIT

#define ft_memset memset
#define ft_setjmp setjmp
#define ft_longjmp longjmp
#define ft_jmp_buf jmp_buf
typedef ptrdiff_t SW_FT_PtrDist;

#define ErrRaster_Invalid_Mode -2
#define ErrRaster_Invalid_Outline -1
#define ErrRaster_Invalid_Argument -3
#define ErrRaster_Memory_Overflow -4
#define SW_FT_BEGIN_HEADER
#define SW_FT_END_HEADER

#define SW_FT_UNUSED(x) (x) = (x)
#define SW_FT_THROW(e) SW_FT_ERR_CAT(ErrRaster_, e)
#define SW_FT_RENDER_POOL_SIZE 16384L

typedef int (*SW_FT_Outline_MoveToFunc)(const SW_FT_Vector* to, void* user);
#define SW_FT_Outline_MoveTo_Func SW_FT_Outline_MoveToFunc
typedef int (*SW_FT_Outline_LineToFunc)(const SW_FT_Vector* to, void* user);
#define SW_FT_Outline_LineTo_Func SW_FT_Outline_LineToFunc
typedef int (*SW_FT_Outline_ConicToFunc)(const SW_FT_Vector* control,
                                         const SW_FT_Vector* to, void* user);
#define SW_FT_Outline_ConicTo_Func SW_FT_Outline_ConicToFunc
typedef int (*SW_FT_Outline_CubicToFunc)(const SW_FT_Vector* control1,
                                         const SW_FT_Vector* control2,
                                         const SW_FT_Vector* to, void* user);
#define SW_FT_Outline_CubicTo_Func SW_FT_Outline_CubicToFunc

typedef struct SW_FT_Outline_Funcs_ {
	SW_FT_Outline_MoveToFunc move_to;
	SW_FT_Outline_LineToFunc line_to;
	SW_FT_Outline_ConicToFunc conic_to;
	SW_FT_Outline_CubicToFunc cubic_to;
	int shift;
	SW_FT_Pos delta;
} SW_FT_Outline_Funcs;

#define SW_FT_DEFINE_OUTLINE_FUNCS(class_, move_to_, line_to_, conic_to_,      \
                                   cubic_to_, shift_, delta_)                  \
    static const SW_FT_Outline_Funcs class_ = {move_to_,  line_to_, conic_to_, \
                                               cubic_to_, shift_,   delta_};

#define SW_FT_DEFINE_RASTER_FUNCS(class_, raster_new_, raster_reset_, \
                                  raster_render_, raster_done_)       \
    const SW_FT_Raster_Funcs class_ = {raster_new_, raster_reset_,    \
                                       raster_render_, raster_done_};

#ifndef SW_FT_MEM_SET
#define SW_FT_MEM_SET(d, s, c) ft_memset(d, s, c)
#endif

#ifndef SW_FT_MEM_ZERO
#define SW_FT_MEM_ZERO(dest, count) SW_FT_MEM_SET(dest, 0, count)
#endif

#undef RAS_ARG
#undef RAS_ARG_
#undef RAS_VAR
#undef RAS_VAR_

#ifndef SW_FT_STATIC_RASTER
#define RAS_ARG gray_PWorker worker
#define RAS_ARG_ gray_PWorker worker,
#define RAS_VAR worker
#define RAS_VAR_ worker,
#else
#define RAS_ARG
#define RAS_ARG_
#define RAS_VAR
#define RAS_VAR_
#endif

#define PIXEL_BITS	8
#undef FLOOR
#undef CEILING
#undef TRUNC
#undef SCALED

#define ONE_PIXEL (1L << PIXEL_BITS)
#define PIXEL_MASK (-1L << PIXEL_BITS)
#define TRUNC(x) ((TCoord)((x) >> PIXEL_BITS))
#define SUBPIXELS(x) ((TPos)(x) << PIXEL_BITS)
#define FLOOR(x) ((x) & -ONE_PIXEL)
#define CEILING(x) (((x) + ONE_PIXEL - 1) & -ONE_PIXEL)
#define ROUND(x) (((x) + ONE_PIXEL / 2) & -ONE_PIXEL)

#if PIXEL_BITS >= 6
#define UPSCALE(x) ((x) << (PIXEL_BITS - 6))
#define DOWNSCALE(x) ((x) >> (PIXEL_BITS - 6))
#else
#define UPSCALE(x) ((x) >> (6 - PIXEL_BITS))
#define DOWNSCALE(x) ((x) << (6 - PIXEL_BITS))
#endif

#define SW_FT_DIV_MOD(type, dividend, divisor, quotient, remainder) \
    SW_FT_BEGIN_STMNT(quotient) = (type)((dividend) / (divisor));   \
    (remainder) = (type)((dividend) % (divisor));                   \
    if ((remainder) < 0) {                                          \
        (quotient)--;                                               \
        (remainder) += (type)(divisor);                             \
    }                                                               \
    SW_FT_END_STMNT

#ifdef __arm__
#undef SW_FT_DIV_MOD
#define SW_FT_DIV_MOD(type, dividend, divisor, quotient, remainder) \
    SW_FT_BEGIN_STMNT(quotient) = (type)((dividend) / (divisor));   \
    (remainder) = (type)((dividend) - (quotient) * (divisor));      \
    if ((remainder) < 0) {                                          \
        (quotient)--;                                               \
        (remainder) += (type)(divisor);                             \
    }                                                               \
    SW_FT_END_STMNT
#endif
#define SW_FT_UDIVPREP(b) \
    long b##_r = (long)(SW_FT_ULONG_MAX >> PIXEL_BITS) / (b)
#define SW_FT_UDIV(a, b)                              \
    (((unsigned long)(a) * (unsigned long)(b##_r)) >> \
     (sizeof(long) * SW_FT_CHAR_BIT - PIXEL_BITS))

typedef long TCoord;
typedef long TPos;

#if PIXEL_BITS <= 7
typedef int TArea;
#else
#if SW_FT_UINT_MAX == 0xFFFFU
typedef long Area;
#else
typedef int TArea;
#endif
#endif

#define SW_FT_MAX_GRAY_SPANS	256
typedef struct TCell_* PCell;

typedef struct TCell_ {
	TPos x;
	TCoord cover;
	TArea area;
	PCell next;
} TCell;

typedef struct gray_TWorker_ {
	TCoord ex, ey;
	TPos min_ex, max_ex;
	TPos min_ey, max_ey;
	TPos count_ex, count_ey;
	TArea area;
	TCoord cover;
	int invalid;
	PCell cells;
	SW_FT_PtrDist max_cells;
	SW_FT_PtrDist num_cells;
	TPos x, y;
	SW_FT_Vector bez_stack[32 * 3 + 1];
	int lev_stack[32];
	SW_FT_Outline outline;
	SW_FT_BBox clip_box;
	int bound_left;
	int bound_top;
	int bound_right;
	int bound_bottom;
	SW_FT_Span gray_spans[SW_FT_MAX_GRAY_SPANS];
	int num_gray_spans;
	SW_FT_Raster_Span_Func render_span;
	void *render_span_data;
	int band_size;
	int band_shoot;
	ft_jmp_buf jump_buffer;
	void *buffer;
	long buffer_size;
	PCell *ycells;
	TPos ycount;
} gray_TWorker, *gray_PWorker;

#ifndef SW_FT_STATIC_RASTER
#define ras (*worker)
#else
static gray_TWorker ras;
#endif

typedef struct gray_TRaster_ {
	void * memory;
} gray_TRaster, *gray_PRaster;

static void gray_init_cells(RAS_ARG_ void* buffer, long byte_size)
{
	ras.buffer = buffer;
	ras.buffer_size = byte_size;
	ras.ycells = (PCell*)buffer;
	ras.cells = NULL;
	ras.max_cells = 0;
	ras.num_cells = 0;
	ras.area = 0;
	ras.cover = 0;
	ras.invalid = 1;
	ras.bound_left = INT_MAX;
	ras.bound_top = INT_MAX;
	ras.bound_right = INT_MIN;
	ras.bound_bottom = INT_MIN;
}

static void gray_compute_cbox(RAS_ARG)
{
	SW_FT_Outline *outline = &ras.outline;
	SW_FT_Vector *vec = outline->points;
	SW_FT_Vector *limit = vec + outline->n_points;

	if(outline->n_points <= 0)
	{
		ras.min_ex = ras.max_ex = 0;
		ras.min_ey = ras.max_ey = 0;
		return;
	}
	ras.min_ex = ras.max_ex = vec->x;
	ras.min_ey = ras.max_ey = vec->y;
	vec++;
	for(; vec < limit; vec++)
	{
		TPos x = vec->x;
		TPos y = vec->y;
		if(x < ras.min_ex)
			ras.min_ex = x;
		if(x > ras.max_ex)
			ras.max_ex = x;
		if(y < ras.min_ey)
			ras.min_ey = y;
		if(y > ras.max_ey)
			ras.max_ey = y;
	}
	ras.min_ex = ras.min_ex >> 6;
	ras.min_ey = ras.min_ey >> 6;
	ras.max_ex = (ras.max_ex + 63) >> 6;
	ras.max_ey = (ras.max_ey + 63) >> 6;
}

static PCell gray_find_cell(RAS_ARG)
{
	PCell *pcell, cell;
	TPos x = ras.ex;

	if(x > ras.count_ex)
		x = ras.count_ex;
	pcell = &ras.ycells[ras.ey];
	for(;;)
	{
		cell = *pcell;
		if(cell == NULL || cell->x > x)
			break;
		if(cell->x == x)
			goto Exit;
		pcell = &cell->next;
	}
	if(ras.num_cells >= ras.max_cells)
		ft_longjmp(ras.jump_buffer, 1);
	cell = ras.cells + ras.num_cells++;
	cell->x = x;
	cell->area = 0;
	cell->cover = 0;
	cell->next = *pcell;
	*pcell = cell;
Exit:
	return cell;
}

static void gray_record_cell(RAS_ARG)
{
	if(ras.area | ras.cover)
	{
		PCell cell = gray_find_cell(RAS_VAR);
		cell->area += ras.area;
		cell->cover += ras.cover;
	}
}

static void gray_set_cell(RAS_ARG_ TCoord ex, TCoord ey)
{
	ey -= ras.min_ey;
	if(ex > ras.max_ex)
		ex = ras.max_ex;
	ex -= ras.min_ex;
	if(ex < 0)
		ex = -1;
	if(ex != ras.ex || ey != ras.ey)
	{
		if(!ras.invalid)
			gray_record_cell(RAS_VAR);
		ras.area = 0;
		ras.cover = 0;
		ras.ex = ex;
		ras.ey = ey;
	}
	ras.invalid = ((unsigned)ey >= (unsigned)ras.count_ey || ex >= ras.count_ex);
}

static void gray_start_cell(RAS_ARG_ TCoord ex, TCoord ey)
{
	if(ex > ras.max_ex)
		ex = (TCoord)(ras.max_ex);
	if(ex < ras.min_ex)
		ex = (TCoord)(ras.min_ex - 1);
	ras.area = 0;
	ras.cover = 0;
	ras.ex = ex - ras.min_ex;
	ras.ey = ey - ras.min_ey;
	ras.invalid = 0;
	gray_set_cell(RAS_VAR_ ex, ey);
}

static void gray_render_line(RAS_ARG_ TPos to_x, TPos to_y)
{
	TPos dx, dy, fx1, fy1, fx2, fy2;
	TCoord ex1, ex2, ey1, ey2;

	ex1 = TRUNC(ras.x);
	ex2 = TRUNC(to_x);
	ey1 = TRUNC(ras.y);
	ey2 = TRUNC(to_y);
	if((ey1 >= ras.max_ey && ey2 >= ras.max_ey) || (ey1 < ras.min_ey && ey2 < ras.min_ey))
		goto End;
	dx = to_x - ras.x;
	dy = to_y - ras.y;
	fx1 = ras.x - SUBPIXELS(ex1);
	fy1 = ras.y - SUBPIXELS(ey1);
	if(ex1 == ex2 && ey1 == ey2)
		;
	else if(dy == 0)
	{
		ex1 = ex2;
		gray_set_cell(RAS_VAR_ ex1, ey1);
	}
	else if(dx == 0)
	{
		if(dy > 0)
			do {
				fy2 = ONE_PIXEL;
				ras.cover += (fy2 - fy1);
				ras.area += (fy2 - fy1) * fx1 * 2;
				fy1 = 0;
				ey1++;
				gray_set_cell(RAS_VAR_ ex1, ey1);
			} while(ey1 != ey2);
		else
			do {
				fy2 = 0;
				ras.cover += (fy2 - fy1);
				ras.area += (fy2 - fy1) * fx1 * 2;
				fy1 = ONE_PIXEL;
				ey1--;
				gray_set_cell(RAS_VAR_ ex1, ey1);
			} while(ey1 != ey2);
	}
	else
	{
		TArea prod = dx * fy1 - dy * fx1;
		SW_FT_UDIVPREP(dx);
		SW_FT_UDIVPREP(dy);
		do {
			if(prod <= 0 && prod - dx * ONE_PIXEL > 0)
			{
				fx2 = 0;
				fy2 = (TPos)SW_FT_UDIV(-prod, -dx);
				prod -= dy * ONE_PIXEL;
				ras.cover += (fy2 - fy1);
				ras.area += (fy2 - fy1) * (fx1 + fx2);
				fx1 = ONE_PIXEL;
				fy1 = fy2;
				ex1--;
			}
			else if(prod - dx * ONE_PIXEL <= 0 && prod - dx * ONE_PIXEL + dy * ONE_PIXEL > 0)
			{
				prod -= dx * ONE_PIXEL;
				fx2 = (TPos)SW_FT_UDIV(-prod, dy);
				fy2 = ONE_PIXEL;
				ras.cover += (fy2 - fy1);
				ras.area += (fy2 - fy1) * (fx1 + fx2);
				fx1 = fx2;
				fy1 = 0;
				ey1++;
			}
			else if(prod - dx * ONE_PIXEL + dy * ONE_PIXEL <= 0 && prod + dy * ONE_PIXEL >= 0)
			{
				prod += dy * ONE_PIXEL;
				fx2 = ONE_PIXEL;
				fy2 = (TPos)SW_FT_UDIV(prod, dx);
				ras.cover += (fy2 - fy1);
				ras.area += (fy2 - fy1) * (fx1 + fx2);
				fx1 = 0;
				fy1 = fy2;
				ex1++;
			}
			else
			{
				fx2 = (TPos)SW_FT_UDIV(prod, -dy);
				fy2 = 0;
				prod += dx * ONE_PIXEL;
				ras.cover += (fy2 - fy1);
				ras.area += (fy2 - fy1) * (fx1 + fx2);
				fx1 = fx2;
				fy1 = ONE_PIXEL;
				ey1--;
			}
			gray_set_cell(RAS_VAR_ ex1, ey1);
		} while(ex1 != ex2 || ey1 != ey2);
	}
	fx2 = to_x - SUBPIXELS(ex2);
	fy2 = to_y - SUBPIXELS(ey2);
	ras.cover += (fy2 - fy1);
	ras.area += (fy2 - fy1) * (fx1 + fx2);
End:
	ras.x = to_x;
	ras.y = to_y;
}

static void gray_split_conic(SW_FT_Vector * base)
{
	TPos a, b;

	base[4].x = base[2].x;
	a = base[0].x + base[1].x;
	b = base[1].x + base[2].x;
	base[3].x = b >> 1;
	base[2].x = (a + b) >> 2;
	base[1].x = a >> 1;

	base[4].y = base[2].y;
	a = base[0].y + base[1].y;
	b = base[1].y + base[2].y;
	base[3].y = b >> 1;
	base[2].y = (a + b) >> 2;
	base[1].y = a >> 1;
}

static void gray_render_conic(RAS_ARG_ const SW_FT_Vector * control, const SW_FT_Vector * to)
{
	TPos dx, dy;
	TPos min, max, y;
	int top, level;
	int *levels;
	SW_FT_Vector *arc;

	levels = ras.lev_stack;
	arc = ras.bez_stack;
	arc[0].x = UPSCALE(to->x);
	arc[0].y = UPSCALE(to->y);
	arc[1].x = UPSCALE(control->x);
	arc[1].y = UPSCALE(control->y);
	arc[2].x = ras.x;
	arc[2].y = ras.y;
	top = 0;
	dx = SW_FT_ABS(arc[2].x + arc[0].x - 2 * arc[1].x);
	dy = SW_FT_ABS(arc[2].y + arc[0].y - 2 * arc[1].y);
	if(dx < dy)
		dx = dy;
	if(dx < ONE_PIXEL / 4)
		goto Draw;
	min = max = arc[0].y;
	y = arc[1].y;
	if(y < min)
		min = y;
	if(y > max)
		max = y;
	y = arc[2].y;
	if(y < min)
		min = y;
	if(y > max)
		max = y;
	if(TRUNC(min) >= ras.max_ey || TRUNC(max) < ras.min_ey)
		goto Draw;
	level = 0;
	do {
		dx >>= 2;
		level++;
	} while(dx > ONE_PIXEL / 4);
	levels[0] = level;
	do {
		level = levels[top];
		if(level > 0)
		{
			gray_split_conic(arc);
			arc += 2;
			top++;
			levels[top] = levels[top - 1] = level - 1;
			continue;
		}
Draw:
		gray_render_line(RAS_VAR_ arc[0].x, arc[0].y);
		top--;
		arc -= 2;
	} while(top >= 0);
}

static void gray_split_cubic(SW_FT_Vector * base)
{
	TPos a, b, c;

	base[6].x = base[3].x;
	a = base[0].x + base[1].x;
	b = base[1].x + base[2].x;
	c = base[2].x + base[3].x;
	base[5].x = c >> 1;
	c += b;
	base[4].x = c >> 2;
	base[1].x = a >> 1;
	a += b;
	base[2].x = a >> 2;
	base[3].x = (a + c) >> 3;

	base[6].y = base[3].y;
	a = base[0].y + base[1].y;
	b = base[1].y + base[2].y;
	c = base[2].y + base[3].y;
	base[5].y = c >> 1;
	c += b;
	base[4].y = c >> 2;
	base[1].y = a >> 1;
	a += b;
	base[2].y = a >> 2;
	base[3].y = (a + c) >> 3;
}

static void gray_render_cubic(RAS_ARG_ const SW_FT_Vector * control1, const SW_FT_Vector * control2, const SW_FT_Vector * to)
{
	SW_FT_Vector * arc = ras.bez_stack;

	arc[0].x = UPSCALE(to->x);
	arc[0].y = UPSCALE(to->y);
	arc[1].x = UPSCALE(control2->x);
	arc[1].y = UPSCALE(control2->y);
	arc[2].x = UPSCALE(control1->x);
	arc[2].y = UPSCALE(control1->y);
	arc[3].x = ras.x;
	arc[3].y = ras.y;

	if(( TRUNC( arc[0].y ) >= ras.max_ey &&
		TRUNC( arc[1].y ) >= ras.max_ey &&
		TRUNC( arc[2].y ) >= ras.max_ey &&
		TRUNC( arc[3].y ) >= ras.max_ey) || ( TRUNC( arc[0].y ) < ras.min_ey &&
		TRUNC( arc[1].y ) < ras.min_ey &&
		TRUNC( arc[2].y ) < ras.min_ey &&
		TRUNC( arc[3].y ) < ras.min_ey))
	{
		ras.x = arc[0].x;
		ras.y = arc[0].y;
		return;
	}
	for(;;)
	{
		if( SW_FT_ABS( 2 * arc[0].x - 3 * arc[1].x + arc[3].x ) > ONE_PIXEL / 2 ||
			SW_FT_ABS( 2 * arc[0].y - 3 * arc[1].y + arc[3].y ) > ONE_PIXEL / 2 ||
			SW_FT_ABS( arc[0].x - 3 * arc[2].x + 2 * arc[3].x ) > ONE_PIXEL / 2 ||
			SW_FT_ABS( arc[0].y - 3 * arc[2].y + 2 * arc[3].y ) > ONE_PIXEL / 2)
			goto Split;
		gray_render_line( RAS_VAR_ arc[0].x, arc[0].y);
		if(arc == ras.bez_stack)
			return;
		arc -= 3;
		continue;
		Split: gray_split_cubic(arc);
		arc += 3;
	}
}

static int gray_move_to(const SW_FT_Vector * to, gray_PWorker worker)
{
	TPos x, y;

	if(!ras.invalid)
		gray_record_cell(RAS_VAR);
	x = UPSCALE(to->x);
	y = UPSCALE(to->y);
	gray_start_cell(RAS_VAR_ TRUNC(x), TRUNC(y));
	worker->x = x;
	worker->y = y;
	return 0;
}

static int gray_line_to(const SW_FT_Vector *to, gray_PWorker worker)
{
	gray_render_line(RAS_VAR_ UPSCALE(to->x), UPSCALE(to->y));
	return 0;
}

static int gray_conic_to(const SW_FT_Vector * control, const SW_FT_Vector * to, gray_PWorker worker)
{
	gray_render_conic(RAS_VAR_ control, to);
	return 0;
}

static int gray_cubic_to(const SW_FT_Vector * control1, const SW_FT_Vector * control2, const SW_FT_Vector * to, gray_PWorker worker)
{
	gray_render_cubic(RAS_VAR_ control1, control2, to);
	return 0;
}

static void gray_hline(RAS_ARG_ TCoord x, TCoord y, TPos area, TCoord acount)
{
	int coverage;

	coverage = (int)(area >> (PIXEL_BITS * 2 + 1 - 8));
	if(coverage < 0)
		coverage = -coverage;
	if(ras.outline.flags & SW_FT_OUTLINE_EVEN_ODD_FILL)
	{
		coverage &= 511;
		if(coverage > 256)
			coverage = 512 - coverage;
		else if(coverage == 256)
			coverage = 255;
	}
	else
	{
		if(coverage >= 256)
			coverage = 255;
	}
	y += (TCoord)ras.min_ey;
	x += (TCoord)ras.min_ex;
	if(x >= 32767)
		x = 32767;
	if(y >= SW_FT_INT_MAX)
		y = SW_FT_INT_MAX;
	if(coverage)
	{
		SW_FT_Span * span;
		int count;

		if(x < ras.bound_left)
			ras.bound_left = x;
		if(y < ras.bound_top)
			ras.bound_top = y;
		if(y > ras.bound_bottom)
			ras.bound_bottom = y;
		if(x + acount > ras.bound_right)
			ras.bound_right = x + acount;
		count = ras.num_gray_spans;
		span = ras.gray_spans + count - 1;
		if(count > 0 && span->y == y && (int)span->x + span->len == (int)x && span->coverage == coverage)
		{
			span->len = (unsigned short)(span->len + acount);
			return;
		}
		if(count >= SW_FT_MAX_GRAY_SPANS)
		{
			if(ras.render_span && count > 0)
				ras.render_span(count, ras.gray_spans, ras.render_span_data);
			ras.num_gray_spans = 0;
			span = ras.gray_spans;
		}
		else
			span++;
		span->x = (short)x;
		span->y = (short)y;
		span->len = (unsigned short)acount;
		span->coverage = (unsigned char)coverage;
		ras.num_gray_spans++;
	}
}

static void gray_sweep(RAS_ARG)
{
	int yindex;

	if(ras.num_cells == 0)
		return;
	ras.num_gray_spans = 0;
	for(yindex = 0; yindex < ras.ycount; yindex++)
	{
		PCell cell = ras.ycells[yindex];
		TCoord cover = 0;
		TCoord x = 0;

		for(; cell != NULL; cell = cell->next)
		{
			TPos area;
			if(cell->x > x && cover != 0)
				gray_hline(RAS_VAR_ x, yindex, cover * (ONE_PIXEL * 2), cell->x - x);
			cover += cell->cover;
			area = cover * (ONE_PIXEL * 2) - cell->area;
			if(area != 0 && cell->x >= 0)
				gray_hline(RAS_VAR_ cell->x, yindex, area, 1);
			x = cell->x + 1;
		}
		if(cover != 0)
			gray_hline(RAS_VAR_ x, yindex, cover * (ONE_PIXEL * 2),
			ras.count_ex - x);
	}
	if(ras.render_span && ras.num_gray_spans > 0)
		ras.render_span(ras.num_gray_spans, ras.gray_spans, ras.render_span_data);
}

static int SW_FT_Outline_Decompose(const SW_FT_Outline * outline, const SW_FT_Outline_Funcs * func_interface, void * user)
{
#undef SCALED
#define SCALED(x) (((x) << shift) - delta)
	SW_FT_Vector v_last;
	SW_FT_Vector v_control;
	SW_FT_Vector v_start;
	SW_FT_Vector *point;
	SW_FT_Vector *limit;
	char *tags;
	int error;
	int n;
	int first;
	char tag;
	int shift;
	TPos delta;

	if(!outline || !func_interface)
		return SW_FT_THROW(Invalid_Argument);
	shift = func_interface->shift;
	delta = func_interface->delta;
	first = 0;
	for(n = 0; n < outline->n_contours; n++)
	{
		int last;
		last = outline->contours[n];
		if(last < 0)
			goto Invalid_Outline;
		limit = outline->points + last;
		v_start = outline->points[first];
		v_start.x = SCALED(v_start.x);
		v_start.y = SCALED(v_start.y);
		v_last = outline->points[last];
		v_last.x = SCALED(v_last.x);
		v_last.y = SCALED(v_last.y);
		v_control = v_start;
		point = outline->points + first;
		tags = outline->tags + first;
		tag = SW_FT_CURVE_TAG(tags[0]);
		if(tag == SW_FT_CURVE_TAG_CUBIC)
			goto Invalid_Outline;
		if(tag == SW_FT_CURVE_TAG_CONIC)
		{
			if(SW_FT_CURVE_TAG(outline->tags[last]) == SW_FT_CURVE_TAG_ON)
			{
				v_start = v_last;
				limit--;
			}
			else
			{
				v_start.x = (v_start.x + v_last.x) / 2;
				v_start.y = (v_start.y + v_last.y) / 2;
			}
			point--;
			tags--;
		}
		error = func_interface->move_to(&v_start, user);
		if(error)
			goto Exit;
		while(point < limit)
		{
			point++;
			tags++;
			tag = SW_FT_CURVE_TAG(tags[0]);
			switch(tag)
			{
			case SW_FT_CURVE_TAG_ON:
			{
				SW_FT_Vector vec;
				vec.x = SCALED(point->x);
				vec.y = SCALED(point->y);
				error = func_interface->line_to(&vec, user);
				if(error)
					goto Exit;
				continue;
			}
			case SW_FT_CURVE_TAG_CONIC:
				v_control.x = SCALED(point->x);
				v_control.y = SCALED(point->y);
Do_Conic:
				if(point < limit)
				{
					SW_FT_Vector vec;
					SW_FT_Vector v_middle;
					point++;
					tags++;
					tag = SW_FT_CURVE_TAG(tags[0]);
					vec.x = SCALED(point->x);
					vec.y = SCALED(point->y);
					if(tag == SW_FT_CURVE_TAG_ON)
					{
						error = func_interface->conic_to(&v_control, &vec, user);
						if(error)
							goto Exit;
						continue;
					}
					if(tag != SW_FT_CURVE_TAG_CONIC)
						goto Invalid_Outline;
					v_middle.x = (v_control.x + vec.x) / 2;
					v_middle.y = (v_control.y + vec.y) / 2;
					error = func_interface->conic_to(&v_control, &v_middle, user);
					if(error)
						goto Exit;
					v_control = vec;
					goto Do_Conic;
				}
				error = func_interface->conic_to(&v_control, &v_start, user);
				goto Close;
			default:
			{
				SW_FT_Vector vec1, vec2;
				if(point + 1 > limit ||
				SW_FT_CURVE_TAG(tags[1]) != SW_FT_CURVE_TAG_CUBIC)
					goto Invalid_Outline;
				point += 2;
				tags += 2;
				vec1.x = SCALED(point[-2].x);
				vec1.y = SCALED(point[-2].y);
				vec2.x = SCALED(point[-1].x);
				vec2.y = SCALED(point[-1].y);
				if(point <= limit)
				{
					SW_FT_Vector vec;
					vec.x = SCALED(point->x);
					vec.y = SCALED(point->y);
					error = func_interface->cubic_to(&vec1, &vec2, &vec, user);
					if(error)
						goto Exit;
					continue;
				}
				error = func_interface->cubic_to(&vec1, &vec2, &v_start, user);
				goto Close;
			}
			}
		}
		error = func_interface->line_to(&v_start, user);
Close:
		if(error)
			goto Exit;
		first = last + 1;
	}
	return 0;
Exit:
	return error;
Invalid_Outline:
	return SW_FT_THROW(Invalid_Outline);
}

typedef struct gray_TBand_ {
	TPos min, max;
} gray_TBand;

SW_FT_DEFINE_OUTLINE_FUNCS(func_interface,
		(SW_FT_Outline_MoveTo_Func)gray_move_to,
		(SW_FT_Outline_LineTo_Func)gray_line_to,
		(SW_FT_Outline_ConicTo_Func)gray_conic_to,
		(SW_FT_Outline_CubicTo_Func)gray_cubic_to, 0, 0)

static int gray_convert_glyph_inner(RAS_ARG)
{
	volatile int error = 0;

	if(ft_setjmp(ras.jump_buffer) == 0)
	{
		error = SW_FT_Outline_Decompose(&ras.outline, &func_interface, &ras);
		if(!ras.invalid)
			gray_record_cell(RAS_VAR);
	}
	else
		error = SW_FT_THROW(Memory_Overflow);
	return error;
}

static int gray_convert_glyph(RAS_ARG)
{
	gray_TBand bands[40];
	gray_TBand *volatile band;
	int volatile n, num_bands;
	TPos volatile min, max, max_y;
	SW_FT_BBox *clip;

	gray_compute_cbox(RAS_VAR);
	clip = &ras.clip_box;
	if(ras.max_ex <= clip->xMin || ras.min_ex >= clip->xMax ||
	ras.max_ey <= clip->yMin || ras.min_ey >= clip->yMax)
		return 0;
	if(ras.min_ex < clip->xMin)
		ras.min_ex = clip->xMin;
	if(ras.min_ey < clip->yMin)
		ras.min_ey = clip->yMin;
	if(ras.max_ex > clip->xMax)
		ras.max_ex = clip->xMax;
	if(ras.max_ey > clip->yMax)
		ras.max_ey = clip->yMax;
	ras.count_ex = ras.max_ex - ras.min_ex;
	ras.count_ey = ras.max_ey - ras.min_ey;
	num_bands = (int)((ras.max_ey - ras.min_ey) / ras.band_size);
	if(num_bands == 0)
		num_bands = 1;
	if(num_bands >= 39)
		num_bands = 39;
	ras.band_shoot = 0;
	min = ras.min_ey;
	max_y = ras.max_ey;
	for(n = 0; n < num_bands; n++, min = max)
	{
		max = min + ras.band_size;
		if(n == num_bands - 1 || max > max_y)
			max = max_y;
		bands[0].min = min;
		bands[0].max = max;
		band = bands;
		while(band >= bands)
		{
			TPos bottom, top, middle;
			int error;
			{
				PCell cells_max;
				int yindex;
				long cell_start, cell_end, cell_mod;
				ras.ycells = (PCell*)ras.buffer;
				ras.ycount = band->max - band->min;
				cell_start = sizeof(PCell) * ras.ycount;
				cell_mod = cell_start % sizeof(TCell);
				if(cell_mod > 0)
					cell_start += sizeof(TCell) - cell_mod;
				cell_end = ras.buffer_size;
				cell_end -= cell_end % sizeof(TCell);
				cells_max = (PCell)((char*)ras.buffer + cell_end);
				ras.cells = (PCell)((char*)ras.buffer + cell_start);
				if(ras.cells >= cells_max)
					goto ReduceBands;
				ras.max_cells = cells_max - ras.cells;
				if(ras.max_cells < 2)
					goto ReduceBands;
				for(yindex = 0; yindex < ras.ycount; yindex++)
					ras.ycells[yindex] = NULL;
			}
			ras.num_cells = 0;
			ras.invalid = 1;
			ras.min_ey = band->min;
			ras.max_ey = band->max;
			ras.count_ey = band->max - band->min;
			error = gray_convert_glyph_inner(RAS_VAR);
			if(!error)
			{
				gray_sweep(RAS_VAR);
				band--;
				continue;
			}
			else if(error != ErrRaster_Memory_Overflow)
				return 1;
ReduceBands:
			bottom = band->min;
			top = band->max;
			middle = bottom + ((top - bottom) >> 1);
			if(middle == bottom)
			{
				return 1;
			}
			if(bottom - top >= ras.band_size)
				ras.band_shoot++;
			band[1].min = bottom;
			band[1].max = middle;
			band[0].min = middle;
			band[0].max = top;
			band++;
		}
	}
	if(ras.band_shoot > 8 && ras.band_size > 16)
		ras.band_size = ras.band_size / 2;
	return 0;
}

static int gray_raster_render(gray_PRaster raster, const SW_FT_Raster_Params * params)
{
	SW_FT_UNUSED(raster);
	const SW_FT_Outline * outline = (const SW_FT_Outline*)params->source;
	gray_TWorker worker[1];
	TCell buffer[SW_FT_RENDER_POOL_SIZE / sizeof(TCell)];
	long buffer_size = sizeof(buffer);
	int band_size = (int)(buffer_size / (long)(sizeof(TCell) * 8));
	if(!outline)
		return SW_FT_THROW(Invalid_Outline);
	if(outline->n_points == 0 || outline->n_contours <= 0)
		return 0;
	if(!outline->contours || !outline->points)
		return SW_FT_THROW(Invalid_Outline);
	if(outline->n_points != outline->contours[outline->n_contours - 1] + 1)
		return SW_FT_THROW(Invalid_Outline);
	if(!(params->flags & SW_FT_RASTER_FLAG_AA))
		return SW_FT_THROW(Invalid_Mode);
	if(params->flags & SW_FT_RASTER_FLAG_CLIP)
		ras.clip_box = params->clip_box;
	else
	{
		ras.clip_box.xMin = -32768L;
		ras.clip_box.yMin = -32768L;
		ras.clip_box.xMax = 32767L;
		ras.clip_box.yMax = 32767L;
	}
	gray_init_cells(RAS_VAR_ buffer, buffer_size);
	ras.outline = *outline;
	ras.num_cells = 0;
	ras.invalid = 1;
	ras.band_size = band_size;
	ras.num_gray_spans = 0;
	ras.render_span = (SW_FT_Raster_Span_Func)params->gray_spans;
	ras.render_span_data = params->user;
	gray_convert_glyph(RAS_VAR);
	params->bbox_cb(ras.bound_left, ras.bound_top,
	ras.bound_right - ras.bound_left,
	ras.bound_bottom - ras.bound_top + 1, params->user);
	return 1;
}

static int gray_raster_new(SW_FT_Raster *araster)
{
	static gray_TRaster the_raster;
	*araster = (SW_FT_Raster)&the_raster;
	SW_FT_MEM_ZERO(&the_raster, sizeof(the_raster));
	return 0;
}

static void gray_raster_done(SW_FT_Raster raster)
{
	SW_FT_UNUSED(raster);
}

static void gray_raster_reset(SW_FT_Raster raster, char *pool_base, long pool_size)
{
	SW_FT_UNUSED(raster);
	SW_FT_UNUSED(pool_base);
	SW_FT_UNUSED(pool_size);
}

SW_FT_DEFINE_RASTER_FUNCS(sw_ft_grays_raster,
	(SW_FT_Raster_New_Func)gray_raster_new,
	(SW_FT_Raster_Reset_Func)gray_raster_reset,
	(SW_FT_Raster_Render_Func)gray_raster_render,
	(SW_FT_Raster_Done_Func)gray_raster_done)
