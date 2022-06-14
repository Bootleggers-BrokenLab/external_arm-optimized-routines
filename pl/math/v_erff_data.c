/*
 * Data for approximation of vector erff.
 *
 * Copyright (c) 2019-2022, Arm Limited.
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include "math_config.h"

/* Minimax approximation of erff.  */
const struct v_erff_data __v_erff_data
  = {.coeffs = {{0x0p0f, 0x1.079d0cp-3f},
		{0x1.06eba6p-03f, 0x1.450aa0p-1},
		{-0x1.8126e0p-02f, 0x1.b55cb0p-4f},
		{0x1.ce1a46p-04f, -0x1.8d6300p-6f},
		{-0x1.b68bd2p-06f, 0x1.fd1336p-9f},
		{0x1.473f48p-08f, -0x1.91d2ccp-12f},
		{-0x1.3a1a82p-11f, 0x1.222900p-16f}}};
