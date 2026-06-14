/*
 * Copyright(c) Jianjun Jiang <8192542@qq.com>
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <cg.h>

#ifndef CG_MIN
#define CG_MIN(a, b)		({typeof(a) _amin = (a); typeof(b) _bmin = (b); (void)(&_amin == &_bmin); _amin < _bmin ? _amin : _bmin;})
#endif
#ifndef CG_MAX
#define CG_MAX(a, b)		({typeof(a) _amax = (a); typeof(b) _bmax = (b); (void)(&_amax == &_bmax); _amax > _bmax ? _amax : _bmax;})
#endif
#ifndef CG_CLAMP
#define CG_CLAMP(v, a, b)	CG_MIN(CG_MAX(a, v), b)
#endif
#ifndef CG_ALPHA
#define CG_ALPHA(c)			((c) >> 24)
#endif
#ifndef CG_DIV255
#define CG_DIV255(x)		(((x) + ((x) >> 8) + 0x80) >> 8)
#endif
#ifndef CG_ROUND
#define CG_ROUND(x)			((x) > 0 ? (int)((x) + 0.5f) : (int)((x) - 0.5f))
#endif
#ifndef CG_BYTE_MUL
#define CG_BYTE_MUL(x, a)	((((((x) >> 8) & 0x00ff00ff) * (a)) & 0xff00ff00) + (((((x) & 0x00ff00ff) * (a)) >> 8) & 0x00ff00ff))
#endif
#ifndef CG_TWO_PI
#define CG_TWO_PI			(6.28318530717958647693f)
#endif
#ifndef CG_HALF_PI
#define CG_HALF_PI			(1.57079632679489661923f)
#endif
#ifndef CG_SQRT2
#define CG_SQRT2			(1.41421356237309504880f)
#endif
#ifndef CG_KAPPA
#define CG_KAPPA			(0.55228474983079339840f)
#endif

#define cg_array_init(array) \
	do { \
		(array).data = NULL; \
		(array).size = 0; \
		(array).capacity = 0; \
	} while(0)

#define cg_array_ensure(array, count) \
	do { \
		if((array).size + (count) > (array).capacity) { \
			int capacity = (array).size + (count); \
			int newcapacity = (array).capacity == 0 ? 8 : (array).capacity; \
			while(newcapacity < capacity) { newcapacity <<= 1; } \
			(array).data = realloc((array).data, (size_t)newcapacity * sizeof((array).data[0])); \
			(array).capacity = newcapacity; \
		} \
	} while(0)

#define cg_array_append_data(array, newdata, count) \
	do { \
		if(newdata && count > 0) { \
			cg_array_ensure(array, count); \
			memcpy((array).data + (array).size, newdata, (count) * sizeof((newdata)[0])); \
			(array).size += count; \
		} \
	} while(0)

#define cg_array_append(array, other)	cg_array_append_data(array, (other).data, (other).size)
#define cg_array_clear(array)			((array).size = 0)
#define cg_array_destroy(array)			free((array).data)

void cg_matrix_init(struct cg_matrix_t * m, float a, float b, float c, float d, float tx, float ty)
{
	m->a = a;   m->b = b;
	m->c = c;   m->d = d;
	m->tx = tx; m->ty = ty;
}

void cg_matrix_init_identity(struct cg_matrix_t * m)
{
	m->a = 1;  m->b = 0;
	m->c = 0;  m->d = 1;
	m->tx = 0; m->ty = 0;
}

void cg_matrix_init_translate(struct cg_matrix_t * m, float tx, float ty)
{
	m->a = 1;   m->b = 0;
	m->c = 0;   m->d = 1;
	m->tx = tx; m->ty = ty;
}

void cg_matrix_init_scale(struct cg_matrix_t * m, float sx, float sy)
{
	m->a = sx; m->b = 0;
	m->c = 0;  m->d = sy;
	m->tx = 0; m->ty = 0;
}

void cg_matrix_init_rotate(struct cg_matrix_t * m, float r)
{
	float s = sinf(r);
	float c = cosf(r);

	m->a = c;   m->b = s;
	m->c = -s;  m->d = c;
	m->tx = 0;  m->ty = 0;
}

void cg_matrix_init_shear(struct cg_matrix_t * m, float shx, float shy)
{
	m->a = 1;   m->b = shy;
	m->c = shx; m->d = 1;
	m->tx = 0;  m->ty = 0;
}

void cg_matrix_translate(struct cg_matrix_t * m, float tx, float ty)
{
	m->tx += m->a * tx + m->c * ty;
	m->ty += m->b * tx + m->d * ty;
}

void cg_matrix_scale(struct cg_matrix_t * m, float sx, float sy)
{
	m->a *= sx;
	m->b *= sx;
	m->c *= sy;
	m->d *= sy;
}

void cg_matrix_rotate(struct cg_matrix_t * m, float r)
{
	float s = sinf(r);
	float c = cosf(r);
	float ca = c * m->a;
	float cb = c * m->b;
	float cc = c * m->c;
	float cd = c * m->d;
	float sa = s * m->a;
	float sb = s * m->b;
	float sc = s * m->c;
	float sd = s * m->d;

	m->a = ca + sc;
	m->b = cb + sd;
	m->c = cc - sa;
	m->d = cd - sb;
}

void cg_matrix_shear(struct cg_matrix_t * m, float shx, float shy)
{
	float ma = m->a;
	float mb = m->b;
	float mc = m->c;
	float md = m->d;

	m->a = ma + mc * shy;
	m->b = mb + md * shy;
	m->c = ma * shx + mc;
	m->d = mb * shx + md;
}

void cg_matrix_multiply(struct cg_matrix_t * m, struct cg_matrix_t * m1, struct cg_matrix_t * m2)
{
	struct cg_matrix_t t;

	t.a = m1->a * m2->a;
	t.b = 0.0;
	t.c = 0.0;
	t.d = m1->d * m2->d;
	t.tx = m1->tx * m2->a + m2->tx;
	t.ty = m1->ty * m2->d + m2->ty;
	if(m1->b != 0.0 || m1->c != 0.0 || m2->b != 0.0 || m2->c != 0.0)
	{
		t.a += m1->b * m2->c;
		t.b += m1->a * m2->b + m1->b * m2->d;
		t.c += m1->c * m2->a + m1->d * m2->c;
		t.d += m1->c * m2->b;
		t.tx += m1->ty * m2->c;
		t.ty += m1->tx * m2->b;
	}
	memcpy(m, &t, sizeof(struct cg_matrix_t));
}

int cg_matrix_invert(struct cg_matrix_t * m)
{
	if((m->c == 0.0f) && (m->b == 0.0f))
	{
		if((m->a == 0.0f) || (m->d == 0.0f))
			return 0;
		m->a = 1.0f / m->a;
		m->d = 1.0f / m->d;
		m->tx = -m->tx * m->a;
		m->ty = -m->ty * m->d;
		return 1;
	}
	else
	{
		float det = m->a * m->d - m->b * m->c;
		if(fabsf(det) > 1e-20f)
		{
			float inv = 1.0f / det;
			float a = m->a * inv;
			float b = m->b * inv;
			float c = m->c * inv;
			float d = m->d * inv;
			float tx = (m->c * m->ty - m->d * m->tx) * inv;
			float ty = (m->b * m->tx - m->a * m->ty) * inv;
			m->a = d; m->b = -b;
			m->c = -c; m->d = a;
			m->tx = tx; m->ty = ty;
			return 1;
		}
	}
	return 0;
}

static void cg_matrix_map_point(struct cg_matrix_t * m, struct cg_point_t * p1, struct cg_point_t * p2)
{
	p2->x = p1->x * m->a + p1->y * m->c + m->tx;
	p2->y = p1->x * m->b + p1->y * m->d + m->ty;
}

static void cg_matrix_map_points(struct cg_matrix_t * m, struct cg_point_t * src, struct cg_point_t * dst, int count)
{
	for(int i = 0; i < count; ++i)
		cg_matrix_map_point(m, &src[i], &dst[i]);
}

struct cg_surface_t * cg_surface_create(int width, int height)
{
	struct cg_surface_t * surface = malloc(sizeof(struct cg_surface_t));
	surface->refcnt = 1;
	surface->width = width;
	surface->height = height;
	surface->stride = width << 2;
	surface->owndata = 1;
	surface->pixels = calloc(1, (size_t)(height * surface->stride));
	return surface;
}

struct cg_surface_t * cg_surface_create_for_data(int width, int height, void * pixels)
{
	struct cg_surface_t * surface = malloc(sizeof(struct cg_surface_t));
	surface->refcnt = 1;
	surface->width = width;
	surface->height = height;
	surface->stride = width << 2;
	surface->owndata = 0;
	surface->pixels = pixels;
	return surface;
}

void cg_surface_destroy(struct cg_surface_t * surface)
{
	if(surface && (--surface->refcnt == 0))
	{
		if(surface->owndata)
			free(surface->pixels);
		free(surface);
	}
}

struct cg_surface_t * cg_surface_reference(struct cg_surface_t * surface)
{
	if(surface)
		++surface->refcnt;
	return surface;
}

static void * cg_paint_create(enum cg_paint_type_t type, size_t size)
{
	struct cg_paint_t * paint = malloc(size);
	paint->refcnt = 1;
	paint->type = type;
	return paint;
}

struct cg_paint_t * cg_paint_create_rgb(float r, float g, float b)
{
	return cg_paint_create_rgba(r, g, b, 1.0f);
}

struct cg_paint_t * cg_paint_create_rgba(float r, float g, float b, float a)
{
	struct cg_solid_paint_t * solid = cg_paint_create(CG_PAINT_TYPE_COLOR, sizeof(struct cg_solid_paint_t));
	solid->color.r = CG_CLAMP(r, 0.0f, 1.0f);
	solid->color.g = CG_CLAMP(g, 0.0f, 1.0f);
	solid->color.b = CG_CLAMP(b, 0.0f, 1.0f);
	solid->color.a = CG_CLAMP(a, 0.0f, 1.0f);
	return &solid->base;
}

struct cg_paint_t * cg_paint_create_color(struct cg_color_t * color)
{
	return cg_paint_create_rgba(color->r, color->g, color->b, color->a);
}

static struct cg_gradient_paint_t * cg_gradient_create(enum cg_gradient_type_t type, enum cg_spread_method_t spread, struct cg_gradient_stop_t * stops, int nstops, struct cg_matrix_t * m)
{
	struct cg_gradient_paint_t * gradient = cg_paint_create(CG_PAINT_TYPE_GRADIENT, sizeof(struct cg_gradient_paint_t) + nstops * sizeof(struct cg_gradient_stop_t));
	gradient->type = type;
	gradient->spread = spread;
	gradient->matrix = m ? *m : ((struct cg_matrix_t){1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f});
	gradient->stops = (struct cg_gradient_stop_t *)(gradient + 1);
	gradient->nstops = nstops;

	float offset = 0.0f;
	for(int i = 0; i < nstops; ++i)
	{
		struct cg_gradient_stop_t * stop = stops + i;
		gradient->stops[i].offset = CG_MAX(offset, CG_CLAMP(stop->offset, 0.0f, 1.0f));
		gradient->stops[i].color.r = CG_CLAMP(stop->color.r, 0.0f, 1.0f);
		gradient->stops[i].color.g = CG_CLAMP(stop->color.g, 0.0f, 1.0f);
		gradient->stops[i].color.b = CG_CLAMP(stop->color.b, 0.0f, 1.0f);
		gradient->stops[i].color.a = CG_CLAMP(stop->color.a, 0.0f, 1.0f);
		offset = gradient->stops[i].offset;
	}
	return gradient;
}

struct cg_paint_t * cg_paint_create_linear_gradient(float x1, float y1, float x2, float y2, enum cg_spread_method_t spread, struct cg_gradient_stop_t * stops, int nstops, struct cg_matrix_t * m)
{
	struct cg_gradient_paint_t * gradient = cg_gradient_create(CG_GRADIENT_TYPE_LINEAR, spread, stops, nstops, m);
	gradient->values[0] = x1;
	gradient->values[1] = y1;
	gradient->values[2] = x2;
	gradient->values[3] = y2;
	return &gradient->base;
}

struct cg_paint_t * cg_paint_create_radial_gradient(float cx0, float cy0, float r0, float cx1, float cy1, float r1, enum cg_spread_method_t spread, struct cg_gradient_stop_t * stops, int nstops, struct cg_matrix_t * m)
{
	struct cg_gradient_paint_t * gradient = cg_gradient_create(CG_GRADIENT_TYPE_RADIAL, spread, stops, nstops, m);
	gradient->values[0] = cx0;
	gradient->values[1] = cy0;
	gradient->values[2] = r0;
	gradient->values[3] = cx1;
	gradient->values[4] = cy1;
	gradient->values[5] = r1;
	return &gradient->base;
}

struct cg_paint_t * cg_paint_create_texture(struct cg_surface_t * surface, enum cg_texture_type_t type, float opacity, struct cg_matrix_t * m)
{
	struct cg_texture_paint_t * texture = cg_paint_create(CG_PAINT_TYPE_TEXTURE, sizeof(struct cg_texture_paint_t));
	texture->type = type;
	texture->opacity = CG_CLAMP(opacity, 0.0f, 1.0f);
	texture->matrix = m ? *m : ((struct cg_matrix_t){1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f});
	texture->surface = cg_surface_reference(surface);
	return &texture->base;
}

void cg_paint_destroy(struct cg_paint_t * paint)
{
	if(paint && (--paint->refcnt == 0))
	{
		if(paint->type == CG_PAINT_TYPE_TEXTURE)
		{
			struct cg_texture_paint_t * texture = (struct cg_texture_paint_t *)(paint);
			cg_surface_destroy(texture->surface);
		}
		free(paint);
	}
}

struct cg_paint_t * cg_paint_reference(struct cg_paint_t * paint)
{
	if(paint)
		++paint->refcnt;
	return paint;
}

struct cg_path_iterator_t {
	union cg_path_element_t * elements;
	int size;
	int index;
};

static void cg_path_iterator_init(struct cg_path_iterator_t * it, struct cg_path_t * path)
{
	it->elements = path->elements.data;
	it->size = path->elements.size;
	it->index = 0;
}

static int cg_path_iterator_has_next(struct cg_path_iterator_t * it)
{
	return (it->index < it->size);
}

static enum cg_path_command_t cg_path_iterator_next(struct cg_path_iterator_t * it, struct cg_point_t points[3])
{
	union cg_path_element_t * elements = it->elements + it->index;
	switch(elements[0].header.command)
	{
	case CG_PATH_COMMAND_MOVE_TO:
	case CG_PATH_COMMAND_LINE_TO:
	case CG_PATH_COMMAND_CLOSE:
		points[0] = elements[1].point;
		break;
	case CG_PATH_COMMAND_CUBIC_TO:
		points[0] = elements[1].point;
		points[1] = elements[2].point;
		points[2] = elements[3].point;
		break;
	default:
		break;
	}
	it->index += elements[0].header.length;
	return elements[0].header.command;
}

struct cg_path_t * cg_path_create(void)
{
	struct cg_path_t * path = malloc(sizeof(struct cg_path_t));
	path->refcnt = 1;
	path->num_points = 0;
	path->num_contours = 0;
	path->num_curves = 0;
	path->sub_path = 0;
	path->start_point = (struct cg_point_t){0.0f, 0.0f};
	cg_array_init(path->elements);
	return path;
}

void cg_path_destroy(struct cg_path_t * path)
{
	if(path && (--path->refcnt == 0))
	{
		cg_array_destroy(path->elements);
		free(path);
	}
}

struct cg_path_t * cg_path_reference(struct cg_path_t * path)
{
	if(path)
		++path->refcnt;
	return path;
}

static union cg_path_element_t * cg_path_add_command(struct cg_path_t * path, enum cg_path_command_t command, int npoints)
{
	int length = npoints + 1;
	cg_array_ensure(path->elements, length);
	union cg_path_element_t * elements = path->elements.data + path->elements.size;
	elements->header.command = command;
	elements->header.length = length;
	path->elements.size += length;
	path->num_points += npoints;
	return elements + 1;
}

void cg_path_reset(struct cg_path_t * path)
{
	cg_array_clear(path->elements);
	path->start_point = (struct cg_point_t){0.0f, 0.0f};
	path->num_points = 0;
	path->num_contours = 0;
	path->num_curves = 0;
	path->sub_path = 0;
}

void cg_path_sub_path(struct cg_path_t * path)
{
	path->sub_path = 1;
}

void cg_path_close(struct cg_path_t * path)
{
	if(path->elements.size == 0)
		return;
	union cg_path_element_t * elements = cg_path_add_command(path, CG_PATH_COMMAND_CLOSE, 1);
	elements[0].point = path->start_point;
	path->sub_path = 0;
}

void cg_path_transform(struct cg_path_t * path, struct cg_matrix_t * m)
{
	union cg_path_element_t * elements = path->elements.data;
	for(int i = 0; i < path->elements.size; i += elements[i].header.length)
	{
		switch(elements[i].header.command)
		{
		case CG_PATH_COMMAND_MOVE_TO:
		case CG_PATH_COMMAND_LINE_TO:
		case CG_PATH_COMMAND_CLOSE:
			cg_matrix_map_point(m, &elements[i + 1].point, &elements[i + 1].point);
			break;
		case CG_PATH_COMMAND_CUBIC_TO:
			cg_matrix_map_point(m, &elements[i + 1].point, &elements[i + 1].point);
			cg_matrix_map_point(m, &elements[i + 2].point, &elements[i + 2].point);
			cg_matrix_map_point(m, &elements[i + 3].point, &elements[i + 3].point);
			break;
		default:
			break;
		}
	}
}

void cg_path_get_current_point(struct cg_path_t * path, float * x, float * y)
{
	float xx = 0.0f;
	float yy = 0.0f;
	if(path->num_points > 0 && !path->sub_path)
	{
		xx = path->elements.data[path->elements.size - 1].point.x;
		yy = path->elements.data[path->elements.size - 1].point.y;
	}
	if(x)
		*x = xx;
	if(y)
		*y = yy;
}

void cg_path_move_to(struct cg_path_t * path, float x, float y)
{
	union cg_path_element_t * elements = cg_path_add_command(path, CG_PATH_COMMAND_MOVE_TO, 1);
	elements[0].point = (struct cg_point_t){x, y};
	path->start_point = (struct cg_point_t){x, y};
	path->num_contours += 1;
	path->sub_path = 0;
}

void cg_path_line_to(struct cg_path_t * path, float x, float y)
{
	if(path->elements.size == 0 || path->sub_path)
		cg_path_move_to(path, 0, 0);
	union cg_path_element_t * elements = cg_path_add_command(path, CG_PATH_COMMAND_LINE_TO, 1);
	elements[0].point = (struct cg_point_t){x, y};
	path->sub_path = 0;
}

void cg_path_quad_to(struct cg_path_t * path, float x1, float y1, float x2, float y2)
{
	if(path->elements.size == 0 || path->sub_path)
		cg_path_move_to(path, 0, 0);
	float current_x, current_y;
	cg_path_get_current_point(path, &current_x, &current_y);
	float cp1x = 2.0f / 3.0f * x1 + 1.0f / 3.0f * current_x;
	float cp1y = 2.0f / 3.0f * y1 + 1.0f / 3.0f * current_y;
	float cp2x = 2.0f / 3.0f * x1 + 1.0f / 3.0f * x2;
	float cp2y = 2.0f / 3.0f * y1 + 1.0f / 3.0f * y2;
	cg_path_cubic_to(path, cp1x, cp1y, cp2x, cp2y, x2, y2);
}

void cg_path_cubic_to(struct cg_path_t * path, float x1, float y1, float x2, float y2, float x3, float y3)
{
	if(path->elements.size == 0 || path->sub_path)
		cg_path_move_to(path, 0, 0);
	union cg_path_element_t * elements = cg_path_add_command(path, CG_PATH_COMMAND_CUBIC_TO, 3);
	elements[0].point = (struct cg_point_t){x1, y1};
	elements[1].point = (struct cg_point_t){x2, y2};
	elements[2].point = (struct cg_point_t){x3, y3};
	path->num_curves += 1;
	path->sub_path = 0;
}

static inline void cg_matrix_map(struct cg_matrix_t * m, float x, float y, float * xx, float * yy)
{
	*xx = x * m->a + y * m->c + m->tx;
	*yy = x * m->b + y * m->d + m->ty;
}

void cg_path_arc_to(struct cg_path_t * path, float rx, float ry, float angle, int large, int sweep, float x, float y)
{
	float current_x, current_y;
	cg_path_get_current_point(path, &current_x, &current_y);
	if(rx == 0.0f || ry == 0.0f || (current_x == x && current_y == y))
	{
		cg_path_line_to(path, x, y);
		return;
	}

	if(rx < 0.0f)
		rx = -rx;
	if(ry < 0.0f)
		ry = -ry;

	float dx = (current_x - x) * 0.5f;
	float dy = (current_y - y) * 0.5f;

	struct cg_matrix_t matrix;
	cg_matrix_init_rotate(&matrix, -angle);
	cg_matrix_map(&matrix, dx, dy, &dx, &dy);

	float rxrx = rx * rx;
	float ryry = ry * ry;
	float dxdx = dx * dx;
	float dydy = dy * dy;
	float radius = dxdx / rxrx + dydy / ryry;
	if(radius > 1.0f)
	{
		rx *= sqrtf(radius);
		ry *= sqrtf(radius);
	}

	cg_matrix_init_scale(&matrix, 1.0f / rx, 1.0f / ry);
	cg_matrix_rotate(&matrix, -angle);

	float x1, y1;
	float x2, y2;
	cg_matrix_map(&matrix, current_x, current_y, &x1, &y1);
	cg_matrix_map(&matrix, x, y, &x2, &y2);

	float dx1 = x2 - x1;
	float dy1 = y2 - y1;
	float d = dx1 * dx1 + dy1 * dy1;
	float scale_sq = 1.0f / d - 0.25f;
	if(scale_sq < 0.0f)
		scale_sq = 0.0f;
	float scale = sqrtf(scale_sq);
	if(sweep == large)
		scale = -scale;
	dx1 *= scale;
	dy1 *= scale;

	float cx1 = 0.5f * (x1 + x2) - dy1;
	float cy1 = 0.5f * (y1 + y2) + dx1;

	float th1 = atan2f(y1 - cy1, x1 - cx1);
	float th2 = atan2f(y2 - cy1, x2 - cx1);
	float th_arc = th2 - th1;
	if(th_arc < 0.0f && sweep)
		th_arc += CG_TWO_PI;
	else if(th_arc > 0.0f && !sweep)
		th_arc -= CG_TWO_PI;
	cg_matrix_init_rotate(&matrix, angle);
	cg_matrix_scale(&matrix, rx, ry);
	int segments = (int)(ceilf(fabsf(th_arc / (CG_HALF_PI + 0.001f))));
	for(int i = 0; i < segments; i++)
	{
		float th_start = th1 + i * th_arc / segments;
		float th_end = th1 + (i + 1) * th_arc / segments;
		float t = (8.0f / 6.0f) * tanf(0.25f * (th_end - th_start));

		float x3 = cosf(th_end) + cx1;
		float y3 = sinf(th_end) + cy1;

		float cp2x = x3 + t * sinf(th_end);
		float cp2y = y3 - t * cosf(th_end);

		float cp1x = cosf(th_start) - t * sinf(th_start);
		float cp1y = sinf(th_start) + t * cosf(th_start);

		cp1x += cx1;
		cp1y += cy1;

		cg_matrix_map(&matrix, cp1x, cp1y, &cp1x, &cp1y);
		cg_matrix_map(&matrix, cp2x, cp2y, &cp2x, &cp2y);
		cg_matrix_map(&matrix, x3, y3, &x3, &y3);

		cg_path_cubic_to(path, cp1x, cp1y, cp2x, cp2y, x3, y3);
	}
}

static void cg_path_reserve(struct cg_path_t * path, int count)
{
	cg_array_ensure(path->elements, count);
}

void cg_path_add_rectangle(struct cg_path_t * path, float x, float y, float w, float h)
{
	cg_path_reserve(path, 6 * 2);
	cg_path_move_to(path, x, y);
	cg_path_line_to(path, x + w, y);
	cg_path_line_to(path, x + w, y + h);
	cg_path_line_to(path, x, y + h);
	cg_path_line_to(path, x, y);
	cg_path_close(path);
}

void cg_path_add_round_rectangle(struct cg_path_t * path, float x, float y, float w, float h, float rx, float ry)
{
	rx = CG_MIN(rx, w * 0.5f);
	ry = CG_MIN(ry, h * 0.5f);
	if(rx == 0.0f && ry == 0.0f)
	{
		cg_path_add_rectangle(path, x, y, w, h);
		return;
	}

	float right = x + w;
	float bottom = y + h;
	float cpx = rx * CG_KAPPA;
	float cpy = ry * CG_KAPPA;

	cg_path_reserve(path, 6 * 2 + 4 * 4);
	cg_path_move_to(path, x, y + ry);
	cg_path_cubic_to(path, x, y + ry - cpy, x + rx - cpx, y, x + rx, y);
	cg_path_line_to(path, right - rx, y);
	cg_path_cubic_to(path, right - rx + cpx, y, right, y + ry - cpy, right, y + ry);
	cg_path_line_to(path, right, bottom - ry);
	cg_path_cubic_to(path, right, bottom - ry + cpy, right - rx + cpx, bottom, right - rx, bottom);
	cg_path_line_to(path, x + rx, bottom);
	cg_path_cubic_to(path, x + rx - cpx, bottom, x, bottom - ry + cpy, x, bottom - ry);
	cg_path_line_to(path, x, y + ry);
	cg_path_close(path);
}

void cg_path_add_ellipse(struct cg_path_t * path, float cx, float cy, float rx, float ry)
{
	float left = cx - rx;
	float top = cy - ry;
	float right = cx + rx;
	float bottom = cy + ry;
	float cpx = rx * CG_KAPPA;
	float cpy = ry * CG_KAPPA;

	cg_path_reserve(path, 2 * 2 + 4 * 4);
	cg_path_move_to(path, cx, top);
	cg_path_cubic_to(path, cx + cpx, top, right, cy - cpy, right, cy);
	cg_path_cubic_to(path, right, cy + cpy, cx + cpx, bottom, cx, bottom);
	cg_path_cubic_to(path, cx - cpx, bottom, left, cy + cpy, left, cy);
	cg_path_cubic_to(path, left, cy - cpy, cx - cpx, top, cx, top);
	cg_path_close(path);
}

void cg_path_add_circle(struct cg_path_t * path, float cx, float cy, float r)
{
	cg_path_add_ellipse(path, cx, cy, r, r);
}

void cg_path_add_arc(struct cg_path_t * path, float cx, float cy, float r, float a0, float a1, int ccw)
{
	float da = a1 - a0;
	if(fabsf(da) > CG_TWO_PI)
		da = CG_TWO_PI;
	else if(da != 0.0f && ccw != (da < 0.0f))
		da += CG_TWO_PI * (ccw ? -1 : 1);
	int seg_n = (int)(ceilf(fabsf(da) / CG_HALF_PI));
	float a = a0;
	float ax = cx + cosf(a) * r;
	float ay = cy + sinf(a) * r;
	cg_path_reserve(path, 2 + 4 * (seg_n > 0 ? seg_n : 1));
	if(path->elements.size == 0 || path->sub_path)
		cg_path_move_to(path, ax, ay);
	else
		cg_path_line_to(path, ax, ay);
	if(seg_n == 0)
		return;
	float seg_a = da / seg_n;
	float d = (seg_a / CG_HALF_PI) * CG_KAPPA * r;
	float dx = -sinf(a) * d;
	float dy = cosf(a) * d;
	for(int i = 0; i < seg_n; i++)
	{
		float cp1x = ax + dx;
		float cp1y = ay + dy;
		a += seg_a;
		ax = cx + cosf(a) * r;
		ay = cy + sinf(a) * r;
		dx = -sinf(a) * d;
		dy = cosf(a) * d;
		float cp2x = ax - dx;
		float cp2y = ay - dy;
		cg_path_cubic_to(path, cp1x, cp1y, cp2x, cp2y, ax, ay);
	}
}

void cg_path_add_path(struct cg_path_t * path, struct cg_path_t * source, struct cg_matrix_t * m)
{
	if(m == NULL)
	{
		cg_array_append(path->elements, source->elements);
		path->start_point = source->start_point;
		path->num_points += source->num_points;
		path->num_contours += source->num_contours;
		path->num_curves += source->num_curves;
		path->sub_path = source->sub_path;
		return;
	}
	struct cg_path_iterator_t it;
	cg_path_iterator_init(&it, source);

	struct cg_point_t points[3];
	cg_array_ensure(path->elements, source->elements.size);
	while(cg_path_iterator_has_next(&it))
	{
		switch(cg_path_iterator_next(&it, points))
		{
		case CG_PATH_COMMAND_MOVE_TO:
			cg_matrix_map_points(m, points, points, 1);
			cg_path_move_to(path, points[0].x, points[0].y);
			break;
		case CG_PATH_COMMAND_LINE_TO:
			cg_matrix_map_points(m, points, points, 1);
			cg_path_line_to(path, points[0].x, points[0].y);
			break;
		case CG_PATH_COMMAND_CUBIC_TO:
			cg_matrix_map_points(m, points, points, 3);
			cg_path_cubic_to(path, points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y);
			break;
		case CG_PATH_COMMAND_CLOSE:
			cg_path_close(path);
			break;
		default:
			break;
		}
	}
}

typedef void (*cg_path_traverse_func_t)(void * closure, enum cg_path_command_t command, struct cg_point_t * points, int npoints);

static void cg_path_traverse(struct cg_path_t * path, cg_path_traverse_func_t traverse_func, void * closure)
{
	struct cg_path_iterator_t it;
	cg_path_iterator_init(&it, path);

	struct cg_point_t points[3];
	while(cg_path_iterator_has_next(&it))
	{
		switch(cg_path_iterator_next(&it, points))
		{
		case CG_PATH_COMMAND_MOVE_TO:
			traverse_func(closure, CG_PATH_COMMAND_MOVE_TO, points, 1);
			break;
		case CG_PATH_COMMAND_LINE_TO:
			traverse_func(closure, CG_PATH_COMMAND_LINE_TO, points, 1);
			break;
		case CG_PATH_COMMAND_CUBIC_TO:
			traverse_func(closure, CG_PATH_COMMAND_CUBIC_TO, points, 3);
			break;
		case CG_PATH_COMMAND_CLOSE:
			traverse_func(closure, CG_PATH_COMMAND_CLOSE, points, 1);
			break;
		default:
			break;
		}
	}
}

struct cg_bezier_t {
	float x1; float y1;
	float x2; float y2;
	float x3; float y3;
	float x4; float y4;
};

static inline void split_bezier(struct cg_bezier_t * b, struct cg_bezier_t * first, struct cg_bezier_t * second)
{
	float c = (b->x2 + b->x3) * 0.5f;
	first->x2 = (b->x1 + b->x2) * 0.5f;
	second->x3 = (b->x3 + b->x4) * 0.5f;
	first->x1 = b->x1;
	second->x4 = b->x4;
	first->x3 = (first->x2 + c) * 0.5f;
	second->x2 = (second->x3 + c) * 0.5f;
	first->x4 = second->x1 = (first->x3 + second->x2) * 0.5f;

	c = (b->y2 + b->y3) * 0.5f;
	first->y2 = (b->y1 + b->y2) * 0.5f;
	second->y3 = (b->y3 + b->y4) * 0.5f;
	first->y1 = b->y1;
	second->y4 = b->y4;
	first->y3 = (first->y2 + c) * 0.5f;
	second->y2 = (second->y3 + c) * 0.5f;
	first->y4 = second->y1 = (first->y3 + second->y2) * 0.5f;
}

static void cg_path_traverse_flatten(struct cg_path_t * path, cg_path_traverse_func_t traverse_func, void * closure)
{
	if(path->num_curves == 0)
	{
		cg_path_traverse(path, traverse_func, closure);
		return;
	}
	float threshold = 0.25f;

	struct cg_path_iterator_t it;
	cg_path_iterator_init(&it, path);

	struct cg_bezier_t beziers[32];
	struct cg_point_t points[3];
	struct cg_point_t current_point = { 0, 0 };
	while(cg_path_iterator_has_next(&it))
	{
		enum cg_path_command_t command = cg_path_iterator_next(&it, points);
		switch(command)
		{
		case CG_PATH_COMMAND_MOVE_TO:
		case CG_PATH_COMMAND_LINE_TO:
		case CG_PATH_COMMAND_CLOSE:
			traverse_func(closure, command, points, 1);
			current_point = points[0];
			break;
		case CG_PATH_COMMAND_CUBIC_TO:
			beziers[0].x1 = current_point.x;
			beziers[0].y1 = current_point.y;
			beziers[0].x2 = points[0].x;
			beziers[0].y2 = points[0].y;
			beziers[0].x3 = points[1].x;
			beziers[0].y3 = points[1].y;
			beziers[0].x4 = points[2].x;
			beziers[0].y4 = points[2].y;
			struct cg_bezier_t * b = beziers;
			while(b >= beziers)
			{
				float y4y1 = b->y4 - b->y1;
				float x4x1 = b->x4 - b->x1;
				float l = fabsf(x4x1) + fabsf(y4y1);
				float d;
				if(l > 1.0f)
				{
					d = fabsf((x4x1) * (b->y1 - b->y2) - (y4y1) * (b->x1 - b->x2)) + fabsf((x4x1) * (b->y1 - b->y3) - (y4y1) * (b->x1 - b->x3));
				}
				else
				{
					d = fabsf(b->x1 - b->x2) + fabsf(b->y1 - b->y2) + fabsf(b->x1 - b->x3) + fabsf(b->y1 - b->y3);
					l = 1.0f;
				}

				if(d < threshold * l || b == beziers + 31)
				{
					struct cg_point_t p = { b->x4, b->y4 };
					traverse_func(closure, CG_PATH_COMMAND_LINE_TO, &p, 1);
					--b;
				}
				else
				{
					split_bezier(b, b + 1, b);
					++b;
				}
			}
			current_point = points[2];
			break;
		default:
			break;
		}
	}
}

struct cg_dasher_t {
	float * dashes;
	int ndashes;
	float start_phase;
	float phase;
	int start_index;
	int index;
	int start_toggle;
	int toggle;
	struct cg_point_t current_point;
	cg_path_traverse_func_t traverse_func;
	void * closure;
};

static void dash_traverse_func(void * closure, enum cg_path_command_t command, struct cg_point_t * points, int npoints)
{
	struct cg_dasher_t * dasher = (struct cg_dasher_t *)(closure);
	if(command == CG_PATH_COMMAND_MOVE_TO)
	{
		if(dasher->start_toggle)
			dasher->traverse_func(dasher->closure, CG_PATH_COMMAND_MOVE_TO, points, npoints);
		dasher->current_point = points[0];
		dasher->phase = dasher->start_phase;
		dasher->index = dasher->start_index;
		dasher->toggle = dasher->start_toggle;
		return;
	}

	struct cg_point_t p0 = dasher->current_point;
	struct cg_point_t p1 = points[0];
	float dx = p1.x - p0.x;
	float dy = p1.y - p0.y;
	float dist0 = sqrtf(dx * dx + dy * dy);
	float dist1 = 0.0f;
	while(dist0 - dist1 > dasher->dashes[dasher->index % dasher->ndashes] - dasher->phase)
	{
		dist1 += dasher->dashes[dasher->index % dasher->ndashes] - dasher->phase;
		float a = dist1 / dist0;
		struct cg_point_t p = { p0.x + a * dx, p0.y + a * dy };
		if(dasher->toggle)
			dasher->traverse_func(dasher->closure, CG_PATH_COMMAND_LINE_TO, &p, 1);
		else
			dasher->traverse_func(dasher->closure, CG_PATH_COMMAND_MOVE_TO, &p, 1);
		dasher->phase = 0.0f;
		dasher->toggle = !dasher->toggle;
		dasher->index++;
	}
	if(dasher->toggle)
		dasher->traverse_func(dasher->closure, CG_PATH_COMMAND_LINE_TO, &p1, 1);
	dasher->phase += dist0 - dist1;
	dasher->current_point = p1;
}

static void cg_path_traverse_dashed(struct cg_path_t * path, float * dashes, int ndashes, float offset, cg_path_traverse_func_t traverse_func, void * closure)
{
	float dash_sum = 0.0f;
	for(int i = 0; i < ndashes; ++i)
		dash_sum += dashes[i];
	if(ndashes % 2 == 1)
		dash_sum *= 2.0f;
	if(dash_sum <= 0.0f)
	{
		cg_path_traverse(path, traverse_func, closure);
		return;
	}

	struct cg_dasher_t dasher;
	dasher.dashes = dashes;
	dasher.ndashes = ndashes;
	dasher.start_phase = fmodf(offset, dash_sum);
	if(dasher.start_phase < 0.0f)
		dasher.start_phase += dash_sum;
	dasher.start_index = 0;
	dasher.start_toggle = 1;
	while(dasher.start_phase > 0.0f && dasher.start_phase >= dasher.dashes[dasher.start_index % dasher.ndashes])
	{
		dasher.start_phase -= dashes[dasher.start_index % dasher.ndashes];
		dasher.start_toggle = !dasher.start_toggle;
		dasher.start_index++;
	}

	dasher.phase = dasher.start_phase;
	dasher.index = dasher.start_index;
	dasher.toggle = dasher.start_toggle;
	dasher.current_point = (struct cg_point_t){0.0f, 0.0f};
	dasher.traverse_func = traverse_func;
	dasher.closure = closure;
	cg_path_traverse_flatten(path, dash_traverse_func, &dasher);
}

static void clone_traverse_func(void * closure, enum cg_path_command_t command, struct cg_point_t * points, int npoints)
{
	struct cg_path_t * path = (struct cg_path_t *)(closure);
	switch(command)
	{
	case CG_PATH_COMMAND_MOVE_TO:
		cg_path_move_to(path, points[0].x, points[0].y);
		break;
	case CG_PATH_COMMAND_LINE_TO:
		cg_path_line_to(path, points[0].x, points[0].y);
		break;
	case CG_PATH_COMMAND_CUBIC_TO:
		cg_path_cubic_to(path, points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y);
		break;
	case CG_PATH_COMMAND_CLOSE:
		cg_path_close(path);
		break;
	default:
		break;
	}
}

static struct cg_path_t * cg_path_clone_dashed(struct cg_path_t * path, float * dashes, int ndashes, float offset)
{
	struct cg_path_t * clone = cg_path_create();
	cg_path_reserve(clone, path->elements.size + path->num_curves * 32);
	cg_path_traverse_dashed(path, dashes, ndashes, offset, clone_traverse_func, clone);
	return clone;
}

struct cg_extents_calculator_t {
	struct cg_point_t current_point;
	int is_first_point;
	float length;
	float x1;
	float y1;
	float x2;
	float y2;
};

static void extents_traverse_func(void * closure, enum cg_path_command_t command, struct cg_point_t * points, int npoints)
{
	struct cg_extents_calculator_t * calculator = (struct cg_extents_calculator_t *)(closure);
	if(calculator->is_first_point)
	{
		calculator->is_first_point = 0;
		calculator->current_point = points[0];
		calculator->x1 = points[0].x;
		calculator->y1 = points[0].y;
		calculator->x2 = points[0].x;
		calculator->y2 = points[0].y;
		calculator->length = 0;
		return;
	}
	for(int i = 0; i < npoints; ++i)
	{
		calculator->x1 = CG_MIN(calculator->x1, points[i].x);
		calculator->y1 = CG_MIN(calculator->y1, points[i].y);
		calculator->x2 = CG_MAX(calculator->x2, points[i].x);
		calculator->y2 = CG_MAX(calculator->y2, points[i].y);
		if(command != CG_PATH_COMMAND_MOVE_TO)
			calculator->length += hypotf(points[i].x - calculator->current_point.x, points[i].y - calculator->current_point.y);
		calculator->current_point = points[i];
	}
}

float cg_path_extents(struct cg_path_t * path, struct cg_rect_t * extents, int tight)
{
	struct cg_extents_calculator_t calculator = { { 0, 0 }, 1, 0, 0, 0, 0, 0 };
	if(tight)
		cg_path_traverse_flatten(path, extents_traverse_func, &calculator);
	else
		cg_path_traverse(path, extents_traverse_func, &calculator);
	if(extents)
	{
		extents->x = calculator.x1;
		extents->y = calculator.y1;
		extents->w = calculator.x2 - calculator.x1;
		extents->h = calculator.y2 - calculator.y1;
	}
	return calculator.length;
}

static void cg_span_buffer_reset(struct cg_span_buffer_t * span_buffer)
{
	cg_array_clear(span_buffer->spans);
	span_buffer->x = 0;
	span_buffer->y = 0;
	span_buffer->w = -1;
	span_buffer->h = -1;
}

static void cg_span_buffer_init(struct cg_span_buffer_t * span_buffer)
{
	cg_array_init(span_buffer->spans);
	cg_span_buffer_reset(span_buffer);
}

static void cg_span_buffer_init_rect(struct cg_span_buffer_t * span_buffer, int x, int y, int width, int height)
{
	cg_array_clear(span_buffer->spans);
	cg_array_ensure(span_buffer->spans, height);
	struct cg_span_t * spans = span_buffer->spans.data;
	for(int i = 0; i < height; i++)
	{
		spans[i].x = x;
		spans[i].y = y + i;
		spans[i].len = width;
		spans[i].coverage = 255;
	}
	span_buffer->x = x;
	span_buffer->y = y;
	span_buffer->w = width;
	span_buffer->h = height;
	span_buffer->spans.size = height;
}

static void cg_span_buffer_destroy(struct cg_span_buffer_t * span_buffer)
{
	cg_array_destroy(span_buffer->spans);
}

static void cg_span_buffer_copy(struct cg_span_buffer_t * span_buffer, struct cg_span_buffer_t * source)
{
	cg_array_clear(span_buffer->spans);
	cg_array_append(span_buffer->spans, source->spans);
	span_buffer->x = source->x;
	span_buffer->y = source->y;
	span_buffer->w = source->w;
	span_buffer->h = source->h;
}

static int cg_span_buffer_contains(struct cg_span_buffer_t * span_buffer, float x, float y)
{
	int ix = (int)floorf(x);
	int iy = (int)floorf(y);
	for(int i = 0; i < span_buffer->spans.size; i++)
	{
		struct cg_span_t * span = &span_buffer->spans.data[i];
		if(span->y != iy)
			continue;
		if(ix >= span->x && ix < (span->x + span->len))
			return 1;
	}
	return 0;
}

static void cg_span_buffer_update_extents(struct cg_span_buffer_t * span_buffer)
{
	if(span_buffer->w != -1 && span_buffer->h != -1)
		return;
	if(span_buffer->spans.size == 0)
	{
		span_buffer->x = 0;
		span_buffer->y = 0;
		span_buffer->w = 0;
		span_buffer->h = 0;
		return;
	}
	struct cg_span_t * spans = span_buffer->spans.data;
	int x1 = INT_MAX;
	int y1 = spans[0].y;
	int x2 = 0;
	int y2 = spans[span_buffer->spans.size - 1].y;
	for(int i = 0; i < span_buffer->spans.size; i++)
	{
		if(spans[i].x < x1)
			x1 = spans[i].x;
		if(spans[i].x + spans[i].len > x2)
			x2 = spans[i].x + spans[i].len;
	}
	span_buffer->x = x1;
	span_buffer->y = y1;
	span_buffer->w = x2 - x1;
	span_buffer->h = y2 - y1 + 1;
}

static void cg_span_buffer_extents(struct cg_span_buffer_t * span_buffer, struct cg_rect_t * extents)
{
	cg_span_buffer_update_extents(span_buffer);
	extents->x = span_buffer->x;
	extents->y = span_buffer->y;
	extents->w = span_buffer->w;
	extents->h = span_buffer->h;
}

static void cg_span_buffer_intersect(struct cg_span_buffer_t * span_buffer, struct cg_span_buffer_t * a, struct cg_span_buffer_t * b)
{
	cg_span_buffer_reset(span_buffer);
	cg_array_ensure(span_buffer->spans, CG_MAX(a->spans.size, b->spans.size));

	struct cg_span_t *a_spans = a->spans.data;
	struct cg_span_t *a_end = a_spans + a->spans.size;
	struct cg_span_t *b_spans = b->spans.data;
	struct cg_span_t *b_end = b_spans + b->spans.size;
	while(a_spans < a_end && b_spans < b_end)
	{
		if(b_spans->y > a_spans->y)
		{
			++a_spans;
			continue;
		}
		if(a_spans->y != b_spans->y)
		{
			++b_spans;
			continue;
		}
		int ax1 = a_spans->x;
		int ax2 = ax1 + a_spans->len;
		int bx1 = b_spans->x;
		int bx2 = bx1 + b_spans->len;
		if(bx1 < ax1 && bx2 < ax1)
		{
			++b_spans;
			continue;
		}
		if(ax1 < bx1 && ax2 < bx1)
		{
			++a_spans;
			continue;
		}
		int x = CG_MAX(ax1, bx1);
		int len = CG_MIN(ax2, bx2) - x;
		if(len)
		{
			cg_array_ensure(span_buffer->spans, 1);
			struct cg_span_t *span = span_buffer->spans.data + span_buffer->spans.size;
			span->x = x;
			span->len = len;
			span->y = a_spans->y;
			span->coverage = (a_spans->coverage * b_spans->coverage) / 255;
			span_buffer->spans.size += 1;
		}
		if(ax2 < bx2)
			++a_spans;
		else
			++b_spans;
	}
}

#define ALIGN_SIZE(size)	(((size) + 7ul) & ~7ul)
static XCG_FT_Outline * ft_outline_create(int points, int contours)
{
	size_t points_size = ALIGN_SIZE((points + contours) * sizeof(XCG_FT_Vector));
	size_t tags_size = ALIGN_SIZE((points + contours) * sizeof(char));
	size_t contours_size = ALIGN_SIZE(contours * sizeof(int));
	size_t contours_flag_size = ALIGN_SIZE(contours * sizeof(char));
	XCG_FT_Outline * outline = malloc(points_size + tags_size + contours_size + contours_flag_size + sizeof(XCG_FT_Outline));

	XCG_FT_Byte * outline_data = (XCG_FT_Byte*)(outline + 1);
	outline->points = (XCG_FT_Vector*)(outline_data);
	outline->tags = (char*)(outline_data + points_size);
	outline->contours = (int*)(outline_data + points_size + tags_size);
	outline->contours_flag = (char*)(outline_data + points_size + tags_size + contours_size);
	outline->n_points = 0;
	outline->n_contours = 0;
	outline->flags = 0x0;
	return outline;
}

static void ft_outline_destroy(XCG_FT_Outline * outline)
{
	free(outline);
}

#define FT_COORD(x)		(XCG_FT_Pos)(roundf(x * 64))
static void ft_outline_move_to(XCG_FT_Outline * ft, float x, float y)
{
	ft->points[ft->n_points].x = FT_COORD(x);
	ft->points[ft->n_points].y = FT_COORD(y);
	ft->tags[ft->n_points] = XCG_FT_CURVE_TAG_ON;
	if(ft->n_points)
	{
		ft->contours[ft->n_contours] = ft->n_points - 1;
		ft->n_contours++;
	}
	ft->contours_flag[ft->n_contours] = 1;
	ft->n_points++;
}

static void ft_outline_line_to(XCG_FT_Outline * ft, float x, float y)
{
	ft->points[ft->n_points].x = FT_COORD(x);
	ft->points[ft->n_points].y = FT_COORD(y);
	ft->tags[ft->n_points] = XCG_FT_CURVE_TAG_ON;
	ft->n_points++;
}

static void ft_outline_cubic_to(XCG_FT_Outline * ft, float x1, float y1, float x2, float y2, float x3, float y3)
{
	ft->points[ft->n_points].x = FT_COORD(x1);
	ft->points[ft->n_points].y = FT_COORD(y1);
	ft->tags[ft->n_points] = XCG_FT_CURVE_TAG_CUBIC;
	ft->n_points++;

	ft->points[ft->n_points].x = FT_COORD(x2);
	ft->points[ft->n_points].y = FT_COORD(y2);
	ft->tags[ft->n_points] = XCG_FT_CURVE_TAG_CUBIC;
	ft->n_points++;

	ft->points[ft->n_points].x = FT_COORD(x3);
	ft->points[ft->n_points].y = FT_COORD(y3);
	ft->tags[ft->n_points] = XCG_FT_CURVE_TAG_ON;
	ft->n_points++;
}

static void ft_outline_close(XCG_FT_Outline * ft)
{
	ft->contours_flag[ft->n_contours] = 0;
	int index = ft->n_contours ? ft->contours[ft->n_contours - 1] + 1 : 0;
	if(index == ft->n_points)
		return;
	ft->points[ft->n_points].x = ft->points[index].x;
	ft->points[ft->n_points].y = ft->points[index].y;
	ft->tags[ft->n_points] = XCG_FT_CURVE_TAG_ON;
	ft->n_points++;
}

static void ft_outline_end(XCG_FT_Outline * ft)
{
	if(ft->n_points)
	{
		ft->contours[ft->n_contours] = ft->n_points - 1;
		ft->n_contours++;
	}
}

static XCG_FT_Outline * ft_outline_convert_stroke(struct cg_path_t * path, struct cg_matrix_t * matrix, struct cg_stroke_data_t * stroke_data);
static XCG_FT_Outline * ft_outline_convert(struct cg_path_t * path, struct cg_matrix_t * matrix, struct cg_stroke_data_t * stroke_data)
{
	if(stroke_data)
		return ft_outline_convert_stroke(path, matrix, stroke_data);

	struct cg_path_iterator_t it;
	cg_path_iterator_init(&it, path);

	struct cg_point_t points[3];
	XCG_FT_Outline *outline = ft_outline_create(path->num_points, path->num_contours);
	while(cg_path_iterator_has_next(&it))
	{
		switch(cg_path_iterator_next(&it, points))
		{
		case CG_PATH_COMMAND_MOVE_TO:
			cg_matrix_map_points(matrix, points, points, 1);
			ft_outline_move_to(outline, points[0].x, points[0].y);
			break;
		case CG_PATH_COMMAND_LINE_TO:
			cg_matrix_map_points(matrix, points, points, 1);
			ft_outline_line_to(outline, points[0].x, points[0].y);
			break;
		case CG_PATH_COMMAND_CUBIC_TO:
			cg_matrix_map_points(matrix, points, points, 3);
			ft_outline_cubic_to(outline, points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y);
			break;
		case CG_PATH_COMMAND_CLOSE:
			ft_outline_close(outline);
			break;
		default:
			break;
		}
	}
	ft_outline_end(outline);
	return outline;
}

static XCG_FT_Outline * ft_outline_convert_dash(struct cg_path_t * path, struct cg_matrix_t * matrix, struct cg_stroke_dash_t * stroke_dash)
{
	if(stroke_dash->array.size == 0)
		return ft_outline_convert(path, matrix, NULL);
	struct cg_path_t * dashed = cg_path_clone_dashed(path, stroke_dash->array.data, stroke_dash->array.size, stroke_dash->offset);
	XCG_FT_Outline * outline = ft_outline_convert(dashed, matrix, NULL);
	cg_path_destroy(dashed);
	return outline;
}

static XCG_FT_Outline * ft_outline_convert_stroke(struct cg_path_t * path, struct cg_matrix_t * matrix, struct cg_stroke_data_t * stroke_data)
{
	double scale_x = sqrt(matrix->a * matrix->a + matrix->b * matrix->b);
	double scale_y = sqrt(matrix->c * matrix->c + matrix->d * matrix->d);

	double scale = hypot(scale_x, scale_y) / CG_SQRT2;
	double width = stroke_data->style.width * scale;

	XCG_FT_Fixed ftWidth = (XCG_FT_Fixed)(width * 0.5 * (1 << 6));
	XCG_FT_Fixed ftMiterLimit = (XCG_FT_Fixed)(stroke_data->style.miter_limit * (1 << 16));

	XCG_FT_Stroker_LineCap ftCap;
	switch(stroke_data->style.cap)
	{
	case CG_LINE_CAP_SQUARE:
		ftCap = XCG_FT_STROKER_LINECAP_SQUARE;
		break;
	case CG_LINE_CAP_ROUND:
		ftCap = XCG_FT_STROKER_LINECAP_ROUND;
		break;
	default:
		ftCap = XCG_FT_STROKER_LINECAP_BUTT;
		break;
	}

	XCG_FT_Stroker_LineJoin ftJoin;
	switch(stroke_data->style.join)
	{
	case CG_LINE_JOIN_BEVEL:
		ftJoin = XCG_FT_STROKER_LINEJOIN_BEVEL;
		break;
	case CG_LINE_JOIN_ROUND:
		ftJoin = XCG_FT_STROKER_LINEJOIN_ROUND;
		break;
	default:
		ftJoin = XCG_FT_STROKER_LINEJOIN_MITER_FIXED;
		break;
	}

	XCG_FT_Stroker stroker;
	XCG_FT_Stroker_New(&stroker);
	XCG_FT_Stroker_Set(stroker, ftWidth, ftCap, ftJoin, ftMiterLimit);

	XCG_FT_Outline *outline = ft_outline_convert_dash(path, matrix, &stroke_data->dash);
	XCG_FT_Stroker_ParseOutline(stroker, outline);

	XCG_FT_UInt points;
	XCG_FT_UInt contours;
	XCG_FT_Stroker_GetCounts(stroker, &points, &contours);

	XCG_FT_Outline * stroke_outline = ft_outline_create(points, contours);
	XCG_FT_Stroker_Export(stroker, stroke_outline);

	XCG_FT_Stroker_Done(stroker);
	ft_outline_destroy(outline);
	return stroke_outline;
}

static void spans_generation_callback(int count, const XCG_FT_Span * spans, void * user)
{
	struct cg_span_buffer_t * span_buffer = (struct cg_span_buffer_t *)(user);
	cg_array_append_data(span_buffer->spans, spans, count);
}

static void cg_rasterize(struct cg_span_buffer_t * span_buffer, struct cg_path_t * path, struct cg_matrix_t * matrix, struct cg_rect_t * clip_rect, struct cg_stroke_data_t * stroke_data, enum cg_fill_rule_t rule)
{
	XCG_FT_Outline * outline = ft_outline_convert(path, matrix, stroke_data);
	if(stroke_data)
	{
		outline->flags = XCG_FT_OUTLINE_NONE;
	}
	else
	{
		switch(rule)
		{
		case CG_FILL_RULE_EVEN_ODD:
			outline->flags = XCG_FT_OUTLINE_EVEN_ODD_FILL;
			break;
		default:
			outline->flags = XCG_FT_OUTLINE_NONE;
			break;
		}
	}

	XCG_FT_Raster_Params params;
	params.flags = XCG_FT_RASTER_FLAG_DIRECT | XCG_FT_RASTER_FLAG_AA;
	params.gray_spans = spans_generation_callback;
	params.user = span_buffer;
	params.source = outline;
	if(clip_rect)
	{
		params.flags |= XCG_FT_RASTER_FLAG_CLIP;
		params.clip_box.xMin = (XCG_FT_Pos)clip_rect->x;
		params.clip_box.yMin = (XCG_FT_Pos)clip_rect->y;
		params.clip_box.xMax = (XCG_FT_Pos)(clip_rect->x + clip_rect->w);
		params.clip_box.yMax = (XCG_FT_Pos)(clip_rect->y + clip_rect->h);
	}
	cg_span_buffer_reset(span_buffer);
	XCG_FT_Raster_Render(&params);
	ft_outline_destroy(outline);
}

struct cg_gradient_data_t {
	struct cg_matrix_t matrix;
	enum cg_spread_method_t spread;
	uint32_t colortable[1024];
	union {
		struct {
			float x1, y1;
			float x2, y2;
		} linear;
		struct {
			float fx, fy, fr;
			float cx, cy, cr;
		} radial;
	} values;
};

struct cg_texture_data_t {
	struct cg_matrix_t matrix;
	uint8_t * data;
	int width;
	int height;
	int stride;
	int const_alpha;
};

struct cg_linear_gradient_values_t {
	float dx;
	float dy;
	float l;
	float off;
};

struct cg_radial_gradient_values_t {
	float dx;
	float dy;
	float dr;
	float sqrfr;
	float a;
	int extended;
};

static inline uint32_t premultiply_color_with_opacity(struct cg_color_t * color, float opacity)
{
	uint32_t alpha = CG_ROUND(color->a * opacity * 255);
	uint32_t pr = CG_ROUND(color->r * alpha);
	uint32_t pg = CG_ROUND(color->g * alpha);
	uint32_t pb = CG_ROUND(color->b * alpha);
	return (alpha << 24) | (pr << 16) | (pg << 8) | (pb);
}

static inline uint32_t interpolate_pixel_255(uint32_t x, uint32_t a, uint32_t y, uint32_t b)
{
	uint32_t t = (x & 0xff00ff) * a + (y & 0xff00ff) * b;
	t = (t + ((t >> 8) & 0xff00ff) + 0x800080) >> 8;
	t &= 0xff00ff;
	x = ((x >> 8) & 0xff00ff) * a + ((y >> 8) & 0xff00ff) * b;
	x = (x + ((x >> 8) & 0xff00ff) + 0x800080);
	x &= 0xff00ff00;
	x |= t;
	return x;
}

static inline uint32_t interpolate_pixel_256(uint32_t x, uint32_t a, uint32_t y, uint32_t b)
{
	uint32_t t = (x & 0xff00ff) * a + (y & 0xff00ff) * b;
	t >>= 8;
	t &= 0xff00ff;
	x = ((x >> 8) & 0xff00ff) * a + ((y >> 8) & 0xff00ff) * b;
	x &= 0xff00ff00;
	x |= t;
	return x;
}

static void cg_memfill32(uint32_t * dst, int len, uint32_t val)
{
	for(int i = 0; i < len; i++)
		dst[i] = val;
}

static inline int gradient_clamp(struct cg_gradient_data_t *gradient, int ipos)
{
	if(gradient->spread == CG_SPREAD_METHOD_REPEAT)
	{
		ipos = ipos % 1024;
		ipos = ipos < 0 ? 1024 + ipos : ipos;
	}
	else if(gradient->spread == CG_SPREAD_METHOD_REFLECT)
	{
		int limit = 1024 * 2;
		ipos = ipos % limit;
		ipos = ipos < 0 ? limit + ipos : ipos;
		ipos = ipos >= 1024 ? limit - 1 - ipos : ipos;
	}
	else
	{
		if(ipos < 0)
			ipos = 0;
		else if(ipos >= 1024)
			ipos = 1024 - 1;
	}
	return ipos;
}

#define FIXPT_BITS	8
#define FIXPT_SIZE	(1 << FIXPT_BITS)
static inline uint32_t gradient_pixel_fixed(struct cg_gradient_data_t * gradient, int fixed_pos)
{
	int ipos = (fixed_pos + (FIXPT_SIZE / 2)) >> FIXPT_BITS;
	return gradient->colortable[gradient_clamp(gradient, ipos)];
}

static inline uint32_t gradient_pixel(struct cg_gradient_data_t * gradient, float pos)
{
	int ipos = (int)(pos * (1024 - 1) + 0.5f);
	return gradient->colortable[gradient_clamp(gradient, ipos)];
}

static void fetch_linear_gradient(uint32_t * buffer, struct cg_linear_gradient_values_t * v, struct cg_gradient_data_t * gradient, int y, int x, int length)
{
	float t, inc;
	float rx = 0, ry = 0;

	if(v->l == 0.0f)
	{
		t = inc = 0;
	}
	else
	{
		rx = gradient->matrix.c * (y + 0.5f) + gradient->matrix.a * (x + 0.5f) + gradient->matrix.tx;
		ry = gradient->matrix.d * (y + 0.5f) + gradient->matrix.b * (x + 0.5f) + gradient->matrix.ty;
		t = v->dx * rx + v->dy * ry + v->off;
		inc = v->dx * gradient->matrix.a + v->dy * gradient->matrix.b;
		t *= (1024 - 1);
		inc *= (1024 - 1);
	}

	uint32_t * end = buffer + length;
	if(inc > -1e-5f && inc < 1e-5f)
	{
		cg_memfill32(buffer, length, gradient_pixel_fixed(gradient, (int)(t * FIXPT_SIZE)));
	}
	else
	{
		if(t + inc * length < (float)(INT_MAX >> (FIXPT_BITS + 1)) && t + inc * length > (float)(INT_MIN >> (FIXPT_BITS + 1)))
		{
			int t_fixed = (int)(t * FIXPT_SIZE);
			int inc_fixed = (int)(inc * FIXPT_SIZE);
			while(buffer < end)
			{
				*buffer = gradient_pixel_fixed(gradient, t_fixed);
				t_fixed += inc_fixed;
				++buffer;
			}
		}
		else
		{
			while(buffer < end)
			{
				*buffer = gradient_pixel(gradient, t / 1024);
				t += inc;
				++buffer;
			}
		}
	}
}

static void fetch_radial_gradient(uint32_t * buffer, struct cg_radial_gradient_values_t * v, struct cg_gradient_data_t * gradient, int y, int x, int length)
{
	if(v->a == 0.0f)
	{
		cg_memfill32(buffer, length, 0);
		return;
	}

	float rx = gradient->matrix.c * (y + 0.5f) + gradient->matrix.tx + gradient->matrix.a * (x + 0.5f);
	float ry = gradient->matrix.d * (y + 0.5f) + gradient->matrix.ty + gradient->matrix.b * (x + 0.5f);
	rx -= gradient->values.radial.fx;
	ry -= gradient->values.radial.fy;

	float inv_a = 1.0f / (2.0f * v->a);
	float delta_rx = gradient->matrix.a;
	float delta_ry = gradient->matrix.b;

	float b = 2 * (v->dr * gradient->values.radial.fr + rx * v->dx + ry * v->dy);
	float delta_b = 2 * (delta_rx * v->dx + delta_ry * v->dy);
	float b_delta_b = 2 * b * delta_b;
	float delta_b_delta_b = 2 * delta_b * delta_b;

	float bb = b * b;
	float delta_bb = delta_b * delta_b;

	b *= inv_a;
	delta_b *= inv_a;

	float rxrxryry = rx * rx + ry * ry;
	float delta_rxrxryry = delta_rx * delta_rx + delta_ry * delta_ry;
	float rx_plus_ry = 2 * (rx * delta_rx + ry * delta_ry);
	float delta_rx_plus_ry = 2 * delta_rxrxryry;

	inv_a *= inv_a;

	float det = (bb - 4 * v->a * (v->sqrfr - rxrxryry)) * inv_a;
	float delta_det = (b_delta_b + delta_bb + 4 * v->a * (rx_plus_ry + delta_rxrxryry)) * inv_a;
	float delta_delta_det = (delta_b_delta_b + 4 * v->a * delta_rx_plus_ry) * inv_a;

	uint32_t * end = buffer + length;
	if(v->extended)
	{
		while(buffer < end)
		{
			uint32_t result = 0;
			if(det >= 0)
			{
				float w = sqrtf(det) - b;
				if(gradient->values.radial.fr + v->dr * w >= 0)
					result = gradient_pixel(gradient, w);
			}
			*buffer = result;
			det += delta_det;
			delta_det += delta_delta_det;
			b += delta_b;
			++buffer;
		}
	}
	else
	{
		while(buffer < end)
		{
			*buffer++ = gradient_pixel(gradient, sqrtf(det) - b);
			det += delta_det;
			delta_det += delta_delta_det;
			b += delta_b;
		}
	}
}

static void composition_solid_clear(uint32_t * dest, int length, uint32_t color, uint32_t const_alpha)
{
	if(const_alpha == 255)
		cg_memfill32(dest, length, 0);
	else
	{
		uint32_t ialpha = 255 - const_alpha;
		for(int i = 0; i < length; i++)
			dest[i] = CG_BYTE_MUL(dest[i], ialpha);
	}
}

static void composition_solid_source(uint32_t * dest, int length, uint32_t color, uint32_t const_alpha)
{
	if(const_alpha == 255)
		cg_memfill32(dest, length, color);
	else
	{
		uint32_t ialpha = 255 - const_alpha;
		color = CG_BYTE_MUL(color, const_alpha);
		for(int i = 0; i < length; i++)
			dest[i] = color + CG_BYTE_MUL(dest[i], ialpha);
	}
}

static void composition_solid_destination(uint32_t * dest, int length, uint32_t color, uint32_t const_alpha)
{
}

static void composition_solid_source_over(uint32_t * dest, int length, uint32_t color, uint32_t const_alpha)
{
	if(const_alpha != 255)
		color = CG_BYTE_MUL(color, const_alpha);
	uint32_t ialpha = 255 - CG_ALPHA(color);
	for(int i = 0; i < length; i++)
		dest[i] = color + CG_BYTE_MUL(dest[i], ialpha);
}

static void composition_solid_destination_over(uint32_t * dest, int length, uint32_t color, uint32_t const_alpha)
{
	if(const_alpha != 255)
		color = CG_BYTE_MUL(color, const_alpha);
	for(int i = 0; i < length; i++)
	{
		uint32_t d = dest[i];
		dest[i] = d + CG_BYTE_MUL(color, CG_ALPHA(~d));
	}
}

static void composition_solid_source_in(uint32_t * dest, int length, uint32_t color, uint32_t const_alpha)
{
	if(const_alpha == 255)
	{
		for(int i = 0; i < length; i++)
			dest[i] = CG_BYTE_MUL(color, CG_ALPHA(dest[i]));
	}
	else
	{
		color = CG_BYTE_MUL(color, const_alpha);
		uint32_t cia = 255 - const_alpha;
		for(int i = 0; i < length; i++)
		{
			uint32_t d = dest[i];
			dest[i] = interpolate_pixel_255(color, CG_ALPHA(d), d, cia);
		}
	}
}

static void composition_solid_destination_in(uint32_t * dest, int length, uint32_t color, uint32_t const_alpha)
{
	uint32_t a = CG_ALPHA(color);
	if(const_alpha != 255)
		a = CG_BYTE_MUL(a, const_alpha) + 255 - const_alpha;
	for(int i = 0; i < length; i++)
		dest[i] = CG_BYTE_MUL(dest[i], a);
}

static void composition_solid_source_out(uint32_t * dest, int length, uint32_t color, uint32_t const_alpha)
{
	if(const_alpha == 255)
	{
		for(int i = 0; i < length; i++)
			dest[i] = CG_BYTE_MUL(color, CG_ALPHA(~dest[i]));
	}
	else
	{
		color = CG_BYTE_MUL(color, const_alpha);
		uint32_t cia = 255 - const_alpha;
		for(int i = 0; i < length; i++)
		{
			uint32_t d = dest[i];
			dest[i] = interpolate_pixel_255(color, CG_ALPHA(~d), d, cia);
		}
	}
}

static void composition_solid_destination_out(uint32_t * dest, int length, uint32_t color, uint32_t const_alpha)
{
	uint32_t a = CG_ALPHA(~color);
	if(const_alpha != 255)
		a = CG_BYTE_MUL(a, const_alpha) + 255 - const_alpha;
	for(int i = 0; i < length; i++)
		dest[i] = CG_BYTE_MUL(dest[i], a);
}

static void composition_solid_source_atop(uint32_t * dest, int length, uint32_t color, uint32_t const_alpha)
{
	if(const_alpha != 255)
		color = CG_BYTE_MUL(color, const_alpha);
	uint32_t sia = CG_ALPHA(~color);
	for(int i = 0; i < length; i++)
	{
		uint32_t d = dest[i];
		dest[i] = interpolate_pixel_255(color, CG_ALPHA(d), d, sia);
	}
}

static void composition_solid_destination_atop(uint32_t * dest, int length, uint32_t color, uint32_t const_alpha)
{
	uint32_t a = CG_ALPHA(color);
	if(const_alpha != 255)
	{
		color = CG_BYTE_MUL(color, const_alpha);
		a = CG_ALPHA(color) + 255 - const_alpha;
	}
	for(int i = 0; i < length; i++)
	{
		uint32_t d = dest[i];
		dest[i] = interpolate_pixel_255(d, a, color, CG_ALPHA(~d));
	}
}

static void composition_solid_xor(uint32_t * dest, int length, uint32_t color, uint32_t const_alpha)
{
	if(const_alpha != 255)
		color = CG_BYTE_MUL(color, const_alpha);
	uint32_t sia = CG_ALPHA(~color);
	for(int i = 0; i < length; i++)
	{
		uint32_t d = dest[i];
		dest[i] = interpolate_pixel_255(color, CG_ALPHA(~d), d, sia);
	}
}

typedef void (*composition_solid_function_t)(uint32_t * dest, int length, uint32_t color, uint32_t const_alpha);
static composition_solid_function_t composition_solid_table[] = {
	composition_solid_clear,
	composition_solid_source,
	composition_solid_destination,
	composition_solid_source_over,
	composition_solid_destination_over,
	composition_solid_source_in,
	composition_solid_destination_in,
	composition_solid_source_out,
	composition_solid_destination_out,
	composition_solid_source_atop,
	composition_solid_destination_atop,
	composition_solid_xor,
};

static void composition_clear(uint32_t * dest, int length, uint32_t * src, uint32_t const_alpha)
{
	if(const_alpha == 255)
		cg_memfill32(dest, length, 0);
	else
	{
		uint32_t ialpha = 255 - const_alpha;
		for(int i = 0; i < length; i++)
			dest[i] = CG_BYTE_MUL(dest[i], ialpha);
	}
}

static void composition_source(uint32_t * dest, int length, uint32_t * src, uint32_t const_alpha)
{
	if(const_alpha == 255)
		memcpy(dest, src, length * sizeof(uint32_t));
	else
	{
		uint32_t ialpha = 255 - const_alpha;
		for(int i = 0; i < length; i++)
			dest[i] = interpolate_pixel_255(src[i], const_alpha, dest[i], ialpha);
	}
}

static void composition_destination(uint32_t * dest, int length, uint32_t * src, uint32_t const_alpha)
{
}

static void composition_source_over(uint32_t * dest, int length, uint32_t *src, uint32_t const_alpha)
{
	if(const_alpha == 255)
	{
		for(int i = 0; i < length; i++)
		{
			uint32_t s = src[i];
			if(s >= 0xff000000)
				dest[i] = s;
			else if(s != 0)
				dest[i] = s + CG_BYTE_MUL(dest[i], CG_ALPHA(~s));
		}
	}
	else
	{
		for(int i = 0; i < length; i++)
		{
			uint32_t s = CG_BYTE_MUL(src[i], const_alpha);
			dest[i] = s + CG_BYTE_MUL(dest[i], CG_ALPHA(~s));
		}
	}
}

static void composition_destination_over(uint32_t * dest, int length, uint32_t * src, uint32_t const_alpha)
{
	if(const_alpha == 255)
	{
		for(int i = 0; i < length; i++)
		{
			uint32_t d = dest[i];
			dest[i] = d + CG_BYTE_MUL(src[i], CG_ALPHA(~d));
		}
	}
	else
	{
		for(int i = 0; i < length; i++)
		{
			uint32_t d = dest[i];
			uint32_t s = CG_BYTE_MUL(src[i], const_alpha);
			dest[i] = d + CG_BYTE_MUL(s, CG_ALPHA(~d));
		}
	}
}

static void composition_source_in(uint32_t * dest, int length, uint32_t * src, uint32_t const_alpha)
{
	if(const_alpha == 255)
	{
		for(int i = 0; i < length; i++)
			dest[i] = CG_BYTE_MUL(src[i], CG_ALPHA(dest[i]));
	}
	else
	{
		uint32_t cia = 255 - const_alpha;
		for(int i = 0; i < length; i++)
		{
			uint32_t d = dest[i];
			uint32_t s = CG_BYTE_MUL(src[i], const_alpha);
			dest[i] = interpolate_pixel_255(s, CG_ALPHA(d), d, cia);
		}
	}
}

static void composition_destination_in(uint32_t * dest, int length, uint32_t * src, uint32_t const_alpha)
{
	if(const_alpha == 255)
	{
		for(int i = 0; i < length; i++)
			dest[i] = CG_BYTE_MUL(dest[i], CG_ALPHA(src[i]));
	}
	else
	{
		uint32_t cia = 255 - const_alpha;
		for(int i = 0; i < length; i++)
		{
			uint32_t a = CG_BYTE_MUL(CG_ALPHA(src[i]), const_alpha) + cia;
			dest[i] = CG_BYTE_MUL(dest[i], a);
		}
	}
}

static void composition_source_out(uint32_t * dest, int length, uint32_t * src, uint32_t const_alpha)
{
	if(const_alpha == 255)
	{
		for(int i = 0; i < length; i++)
			dest[i] = CG_BYTE_MUL(src[i], CG_ALPHA(~dest[i]));
	}
	else
	{
		uint32_t cia = 255 - const_alpha;
		for(int i = 0; i < length; i++)
		{
			uint32_t s = CG_BYTE_MUL(src[i], const_alpha);
			uint32_t d = dest[i];
			dest[i] = interpolate_pixel_255(s, CG_ALPHA(~d), d, cia);
		}
	}
}

static void composition_destination_out(uint32_t * dest, int length, uint32_t * src, uint32_t const_alpha)
{
	if(const_alpha == 255)
	{
		for(int i = 0; i < length; i++)
			dest[i] = CG_BYTE_MUL(dest[i], CG_ALPHA(~src[i]));
	}
	else
	{
		uint32_t cia = 255 - const_alpha;
		for(int i = 0; i < length; i++)
		{
			uint32_t sia = CG_BYTE_MUL(CG_ALPHA(~src[i]), const_alpha) + cia;
			dest[i] = CG_BYTE_MUL(dest[i], sia);
		}
	}
}

static void composition_source_atop(uint32_t * dest, int length, uint32_t * src, uint32_t const_alpha)
{
	if(const_alpha == 255)
	{
		for(int i = 0; i < length; i++)
		{
			uint32_t s = src[i];
			uint32_t d = dest[i];
			dest[i] = interpolate_pixel_255(s, CG_ALPHA(d), d, CG_ALPHA(~s));
		}
	}
	else
	{
		for(int i = 0; i < length; i++)
		{
			uint32_t s = CG_BYTE_MUL(src[i], const_alpha);
			uint32_t d = dest[i];
			dest[i] = interpolate_pixel_255(s, CG_ALPHA(d), d, CG_ALPHA(~s));
		}
	}
}

static void composition_destination_atop(uint32_t * dest, int length, uint32_t * src, uint32_t const_alpha)
{
	if(const_alpha == 255)
	{
		for(int i = 0; i < length; i++)
		{
			uint32_t s = src[i];
			uint32_t d = dest[i];
			dest[i] = interpolate_pixel_255(d, CG_ALPHA(s), s, CG_ALPHA(~d));
		}
	}
	else
	{
		uint32_t cia = 255 - const_alpha;
		for(int i = 0; i < length; i++)
		{
			uint32_t s = CG_BYTE_MUL(src[i], const_alpha);
			uint32_t d = dest[i];
			uint32_t a = CG_ALPHA(s) + cia;
			dest[i] = interpolate_pixel_255(d, a, s, CG_ALPHA(~d));
		}
	}
}

static void composition_xor(uint32_t * dest, int length, uint32_t * src, uint32_t const_alpha)
{
	if(const_alpha == 255)
	{
		for(int i = 0; i < length; i++)
		{
			uint32_t d = dest[i];
			uint32_t s = src[i];
			dest[i] = interpolate_pixel_255(s, CG_ALPHA(~d), d, CG_ALPHA(~s));
		}
	}
	else
	{
		for(int i = 0; i < length; i++)
		{
			uint32_t d = dest[i];
			uint32_t s = CG_BYTE_MUL(src[i], const_alpha);
			dest[i] = interpolate_pixel_255(s, CG_ALPHA(~d), d, CG_ALPHA(~s));
		}
	}
}

typedef void (*composition_function_t)(uint32_t * dest, int length, uint32_t * src, uint32_t const_alpha);
static composition_function_t composition_table[] = {
	composition_clear,
	composition_source,
	composition_destination,
	composition_source_over,
	composition_destination_over,
	composition_source_in,
	composition_destination_in,
	composition_source_out,
	composition_destination_out,
	composition_source_atop,
	composition_destination_atop,
	composition_xor,
};

static void blend_solid(struct cg_surface_t * surface, enum cg_operator_t op, uint32_t solid, struct cg_span_buffer_t * span_buffer)
{
	composition_solid_function_t func = composition_solid_table[op];
	int count = span_buffer->spans.size;
	struct cg_span_t * spans = span_buffer->spans.data;
	while(count--)
	{
		uint32_t * target = (uint32_t *)(surface->pixels + spans->y * surface->stride) + spans->x;
		func(target, spans->len, solid, spans->coverage);
		++spans;
	}
}

static void blend_linear_gradient(struct cg_surface_t * surface, enum cg_operator_t op, struct cg_gradient_data_t * gradient, struct cg_span_buffer_t * span_buffer)
{
	composition_function_t func = composition_table[op];
	unsigned int buffer[1024];

	struct cg_linear_gradient_values_t v;
	v.dx = gradient->values.linear.x2 - gradient->values.linear.x1;
	v.dy = gradient->values.linear.y2 - gradient->values.linear.y1;
	v.l = v.dx * v.dx + v.dy * v.dy;
	v.off = 0.0f;
	if(v.l != 0.0f)
	{
		v.dx /= v.l;
		v.dy /= v.l;
		v.off = -v.dx * gradient->values.linear.x1 - v.dy * gradient->values.linear.y1;
	}

	int count = span_buffer->spans.size;
	struct cg_span_t *spans = span_buffer->spans.data;
	while(count--)
	{
		int length = spans->len;
		int x = spans->x;
		while(length)
		{
			int l = CG_MIN(length, 1024);
			fetch_linear_gradient(buffer, &v, gradient, spans->y, x, l);
			uint32_t * target = (uint32_t *)(surface->pixels + spans->y * surface->stride) + x;
			func(target, l, buffer, spans->coverage);
			x += l;
			length -= l;
		}
		++spans;
	}
}

static void blend_radial_gradient(struct cg_surface_t * surface, enum cg_operator_t op, struct cg_gradient_data_t * gradient, struct cg_span_buffer_t * span_buffer)
{
	composition_function_t func = composition_table[op];
	unsigned int buffer[1024];

	struct cg_radial_gradient_values_t v;
	v.dx = gradient->values.radial.cx - gradient->values.radial.fx;
	v.dy = gradient->values.radial.cy - gradient->values.radial.fy;
	v.dr = gradient->values.radial.cr - gradient->values.radial.fr;
	v.sqrfr = gradient->values.radial.fr * gradient->values.radial.fr;
	v.a = v.dr * v.dr - v.dx * v.dx - v.dy * v.dy;
	v.extended = gradient->values.radial.fr != 0.0f || v.a <= 0.0f;

	int count = span_buffer->spans.size;
	struct cg_span_t * spans = span_buffer->spans.data;
	while(count--)
	{
		int length = spans->len;
		int x = spans->x;
		while(length)
		{
			int l = CG_MIN(length, 1024);
			fetch_radial_gradient(buffer, &v, gradient, spans->y, x, l);
			uint32_t * target = (uint32_t*)(surface->pixels + spans->y * surface->stride) + x;
			func(target, l, buffer, spans->coverage);
			x += l;
			length -= l;
		}
		++spans;
	}
}

static void blend_untransformed_argb(struct cg_surface_t * surface, enum cg_operator_t op, struct cg_texture_data_t * texture, struct cg_span_buffer_t * span_buffer)
{
	composition_function_t func = composition_table[op];

	int image_width = texture->width;
	int image_height = texture->height;
	int xoff = (int)(texture->matrix.tx);
	int yoff = (int)(texture->matrix.ty);
	int count = span_buffer->spans.size;
	struct cg_span_t * spans = span_buffer->spans.data;
	while(count--)
	{
		int x = spans->x;
		int length = spans->len;
		int sx = xoff + x;
		int sy = yoff + spans->y;
		if(sy >= 0 && sy < image_height && sx < image_width)
		{
			if(sx < 0)
			{
				x -= sx;
				length += sx;
				sx = 0;
			}
			if(sx + length > image_width)
				length = image_width - sx;
			if(length > 0)
			{
				int coverage = (spans->coverage * texture->const_alpha) >> 8;
				uint32_t * src = (uint32_t *)(texture->data + sy * texture->stride) + sx;
				uint32_t * dest = (uint32_t *)(surface->pixels + spans->y * surface->stride) + x;
				func(dest, length, src, coverage);
			}
		}
		++spans;
	}
}

#define FIXED_SCALE (1 << 16)
static void blend_transformed_argb(struct cg_surface_t * surface, enum cg_operator_t op, struct cg_texture_data_t * texture, struct cg_span_buffer_t * span_buffer)
{
	composition_function_t func = composition_table[op];
	uint32_t buffer[1024];

	int image_width = texture->width;
	int image_height = texture->height;
	int fdx = (int)(texture->matrix.a * FIXED_SCALE);
	int fdy = (int)(texture->matrix.b * FIXED_SCALE);
	int count = span_buffer->spans.size;
	struct cg_span_t *spans = span_buffer->spans.data;
	while(count--)
	{
		uint32_t * target = (uint32_t *)(surface->pixels + spans->y * surface->stride) + spans->x;
		float cx = spans->x + 0.5f;
		float cy = spans->y + 0.5f;
		int x = (int)((texture->matrix.c * cy + texture->matrix.a * cx + texture->matrix.tx) * FIXED_SCALE);
		int y = (int)((texture->matrix.d * cy + texture->matrix.b * cx + texture->matrix.ty) * FIXED_SCALE);
		int length = spans->len;
		int coverage = (spans->coverage * texture->const_alpha) >> 8;
		while(length)
		{
			int l = CG_MIN(length, 1024);
			uint32_t * end = buffer + l;
			uint32_t * b = buffer;
			int start = 0;
			int clen = 0;
			while(b < end)
			{
				int px = x >> 16;
				int py = y >> 16;
				if(((unsigned int)px < (unsigned int)image_width) && ((unsigned int)py < (unsigned int)image_height))
				{
					*b = ((uint32_t *)(texture->data + py * texture->stride))[px];
					clen++;
				}
				x += fdx;
				y += fdy;
				++b;
				if(clen == 0)
					start++;
			}
			if(clen > 0)
				func(target + start, clen, buffer + start, coverage);
			target += l;
			length -= l;
		}
		++spans;
	}
}

static void blend_untransformed_tiled_argb(struct cg_surface_t * surface, enum cg_operator_t op, struct cg_texture_data_t * texture, struct cg_span_buffer_t * span_buffer)
{
	composition_function_t func = composition_table[op];

	int image_width = texture->width;
	int image_height = texture->height;

	int xoff = (int)(texture->matrix.tx) % image_width;
	int yoff = (int)(texture->matrix.ty) % image_height;

	if(xoff < 0)
		xoff += image_width;
	if(yoff < 0)
	{
		yoff += image_height;
	}

	int count = span_buffer->spans.size;
	struct cg_span_t * spans = span_buffer->spans.data;
	while(count--)
	{
		int x = spans->x;
		int length = spans->len;
		int sx = (xoff + spans->x) % image_width;
		int sy = (spans->y + yoff) % image_height;
		if(sx < 0)
			sx += image_width;
		if(sy < 0)
		{
			sy += image_height;
		}

		int coverage = (spans->coverage * texture->const_alpha) >> 8;
		while(length)
		{
			int l = CG_MIN(image_width - sx, length);
			if(1024 < l)
				l = 1024;
			uint32_t * src = (uint32_t *)(texture->data + sy * texture->stride) + sx;
			uint32_t * dest = (uint32_t *)(surface->pixels + spans->y * surface->stride) + x;
			func(dest, l, src, coverage);
			x += l;
			sx += l;
			length -= l;
			if(sx >= image_width)
			{
				sx = 0;
			}
		}
		++spans;
	}
}

static void blend_transformed_tiled_argb(struct cg_surface_t * surface, enum cg_operator_t op, struct cg_texture_data_t * texture, struct cg_span_buffer_t * span_buffer)
{
	composition_function_t func = composition_table[op];
	uint32_t buffer[1024];

	int image_width = texture->width;
	int image_height = texture->height;
	int scanline_offset = texture->stride / 4;

	int fdx = (int)(texture->matrix.a * FIXED_SCALE);
	int fdy = (int)(texture->matrix.b * FIXED_SCALE);

	int count = span_buffer->spans.size;
	struct cg_span_t * spans = span_buffer->spans.data;
	while(count--)
	{
		uint32_t * target = (uint32_t *)(surface->pixels + spans->y * surface->stride) + spans->x;
		uint32_t * image_bits = (uint32_t *)texture->data;

		float cx = spans->x + 0.5f;
		float cy = spans->y + 0.5f;

		int x = (int)((texture->matrix.c * cy + texture->matrix.a * cx + texture->matrix.tx) * FIXED_SCALE);
		int y = (int)((texture->matrix.d * cy + texture->matrix.b * cx + texture->matrix.ty) * FIXED_SCALE);

		int coverage = (spans->coverage * texture->const_alpha) >> 8;
		int length = spans->len;
		while(length)
		{
			int l = CG_MIN(length, 1024);
			uint32_t * end = buffer + l;
			uint32_t * b = buffer;
			while(b < end)
			{
				int px = x >> 16;
				int py = y >> 16;
				px %= image_width;
				py %= image_height;
				if(px < 0)
					px += image_width;
				if(py < 0)
					py += image_height;
				int y_offset = py * scanline_offset;

				*b = image_bits[y_offset + px];
				x += fdx;
				y += fdy;
				++b;
			}

			func(target, l, buffer, coverage);
			target += l;
			length -= l;
		}
		++spans;
	}
}

static inline uint32_t interpolate_4_pixels(uint32_t tl, uint32_t tr, uint32_t bl, uint32_t br, uint32_t distx, uint32_t disty)
{
	uint32_t idistx = 256 - distx;
	uint32_t idisty = 256 - disty;
	uint32_t xtop = interpolate_pixel_256(tl, idistx, tr, distx);
	uint32_t xbot = interpolate_pixel_256(bl, idistx, br, distx);
	return interpolate_pixel_256(xtop, idisty, xbot, disty);
}

#define HALF_POINT (1 << 15)
static void blend_transformed_bilinear_tiled_argb(struct cg_surface_t * surface, enum cg_operator_t op, struct cg_texture_data_t * texture, struct cg_span_buffer_t * span_buffer)
{
	composition_function_t func = composition_table[op];
	uint32_t buffer[1024];

	int image_width = texture->width;
	int image_height = texture->height;

	int fdx = (int)(texture->matrix.a * FIXED_SCALE);
	int fdy = (int)(texture->matrix.b * FIXED_SCALE);

	int count = span_buffer->spans.size;
	struct cg_span_t * spans = span_buffer->spans.data;
	while(count--)
	{
		uint32_t * target = (uint32_t *)(surface->pixels + spans->y * surface->stride) + spans->x;

		float cx = spans->x + 0.5f;
		float cy = spans->y + 0.5f;

		int fx = (int)((texture->matrix.c * cy + texture->matrix.a * cx + texture->matrix.tx) * FIXED_SCALE);
		int fy = (int)((texture->matrix.d * cy + texture->matrix.b * cx + texture->matrix.ty) * FIXED_SCALE);

		fx -= HALF_POINT;
		fy -= HALF_POINT;

		int coverage = (spans->coverage * texture->const_alpha) >> 8;
		int length = spans->len;
		while(length)
		{
			int l = CG_MIN(length, 1024);
			uint32_t *end = buffer + l;
			uint32_t *b = buffer;
			while(b < end)
			{
				int x1 = (fx >> 16) % image_width;
				int y1 = (fy >> 16) % image_height;

				if(x1 < 0)
					x1 += image_width;
				if(y1 < 0)
					y1 += image_height;

				int x2 = (x1 + 1) % image_width;
				int y2 = (y1 + 1) % image_height;

				uint32_t * s1 = (uint32_t *)(texture->data + y1 * texture->stride);
				uint32_t * s2 = (uint32_t *)(texture->data + y2 * texture->stride);

				uint32_t tl = s1[x1];
				uint32_t tr = s1[x2];
				uint32_t bl = s2[x1];
				uint32_t br = s2[x2];

				int distx = (fx & 0x0000ffff) >> 8;
				int disty = (fy & 0x0000ffff) >> 8;
				*b = interpolate_4_pixels(tl, tr, bl, br, distx, disty);

				fx += fdx;
				fy += fdy;
				++b;
			}
			func(target, l, buffer, coverage);
			target += l;
			length -= l;
		}
		++spans;
	}
}

static void cg_blend_color(struct cg_ctx_t * ctx, struct cg_color_t * color, struct cg_span_buffer_t * span_buffer)
{
	struct cg_state_t * state = ctx->state;
	uint32_t solid = premultiply_color_with_opacity(color, state->opacity);
	uint32_t alpha = CG_ALPHA(solid);

	if((alpha == 255) && (state->op == CG_OPERATOR_SRC_OVER))
		blend_solid(ctx->surface, CG_OPERATOR_SRC, solid, span_buffer);
	else
		blend_solid(ctx->surface, state->op, solid, span_buffer);
}

static void cg_blend_gradient(struct cg_ctx_t * ctx, struct cg_gradient_paint_t * gradient, struct cg_span_buffer_t * span_buffer)
{
	if(gradient->nstops == 0)
		return;
	struct cg_state_t * state = ctx->state;
	struct cg_gradient_data_t data;
	data.spread = gradient->spread;
	data.matrix = gradient->matrix;
	cg_matrix_multiply(&data.matrix, &data.matrix, &state->matrix);
	if(!cg_matrix_invert(&data.matrix))
		return;
	int i, pos = 0, nstops = gradient->nstops;
	struct cg_gradient_stop_t * curr, * next, * start, * last;
	uint32_t curr_color, next_color, last_color;
	uint32_t dist, idist;
	float delta, t, incr, fpos;
	float opacity = state->opacity;

	start = gradient->stops;
	curr = start;
	curr_color = premultiply_color_with_opacity(&curr->color, opacity);

	data.colortable[pos++] = curr_color;
	incr = 1.0f / 1024;
	fpos = 1.5f * incr;

	while(fpos <= curr->offset)
	{
		data.colortable[pos] = data.colortable[pos - 1];
		++pos;
		fpos += incr;
	}

	for(i = 0; i < nstops - 1; i++)
	{
		curr = (start + i);
		next = (start + i + 1);
		if(curr->offset == next->offset)
			continue;
		delta = 1.0f / (next->offset - curr->offset);
		next_color = premultiply_color_with_opacity(&next->color, opacity);
		while(fpos < next->offset && pos < 1024)
		{
			t = (fpos - curr->offset) * delta;
			dist = (uint32_t)(255 * t);
			idist = 255 - dist;
			data.colortable[pos] = interpolate_pixel_255(curr_color, idist, next_color, dist);
			++pos;
			fpos += incr;
		}

		curr_color = next_color;
	}

	last = start + nstops - 1;
	last_color = premultiply_color_with_opacity(&last->color, opacity);
	for(; pos < 1024; ++pos)
	{
		data.colortable[pos] = last_color;
	}

	if(gradient->type == CG_GRADIENT_TYPE_LINEAR)
	{
		data.values.linear.x1 = gradient->values[0];
		data.values.linear.y1 = gradient->values[1];
		data.values.linear.x2 = gradient->values[2];
		data.values.linear.y2 = gradient->values[3];
		blend_linear_gradient(ctx->surface, state->op, &data, span_buffer);
	}
	else
	{
		data.values.radial.fx = gradient->values[0];
		data.values.radial.fy = gradient->values[1];
		data.values.radial.fr = gradient->values[2];
		data.values.radial.cx = gradient->values[3];
		data.values.radial.cy = gradient->values[4];
		data.values.radial.cr = gradient->values[5];
		blend_radial_gradient(ctx->surface, state->op, &data, span_buffer);
	}
}

static void cg_blend_texture(struct cg_ctx_t * ctx, struct cg_texture_paint_t * texture, struct cg_span_buffer_t * span_buffer)
{
	if(texture->surface == NULL)
		return;
	struct cg_state_t *state = ctx->state;
	struct cg_texture_data_t data;
	data.matrix = texture->matrix;
	data.data = texture->surface->pixels;
	data.width = texture->surface->width;
	data.height = texture->surface->height;
	data.stride = texture->surface->stride;
	data.const_alpha = CG_ROUND(state->opacity * texture->opacity * 256);

	cg_matrix_multiply(&data.matrix, &data.matrix, &state->matrix);
	if(!cg_matrix_invert(&data.matrix))
		return;
	struct cg_matrix_t * matrix = &data.matrix;
	if(matrix->a == 1.0 && matrix->b == 0.0 && matrix->c == 0.0 && matrix->d == 1.0)
	{
		if(texture->type == CG_TEXTURE_TYPE_PLAIN)
			blend_untransformed_argb(ctx->surface, state->op, &data, span_buffer);
		else
			blend_untransformed_tiled_argb(ctx->surface, state->op, &data, span_buffer);
	}
	else
	{
		if(texture->type == CG_TEXTURE_TYPE_PLAIN)
			blend_transformed_argb(ctx->surface, state->op, &data, span_buffer);
		else if(fabsf(matrix->b) > 1e-6f || fabsf(matrix->c) > 1e-6f)
			blend_transformed_bilinear_tiled_argb(ctx->surface, state->op, &data, span_buffer);
		else
			blend_transformed_tiled_argb(ctx->surface, state->op, &data, span_buffer);
	}
}

static void blend_surface_with_mask(struct cg_surface_t * dst, struct cg_surface_t * src, struct cg_surface_t * mask, struct cg_matrix_t * m, float opacity, enum cg_operator_t op, struct cg_span_buffer_t * spans)
{
	composition_function_t func = composition_table[op];
	uint32_t buffer[1024];
	int mask_width = mask->width;
	int mask_height = mask->height;
	int mask_stride = mask->stride;
	int opacity255 = (int)(opacity * 255.0f + 0.5f);
	int count = spans->spans.size;
	struct cg_span_t * s = spans->spans.data;
	int fdx = (int)(m->a * FIXED_SCALE);
	int fdy = (int)(m->b * FIXED_SCALE);

	while(count--)
	{
		int x = s->x;
		int y = s->y;
		int length = s->len;
		uint32_t * dst_row = (uint32_t *)(dst->pixels + y * dst->stride);
		uint32_t * src_row = (uint32_t *)(src->pixels + y * src->stride);
		float cx = x + 0.5f;
		float cy = y + 0.5f;
		int fx = (int)((m->a * cx + m->c * cy + m->tx) * FIXED_SCALE);
		int fy = (int)((m->b * cx + m->d * cy + m->ty) * FIXED_SCALE);

		while(length)
		{
			int l = CG_MIN(length, 1024);
			for(int i = 0; i < l; i++)
			{
				int px = fx >> 16;
				int py = fy >> 16;
				uint32_t mask_alpha = 255;
				if((unsigned)px < (unsigned)mask_width && (unsigned)py < (unsigned)mask_height)
					mask_alpha = CG_ALPHA(((uint32_t *)(mask->pixels + py * mask_stride))[px]);
				uint32_t p = src_row[x + i];
				if(mask_alpha != 255)
					p = CG_BYTE_MUL(p, mask_alpha);
				if(opacity255 != 255)
					p = CG_BYTE_MUL(p, opacity255);
				buffer[i] = p;
				fx += fdx;
				fy += fdy;
			}
			func(dst_row + x, l, buffer, 255);
			x += l;
			length -= l;
		}
		++s;
	}
}

static void cg_blend(struct cg_ctx_t * ctx, struct cg_span_buffer_t * span_buffer)
{
	if(span_buffer->spans.size == 0)
		return;
	if(ctx->state->paint == NULL)
	{
		cg_blend_color(ctx, &ctx->state->color, span_buffer);
		return;
	}

	struct cg_paint_t * paint = ctx->state->paint;
	if(paint->type == CG_PAINT_TYPE_COLOR)
	{
		struct cg_solid_paint_t * solid = (struct cg_solid_paint_t*)(paint);
		cg_blend_color(ctx, &solid->color, span_buffer);
	}
	else if(paint->type == CG_PAINT_TYPE_GRADIENT)
	{
		struct cg_gradient_paint_t *gradient = (struct cg_gradient_paint_t*)(paint);
		cg_blend_gradient(ctx, gradient, span_buffer);
	}
	else
	{
		struct cg_texture_paint_t * texture = (struct cg_texture_paint_t*)(paint);
		cg_blend_texture(ctx, texture, span_buffer);
	}
}

static struct cg_state_t * cg_state_create(void)
{
	struct cg_state_t * state = malloc(sizeof(struct cg_state_t));
	state->paint = NULL;
	state->color = (struct cg_color_t){0.0f, 0.0f, 0.0f, 1.0f};
	state->matrix = (struct cg_matrix_t){1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f};
	state->stroke.style = (struct cg_stroke_style_t){ 1.0f, CG_LINE_CAP_BUTT, CG_LINE_JOIN_MITER, 10.0f };
	state->stroke.dash.offset = 0.0f;
	cg_array_init(state->stroke.dash.array);
	cg_span_buffer_init(&state->clip_spans);
	state->fill_rule = CG_FILL_RULE_NON_ZERO;
	state->op = CG_OPERATOR_SRC_OVER;
	state->opacity = 1.0f;
	state->clipping = 0;
	state->next = NULL;
	return state;
}

static void cg_state_reset(struct cg_state_t * state)
{
	cg_paint_destroy(state->paint);
	state->paint = NULL;
	state->color = (struct cg_color_t){0.0f, 0.0f, 0.0f, 1.0f};
	state->matrix = (struct cg_matrix_t){1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f};
	state->stroke.style = (struct cg_stroke_style_t){ 1.0f, CG_LINE_CAP_BUTT, CG_LINE_JOIN_MITER, 10.0f };
	state->stroke.dash.offset = 0.0f;
	cg_array_clear(state->stroke.dash.array);
	cg_span_buffer_reset(&state->clip_spans);
	state->fill_rule = CG_FILL_RULE_NON_ZERO;
	state->op = CG_OPERATOR_SRC_OVER;
	state->opacity = 1.0f;
	state->clipping = 0;
}

static void cg_state_copy(struct cg_state_t * state, struct cg_state_t * source)
{
	state->paint = cg_paint_reference(source->paint);
	state->color = source->color;
	state->matrix = source->matrix;
	state->stroke.style = source->stroke.style;
	state->stroke.dash.offset = source->stroke.dash.offset;
	cg_array_clear(state->stroke.dash.array);
	cg_array_append(state->stroke.dash.array, source->stroke.dash.array);
	cg_span_buffer_copy(&state->clip_spans, &source->clip_spans);
	state->fill_rule = source->fill_rule;
	state->op = source->op;
	state->opacity = source->opacity;
	state->clipping = source->clipping;
}

static void cg_state_destroy(struct cg_state_t * state)
{
	cg_paint_destroy(state->paint);
	cg_array_destroy(state->stroke.dash.array);
	cg_span_buffer_destroy(&state->clip_spans);
	free(state);
}

struct cg_paint_t * cg_get_source(struct cg_ctx_t * ctx, struct cg_color_t * color)
{
	if(color)
		*color = ctx->state->color;
	return ctx->state->paint;
}

struct cg_surface_t * cg_get_surface(struct cg_ctx_t * ctx)
{
	return ctx->surface;
}

struct cg_path_t * cg_get_path(struct cg_ctx_t * ctx)
{
	return ctx->path;
}

struct cg_matrix_t * cg_get_matrix(struct cg_ctx_t * ctx)
{
	return &ctx->state->matrix;
}

enum cg_operator_t cg_get_operator(struct cg_ctx_t * ctx)
{
	return ctx->state->op;
}

float cg_get_opacity(struct cg_ctx_t * ctx)
{
	return ctx->state->opacity;
}

void cg_get_current_point(struct cg_ctx_t * ctx, float * x, float * y)
{
	cg_path_get_current_point(ctx->path, x, y);
}

int cg_has_current_point(struct cg_ctx_t * ctx)
{
	return (ctx->path->num_points > 0) && !ctx->path->sub_path;
}

int cg_in_fill(struct cg_ctx_t * ctx, float x, float y)
{
	cg_rasterize(&ctx->fill_spans, ctx->path, &ctx->state->matrix, NULL, NULL, ctx->state->fill_rule);
	return cg_span_buffer_contains(&ctx->fill_spans, x, y);
}

int cg_in_stroke(struct cg_ctx_t * ctx, float x, float y)
{
	cg_rasterize(&ctx->fill_spans, ctx->path, &ctx->state->matrix, NULL, &ctx->state->stroke, CG_FILL_RULE_NON_ZERO);
	return cg_span_buffer_contains(&ctx->fill_spans, x, y);
}

int cg_in_clip(struct cg_ctx_t * ctx, float x, float y)
{
	if(ctx->state->clipping)
		return cg_span_buffer_contains(&ctx->state->clip_spans, x, y);
	float l = ctx->clip_rect.x;
	float t = ctx->clip_rect.y;
	float r = ctx->clip_rect.x + ctx->clip_rect.w;
	float b = ctx->clip_rect.y + ctx->clip_rect.h;
	return ((x >= l) && (x <= r) && (y >= t) && (y <= b));
}

void cg_fill_extents(struct cg_ctx_t * ctx, struct cg_rect_t * extents)
{
	cg_rasterize(&ctx->fill_spans, ctx->path, &ctx->state->matrix, NULL, NULL, ctx->state->fill_rule);
	cg_span_buffer_extents(&ctx->fill_spans, extents);
}

void cg_stroke_extents(struct cg_ctx_t * ctx, struct cg_rect_t * extents)
{
	cg_rasterize(&ctx->fill_spans, ctx->path, &ctx->state->matrix, NULL, &ctx->state->stroke, CG_FILL_RULE_NON_ZERO);
	cg_span_buffer_extents(&ctx->fill_spans, extents);
}

void cg_clip_extents(struct cg_ctx_t * ctx, struct cg_rect_t * extents)
{
	if(ctx->state->clipping)
		cg_span_buffer_extents(&ctx->state->clip_spans, extents);
	else
	{
		extents->x = ctx->clip_rect.x;
		extents->y = ctx->clip_rect.y;
		extents->w = ctx->clip_rect.w;
		extents->h = ctx->clip_rect.h;
	}
}

struct cg_ctx_t * cg_create(struct cg_surface_t * surface)
{
	struct cg_ctx_t * ctx = malloc(sizeof(struct cg_ctx_t));
	ctx->surface = cg_surface_reference(surface);
	ctx->path = cg_path_create();
	ctx->state = cg_state_create();
	ctx->freed_state = NULL;
	ctx->clip_rect = ((struct cg_rect_t){0, 0, surface->width, surface->height});
	cg_span_buffer_init(&ctx->clip_spans);
	cg_span_buffer_init(&ctx->fill_spans);
	return ctx;
}

void cg_destroy(struct cg_ctx_t * ctx)
{
	if(ctx)
	{
		while(ctx->state)
		{
			struct cg_state_t * state = ctx->state;
			ctx->state = state->next;
			cg_state_destroy(state);
		}
		while(ctx->freed_state)
		{
			struct cg_state_t *state = ctx->freed_state;
			ctx->freed_state = state->next;
			cg_state_destroy(state);
		}
		cg_span_buffer_destroy(&ctx->fill_spans);
		cg_span_buffer_destroy(&ctx->clip_spans);
		cg_surface_destroy(ctx->surface);
		cg_path_destroy(ctx->path);
		free(ctx);
	}
}

void cg_save(struct cg_ctx_t * ctx)
{
	struct cg_state_t * new_state = ctx->freed_state;
	if(new_state == NULL)
		new_state = cg_state_create();
	else
		ctx->freed_state = new_state->next;
	cg_state_copy(new_state, ctx->state);
	new_state->next = ctx->state;
	ctx->state = new_state;
}

void cg_restore(struct cg_ctx_t * ctx)
{
	if(ctx->state->next)
	{
		struct cg_state_t * old_state = ctx->state;
		ctx->state = old_state->next;
		cg_state_reset(old_state);
		old_state->next = ctx->freed_state;
		ctx->freed_state = old_state;
	}
}

void cg_set_source(struct cg_ctx_t * ctx, struct cg_paint_t * paint)
{
	paint = cg_paint_reference(paint);
	cg_paint_destroy(ctx->state->paint);
	ctx->state->paint = paint;
}

void cg_set_source_rgb(struct cg_ctx_t * ctx, float r, float g, float b)
{
	struct cg_color_t * c = &ctx->state->color;
	c->r = r;
	c->g = g;
	c->b = b;
	c->a = 1.0f;
	cg_set_source(ctx, NULL);
}

void cg_set_source_rgba(struct cg_ctx_t * ctx, float r, float g, float b, float a)
{
	struct cg_color_t * c = &ctx->state->color;
	c->r = r;
	c->g = g;
	c->b = b;
	c->a = a;
	cg_set_source(ctx, NULL);
}

void cg_set_source_surface(struct cg_ctx_t * ctx, struct cg_surface_t * surface, float x, float y)
{
	struct cg_matrix_t m;
	cg_matrix_init_translate(&m, x, y);
	struct cg_paint_t * paint = cg_paint_create_texture(surface, CG_TEXTURE_TYPE_PLAIN, 1.0f, &m);
	cg_set_source(ctx, paint);
	cg_paint_destroy(paint);
}

void cg_set_linear_gradient(struct cg_ctx_t * ctx, float x1, float y1, float x2, float y2, enum cg_spread_method_t spread, struct cg_gradient_stop_t * stops, int nstops, struct cg_matrix_t * m)
{
	struct cg_paint_t * paint = cg_paint_create_linear_gradient(x1, y1, x2, y2, spread, stops, nstops, m);
	cg_set_source(ctx, paint);
	cg_paint_destroy(paint);
}

void cg_set_radial_gradient(struct cg_ctx_t * ctx, float cx0, float cy0, float r0, float cx1, float cy1, float r1, enum cg_spread_method_t spread, struct cg_gradient_stop_t * stops, int nstops, struct cg_matrix_t * m)
{
	struct cg_paint_t * paint = cg_paint_create_radial_gradient(cx0, cy0, r0, cx1, cy1, r1, spread, stops, nstops, m);
	cg_set_source(ctx, paint);
	cg_paint_destroy(paint);
}

void cg_set_texture(struct cg_ctx_t * ctx, struct cg_surface_t * surface, enum cg_texture_type_t type, float opacity, struct cg_matrix_t * m)
{
	struct cg_paint_t * paint = cg_paint_create_texture(surface, type, opacity, m);
	cg_set_source(ctx, paint);
	cg_paint_destroy(paint);
}

void cg_set_operator(struct cg_ctx_t * ctx, enum cg_operator_t op)
{
	ctx->state->op = op;
}

void cg_set_opacity(struct cg_ctx_t * ctx, float opacity)
{
	ctx->state->opacity = CG_CLAMP(opacity, 0.0f, 1.0f);
}

void cg_set_fill_rule(struct cg_ctx_t * ctx, enum cg_fill_rule_t rule)
{
	ctx->state->fill_rule = rule;
}

void cg_set_line_width(struct cg_ctx_t * ctx, float width)
{
	ctx->state->stroke.style.width = width;
}

void cg_set_line_cap(struct cg_ctx_t * ctx, enum cg_line_cap_t cap)
{
	ctx->state->stroke.style.cap = cap;
}

void cg_set_line_join(struct cg_ctx_t * ctx, enum cg_line_join_t join)
{
	ctx->state->stroke.style.join = join;
}

void cg_set_miter_limit(struct cg_ctx_t * ctx, float limit)
{
	ctx->state->stroke.style.miter_limit = limit;
}

void cg_set_dash(struct cg_ctx_t * ctx, float * dashes, int ndashes, float offset)
{
	cg_array_clear(ctx->state->stroke.dash.array);
	cg_array_append_data(ctx->state->stroke.dash.array, dashes, ndashes);
	ctx->state->stroke.dash.offset = offset;
}

void cg_set_dash_array(struct cg_ctx_t * ctx, float * dashes, int ndashes)
{
	cg_array_clear(ctx->state->stroke.dash.array);
	cg_array_append_data(ctx->state->stroke.dash.array, dashes, ndashes);
}

void cg_set_dash_offset(struct cg_ctx_t * ctx, float offset)
{
	ctx->state->stroke.dash.offset = offset;
}

void cg_translate(struct cg_ctx_t * ctx, float tx, float ty)
{
	cg_matrix_translate(&ctx->state->matrix, tx, ty);
}

void cg_scale(struct cg_ctx_t * ctx, float sx, float sy)
{
	cg_matrix_scale(&ctx->state->matrix, sx, sy);
}

void cg_shear(struct cg_ctx_t * ctx, float shx, float shy)
{
	cg_matrix_shear(&ctx->state->matrix, shx, shy);
}

void cg_rotate(struct cg_ctx_t * ctx, float angle)
{
	cg_matrix_rotate(&ctx->state->matrix, angle);
}

void cg_transform(struct cg_ctx_t * ctx, struct cg_matrix_t * m)
{
	cg_matrix_multiply(&ctx->state->matrix, m, &ctx->state->matrix);
}

void cg_set_matrix(struct cg_ctx_t * ctx, struct cg_matrix_t * m)
{
	ctx->state->matrix = *m;
}

void cg_identity_matrix(struct cg_ctx_t * ctx)
{
	cg_matrix_init_identity(&ctx->state->matrix);
}

void cg_move_to(struct cg_ctx_t * ctx, float x, float y)
{
	cg_path_move_to(ctx->path, x, y);
}

void cg_line_to(struct cg_ctx_t * ctx, float x, float y)
{
	cg_path_line_to(ctx->path, x, y);
}

void cg_quad_to(struct cg_ctx_t * ctx, float x1, float y1, float x2, float y2)
{
	cg_path_quad_to(ctx->path, x1, y1, x2, y2);
}

void cg_cubic_to(struct cg_ctx_t * ctx, float x1, float y1, float x2, float y2, float x3, float y3)
{
	cg_path_cubic_to(ctx->path, x1, y1, x2, y2, x3, y3);
}

void cg_arc_to(struct cg_ctx_t * ctx, float rx, float ry, float angle, int large, int sweep, float x, float y)
{
	cg_path_arc_to(ctx->path, rx, ry, angle, large, sweep, x, y);
}

void cg_rel_move_to(struct cg_ctx_t * ctx, float dx, float dy)
{
	float x, y;
	cg_path_get_current_point(ctx->path, &x, &y);
	cg_path_move_to(ctx->path, dx + x, dy + y);
}

void cg_rel_line_to(struct cg_ctx_t * ctx, float dx, float dy)
{
	float x, y;
	cg_path_get_current_point(ctx->path, &x, &y);
	cg_path_line_to(ctx->path, dx + x, dy + y);
}

void cg_rel_quad_to(struct cg_ctx_t * ctx, float dx1, float dy1, float dx2, float dy2)
{
	float x, y;
	cg_path_get_current_point(ctx->path, &x, &y);
	cg_path_quad_to(ctx->path, dx1 + x, dy1 + y, dx2 + x, dy2 + y);
}

void cg_rel_cubic_to(struct cg_ctx_t * ctx, float dx1, float dy1, float dx2, float dy2, float dx3, float dy3)
{
	float x, y;
	cg_path_get_current_point(ctx->path, &x, &y);
	cg_path_cubic_to(ctx->path, dx1 + x, dy1 + y, dx2 + x, dy2 + y, dx3 + x, dy3 + y);
}

void cg_rel_arc_to(struct cg_ctx_t * ctx, float rx, float ry, float angle, int large, int sweep, float dx, float dy)
{
	float x, y;
	cg_path_get_current_point(ctx->path, &x, &y);
	cg_path_arc_to(ctx->path, rx, ry, angle, large, sweep, dx + x, dy + y);
}

void cg_rectangle(struct cg_ctx_t * ctx, float x, float y, float w, float h)
{
	cg_path_add_rectangle(ctx->path, x, y, w, h);
}

void cg_round_rectangle(struct cg_ctx_t * ctx, float x, float y, float w, float h, float rx, float ry)
{
	cg_path_add_round_rectangle(ctx->path, x, y, w, h, rx, ry);
}

void cg_ellipse(struct cg_ctx_t * ctx, float cx, float cy, float rx, float ry)
{
	cg_path_add_ellipse(ctx->path, cx, cy, rx, ry);
}

void cg_circle(struct cg_ctx_t * ctx, float cx, float cy, float r)
{
	cg_path_add_circle(ctx->path, cx, cy, r);
}

void cg_arc(struct cg_ctx_t * ctx, float cx, float cy, float r, float a0, float a1)
{
	cg_path_add_arc(ctx->path, cx, cy, r, a0, a1, 0);
}

void cg_arc_negative(struct cg_ctx_t * ctx, float cx, float cy, float r, float a0, float a1)
{
	cg_path_add_arc(ctx->path, cx, cy, r, a0, a1, 1);
}

void cg_new_path(struct cg_ctx_t * ctx)
{
	cg_path_reset(ctx->path);
}

void cg_new_sub_path(struct cg_ctx_t * ctx)
{
	cg_path_sub_path(ctx->path);
}

void cg_close_path(struct cg_ctx_t * ctx)
{
	cg_path_close(ctx->path);
}

void cg_add_path(struct cg_ctx_t * ctx, struct cg_path_t * path)
{
	cg_path_add_path(ctx->path, path, NULL);
}

void cg_mask(struct cg_ctx_t * ctx, struct cg_paint_t * paint)
{
	if(!paint || ctx->path->elements.size == 0)
		return;
	cg_rasterize(&ctx->fill_spans, ctx->path, &ctx->state->matrix, &ctx->clip_rect, NULL, ctx->state->fill_rule);
	if(ctx->fill_spans.spans.size == 0)
	{
		cg_new_path(ctx);
		return;
	}

	struct cg_surface_t * src_surface = cg_surface_create(ctx->surface->width, ctx->surface->height);
	struct cg_ctx_t * src_ctx = cg_create(src_surface);
	if(ctx->state->paint)
		cg_set_source(src_ctx, ctx->state->paint);
	else
		cg_set_source_rgba(src_ctx, ctx->state->color.r, ctx->state->color.g, ctx->state->color.b, ctx->state->color.a);
	cg_set_operator(src_ctx, CG_OPERATOR_SRC);
	cg_paint(src_ctx);
	cg_destroy(src_ctx);

	struct cg_surface_t * mask_surface = cg_surface_create(ctx->surface->width, ctx->surface->height);
	struct cg_ctx_t * mask_ctx = cg_create(mask_surface);
	cg_set_source(mask_ctx, paint);
	cg_set_matrix(mask_ctx, &ctx->state->matrix);
	cg_set_operator(mask_ctx, CG_OPERATOR_SRC);
	cg_paint(mask_ctx);
	cg_destroy(mask_ctx);

	struct cg_matrix_t id;
	cg_matrix_init_identity(&id);

	if(ctx->state->clipping)
	{
		cg_span_buffer_intersect(&ctx->clip_spans, &ctx->fill_spans, &ctx->state->clip_spans);
		blend_surface_with_mask(ctx->surface, src_surface, mask_surface, &id, ctx->state->opacity, ctx->state->op, &ctx->clip_spans);
	}
	else
	{
		blend_surface_with_mask(ctx->surface, src_surface, mask_surface, &id, ctx->state->opacity, ctx->state->op, &ctx->fill_spans);
	}

	cg_surface_destroy(mask_surface);
	cg_surface_destroy(src_surface);
	cg_new_path(ctx);
}

void cg_mask_surface(struct cg_ctx_t * ctx, struct cg_surface_t * mask, float x, float y)
{
	if(!mask)
		return;
	struct cg_matrix_t m;
	cg_matrix_init_translate(&m, x, y);
	struct cg_paint_t * paint = cg_paint_create_texture(mask, CG_TEXTURE_TYPE_PLAIN, 1.0f, &m);
	cg_mask(ctx, paint);
	cg_paint_destroy(paint);
}

void cg_reset_clip(struct cg_ctx_t * ctx)
{
	ctx->state->clipping = 0;
	cg_span_buffer_reset(&ctx->state->clip_spans);
}

void cg_clip(struct cg_ctx_t * ctx)
{
	cg_clip_preserve(ctx);
	cg_new_path(ctx);
}

void cg_clip_preserve(struct cg_ctx_t * ctx)
{
	if(ctx->state->clipping)
	{
		cg_rasterize(&ctx->fill_spans, ctx->path, &ctx->state->matrix, &ctx->clip_rect, NULL, ctx->state->fill_rule);
		cg_span_buffer_intersect(&ctx->clip_spans, &ctx->fill_spans, &ctx->state->clip_spans);
		cg_span_buffer_copy(&ctx->state->clip_spans, &ctx->clip_spans);
	}
	else
	{
		cg_rasterize(&ctx->state->clip_spans, ctx->path, &ctx->state->matrix, &ctx->clip_rect, NULL, ctx->state->fill_rule);
		ctx->state->clipping = 1;
	}
}

void cg_fill(struct cg_ctx_t * ctx)
{
	cg_fill_preserve(ctx);
	cg_new_path(ctx);
}

void cg_fill_preserve(struct cg_ctx_t * ctx)
{
	cg_rasterize(&ctx->fill_spans, ctx->path, &ctx->state->matrix, &ctx->clip_rect, NULL, ctx->state->fill_rule);
	if(ctx->state->clipping)
	{
		cg_span_buffer_intersect(&ctx->clip_spans, &ctx->fill_spans, &ctx->state->clip_spans);
		cg_blend(ctx, &ctx->clip_spans);
	}
	else
	{
		cg_blend(ctx, &ctx->fill_spans);
	}
}

void cg_stroke(struct cg_ctx_t * ctx)
{
	cg_stroke_preserve(ctx);
	cg_new_path(ctx);
}

void cg_stroke_preserve(struct cg_ctx_t * ctx)
{
	cg_rasterize(&ctx->fill_spans, ctx->path, &ctx->state->matrix, &ctx->clip_rect, &ctx->state->stroke, CG_FILL_RULE_NON_ZERO);
	if(ctx->state->clipping)
	{
		cg_span_buffer_intersect(&ctx->clip_spans, &ctx->fill_spans, &ctx->state->clip_spans);
		cg_blend(ctx, &ctx->clip_spans);
	}
	else
	{
		cg_blend(ctx, &ctx->fill_spans);
	}
}

void cg_paint(struct cg_ctx_t * ctx)
{
	if(ctx->state->clipping)
	{
		cg_blend(ctx, &ctx->state->clip_spans);
	}
	else
	{
		cg_span_buffer_init_rect(&ctx->clip_spans, 0, 0, ctx->surface->width, ctx->surface->height);
		cg_blend(ctx, &ctx->clip_spans);
	}
}
void cg_paint_with_alpha(struct cg_ctx_t * ctx, float alpha)
{
	cg_save(ctx);
	cg_set_opacity(ctx, alpha);
	cg_paint(ctx);
	cg_restore(ctx);
}
