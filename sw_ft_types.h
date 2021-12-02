#ifndef __SW_FT_TYPES_H__
#define __SW_FT_TYPES_H__

#include <assert.h>
#include <limits.h>
#include <setjmp.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef signed long SW_FT_Fixed;
typedef signed int SW_FT_Int;
typedef unsigned int SW_FT_UInt;
typedef signed long SW_FT_Long;
typedef unsigned long SW_FT_ULong;
typedef signed short SW_FT_Short;
typedef unsigned char SW_FT_Byte;
typedef unsigned char SW_FT_Bool;
typedef int SW_FT_Error;
typedef signed long SW_FT_Pos;

typedef struct SW_FT_Vector_ {
	SW_FT_Pos x;
	SW_FT_Pos y;
} SW_FT_Vector;

typedef long long int SW_FT_Int64;
typedef unsigned long long int SW_FT_UInt64;
typedef signed int SW_FT_Int32;
typedef unsigned int SW_FT_UInt32;

#define SW_FT_BOOL(x)		((SW_FT_Bool)(x))
#define SW_FT_SIZEOF_LONG	4

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#endif /* __SW_FT_TYPES_H__ */
