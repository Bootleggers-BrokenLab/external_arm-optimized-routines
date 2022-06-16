/*
 * Single-precision scalar atan2(x) function.
 *
 * Copyright (c) 2021-2022, Arm Limited.
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include <stdbool.h>

#include "math_config.h"
#include "atanf_common.h"

#define Pi (0x1.921fb6p+1f)
#define PiOver2 (0x1.921fb6p+0f)
#define PiOver4 (0x1.921fb6p-1f)
#define SignMask (0x80000000)

static inline int32_t
biased_exponent (float f)
{
  uint32_t fi = asuint (f);
  int32_t ex = (int32_t) ((fi & 0x7f800000) >> 23);
  if (unlikely (ex == 0))
    {
      /* Subnormal case - we still need to get the exponent right for subnormal
	 numbers as division may take us back inside the normal range.  */
      return ex - __builtin_clz (fi << 9);
    }
  return ex;
}

/* Fast implementation of scalar atan2f. Largest observed error is
   2.88ulps in [99.0, 101.0] x [99.0, 101.0]:
   atan2f(0x1.9332d8p+6, 0x1.8cb6c4p+6) got 0x1.964646p-1
				       want 0x1.964640p-1.  */
float
atan2f (float y, float x)
{
  uint32_t ix = asuint (x);
  uint32_t iy = asuint (y);

  uint32_t sign_x = ix & SignMask;
  uint32_t sign_y = iy & SignMask;

  uint32_t iax = ix & ~SignMask;
  uint32_t iay = iy & ~SignMask;

  /* x or y is NaN.  */
  if ((iax > 0x7f800000) || (iay > 0x7f800000))
    {
      if (unlikely ((iax > 0x7f800000) && (iay > 0x7f800000)))
	{
	  /* Both are NaN. Force sign to be +ve.  */
	  return (asfloat (iax) + asfloat (iay));
	}
      return x + y;
    }

  /* m = 2 * sign(x) + sign(y).  */
  uint32_t m = ((iy >> 31) & 1) | ((ix >> 30) & 2);

  /* The following follows glibc ieee754 implementation, except
     that we do not use +-tiny shifts (non-nearest rounding mode).  */

  int32_t exp_diff = biased_exponent (x) - biased_exponent (y);

  /* Special case for (x, y) either on or very close to the x axis. Either y =
     0, or y is tiny and x is huge (difference in exponents >= 126). In the
     second case, we only want to use this special case when x is negative (i.e.
     quadrants 2 or 3).  */
  if (unlikely (iay == 0 || (exp_diff >= 126 && m >= 2)))
    {
      switch (m)
	{
	case 0:
	case 1:
	  return y; /* atan(+-0,+anything)=+-0.  */
	case 2:
	  return Pi; /* atan(+0,-anything) = pi.  */
	case 3:
	  return -Pi; /* atan(-0,-anything) =-pi.  */
	}
    }
  /* Special case for (x, y) either on or very close to the y axis. Either x =
     0, or x is tiny and y is huge (difference in exponents >= 126).  */
  if (unlikely (iax == 0 || exp_diff <= -126))
    return sign_y ? -PiOver2 : PiOver2;

  /* x is INF.  */
  if (iax == 0x7f800000)
    {
      if (iay == 0x7f800000)
	{
	  switch (m)
	    {
	    case 0:
	      return PiOver4; /* atan(+INF,+INF).  */
	    case 1:
	      return -PiOver4; /* atan(-INF,+INF).  */
	    case 2:
	      return 3.0f * PiOver4; /* atan(+INF,-INF).  */
	    case 3:
	      return -3.0f * PiOver4; /* atan(-INF,-INF).  */
	    }
	}
      else
	{
	  switch (m)
	    {
	    case 0:
	      return 0.0f; /* atan(+...,+INF).  */
	    case 1:
	      return -0.0f; /* atan(-...,+INF).  */
	    case 2:
	      return Pi; /* atan(+...,-INF).  */
	    case 3:
	      return -Pi; /* atan(-...,-INF).  */
	    }
	}
    }
  /* y is INF.  */
  if (iay == 0x7f800000)
    return sign_y ? -PiOver2 : PiOver2;

  uint32_t sign_xy = sign_x ^ sign_y;

  float ax = asfloat (iax);
  float ay = asfloat (iay);

  bool pred_aygtax = (ay > ax);

  /* Set up z for call to atanf.  */
  float n = pred_aygtax ? -ax : ay;
  float d = pred_aygtax ? ay : ax;
  float z = n / d;

  /* Work out the correct shift.  */
  float shift = sign_x ? -2.0f : 0.0f;
  shift = pred_aygtax ? shift + 1.0f : shift;
  shift *= PiOver2;

  float ret = eval_poly (z, z, shift);

  /* Account for the sign of x and y.  */
  return asfloat (asuint (ret) ^ sign_xy);
}