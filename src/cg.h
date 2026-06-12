#ifndef __CG_H__
#define __CG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xft.h>

struct cg_point_t {
	float x;
	float y;
};

struct cg_rect_t {
	float x;
	float y;
	float w;
	float h;
};

struct cg_matrix_t {
	float a; float b;
	float c; float d;
	float tx; float ty;
};

struct cg_color_t {
	float r;
	float g;
	float b;
	float a;
};

struct cg_gradient_stop_t {
	float offset;
	struct cg_color_t color;
};

enum cg_path_element_t {
	CG_PATH_ELEMENT_MOVE_TO		= 0,
	CG_PATH_ELEMENT_LINE_TO		= 1,
	CG_PATH_ELEMENT_CURVE_TO	= 2,
	CG_PATH_ELEMENT_CLOSE		= 3,
};

enum cg_spread_method_t {
	CG_SPREAD_METHOD_PAD		= 0,
	CG_SPREAD_METHOD_REFLECT	= 1,
	CG_SPREAD_METHOD_REPEAT		= 2,
};

enum cg_gradient_type_t {
	CG_GRADIENT_TYPE_LINEAR		= 0,
	CG_GRADIENT_TYPE_RADIAL		= 1,
};

enum cg_texture_type_t {
	CG_TEXTURE_TYPE_PLAIN		= 0,
	CG_TEXTURE_TYPE_TILED		= 1,
};

enum cg_line_cap_t {
	CG_LINE_CAP_BUTT			= 0,
	CG_LINE_CAP_ROUND			= 1,
	CG_LINE_CAP_SQUARE			= 2,
};

enum cg_line_join_t {
	CG_LINE_JOIN_MITER			= 0,
	CG_LINE_JOIN_ROUND			= 1,
	CG_LINE_JOIN_BEVEL			= 2,
};

enum cg_fill_rule_t {
	CG_FILL_RULE_NON_ZERO		= 0,
	CG_FILL_RULE_EVEN_ODD		= 1,
};

enum cg_paint_type_t {
	CG_PAINT_TYPE_COLOR			= 0,
	CG_PAINT_TYPE_GRADIENT		= 1,
	CG_PAINT_TYPE_TEXTURE		= 2,
};

enum cg_operator_t {
	CG_OPERATOR_CLEAR			= 0,
	CG_OPERATOR_SRC				= 1,
	CG_OPERATOR_DST				= 2,
	CG_OPERATOR_SRC_OVER		= 3,
	CG_OPERATOR_DST_OVER		= 4,
	CG_OPERATOR_SRC_IN			= 5,
	CG_OPERATOR_DST_IN			= 6,
	CG_OPERATOR_SRC_OUT			= 7,
	CG_OPERATOR_DST_OUT			= 8,
	CG_OPERATOR_SRC_ATOP		= 9,
	CG_OPERATOR_DST_ATOP		= 10,
	CG_OPERATOR_XOR				= 11,
};

struct cg_surface_t {
	int ref;
	int width;
	int height;
	int stride;
	int owndata;
	void * pixels;
};

struct cg_path_t {
	int contours;
	struct cg_point_t start;
	struct {
		enum cg_path_element_t * data;
		int size;
		int capacity;
	} elements;
	struct {
		struct cg_point_t * data;
		int size;
		int capacity;
	} points;
};

struct cg_gradient_t {
	enum cg_gradient_type_t type;
	enum cg_spread_method_t spread;
	struct cg_matrix_t matrix;
	float values[6];
	float opacity;
	struct {
		struct cg_gradient_stop_t * data;
		int size;
		int capacity;
	} stops;
};

struct cg_texture_t {
	enum cg_texture_type_t type;
	struct cg_surface_t * surface;
	struct cg_matrix_t matrix;
	float opacity;
};

struct cg_paint_t {
	enum cg_paint_type_t type;
	struct cg_color_t color;
	struct cg_gradient_t gradient;
	struct cg_texture_t texture;
};

struct cg_span_t {
	int x;
	int len;
	int y;
	unsigned char coverage;
};

struct cg_spanbuf_t {
	struct {
		struct cg_span_t * data;
		int size;
		int capacity;
	} spans;
	int x;
	int y;
	int w;
	int h;
};

struct cg_dash_t {
	float offset;
	float * data;
	int size;
};

struct cg_stroke_data_t {
	float width;
	float miterlimit;
	enum cg_line_cap_t cap;
	enum cg_line_join_t join;
	struct cg_dash_t * dash;
};

