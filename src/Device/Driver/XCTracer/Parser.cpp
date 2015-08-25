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
#include "Device/Parser.hpp"
#include "NMEA/Checksum.hpp"
#include "NMEA/InputLine.hpp"
#include "NMEA/Info.hpp"
#include "Geo/SpeedVector.hpp"
#include "Units/System.hpp"
#include "Atmosphere/Temperature.hpp"
#include "OS/Clock.hpp"
#include "Time/BrokenDate.hpp"
#include "Util/Macros.hpp"
#include "LogFile.hpp"

/**
 * Parser for the XCTracer Vario
 * For the XCTracer Protocol
 * @see https://www.xctracer.com/en/tech-specs/?oid=1861
 *
 * Native XTRC sentences
 * $XCTRC,2015,1,5,16,34,33,36,46.947508,7.453117,540.32,12.35,270.4,2.78,,,,964.93,98*67
 *
 * $XCTRC,year,month,day,hour,minute,second,centisecond,latitude,longitude,altitude,speedoverground,
 *      course,climbrate,res,res,res,rawpressure,batteryindication*checksum
 *
 * OR in LXWP0 mode with GPS sentences
 * $GPGGA,081158.400,4837.7021,N,00806.2928,E,2,5,1.57,113.9,M,47.9,M,,*5B
 * $LXWP0,N,,119.9,0.16,,,,,,259,,*64
 * $GPRMC,081158.800,A,4837.7018,N,00806.2923,E,2.34,261.89,110815,,,D*69
 */

/**
 * the parser for the LXWP0 subset sentence that the XC-Tracer can produce
 */
bool XCTracerDevice::LXWP0(NMEAInputLine &line, NMEAInfo &info,const char *log_string)
{
  /*
   *  $LXWP0,N,,119.9,0.16,,,,,,259,,*64
   *  0 logger stored (Y/N)
   *  1 n.u.
   *  2 baroaltitude (m)
   *  3 vario (m/s)
   *  4-8 n.u.
   *  9 heading of plane
   * 10 n.u
   * 11 n.u.
   */
  int valid_fields = 0;

  line.Skip(2);

  fixed value;
  /* read baroaltitude */
  if (line.ReadChecked(value)) {
    /* XC-Tracer sends uncorrected altitude above 1013.25hPa here */
    info.ProvidePressureAltitude(value);
    valid_fields++;
  }

  /* read vario */
  if (line.ReadChecked(value)) {
    info.ProvideTotalEnergyVario(value);
    valid_fields++;
  }

  line.Skip(5);

  /* read heading */
  if (line.ReadChecked(value)) {
    (void) Angle::Degrees(value); /* XXX */
    valid_fields++;
  }
  if (valid_fields >= 3) {
    last_LXWP0_sentence = MonotonicClockMS();
  }
  else {
    nmea_errors++;
    //LogFormat("XCTracer: Parser error, input %s",log_string);
  }

  return true;
}

/**
 * the parser for the XCTRC sentence
 */
