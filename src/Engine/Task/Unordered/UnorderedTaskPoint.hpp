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


#ifndef UNORDEREDTASKPOINT_HPP
#define UNORDEREDTASKPOINT_HPP

#include "Task/Points/TaskWaypoint.hpp"

/**
 * Class for unordered task points (e.g. goto and abort)
 *
 */
class UnorderedTaskPoint : 
  public TaskWaypoint
{
  fixed safety_height_arrival;

public:
  /**
   * Constructor.
   * 
   * @param wp Waypoint to be used as task point origin
   * @param tb Task Behaviour defining options (esp safety heights)
   */
  UnorderedTaskPoint(const Waypoint & wp,
                     const TaskBehaviour &tb);

  /* virtual methods from class TaskPoint */
  virtual void SetTaskBehaviour(const TaskBehaviour &tb);
  virtual GeoVector GetVectorRemaining(const GeoPoint &reference) const;
  virtual GeoVector GetVectorPlanned() const;
  virtual GeoVector GetVectorTravelled() const;
  virtual bool HasEntered() const {
    return false;
  }
  virtual const AircraftState &GetEnteredState() const;
  virtual fixed GetElevation() const;
};

#endif
