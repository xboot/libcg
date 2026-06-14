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

enum cg_path_command_t {
	CG_PATH_COMMAND_MOVE_TO		= 0,
	CG_PATH_COMMAND_LINE_TO		= 1,
	CG_PATH_COMMAND_CUBIC_TO	= 2,
	CG_PATH_COMMAND_CLOSE		= 3,
};

union cg_path_element_t {
	struct {
		enum cg_path_command_t command;
		int length;
	} header;
	struct cg_point_t point;
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
	int refcnt;
	int width;
	int height;
	int stride;
	int owndata;
	unsigned char * pixels;
};

struct cg_path_t {
	int refcnt;
	int num_points;
	int num_contours;
	int num_curves;
	int sub_path;
	struct cg_point_t start_point;
	struct {
		union cg_path_element_t * data;
		int size;
		int capacity;
	} elements;
};

struct cg_paint_t {
	int refcnt;
	enum cg_paint_type_t type;
};

struct cg_solid_paint_t {
	struct cg_paint_t base;
	struct cg_color_t color;
};

struct cg_gradient_paint_t {
	struct cg_paint_t base;
	enum cg_gradient_type_t type;
	enum cg_spread_method_t spread;
	struct cg_matrix_t matrix;
	struct cg_gradient_stop_t * stops;
	int nstops;
	float values[6];
};

struct cg_texture_paint_t {
	struct cg_paint_t base;
	enum cg_texture_type_t type;
	float opacity;
	struct cg_matrix_t matrix;
	struct cg_surface_t * surface;
};

struct cg_span_t {
	int x;
	int len;
	int y;
	unsigned char coverage;
};