bool XCTracerDevice::XCTRC(NMEAInputLine &line, NMEAInfo &info,const char *log_string)
{
  /*
   * $XCTRC,year,month,day,hour,minute,second,centisecond,latitude,longitude,
   *  altitude,speedoverground,course,climbrate,res,res,res,rawpressure,
   *  batteryindication*checksum
   * $XCTRC,2015,8,11,10,56,23,80,48.62825,8.104885,129.4,0.01,322.76,-0.05,,,,997.79,77*66
   */

  /* count the number of valid fields. If not as expected incr nmea error counter */
  int valid_fields = 0;

  /**
   * we only want to accept GPS fixes with completely sane values
   * for date, time and lat/long
   */
  bool date_valid = false;
  bool time_valid = false;

  /* parse the date */
  long year, month, day;     /* use signed type to catch invalid input */
  BrokenDate date;

  /* parse and absorb all three values, even if one or more is empty, e.g. ",,13" */
  bool year_valid = line.ReadChecked(year);
  bool month_valid = line.ReadChecked(month);
  bool day_valid = line.ReadChecked(day);

  if (year_valid && month_valid && day_valid) {
    date = BrokenDate(year,month,day);
    if (date.IsPlausible()) {
      date_valid = true;
      valid_fields += 3;
    }
  }

  /* parse time */
  long hour,minute,second,centisecond; /* use signed type to catch invalid input */
  fixed time = fixed(0);

  /* parse and check all four values, even if one or more is empty, e.g. ",,33," */
  bool hour_valid = line.ReadChecked(hour) && hour >= 0 && hour < 24;
  bool minute_valid = line.ReadChecked(minute) && minute >= 0 && minute < 60;
  bool second_valid = line.ReadChecked(second) && second >= 0 && second < 60;
  bool centisecond_valid = line.ReadChecked(centisecond) && centisecond >= 0;

  if (hour_valid && minute_valid && second_valid && centisecond_valid) {
    time = fixed(hour*60*60 + minute*60 + second);
    time_valid = true;
    valid_fields += 4;
  }

  /* parse the GPS fix */
  fixed latitude, longitude;

  /* parse and check both  values, even if one or more is empty, e.g. ",3.1234," */
  bool latitude_valid = line.ReadChecked(latitude) &&
      latitude >= fixed(-90.0) && latitude <= fixed(90.0);
  bool longitude_valid = line.ReadChecked(longitude) &&
      longitude >= fixed(-180.0) && longitude <= fixed(180.0);

  if (latitude_valid && longitude_valid) {
    GeoPoint point;
    point.latitude = Angle::Degrees(latitude);
    point.longitude = Angle::Degrees(longitude);

    valid_fields += 2;

    /**
     * only update GPS and date/time
     * - if all time/date fields have sane values
     * - if time has advanced
     * update only once per second
     * - for perfomance reasons
     * - all info time fields have a resolution of one second only anyway
     */

    if (date_valid && time_valid && (time > last_time || date > last_date)) {
      /* update last date/time */
      last_time = time;
      last_date = date;

      /* set time and date_time_utc, don't use ProvideTime() */
      info.time = time;
      info.time_available.Update(info.clock);
      info.date_time_utc.hour = hour;
      info.date_time_utc.minute = minute;
      info.date_time_utc.second = second;

      info.ProvideDate(date);
      info.location = point;
      info.location_available.Update(info.clock);
      info.gps.real = true;
    }
  }

  fixed value;
  /* read gps altitude */
  if (line.ReadChecked(value)) {
    info.gps_altitude = value;
    info.gps_altitude_available.Update(info.clock);
    valid_fields++;
  }

  /* read spead over ground */
  if (line.ReadChecked(value)) {
    info.ground_speed = value;
    info.ground_speed_available.Update(info.clock);
    valid_fields++;
  }

  /* read course*/
  if (line.ReadChecked(value)) {
    /* update only update if we're moving .. */
    if (info.MovementDetected()) {
      info.track = Angle::Degrees(value);
      info.track_available.Update(info.clock);
    }
    valid_fields++;
  }

  /* read climbrate */
  if (line.ReadChecked(value)) {
    info.ProvideTotalEnergyVario(value);
    valid_fields++;
  }

  /* skip 3 reserved values */
  line.Skip(3);

  /* read raw pressure */
  if (line.ReadChecked(value)) {
    // convert to pressure
    info.ProvideStaticPressure(AtmosphericPressure::HectoPascal(value));
    valid_fields++;
  }

  /* read battery level */
  if (line.ReadChecked(value)) {
    if ((value >= fixed(0)) && (value <= fixed(100) )) {
      info.battery_level = value;
      info.battery_level_available.Update(info.clock);
      battery = unsigned(value);
      valid_fields++;

#ifndef NDEBUG
    static int logn = 0;
    if (logn == 0) {
      //LogFormat("XCTracer: Battery level %u",unsigned(value));
    }
    if (logn++ > 5*60*10) /* every ten minutes */
      logn = 0;

#endif
    }
  }

  if (valid_fields >= 15) {
    last_XCTRC_sentence = MonotonicClockMS();
  }
  else {
    nmea_errors++;
    //LogFormat("XCTracer: Parser error, input %s",log_string);
  }

  return true;
}

/**
 * the NMEA parser virtual function
 */
bool
XCTracerDevice::ParseNMEA(const char *String, NMEAInfo &info)
{
  if (!VerifyNMEAChecksum(String))
    return false;

  NMEAInputLine line(String);
  char type[16];
  line.Read(type, 16);

  /*
   * GPS sentences are parsed by the default NMEA parser
   * but we want to check their presence ..
   */
  if (StringIsEqual(type, "$GPRMC") || StringIsEqual(type, "$GPGGA")) {
    last_GPS_sentence = MonotonicClockMS();
    return false;
  }

  if (StringIsEqual(type, "$LXWP0"))
    return LXWP0(line, info,String);

  if (StringIsEqual(type, "$XCTRC"))
    return XCTRC(line, info,String);

  return false;
}
