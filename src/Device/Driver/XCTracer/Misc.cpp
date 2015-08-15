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
XCTracerDevice *XCTracerDevice::theDevice = 0;

XCTracerDevice::XCTracerDevice()
{
  /*
   * remember first (and only .. ) instance
   * used for status retrieval only
   */
  if (!theDevice)
    theDevice = this;
  battery = 0;
  mps = fixed(0);

  gps_last_second = -1 ;

  last_LXWP0_sentence = 0;
  last_XCTRC_sentence = 0;
  last_GPS_sentence = 0;
  nmea_errors = 0;
}

XCTracerDevice::~XCTracerDevice()
{
  theDevice = 0;
}

void
XCTracerDevice::LinkTimeout()
{
  /* nothing to do yet ... */
}

/*
 * return status values for the vario driver
 */
bool  XCTracerVario::getStatus(struct XCTracerVario::XCTStatus &status) {
  XCTracerDevice *device;

  /* are we instantiated ? */
  if (!(device = XCTracerDevice::theDevice))
    return false;

  /* determine protocol in use */
  status.protocol = _T("N/A");
  status.ok = false;
  status.battery_valid = false;

  /* check timestamps of sentences to determine protocol in use */
  if ((MonotonicClockMS() - device->last_XCTRC_sentence) <= 3000 ) {
    status.protocol = _T("XCTRC");
    status.battery_valid = true;
    status.ok = true;
  }
  else if ((MonotonicClockMS() - device->last_LXWP0_sentence) <= 3000 ) {
    status.protocol = _T("LXWP0");
    status.ok = true;
    if ((MonotonicClockMS() - device->last_GPS_sentence) <= 5000 )
        status.protocol = _T("LX&GPS");
  }

  status.mps = MonotonicClockMS() - device->last_XCTRC_sentence;
  status.errors = device->nmea_errors;
  status.battery = device->battery;
  return true;
}
