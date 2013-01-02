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


#ifndef WAYPOINTFILE_HPP
#define WAYPOINTFILE_HPP

#include <tchar.h>

struct Waypoint;
class Waypoints;
class RasterTerrain;
class TLineReader;
class OperationEnvironment;

class WaypointReaderBase 
{
protected:
  const int file_num;
  const RasterTerrain* terrain;
  bool compressed;

protected:
  WaypointReaderBase(const int _file_num,
               bool _compressed = false);

public:
  virtual ~WaypointReaderBase() {}

  /**
   * Parses a waypoint file into the given waypoint list
   * @param way_points The waypoint list to fill
   * @param terrain RasterTerrain (for automatic waypoint height)
   * @return True if the waypoint file parsing was okay, False otherwise
   */
  void Parse(Waypoints &way_points, TLineReader &reader,
             OperationEnvironment &operation);

  void SetTerrain(const RasterTerrain* _terrain) {
    terrain = _terrain;
  }

protected:
  static bool CheckAltitude(Waypoint &new_waypoint, const RasterTerrain *terrain);
  bool CheckAltitude(Waypoint &new_waypoint) const;

  /**
   * Parse a file line
   * @param line The line to parse
   * @param linenum The line number in the file
   * @param way_points The waypoint list to fill
   * @param terrain RasterTerrain (for automatic waypoint height)
   * @return True if the line was parsed correctly or ignored, False if
   * parsing error occured
   */
  virtual bool ParseLine(const TCHAR* line, unsigned linenum,
                         Waypoints &way_points) = 0;

public:
  // Helper functions

  /**
   * Split line (comma separated fields) in individual fields.
   * @param src The source line of comma separated fields
   * @param dst Destination buffer containing processed '\0' separated fields.
   * @param arr Array of pointers pointing to individual fields of dst
   * @param trim Optional flag to request space character removal at beginning
   * and end of fields.
   * @param quote_char Optional character used for quoting of individual fields.
   * Allows handling of quoted strings (e.g. fields containing leading or
   * trailing space or "," characters). Only considers the quote character
   * at the beginning or end of fields e.g. 6°10'22"E would be returned as is.
   * @return number of fields returned. Note: an empty src returns 1 for
   * for consistency (i.e. "" -> 1, "," -> 2)
   */
  static size_t ExtractParameters(const TCHAR *src, TCHAR *dst,
                                  const TCHAR **arr, size_t sz,
                                  const bool trim = false,
                                  const TCHAR quote_char = _T('\0'));
};

#endif
