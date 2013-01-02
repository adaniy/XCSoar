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

#include "InputEvents.hpp"
#include "InputConfig.hpp"
#include "Screen/Key.h"
#include "Asset.hpp"
#include "Util/Macros.hpp"

#include <assert.h>

#ifdef WIN32
/* we don't need that WIN32 API function, it's a macro that will
   disallow using InputConfig::AppendMenu() */
#undef AppendMenu
#endif

/**
 * For data generated by xci2cpp.pl.
 */
struct flat_event_map {
  unsigned char mode;

#if defined(ENABLE_SDL) && !defined(ANDROID)
#if defined(SDLK_SCANCODE_MASK) && SDLK_SCANCODE_MASK >= 0x10000
  /* need a bigger type for SDL 1.3+ */
  unsigned key;
#else
  unsigned short key;
#endif
#else
  unsigned char key;
#endif

  unsigned short event;
};

/**
 * For data generated by xci2cpp.pl.
 */
struct flat_label {
  unsigned char mode, location;
  unsigned short event;
  const TCHAR *label;
};

struct flat_gesture_map {
  unsigned char mode;
  unsigned short event;
  const TCHAR *data;
};

// Make a new label (add to the end each time)
// NOTE: String must already be copied (allows us to use literals
// without taking up more data - but when loading from file must copy string
static void
makeLabel(InputConfig &input_config,
          InputEvents::Mode mode_id, const TCHAR* label,
          unsigned location, unsigned event_id)
{
  input_config.AppendMenu(mode_id, label, location, event_id);
}

static void
apply_defaults(InputConfig &input_config,
               const TCHAR *const* default_modes,
               const InputConfig::Event *default_events,
               unsigned num_default_events,
               const flat_gesture_map *default_gesture2event,
               const flat_event_map *default_key2event,
               const flat_event_map *default_gc2event,
               const flat_event_map *default_n2event,
               const flat_label *default_labels)
{
  assert(num_default_events <= InputConfig::MAX_EVENTS);

  input_config.SetDefaults();
  while (*default_modes != NULL)
    input_config.AppendMode(*default_modes++);

  input_config.events.resize(num_default_events + 1);
  std::copy(default_events, default_events + num_default_events,
            input_config.events.begin() + 1);

  while (default_gesture2event->event > 0) {
    input_config.Gesture2Event.add(default_gesture2event->data,
                                   default_gesture2event->event);
    ++default_gesture2event;
  }
  
  while (default_key2event->event > 0) {
    input_config.Key2Event[default_key2event->mode][default_key2event->key] =
      default_key2event->event;
    ++default_key2event;
  }

  while (default_gc2event->event > 0) {
    input_config.GC2Event[default_gc2event->key] =
      default_gc2event->event;
    ++default_gc2event;
  }

  while (default_n2event->event > 0) {
    input_config.N2Event[default_n2event->key] =
      default_n2event->event;
    ++default_n2event;
  }

  while (default_labels->label != NULL) {
    makeLabel(input_config,
              (InputEvents::Mode)default_labels->mode,
              default_labels->label,
              default_labels->location, default_labels->event);
    ++default_labels;
  }
}

// Read the data files
void
InputEvents::LoadDefaults(InputConfig &input_config)
{
  // Get defaults
  input_config.SetDefaults();

  if (IsAltair()) {
#include "InputEvents_altair.cpp"
    apply_defaults(input_config,
                   default_modes,
                   default_events,
                   ARRAY_SIZE(default_events),
                   default_gesture2event,
                   default_key2event, default_gc2event, default_n2event,
                   default_labels);
  } else {
#include "InputEvents_default.cpp"
    apply_defaults(input_config,
                   default_modes,
                   default_events,
                   ARRAY_SIZE(default_events),
                   default_gesture2event,
                   default_key2event, default_gc2event, default_n2event,
                   default_labels);
  }
}
