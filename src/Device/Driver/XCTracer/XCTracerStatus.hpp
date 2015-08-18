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

#ifndef XCSOAR_XCTRACERVARIO_STATUS_HPP
#define XCSOAR_XCTRACERVARIO_STATUS_HPP

/*
 * separate namespace for these "global" data types and functions
 * status is available w/o knowing the instance or type of the driver class
 * similar to ioctl ...
 */
namespace XCTracerVario {

  /* status structure */
  struct XCTStatus {
    bool ok;           /* overall status: device ok */
    unsigned battery;  /* battery level: 0 .. 100 percent */
    bool battery_valid;
    unsigned mps;
    unsigned errors;   /* number of errors in sentences */

    const TCHAR *protocol; /* the protocol in use */
  };

  /* function to retrieve status */
  extern bool getStatus(struct XCTStatus &status);
}

#endif
