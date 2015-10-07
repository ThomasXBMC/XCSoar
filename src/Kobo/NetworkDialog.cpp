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

#include "Look/DialogLook.hpp"
#include "Screen/TextWindow.hpp"
#include "NetworkDialog.hpp"
#include "WifiDialog.hpp"
#include "Dialogs/WidgetDialog.hpp"
#include "UIGlobals.hpp"
#include "Screen/Key.h"
#include "Language/Language.hpp"
#include "Form/Form.hpp"
#include "Form/ActionListener.hpp"
#include "Widget/RowFormWidget.hpp"
#include "System.hpp"

gcc_pure
static const TCHAR *
GetWifiToggleCaption()
{
  return IsKoboWifiOn() ? _T("Wifi OFF") : _T("Wifi ON");
}

class NetworkWidget final
  : public RowFormWidget, ActionListener {
  enum Buttons {
    TOGGLE_WIFI,
    WIFI,
    TELNET,
    FTP,
    SHOW_IP,
  };

  Button *toggle_wifi_button, *wifi_button;

  TextWindow ip_window;
  PixelRect ip_rc;

public:
  NetworkWidget(const DialogLook &look):RowFormWidget(look) {}

  void UpdateButtons();

  /* virtual methods from class Widget */
  virtual void Prepare(ContainerWindow &parent,
                       const PixelRect &rc) override;

private:
  void ToggleWifi();
  void ShowIPAddress();
  bool GetMyIPAddress(char *addr,unsigned size);

  /* virtual methods from class ActionListener */
  virtual void OnAction(int id) override;
};

void
NetworkWidget::UpdateButtons()
{
  toggle_wifi_button->SetCaption(GetWifiToggleCaption());
  wifi_button->SetEnabled(IsKoboWifiOn());
}

void
NetworkWidget::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  toggle_wifi_button = AddButton(GetWifiToggleCaption(),
                                 *this, TOGGLE_WIFI);

  wifi_button = AddButton(_("Wifi"), *this, WIFI);

  AddButton(_T("Telnet server"), *this, TELNET);

  AddButton(_T("Ftp server"), *this, FTP);

  AddButton(_T("Show IP Address"), *this, SHOW_IP);

  const DialogLook &look = UIGlobals::GetDialogLook();

  /* create text window for IP address, set invisible */
  ip_rc.left = rc.right/2 - 60 ;
  ip_rc.top = rc.bottom/2 - 20;
  ip_rc.right = rc.right/2 + 40;
  ip_rc.bottom = ip_rc.top + 50;

  ip_window.Create(parent,nullptr,ip_rc);
#ifndef USE_GDI
  ip_window.SetFont(look.bold_font);
#endif
  //ip_window.MoveToCenter();
  ip_window.SetVisible(false);

  UpdateButtons();
}

void
NetworkWidget::ToggleWifi()
{
  if (!IsKoboWifiOn()) {
    KoboWifiOn();
  } else {
    KoboWifiOff();
  }

  UpdateButtons();
}

void
NetworkWidget::ShowIPAddress()
{
  char addr[64];
  char text[128];
  strcpy(text,"My IP Address: ");

  if (GetMyIPAddress(addr,sizeof(addr)))
    strcat(text,addr);
  else
    strcat(text,"<Unknown>");

  ip_window.set_text(text);
  ip_window.ShowOnTop();
}

void
NetworkWidget::OnAction(int id)
{
  switch (id) {
  case TOGGLE_WIFI:
    ToggleWifi();
    break;

  case WIFI:
    ShowWifiDialog();
    break;

  case TELNET:
    KoboRunTelnetd();
    break;

  case FTP:
    KoboRunFtpd();
    break;

  case SHOW_IP:
    ShowIPAddress();
    break;
  }
}

void
ShowNetworkDialog()
{
  const DialogLook &look = UIGlobals::GetDialogLook();
  NetworkWidget widget(look);
  WidgetDialog dialog(look);
  dialog.CreateFull(UIGlobals::GetMainWindow(), _("Network"), &widget);
  dialog.AddButton(_("Close"), mrOK);
  dialog.ShowModal();
  dialog.StealWidget();
}


/**
 * get the IP address of interface eth0
 */
bool
NetworkWidget::GetMyIPAddress(char *addr,unsigned size)
{
#if 0

#include "OS/FileUtil.hpp"
#include "OS/Process.hpp"

  const char *path_script = "/tmp/xcsoar.script.XXX";
  const char *path_result = "/tmp/xcsoar.result.XXX";

  char *pos, *end;
  char result[128];
  char script[128];

  /* build the script string */
  // ifconfig | sed -n '/inet addr/s/.*inet addr:\([0-9.]*\).*/\1/p'
  strcpy(script,"ifconfig | grep 'inet addr:' > ");
  strcat(script,path_result);

  /* delete the files, write the script file */
  File::Delete(path_result);
  File::Delete(path_script);
  if(!File::CreateExclusive(path_script) ||
      !File::WriteExisting(path_script,script))
    return false;

  /* run the script and read the result */
  if (!Run("/bin/sh",path_script))
    return false;
  if (!File::ReadString(path_result,result,sizeof(result)))
    return false;

  /* scan result and copy to addr */
  if (!(pos = strstr(result,"inet addr:")))
    return false;
  pos += 10;
  if (!(end = strchr(pos,' ')) || (unsigned(end-pos) >= size))
      return false;
  *end = 0;

  strcpy(addr,pos);

  return true;
#else
  FILE *pipe;

  /* call ifconfig and filter output with sed */
  if (!(pipe = popen("ifconfig | sed -n 's/.*inet addr:\\([0-9.]*\\).*/\\1/p'","r")))
    return false;

  /* get line from pipe, remove trailing newline, found if not local ip */
  while (fgets(addr, size, pipe)) {
    addr[strcspn(addr, "\n")] = 0;
    if (strcmp(addr,"127.0.0.1")) {
      pclose(pipe);
      return true;
    }
  }
  pclose(pipe);
  return false;
#endif
}
