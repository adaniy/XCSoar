/* Copyright_License {

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

#include "TaskAdvance.hpp"
#include "Task/Points/TaskPoint.hpp"
#include "Points/AATPoint.hpp"
#include "Points/IntermediatePoint.hpp"

void
TaskAdvance::Reset()
{
  armed = false;
  request_armed = false;
}

bool
TaskAdvance::IsStateReady(const TaskPoint &tp,
                          const AircraftState &state,
                          const bool x_enter,
                          const bool x_exit) const
{
  if (tp.GetType() == TaskPoint::START)
    return x_exit;

  if (tp.GetType() == TaskPoint::AAT) {
    const AATPoint *ap = (const AATPoint *)&tp;
    return IsAATStateReady(ap->HasEntered(), ap->IsCloseToTarget(state));
  } else if (tp.IsIntermediatePoint()) {
    const IntermediateTaskPoint *ip = (const IntermediateTaskPoint *)&tp;
    return ip->HasEntered();
  }
  return false;
}

bool
TaskAdvance::IsAATStateReady(const bool has_entered,
                             const bool close_to_target) const
{
  return has_entered;
}

void
TaskAdvance::SetArmed(const bool do_armed)
{
  armed = do_armed;
  request_armed = false;
  UpdateState();
}

bool
TaskAdvance::ToggleArmed()
{
  armed = !armed;
  if (armed)
    request_armed = false;

  UpdateState();
  return armed;
}