struct cg_state_t {
	struct cg_spanbuf_t * clip_spans;
	struct cg_paint_t paint;
	struct cg_matrix_t matrix;
	enum cg_fill_rule_t winding;
	struct cg_stroke_data_t stroke;
	enum cg_operator_t op;
	float opacity;
	struct cg_state_t * next;
};

struct cg_ctx_t {
	struct cg_surface_t * surface;
	struct cg_state_t * state;
	struct cg_path_t * path;
	struct cg_rect_t clip;
	struct cg_spanbuf_t * fill_spans;
	struct cg_spanbuf_t * clip_spans;
	void * outline_data;
	size_t outline_size;
};

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
#define CG_ALPHA(c)			(((c) >> 24) & 0xff)
#endif
#ifndef CG_DIV255
#define CG_DIV255(x)		(((x) + ((x) >> 8) + 0x80) >> 8)
#endif
#ifndef CG_BYTE_MUL
#define CG_BYTE_MUL(x, a)	((((((x) >> 8) & 0x00ff00ff) * (a)) & 0xff00ff00) + (((((x) & 0x00ff00ff) * (a)) >> 8) & 0x00ff00ff))
#endif

void cg_matrix_init(struct cg_matrix_t * m, float a, float b, float c, float d, float tx, float ty);
void cg_matrix_init_identity(struct cg_matrix_t * m);
void cg_matrix_init_translate(struct cg_matrix_t * m, float tx, float ty);
void cg_matrix_init_scale(struct cg_matrix_t * m, float sx, float sy);
void cg_matrix_init_rotate(struct cg_matrix_t * m, float r);
void cg_matrix_init_shear(struct cg_matrix_t * m, float shx, float shy);
void cg_matrix_translate(struct cg_matrix_t * m, float tx, float ty);
void cg_matrix_scale(struct cg_matrix_t * m, float sx, float sy);
void cg_matrix_rotate(struct cg_matrix_t * m, float r);
void cg_matrix_shear(struct cg_matrix_t * m, float shx, float shy);
void cg_matrix_multiply(struct cg_matrix_t * m, struct cg_matrix_t * m1, struct cg_matrix_t * m2);
int  cg_matrix_invert(struct cg_matrix_t * m);
void cg_matrix_map_point(struct cg_matrix_t * m, struct cg_point_t * p1, struct cg_point_t * p2);

struct cg_surface_t * cg_surface_create(int width, int height);
struct cg_surface_t * cg_surface_create_for_data(int width, int height, void * pixels);
void cg_surface_destroy(struct cg_surface_t * surface);
struct cg_surface_t * cg_surface_reference(struct cg_surface_t * surface);

void cg_gradient_set_values_linear(struct cg_gradient_t * gradient, float x1, float y1, float x2, float y2);
void cg_gradient_set_values_radial(struct cg_gradient_t * gradient, float cx, float cy, float cr, float fx, float fy, float fr);
void cg_gradient_set_spread(struct cg_gradient_t * gradient, enum cg_spread_method_t spread);
void cg_gradient_set_matrix(struct cg_gradient_t * gradient, struct cg_matrix_t * m);
void cg_gradient_set_opacity(struct cg_gradient_t * gradient, float opacity);
void cg_gradient_add_stop_rgb(struct cg_gradient_t * gradient, float offset, float r, float g, float b);
void cg_gradient_add_stop_rgba(struct cg_gradient_t * gradient, float offset, float r, float g, float b, float a);
void cg_gradient_add_stop_color(struct cg_gradient_t * gradient, float offset, struct cg_color_t * color);
void cg_gradient_add_stop(struct cg_gradient_t * gradient, struct cg_gradient_stop_t * stop);
void cg_gradient_clear_stops(struct cg_gradient_t * gradient);

void cg_texture_set_type(struct cg_texture_t * texture, enum cg_texture_type_t type);
void cg_texture_set_matrix(struct cg_texture_t * texture, struct cg_matrix_t * m);
void cg_texture_set_surface(struct cg_texture_t * texture, struct cg_surface_t * surface);
void cg_texture_set_opacity(struct cg_texture_t * texture, float opacity);

