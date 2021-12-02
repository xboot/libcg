#ifndef __SW_FT_MATH_H__
#define __SW_FT_MATH_H__

#include <sw_ft_types.h>

typedef SW_FT_Fixed			SW_FT_Angle;
#define SW_FT_MIN(a, b)		((a) < (b) ? (a) : (b))
#define SW_FT_MAX(a, b)		((a) > (b) ? (a) : (b))
#define SW_FT_ABS(a)		((a) < 0 ? -(a) : (a))
#define SW_FT_HYPOT(x, y)	(x = SW_FT_ABS(x), y = SW_FT_ABS(y), x > y ? x + (3 * y >> 3) : y + (3 * x >> 3))
#define SW_FT_ANGLE_PI		(180L << 16)
#define SW_FT_ANGLE_2PI		(SW_FT_ANGLE_PI * 2)
#define SW_FT_ANGLE_PI2		(SW_FT_ANGLE_PI / 2)
#define SW_FT_ANGLE_PI4		(SW_FT_ANGLE_PI / 4)

SW_FT_Long SW_FT_MulFix(SW_FT_Long a, SW_FT_Long b);
SW_FT_Long SW_FT_MulDiv(SW_FT_Long a, SW_FT_Long b, SW_FT_Long c);
SW_FT_Long SW_FT_DivFix(SW_FT_Long a, SW_FT_Long b);
SW_FT_Fixed SW_FT_Sin(SW_FT_Angle angle);
SW_FT_Fixed SW_FT_Cos(SW_FT_Angle angle);
SW_FT_Fixed SW_FT_Tan(SW_FT_Angle angle);
SW_FT_Angle SW_FT_Atan2(SW_FT_Fixed x, SW_FT_Fixed y);
SW_FT_Angle SW_FT_Angle_Diff(SW_FT_Angle angle1, SW_FT_Angle angle2);
void SW_FT_Vector_Unit(SW_FT_Vector * vec, SW_FT_Angle angle);
void SW_FT_Vector_Rotate(SW_FT_Vector * vec, SW_FT_Angle angle);
SW_FT_Fixed SW_FT_Vector_Length(SW_FT_Vector * vec);
void SW_FT_Vector_Polarize(SW_FT_Vector * vec, SW_FT_Fixed * length, SW_FT_Angle * angle);
void SW_FT_Vector_From_Polar(SW_FT_Vector * vec, SW_FT_Fixed length, SW_FT_Angle angle);

#endif /* __SW_FT_MATH_H__ */
