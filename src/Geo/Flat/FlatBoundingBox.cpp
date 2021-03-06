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

#include "FlatBoundingBox.hpp"
#include "FlatRay.hpp"
#include "Math/FastMath.hpp"

#include <algorithm>

unsigned
FlatBoundingBox::Distance(const FlatBoundingBox &f) const
{
  if (Overlaps(f))
    return 0;

  int dx = std::max(0, std::min(f.bb_ll.x - bb_ur.x,
                                bb_ll.x - f.bb_ur.x));
  int dy = std::max(0, std::min(f.bb_ll.y - bb_ur.y,
                                bb_ll.y - f.bb_ur.y));

  return ihypot(dx, dy);
}

bool
FlatBoundingBox::Intersects(const FlatRay& ray) const
{
  double tmin = 0, tmax = 1;

  // X
  if (ray.vector.x == 0) {
    // ray is parallel to slab. No hit if origin not within slab
    if (ray.point.x < bb_ll.x || ray.point.x > bb_ur.x)
      return false;
  } else {
    // compute intersection t value of ray with near/far plane of slab
    auto t1 = (bb_ll.x - ray.point.x) * ray.fx;
    auto t2 = (bb_ur.x - ray.point.x) * ray.fx;
    // make t1 be intersection with near plane, t2 with far plane
    if (t1 > t2)
      std::swap(t1, t2);

    tmin = std::max(tmin, t1);
    tmax = std::min(tmax, t2);
    // exit with no collision as soon as slab intersection becomes empty
    if (tmin > tmax)
      return false;
  }

  // Y
  if (ray.vector.y == 0) {
    // ray is parallel to slab. No hit if origin not within slab
    if (ray.point.y < bb_ll.y || ray.point.y > bb_ur.y)
      return false;
  } else {
    // compute intersection t value of ray with near/far plane of slab
    auto t1 = (bb_ll.y - ray.point.y) * ray.fy;
    auto t2 = (bb_ur.y - ray.point.y) * ray.fy;
    // make t1 be intersection with near plane, t2 with far plane
    if (t1 > t2)
      std::swap(t1, t2);

    tmin = std::max(tmin, t1);
    tmax = std::min(tmax, t2);
    // exit with no collision as soon as slab intersection becomes empty
    if (tmin > tmax)
      return false;
  }
  return true;
}

FlatGeoPoint
FlatBoundingBox::GetCenter() const
{
  /// @todo This will break if overlaps 360/0
  return FlatGeoPoint((bb_ll.x + bb_ur.x) / 2,
                      (bb_ll.y + bb_ur.y) / 2);
}

bool
FlatBoundingBox::Overlaps(const FlatBoundingBox& other) const
{
  if (bb_ll.x > other.bb_ur.x)
    return false;
  if (bb_ur.x < other.bb_ll.x)
    return false;
  if (bb_ll.y > other.bb_ur.y)
    return false;
  if (bb_ur.y < other.bb_ll.y)
    return false;

  return true;
}

bool
FlatBoundingBox::IsInside(const FlatGeoPoint& loc) const
{
  if (loc.x < bb_ll.x)
    return false;
  if (loc.x > bb_ur.x)
    return false;
  if (loc.y < bb_ll.y)
    return false;
  if (loc.y > bb_ur.y)
    return false;

  return true;
}
