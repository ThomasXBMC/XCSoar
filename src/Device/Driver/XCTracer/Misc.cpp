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

#include "../XCTracer/Internal.hpp"
#include "Device/Driver/XCTracer.hpp"
#include "Device/Driver/XCTracer/XCTracerStatus.hpp"
#include "OS/Clock.hpp"

/* static member definition */
XCTracerDevice *XCTracerDevice::the_device = 0;

XCTracerDevice::XCTracerDevice()
{
  /*
   * remember first (and only .. ) instance
   * used for status retrieval only
   */
  if (!the_device)
    the_device = this;

  battery = 0;

  gps_last_second = -1;
  last_time = fixed(0);
  last_date = BrokenDate::Invalid();

  last_LXWP0_sentence = 0;
  last_XCTRC_sentence = 0;
  last_GPS_sentence = 0;
  nmea_errors = 0;
}

XCTracerDevice::~XCTracerDevice()
{
  if (this == the_device)
    the_device = nullptr;
}

void
XCTracerDevice::LinkTimeout()
{
  /* nothing to do yet ... */
}

/**
 * return status values for the vario driver
 * @param status The status structure
 */
bool
XCTracerVario::GetStatus(struct XCTracerVario::Status &status)
{
  XCTracerDevice *device;
  unsigned current_time;

  // assert(InMainThread());

  /* are we instantiated ? */
  if (!(device = XCTracerDevice::the_device))
    return false;

  /* determine protocol in use */
  status.protocol = _T("N/A");
  status.ok = false;
  status.battery_valid = false;

  /*
   * check timestamps of sentences to determine protocol in use
   * and connection status
   * if timestamp is too old (comms error) then we don't show any protocol
   */
  current_time = MonotonicClockMS();
  if ((current_time - device->last_XCTRC_sentence) <= 3000 ) {
    status.protocol = _T("XCTRC");
    status.battery_valid = true;
    status.ok = true;
  }
  else if ((current_time - device->last_LXWP0_sentence) <= 3000 ) {
    status.protocol = _T("LXWP0");
    status.ok = true;
    if ((current_time - device->last_GPS_sentence) <= 5000 )
        status.protocol = _T("LX&GPS");
  }

  status.errors = device->nmea_errors;
  status.battery = device->battery;
  return true;
}