struct cg_ctx_t * cg_create(struct cg_surface_t * surface);
void cg_destroy(struct cg_ctx_t * ctx);
void cg_save(struct cg_ctx_t * ctx);
void cg_restore(struct cg_ctx_t * ctx);
void cg_get_matrix(struct cg_ctx_t * ctx, struct cg_matrix_t * m);
struct cg_color_t * cg_set_source_rgb(struct cg_ctx_t * ctx, float r, float g, float b);
struct cg_color_t * cg_set_source_rgba(struct cg_ctx_t * ctx, float r, float g, float b, float a);
struct cg_color_t * cg_set_source_color(struct cg_ctx_t * ctx, struct cg_color_t * color);
struct cg_gradient_t * cg_set_source_linear_gradient(struct cg_ctx_t * ctx, float x1, float y1, float x2, float y2);
struct cg_gradient_t * cg_set_source_radial_gradient(struct cg_ctx_t * ctx, float cx, float cy, float cr, float fx, float fy, float fr);
struct cg_texture_t * cg_set_source_surface(struct cg_ctx_t * ctx, struct cg_surface_t * surface, float x, float y);
void cg_set_operator(struct cg_ctx_t * ctx, enum cg_operator_t op);
void cg_set_opacity(struct cg_ctx_t * ctx, float opacity);
void cg_set_fill_rule(struct cg_ctx_t * ctx, enum cg_fill_rule_t winding);
void cg_set_line_width(struct cg_ctx_t * ctx, float width);
void cg_set_line_cap(struct cg_ctx_t * ctx, enum cg_line_cap_t cap);
void cg_set_line_join(struct cg_ctx_t * ctx, enum cg_line_join_t join);
void cg_set_miter_limit(struct cg_ctx_t * ctx, float limit);
void cg_set_dash(struct cg_ctx_t * ctx, float * dashes, int ndash, float offset);
void cg_translate(struct cg_ctx_t * ctx, float tx, float ty);
void cg_scale(struct cg_ctx_t * ctx, float sx, float sy);
void cg_rotate(struct cg_ctx_t * ctx, float r);
void cg_shear(struct cg_ctx_t * ctx, float shx, float shy);
void cg_transform(struct cg_ctx_t * ctx, struct cg_matrix_t * m);
void cg_set_matrix(struct cg_ctx_t * ctx, struct cg_matrix_t * m);
void cg_identity_matrix(struct cg_ctx_t * ctx);
void cg_move_to(struct cg_ctx_t * ctx, float x, float y);
void cg_line_to(struct cg_ctx_t * ctx, float x, float y);
void cg_curve_to(struct cg_ctx_t * ctx, float x1, float y1, float x2, float y2, float x3, float y3);
void cg_quad_to(struct cg_ctx_t * ctx, float x1, float y1, float x2, float y2);
void cg_rel_move_to(struct cg_ctx_t * ctx, float dx, float dy);
void cg_rel_line_to(struct cg_ctx_t * ctx, float dx, float dy);
void cg_rel_curve_to(struct cg_ctx_t * ctx, float dx1, float dy1, float dx2, float dy2, float dx3, float dy3);
void cg_rel_quad_to(struct cg_ctx_t * ctx, float dx1, float dy1, float dx2, float dy2);
void cg_rectangle(struct cg_ctx_t * ctx, float x, float y, float w, float h);
void cg_round_rectangle(struct cg_ctx_t * ctx, float x, float y, float w, float h, float rx, float ry);
void cg_ellipse(struct cg_ctx_t * ctx, float cx, float cy, float rx, float ry);
void cg_circle(struct cg_ctx_t * ctx, float cx, float cy, float r);
void cg_arc(struct cg_ctx_t * ctx, float cx, float cy, float r, float a0, float a1);
void cg_arc_negative(struct cg_ctx_t * ctx, float cx, float cy, float r, float a0, float a1);
void cg_new_path(struct cg_ctx_t * ctx);
void cg_close_path(struct cg_ctx_t * ctx);
void cg_reset_clip(struct cg_ctx_t * ctx);
void cg_clip(struct cg_ctx_t * ctx);
void cg_clip_preserve(struct cg_ctx_t * ctx);
void cg_fill(struct cg_ctx_t * ctx);
void cg_fill_preserve(struct cg_ctx_t * ctx);
void cg_stroke(struct cg_ctx_t * ctx);
void cg_stroke_preserve(struct cg_ctx_t * ctx);
void cg_paint(struct cg_ctx_t * ctx);

#ifdef __cplusplus
}
#endif

#endif /* __CG_H__ */
