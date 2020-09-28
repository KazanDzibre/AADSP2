#pragma once
#include "stdfix_emu.h"
//broj kanala 
#include "stdfix_emu.h"

#define BLOCK_SIZE 16
#define MAX_NUM_CHANNEL 8

/* DSP type definitions */
typedef short DSPshort;					/* DSP integer */
typedef unsigned short DSPushort;		/* DSP unsigned integer */
typedef int DSPint;						/* native integer */
typedef fract DSPfract;
typedef long_accum DSPaccum;			/* DSP fixed-point fractional */

DSPfract M_PI = 6.28318530717958647692;// fract_num destroys numbers 
DSPfract fc;

