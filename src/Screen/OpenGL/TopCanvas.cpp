/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2016 The XCSoar Project
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
#include "Globals.hpp"
#include "Init.hpp"
#include "Math/Point2D.hpp"

void
TopCanvas::Resume()
{
  OpenGL::SetupContext();
  OpenGL::SetupViewport(UnsignedPoint2D(size.cx, size.cy));
}

void
TopCanvas::OnResize(PixelSize new_size)
{
}

bool
TopCanvas::CheckResize(PixelSize new_native_size)
{
  if ((unsigned)new_native_size.cx == OpenGL::window_size.x &&
      (unsigned)new_native_size.cy == OpenGL::window_size.y)
    return false;

  OpenGL::SetupViewport(UnsignedPoint2D(new_native_size.cx,
                                        new_native_size.cy));
  Canvas::Create(new_native_size);
  return true;
}
