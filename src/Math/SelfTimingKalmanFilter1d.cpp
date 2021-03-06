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

#include "SelfTimingKalmanFilter1d.hpp"
#include "OS/Clock.hpp"

SelfTimingKalmanFilter1d::SelfTimingKalmanFilter1d(const double max_dt,
                                                   const double var_x_accel)
    : filter_(var_x_accel), t_last_update_ms_(0) {
  SetMaxDt(max_dt);
}

SelfTimingKalmanFilter1d::SelfTimingKalmanFilter1d(const double max_dt)
    : t_last_update_ms_(0) {
  SetMaxDt(max_dt);
}

void
SelfTimingKalmanFilter1d::SetMaxDt(const double max_dt)
{
  // It's OK, albeit silly, to have a zero max_dt value. We just always reset.
  max_dt_ms_ = max_dt < 0 ? 0u : unsigned(max_dt * 1000);
}

double
SelfTimingKalmanFilter1d::GetMaxDt() const
{
  return max_dt_ms_ / 1000.;
}

void
SelfTimingKalmanFilter1d::Update(const double z_abs, const double var_z_abs)
{
  const unsigned int t_ms = MonotonicClockMS();
  const unsigned int dt_ms = t_ms - t_last_update_ms_;
  t_last_update_ms_ = t_ms;

  if (dt_ms > max_dt_ms_)
    filter_.Reset();
  filter_.Update(z_abs, var_z_abs, dt_ms / 1000.);
}
