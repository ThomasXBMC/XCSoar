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

#include "InfoBoxes/Content/Other.hpp"
#include "InfoBoxes/Data.hpp"
#include "Interface.hpp"
#include "Renderer/HorizonRenderer.hpp"
#include "Hardware/Battery.hpp"
#include "OS/SystemLoad.hpp"
#include "Language/Language.hpp"
#include "UIGlobals.hpp"
#include "Look/Look.hpp"

#include <tchar.h>

void
UpdateInfoBoxGLoad(InfoBoxData &data)
{
  if (!CommonInterface::Basic().acceleration.available) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValue(_T("%2.2f"), CommonInterface::Basic().acceleration.g_load);
}


/*
 * the "classic" battery info box content
 */
static void
UpdateInfoBoxBatteryClassic(InfoBoxData &data)
{
#ifdef HAVE_BATTERY
  bool DisplaySupplyVoltageAsValue=false;
  switch (Power::External::Status) {
    case Power::External::OFF:
      if (CommonInterface::Basic().battery_level_available)
        data.UnsafeFormatComment(_T("%s; %d%%"),
                                 _("AC Off"),
                                 (int)CommonInterface::Basic().battery_level);
      else
        data.SetComment(_("AC Off"));
      break;
    case Power::External::ON:
      if (!CommonInterface::Basic().voltage_available)
        data.SetComment(_("AC ON"));
      else{
        DisplaySupplyVoltageAsValue = true;
        data.SetValue(_T("%2.1fV"),
                          CommonInterface::Basic().voltage);
      }
      break;
    case Power::External::UNKNOWN:
    default:
      data.SetCommentInvalid();
  }
#ifndef ANDROID
  switch (Power::Battery::Status){
    case Power::Battery::HIGH:
    case Power::Battery::LOW:
    case Power::Battery::CRITICAL:
    case Power::Battery::CHARGING:
      if (Power::Battery::RemainingPercentValid){
#endif
        if (!DisplaySupplyVoltageAsValue)
          data.UnsafeFormatValue(_T("%d%%"), Power::Battery::RemainingPercent);
        else
          data.UnsafeFormatComment(_T("%d%%"), Power::Battery::RemainingPercent);
#ifndef ANDROID
      }
      else
        if (!DisplaySupplyVoltageAsValue)
          data.SetValueInvalid();
        else
          data.SetCommentInvalid();
      break;
    case Power::Battery::NOBATTERY:
    case Power::Battery::UNKNOWN:
      if (!DisplaySupplyVoltageAsValue)
        data.SetValueInvalid();
      else
        data.SetCommentInvalid();
  }
#endif
  return;

#endif

  if (CommonInterface::Basic().voltage_available) {
    data.SetValue(_T("%2.1fV"), CommonInterface::Basic().voltage);
    return;
  } else if (CommonInterface::Basic().battery_level_available) {
    data.SetValue(_T("%.0f%%"), CommonInterface::Basic().battery_level);
    return;
  }

  data.SetInvalid();
}

/*
 * #includes required by the new code
 * will of course be moved to head of file after review
 */
#include "OS/Clock.hpp"
#include "Components.hpp"
#include "Blackboard/DeviceBlackboard.hpp"

/*
 * this infobox displays the system (main) battery
 * as well as the battery_level or voltage of all connected devices
 * it cycles through the devices every few seconds
 * the comment of the InfoBox is set to the DeviceName
 */
