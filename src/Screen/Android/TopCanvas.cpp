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

#include "Screen/Custom/TopCanvas.hpp"
#include "Screen/OpenGL/Init.hpp"
#include "Screen/OpenGL/Globals.hpp"
#include "Android/Main.hpp"
#include "Android/NativeView.hpp"

void
TopCanvas::Create(PixelSize new_size, bool full_screen, bool resizable)
{
#ifdef USE_EGL
  display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  surface = eglGetCurrentSurface(EGL_DRAW);
#endif

  OpenGL::SetupContext();
  OpenGL::SetupViewport(Point2D<unsigned>(new_size.cx, new_size.cy));
  Canvas::Create(new_size);
}

void
TopCanvas::OnResize(PixelSize new_size)
{
  if (new_size == GetSize())
    return;

  OpenGL::SetupViewport(Point2D<unsigned>(new_size.cx, new_size.cy));
  Canvas::Create(new_size);
}

void
TopCanvas::Flip()
{
#ifdef USE_EGL
  eglSwapBuffers(display, surface);
#else
  native_view->swap();
#endif
}