struct cg_span_buffer_t {
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

struct cg_stroke_dash_t {
	float offset;
	struct {
		float * data;
		int size;
		int capacity;
	} array;
};

struct cg_stroke_style_t {
	float width;
	enum cg_line_cap_t cap;
	enum cg_line_join_t join;
	float miter_limit;
};

struct cg_stroke_data_t {
	struct cg_stroke_style_t style;
	struct cg_stroke_dash_t dash;
};

struct cg_state_t {
	struct cg_paint_t * paint;
	struct cg_color_t color;
	struct cg_matrix_t matrix;
	struct cg_stroke_data_t stroke;
	struct cg_span_buffer_t clip_spans;
	enum cg_fill_rule_t fill_rule;
	enum cg_operator_t op;
	float opacity;
	int clipping;
	struct cg_state_t * next;
};

struct cg_ctx_t {
	struct cg_surface_t * surface;
	struct cg_path_t * path;
	struct cg_state_t * state;
	struct cg_state_t * freed_state;
	struct cg_rect_t clip_rect;
	struct cg_span_buffer_t clip_spans;
	struct cg_span_buffer_t fill_spans;
};

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

struct cg_surface_t * cg_surface_create(int width, int height);
struct cg_surface_t * cg_surface_create_for_data(int width, int height, void * pixels);
void cg_surface_destroy(struct cg_surface_t * surface);
struct cg_surface_t * cg_surface_reference(struct cg_surface_t * surface);

struct cg_paint_t * cg_paint_create_rgb(float r, float g, float b);
struct cg_paint_t * cg_paint_create_rgba(float r, float g, float b, float a);
struct cg_paint_t * cg_paint_create_color(struct cg_color_t * color);
struct cg_paint_t * cg_paint_create_linear_gradient(float x1, float y1, float x2, float y2, enum cg_spread_method_t spread, struct cg_gradient_stop_t * stops, int nstops, struct cg_matrix_t * m);
struct cg_paint_t * cg_paint_create_radial_gradient(float cx0, float cy0, float r0, float cx1, float cy1, float r1, enum cg_spread_method_t spread, struct cg_gradient_stop_t * stops, int nstops, struct cg_matrix_t * m);
struct cg_paint_t * cg_paint_create_texture(struct cg_surface_t * surface, enum cg_texture_type_t type, float opacity, struct cg_matrix_t * m);
void cg_paint_destroy(struct cg_paint_t * paint);
struct cg_paint_t * cg_paint_reference(struct cg_paint_t * paint);

struct cg_path_t * cg_path_create(void);
void cg_path_destroy(struct cg_path_t * path);
struct cg_path_t * cg_path_reference(struct cg_path_t * path);
void cg_path_reset(struct cg_path_t * path);
void cg_path_sub_path(struct cg_path_t * path);
void cg_path_close(struct cg_path_t * path);
void cg_path_transform(struct cg_path_t * path, struct cg_matrix_t * m);
void cg_path_get_current_point(struct cg_path_t * path, float * x, float * y);
void cg_path_move_to(struct cg_path_t * path, float x, float y);
void cg_path_line_to(struct cg_path_t * path, float x, float y);
void cg_path_quad_to(struct cg_path_t * path, float x1, float y1, float x2, float y2);
void cg_path_cubic_to(struct cg_path_t * path, float x1, float y1, float x2, float y2, float x3, float y3);
void cg_path_arc_to(struct cg_path_t * path, float rx, float ry, float angle, int large, int sweep, float x, float y);
void cg_path_add_rectangle(struct cg_path_t * path, float x, float y, float w, float h);
void cg_path_add_round_rectangle(struct cg_path_t * path, float x, float y, float w, float h, float rx, float ry);
void cg_path_add_ellipse(struct cg_path_t * path, float cx, float cy, float rx, float ry);
void cg_path_add_circle(struct cg_path_t * path, float cx, float cy, float r);
void cg_path_add_arc(struct cg_path_t * path, float cx, float cy, float r, float a0, float a1, int ccw);
void cg_path_add_path(struct cg_path_t * path, struct cg_path_t * source, struct cg_matrix_t * m);
float cg_path_extents(struct cg_path_t * path, struct cg_rect_t * extents, int tight);

struct cg_paint_t * cg_get_source(struct cg_ctx_t * ctx, struct cg_color_t * color);
struct cg_surface_t * cg_get_surface(struct cg_ctx_t * ctx);
struct cg_path_t * cg_get_path(struct cg_ctx_t * ctx);
struct cg_matrix_t * cg_get_matrix(struct cg_ctx_t * ctx);
enum cg_operator_t cg_get_operator(struct cg_ctx_t * ctx);
float cg_get_opacity(struct cg_ctx_t * ctx);
void cg_get_current_point(struct cg_ctx_t * ctx, float * x, float * y);
int cg_has_current_point(struct cg_ctx_t * ctx);
int cg_in_fill(struct cg_ctx_t * ctx, float x, float y);
int cg_in_stroke(struct cg_ctx_t * ctx, float x, float y);
int cg_in_clip(struct cg_ctx_t * ctx, float x, float y);
void cg_fill_extents(struct cg_ctx_t * ctx, struct cg_rect_t * extents);
void cg_stroke_extents(struct cg_ctx_t * ctx, struct cg_rect_t * extents);
void cg_clip_extents(struct cg_ctx_t * ctx, struct cg_rect_t * extents);

struct cg_ctx_t * cg_create(struct cg_surface_t * surface);
void cg_destroy(struct cg_ctx_t * ctx);
void cg_save(struct cg_ctx_t * ctx);
void cg_restore(struct cg_ctx_t * ctx);
void cg_set_source(struct cg_ctx_t * ctx, struct cg_paint_t * paint);
void cg_set_source_rgb(struct cg_ctx_t * ctx, float r, float g, float b);
void cg_set_source_rgba(struct cg_ctx_t * ctx, float r, float g, float b, float a);
void cg_set_source_surface(struct cg_ctx_t * ctx, struct cg_surface_t * surface, float x, float y);
void cg_set_linear_gradient(struct cg_ctx_t * ctx, float x1, float y1, float x2, float y2, enum cg_spread_method_t spread, struct cg_gradient_stop_t * stops, int nstops, struct cg_matrix_t * m);
void cg_set_radial_gradient(struct cg_ctx_t * ctx, float cx0, float cy0, float r0, float cx1, float cy1, float r1, enum cg_spread_method_t spread, struct cg_gradient_stop_t * stops, int nstops, struct cg_matrix_t * m);
void cg_set_texture(struct cg_ctx_t * ctx, struct cg_surface_t * surface, enum cg_texture_type_t type, float opacity, struct cg_matrix_t * m);
void cg_set_operator(struct cg_ctx_t * ctx, enum cg_operator_t op);
void cg_set_opacity(struct cg_ctx_t * ctx, float opacity);
void cg_set_fill_rule(struct cg_ctx_t * ctx, enum cg_fill_rule_t rule);
void cg_set_line_width(struct cg_ctx_t * ctx, float width);
void cg_set_line_cap(struct cg_ctx_t * ctx, enum cg_line_cap_t cap);
void cg_set_line_join(struct cg_ctx_t * ctx, enum cg_line_join_t join);
void cg_set_miter_limit(struct cg_ctx_t * ctx, float limit);
void cg_set_dash(struct cg_ctx_t * ctx, float * dashes, int ndashes, float offset);
void cg_set_dash_array(struct cg_ctx_t * ctx, float * dashes, int ndashes);
void cg_set_dash_offset(struct cg_ctx_t * ctx, float offset);
void cg_translate(struct cg_ctx_t * ctx, float tx, float ty);
void cg_scale(struct cg_ctx_t * ctx, float sx, float sy);
void cg_shear(struct cg_ctx_t * ctx, float shx, float shy);
void cg_rotate(struct cg_ctx_t * ctx, float angle);
void cg_transform(struct cg_ctx_t * ctx, struct cg_matrix_t * m);
void cg_set_matrix(struct cg_ctx_t * ctx, struct cg_matrix_t * m);
void cg_identity_matrix(struct cg_ctx_t * ctx);
void cg_move_to(struct cg_ctx_t * ctx, float x, float y);
void cg_line_to(struct cg_ctx_t * ctx, float x, float y);
void cg_quad_to(struct cg_ctx_t * ctx, float x1, float y1, float x2, float y2);
void cg_cubic_to(struct cg_ctx_t * ctx, float x1, float y1, float x2, float y2, float x3, float y3);
void cg_arc_to(struct cg_ctx_t * ctx, float rx, float ry, float angle, int large, int sweep, float x, float y);
void cg_rel_move_to(struct cg_ctx_t * ctx, float dx, float dy);
void cg_rel_line_to(struct cg_ctx_t * ctx, float dx, float dy);
void cg_rel_quad_to(struct cg_ctx_t * ctx, float dx1, float dy1, float dx2, float dy2);
void cg_rel_cubic_to(struct cg_ctx_t * ctx, float dx1, float dy1, float dx2, float dy2, float dx3, float dy3);
void cg_rel_arc_to(struct cg_ctx_t * ctx, float rx, float ry, float angle, int large, int sweep, float dx, float dy);
void cg_rectangle(struct cg_ctx_t * ctx, float x, float y, float w, float h);
void cg_round_rectangle(struct cg_ctx_t * ctx, float x, float y, float w, float h, float rx, float ry);
void cg_ellipse(struct cg_ctx_t * ctx, float cx, float cy, float rx, float ry);
void cg_circle(struct cg_ctx_t * ctx, float cx, float cy, float r);
void cg_arc(struct cg_ctx_t * ctx, float cx, float cy, float r, float a0, float a1);
void cg_arc_negative(struct cg_ctx_t * ctx, float cx, float cy, float r, float a0, float a1);
void cg_new_path(struct cg_ctx_t * ctx);
void cg_new_sub_path(struct cg_ctx_t * ctx);
void cg_close_path(struct cg_ctx_t * ctx);
void cg_add_path(struct cg_ctx_t * ctx, struct cg_path_t * path);
void cg_mask(struct cg_ctx_t * ctx, struct cg_paint_t * paint);
void cg_mask_surface(struct cg_ctx_t * ctx, struct cg_surface_t * mask, float x, float y);
void cg_reset_clip(struct cg_ctx_t * ctx);
void cg_clip(struct cg_ctx_t * ctx);
void cg_clip_preserve(struct cg_ctx_t * ctx);
void cg_fill(struct cg_ctx_t * ctx);
void cg_fill_preserve(struct cg_ctx_t * ctx);
void cg_stroke(struct cg_ctx_t * ctx);
void cg_stroke_preserve(struct cg_ctx_t * ctx);
void cg_paint(struct cg_ctx_t * ctx);
void cg_paint_with_alpha(struct cg_ctx_t * ctx, float alpha);

#ifdef __cplusplus
}
#endif

#endif /* __CG_H__ */
