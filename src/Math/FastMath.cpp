/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2015 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "FastMath.hpp"
#include "Util.hpp"
#include "MathTables.h"

#include <algorithm>

#include <assert.h>
#include <stdlib.h>

int
compare_squared(int a, int b, int c)
{
  int a2b2 = a * a + b * b;
  int c2 = c * c;
  if (a2b2 > c2)
    return 1;
  if (a2b2 < c2)
    return -1;
  return 0;
}

/**
 * Calculates the square root of val
 *
 * See http://www.azillionmonkeys.com/qed/sqroot.html
 * @param val Value
 * @return Rounded square root of val
 */
unsigned
isqrt4(unsigned val)
{
#if defined(__i386__) || defined(__x86_64__)
  /* x86 FPUs are extremely fast */
  return (unsigned)sqrt((double)val);
#else
  unsigned int temp, g = 0;

  if (val >= 0x40000000) {
    g = 0x8000;
    val -= 0x40000000;
  }

  #define INNER_MBGSQRT(s)                    \
  temp = (g << (s)) + (1 << ((s) * 2 - 2));   \
  if (val >= temp) {                          \
    g += 1 << ((s)-1);                        \
    val -= temp;                              \
  }

  INNER_MBGSQRT (15)
  INNER_MBGSQRT (14)
  INNER_MBGSQRT (13)
  INNER_MBGSQRT (12)
  INNER_MBGSQRT (11)
  INNER_MBGSQRT (10)
  INNER_MBGSQRT ( 9)
  INNER_MBGSQRT ( 8)
  INNER_MBGSQRT ( 7)
  INNER_MBGSQRT ( 6)
  INNER_MBGSQRT ( 5)
  INNER_MBGSQRT ( 4)
  INNER_MBGSQRT ( 3)
  INNER_MBGSQRT ( 2)

  #undef INNER_MBGSQRT

  temp = g + g + 1;
  if (val >= temp)
    g++;

  return g;
#endif
}

double
thermal_recency_fn(unsigned x)
{
  return x < THERMALRECENCY_SIZE
    ? THERMALRECENCY[x]
    : 0.;
}

unsigned
ShiftedIntegerHypot(int _x, int _y, unsigned bits)
{
  const unsigned x = abs(_x), y = abs(_y);
  const unsigned a = std::min(x, y), b = std::max(x, y);
  if (a == 0)
    /* guard against division by zero */
    return b << bits;

  /* avoid integer overflow in (b/a)^2 by moving part of the bit shift
     out of the square; this can happen if one parameter is very small
     (e.g. 1) and the other is very large */
  const unsigned q = b / a;
  unsigned remaining_bits = 0;
  while (bits > 0 && q > unsigned(0x8000 >> bits)) {
    --bits;
    ++remaining_bits;
  }

  /* this is the classic hypotenuse formula, with the smaller
     parameter moved out of the square root call; this avoids squaring
     the raw parameters, and therefore reduces the risk of overflowing
     32 bit integers */
  return (a * isqrt4((1u << (bits << 1u)) + Square<unsigned>((b << bits) / a)))
    << remaining_bits;
}
