/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2013 The XCSoar Project
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

#include "Form/TabMenu.hpp"
#include "Form/TabMenuDisplay.hpp"
#include "Screen/Key.h"
#include "Widget/WindowWidget.hpp"

#include <assert.h>

TabMenuControl::TabMenuControl(const DialogLook &look)
  :tab_display(new TabMenuDisplay(pager, look)),
   page_flipped_callback(nullptr)
{
  pager.Add(new WindowWidget(tab_display));

  pager.SetPageFlippedCallback([this](){
      tab_display->OnPageFlipped();

      if (page_flipped_callback != nullptr)
        page_flipped_callback();
    });
}

TabMenuControl::~TabMenuControl()
{
  delete tab_display;
}

bool
TabMenuControl::InvokeKeyPress(unsigned key_code)
{
  if (pager.KeyPress(key_code))
    return true;

  switch (key_code) {
  case KEY_LEFT:
#ifdef GNAV
  case '6':
#endif
    PreviousPage();
    return true;

  case KEY_RIGHT:
#ifdef GNAV
  case '7':
#endif
    NextPage();
    return true;

  default:
    return false;
  }
}

const TCHAR *
TabMenuControl::GetPageCaption(TCHAR buffer[], size_t size) const
{
  return tab_display->GetCaption(buffer, size);
}

void TabMenuControl::SetLastContentPage(unsigned page)
{
  tab_display->SetCursor(page);
}

unsigned
TabMenuControl::GetLastContentPage() const
{
  return tab_display->GetCursor();
}

void
TabMenuControl::InitMenu(const TabMenuPage pages_in[],
                         unsigned num_pages,
                         const TabMenuGroup groups[], unsigned n_groups)
{
  tab_display->InitMenu(pages_in, num_pages,
                        groups, n_groups);
}

void
TabMenuControl::OnCreate()
{
  ContainerWindow::OnCreate();

  const PixelRect rc = GetClientRect();

  WindowStyle display_style;
  display_style.Hide();
  display_style.TabStop();
  tab_display->Create(*this, rc, display_style);

  pager.Initialise(*this, rc);
  pager.Prepare(*this, rc);
  pager.Show(rc);
}

void
TabMenuControl::OnDestroy()
{
  pager.Hide();
  pager.Unprepare();
  pager.Clear();

  ContainerWindow::OnDestroy();
}
