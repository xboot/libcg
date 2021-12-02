#ifndef __SW_FT_RASTER_H__
#define __SW_FT_RASTER_H__

#include <sw_ft_types.h>

typedef struct SW_FT_BBox_ {
	SW_FT_Pos xMin, yMin;
	SW_FT_Pos xMax, yMax;
} SW_FT_BBox;

typedef struct SW_FT_Outline_ {
	short n_contours;
	short n_points;
	SW_FT_Vector * points;
	char * tags;
	short * contours;
	char * contours_flag;
	int flags;
} SW_FT_Outline;

#define SW_FT_OUTLINE_NONE			0x0
#define SW_FT_OUTLINE_OWNER			0x1
#define SW_FT_OUTLINE_EVEN_ODD_FILL	0x2
#define SW_FT_OUTLINE_REVERSE_FILL	0x4
#define SW_FT_CURVE_TAG(flag)		(flag & 3)
#define SW_FT_CURVE_TAG_ON			1
#define SW_FT_CURVE_TAG_CONIC		0
#define SW_FT_CURVE_TAG_CUBIC		2
#define SW_FT_Curve_Tag_On			SW_FT_CURVE_TAG_ON
#define SW_FT_Curve_Tag_Conic		SW_FT_CURVE_TAG_CONIC
#define SW_FT_Curve_Tag_Cubic		SW_FT_CURVE_TAG_CUBIC

typedef struct SW_FT_RasterRec_ * SW_FT_Raster;

typedef struct SW_FT_Span_ {
	short x;
	short y;
	unsigned short len;
	unsigned char coverage;
} SW_FT_Span;

typedef void (*SW_FT_SpanFunc)(int count, const SW_FT_Span * spans, void * user);
typedef void (*SW_FT_BboxFunc)(int x, int y, int w, int h, void * user);

#define SW_FT_Raster_Span_Func		SW_FT_SpanFunc
#define SW_FT_RASTER_FLAG_DEFAULT	0x0
#define SW_FT_RASTER_FLAG_AA		0x1
#define SW_FT_RASTER_FLAG_DIRECT	0x2
#define SW_FT_RASTER_FLAG_CLIP		0x4

typedef struct SW_FT_Raster_Params_ {
	const void * source;
	int flags;
	SW_FT_SpanFunc gray_spans;
	SW_FT_BboxFunc bbox_cb;
	void *user;
	SW_FT_BBox clip_box;
} SW_FT_Raster_Params;

SW_FT_Error SW_FT_Outline_Check(SW_FT_Outline *outline);
void SW_FT_Outline_Get_CBox(const SW_FT_Outline *outline, SW_FT_BBox *acbox);
typedef int (*SW_FT_Raster_NewFunc)(SW_FT_Raster *raster);
#define SW_FT_Raster_New_Func  SW_FT_Raster_NewFunc
typedef void (*SW_FT_Raster_DoneFunc)(SW_FT_Raster raster);
#define SW_FT_Raster_Done_Func  SW_FT_Raster_DoneFunc
typedef void (*SW_FT_Raster_ResetFunc)(SW_FT_Raster raster, unsigned char * pool_base, unsigned long pool_size);
#define SW_FT_Raster_Reset_Func  SW_FT_Raster_ResetFunc
typedef int (*SW_FT_Raster_RenderFunc)(SW_FT_Raster raster, const SW_FT_Raster_Params * params);
#define SW_FT_Raster_Render_Func  SW_FT_Raster_RenderFunc
typedef struct SW_FT_Raster_Funcs_ {
	SW_FT_Raster_NewFunc raster_new;
	SW_FT_Raster_ResetFunc raster_reset;
	SW_FT_Raster_RenderFunc raster_render;
	SW_FT_Raster_DoneFunc raster_done;
} SW_FT_Raster_Funcs;
extern const SW_FT_Raster_Funcs sw_ft_grays_raster;

#endif /* __SW_FT_RASTER_H__ */
