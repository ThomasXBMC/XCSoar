/* Copyright_License {

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

#include "FlatLine.hpp"
#include "Math/Util.hpp"
#include "Math/Angle.hpp"

double
FlatLine::dsq() const
{
  return Square(dx()) + Square(dy());
}

Angle
FlatLine::angle() const
{
  return Angle::FromXY(dx(), dy());
}

void
FlatLine::rotate(const Angle theta)
{
  p1.Rotate(theta);
  p2.Rotate(theta);
}

bool
FlatLine::intersect_czero(const double r, FlatPoint &i1, FlatPoint &i2) const
{
  const auto _dx = dx();
  const auto _dy = dy();
  const auto dr = dsq();
  const auto D = cross();

  auto det = Square(r) * dr - Square(D);
  if (det < 0)
    // no solution
    return false;

  det = sqrt(det);
  const auto inv_dr = 1. / dr;
  i1.x = (D * _dy + copysign(_dx, _dy) * det) * inv_dr;
  i2.x = (D * _dy - copysign(_dx, _dy) * det) * inv_dr;
  i1.y = (-D * _dx + fabs(_dy) * det) * inv_dr;
  i2.y = (-D * _dx - fabs(_dy) * det) * inv_dr;
  return true;
}

bool
FlatLine::intersect_circle(const double r, const FlatPoint c,
                           FlatPoint &i1, FlatPoint &i2) const
{
  FlatLine that = *this;
  that.sub(c);
  if (that.intersect_czero(r, i1, i2)) {
    i1 = i1 + c;
    i2 = i2 + c;
    return true;
  }

  return false;
}

double
FlatLine::dot(const FlatLine& that) const
{
  return (p2 - p1).DotProduct(that.p2 - that.p1);
}
