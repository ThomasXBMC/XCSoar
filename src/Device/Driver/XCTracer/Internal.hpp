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

#ifndef XCSOAR_XCTRACERVARIO_INTERNAL_HPP
#define XCSOAR_XCTRACERVARIO_INTERNAL_HPP

#include "Device/Driver.hpp"
#include "Device/Driver/XCTracer/XCTracerStatus.hpp"
#include "NMEA/Info.hpp"
#include "NMEA/InputLine.hpp"
#include "Time/BrokenDate.hpp"

#include <assert.h>

class XCTracerDevice : public AbstractDevice {

private:
  /**
   * time and date of last GPS fix
   * used to check whether date/time has advanced
   */
  fixed last_time;
  BrokenDate last_date;

  /**
   * time stamps
   * remember when we received the last sentence of a certain type
   * types are XTRCR,LXWP,GPS
   */
  unsigned last_XCTRC_sentence;
  unsigned last_LXWP0_sentence;
  unsigned last_GPS_sentence;

  /**
   * error counter and stats
   */
  unsigned nmea_errors;

  /**
   * battery level
   */
  unsigned battery;

  /**
   * last gps update time (second only)
   */
  int gps_last_second;

  /* the first instance of the device class - used for GetStatus only */
  static XCTracerDevice *the_device;

  /**
   * parser for the LXWP0 sentence
   */
  bool LXWP0(NMEAInputLine &line, NMEAInfo &info,const char *log_string);

  /**
   * parser for the XCTRC sentence
   */
  bool XCTRC(NMEAInputLine &line, NMEAInfo &info,const char *log_string);

public:
  XCTracerDevice();
  ~XCTracerDevice();

  /* non-object friend to allow access to device status */
  friend bool  XCTracerVario::GetStatus(struct XCTracerVario::Status &status);

  /**
   * virtual methods from class Device
   */
  void LinkTimeout() override;
  bool ParseNMEA(const char *line, struct NMEAInfo &info) override;
};

#endif
