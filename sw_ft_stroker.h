#ifndef __SW_FT_STROKER_H__
#define __SW_FT_STROKER_H__

#include <sw_ft_raster.h>

typedef struct SW_FT_StrokerRec_ * SW_FT_Stroker;

typedef enum SW_FT_Stroker_LineJoin_ {
	SW_FT_STROKER_LINEJOIN_ROUND = 0,
	SW_FT_STROKER_LINEJOIN_BEVEL = 1,
	SW_FT_STROKER_LINEJOIN_MITER_VARIABLE = 2,
	SW_FT_STROKER_LINEJOIN_MITER = SW_FT_STROKER_LINEJOIN_MITER_VARIABLE,
	SW_FT_STROKER_LINEJOIN_MITER_FIXED = 3,
} SW_FT_Stroker_LineJoin;

typedef enum SW_FT_Stroker_LineCap_ {
	SW_FT_STROKER_LINECAP_BUTT = 0,
	SW_FT_STROKER_LINECAP_ROUND = 1,
	SW_FT_STROKER_LINECAP_SQUARE = 2,
} SW_FT_Stroker_LineCap;

typedef enum SW_FT_StrokerBorder_ {
	SW_FT_STROKER_BORDER_LEFT = 0,
	SW_FT_STROKER_BORDER_RIGHT = 1,
} SW_FT_StrokerBorder;

SW_FT_Error SW_FT_Stroker_New(SW_FT_Stroker * astroker);
void SW_FT_Stroker_Set(SW_FT_Stroker stroker, SW_FT_Fixed radius, SW_FT_Stroker_LineCap line_cap, SW_FT_Stroker_LineJoin line_join, SW_FT_Fixed miter_limit);
SW_FT_Error SW_FT_Stroker_ParseOutline(SW_FT_Stroker stroker, const SW_FT_Outline * outline);
SW_FT_Error SW_FT_Stroker_GetCounts(SW_FT_Stroker stroker, SW_FT_UInt * anum_points, SW_FT_UInt * anum_contours);
void SW_FT_Stroker_Export(SW_FT_Stroker stroker, SW_FT_Outline *outline);
void SW_FT_Stroker_Done(SW_FT_Stroker stroker);

#endif /* __SW_FT_STROKER_H__ */