void
UpdateInfoBoxBattery(InfoBoxData &data)
{
  /**
   * need to keep state between updates
   * keep it in the two static vars index and last_time
   * this works also well for multiple instances of
   * the InfoxBox -- they will all be running in sync
   */

  /**
   * index 0           => "classic" InfoBox (main battery)
   * index 1 .. NUMDEV => InfoBox displays battery level of devices
   */
  static unsigned index = 0;

  /**
   * timestamp of last switch
   * cycle through main battery and all devices every few seconds
   */
  static unsigned last_time = 0;

  /* current time */
  unsigned current_ms = MonotonicClockMS();

  /**
   * very first time (or restart after device lost)
   * set last_time and start with classic InfoBox
   * */
  if (!last_time) {
    last_time = current_ms;
    index = 0;
  }
  /* if time (7s) expired -- go to next device */
  else if (current_ms - last_time > 7000) {
    last_time = current_ms;

    /*
     * cycle through devices and find next one with battery available
     * InterfaceBlackboard CommonInterface::Basic()
     * doesn't provide the per_device data
     * Dev Man says:
     * "everybody else may use the DeviceBlackboard, but be
     * sure to lock it while using its data."
     */
    device_blackboard->mutex.Lock();

    while (++index <= NUMDEV) {
      if (device_blackboard->RealState(index-1).battery_level_available ||
          device_blackboard->RealState(index-1).voltage_available)
        break;
    }
    device_blackboard->mutex.Unlock();

    if (index > NUMDEV) {
      /* back to "classic" */
      index = 0;
    }
  }

  /* index 0 ==> show classic InfoBox */
  if (index == 0) {
    data.SetCommentInvalid();   /* clear device name */
    UpdateInfoBoxBatteryClassic(data);
    return;
  }

  /*
   * lock mutex, copy battery level and voltage, and unlock mutex asap
   * */
  double battery = -1;    /* -1 ==> not available */
  double voltage = -1;

  device_blackboard->mutex.Lock();
  const NMEAInfo &info = device_blackboard->RealState(index-1);
  if (info.battery_level_available)
    battery = info.battery_level;
  else if (info.voltage_available)
    voltage = info.voltage;
  device_blackboard->mutex.Unlock();

  /* finally we can display the values */
  if (battery != -1)
      data.SetValue(_T("%.0f%%"),battery);
  else if (voltage != -1)
    data.SetValue(_T("%2.1fV"),voltage);
  else {
    /*
     * we've lost a device ...
     * e.g. communication problem or reconfigured by the user
     * set data to invalid and start all over on next update
     */
    data.SetValueInvalid();
    last_time = 0;  /* force restart */
    return;
  }

  /*
   * set comment to driver name
   * we set it on every update, just in case the user has changed
   * the config via Device Manager between updates ...
   * get it via  GetSystemSettings()
   * we're in the main thread - I assume this is safe w/o any lock?
   * InMainThread() already asserted in GetSystemSettings()
   */
  const DeviceConfig &device = CommonInterface::GetSystemSettings().devices[index-1];
  if (device.UsesDriver())
    data.SetComment(device.driver_name);
  else
    data.SetComment(_T("---")); /* can this really happen ? */

  return;
}

void
UpdateInfoBoxExperimental1(InfoBoxData &data)
{
  // Set Value
  data.SetInvalid();
}

void
UpdateInfoBoxExperimental2(InfoBoxData &data)
{
  // Set Value
  data.SetInvalid();
}

void
UpdateInfoBoxCPULoad(InfoBoxData &data)
{
  unsigned percent_load = SystemLoadCPU();
  if (percent_load <= 100) {
    data.UnsafeFormatValue(_T("%d%%"), percent_load);
  } else {
    data.SetInvalid();
  }
}

void
UpdateInfoBoxFreeRAM(InfoBoxData &data)
{
  // used to be implemented on WinCE
  data.SetInvalid();
}

void
InfoBoxContentHorizon::OnCustomPaint(Canvas &canvas, const PixelRect &rc)
{
  if (CommonInterface::Basic().acceleration.available) {
    const Look &look = UIGlobals::GetLook();
    HorizonRenderer::Draw(canvas, rc,
                          look.horizon, CommonInterface::Basic().attitude);
  }
}

void
InfoBoxContentHorizon::Update(InfoBoxData &data)
{
  if (!CommonInterface::Basic().attitude.IsBankAngleUseable() &&
      !CommonInterface::Basic().attitude.IsPitchAngleUseable()) {
    data.SetInvalid();
    return;
  }

  data.SetCustom();
}
