/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009

	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@gmail.com>
	Lars H <lars_hn@hotmail.com>
	Rob Dunning <rob@raspberryridgesheepfarm.com>
	Russell King <rmk@arm.linux.org.uk>
	Paolo Ventafridda <coolwind@email.it>
	Tobias Lohner <tobias@lohner-net.de>
	Mirek Jezek <mjezek@ipplc.cz>
	Max Kellermann <max@duempel.org>
	Tobias Bieniek <tobias.bieniek@gmx.de>

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

#include "Dialogs/Internal.hpp"
#include "Screen/Layout.hpp"
#include "Protection.hpp"
#include "Blackboard.hpp"
#include "SettingsTask.hpp"
#include "Logger.hpp"
#include "Math/FastMath.h"
#include "MainWindow.hpp"
#include "LocalPath.hpp"
#include "StringUtil.hpp"

#include "Dialogs/dlgTaskHelpers.hpp"

#include "Task/TaskPoints/StartPoint.hpp"
#include "Task/TaskPoints/FinishPoint.hpp"
#include "Task/Visitors/TaskVisitor.hpp"

#include "RenderTask.hpp"
#include "RenderTaskPoint.hpp"
#include "RenderObservationZone.hpp"
#include "Screen/Chart.hpp"
#include "DataField/Enum.hpp"
#include "ChartProjection.hpp"
#include "Task/Visitors/ObservationZoneVisitor.hpp"

#include <assert.h>

static SingleWindow *parent_window;
static WndForm *wf=NULL;
static WndFrame* wTaskView= NULL;
static OrderedTask* ordered_task= NULL;
static bool task_modified = false;
static unsigned active_index = 0;

static void OnCloseClicked(WindowControl * Sender)
{
  (void)Sender;
  wf->SetModalResult(mrCancel);
}


class LabelObservationZone:
  public ObservationZoneConstVisitor
{
public:
  LabelObservationZone(TCHAR* buff): text(buff) {
    text[0]= 0;
  }

  void Visit(const FAISectorZone& oz) 
    {
      hide_all();
      WndFrame* wp = ((WndFrame *)wf->FindByName(_T("frmOZFAISector")));
      if (wp) {
        wp->show();
      }
      _stprintf(text, _T("FAI Sector"));
    }

  void Visit(const SectorZone& oz) 
    {
      hide_all();
      WndFrame* wp = ((WndFrame *)wf->FindByName(_T("frmOZSector")));
      if (wp) {
        wp->show();
      }
      WndProperty* wv;
      wv = ((WndProperty*)wf->FindByName(_T("prpOZSectorRadius")));
      if (wv) {
        wv->GetDataField()->SetAsFloat(Units::ToUserDistance(oz.getRadius()));
        wv->GetDataField()->SetUnits(Units::GetDistanceName());
        wv->RefreshDisplay();
      }
      wv = ((WndProperty*)wf->FindByName(_T("prpOZSectorStartRadial")));
      if (wv) {
        wv->GetDataField()->SetAsFloat(Units::ToUserDistance(oz.getStartRadial()));
        wv->RefreshDisplay();
      }
      wv = ((WndProperty*)wf->FindByName(_T("prpOZSectorFinishRadial")));
      if (wv) {
        wv->GetDataField()->SetAsFloat(Units::ToUserDistance(oz.getEndRadial()));
        wv->RefreshDisplay();
      }
      _stprintf(text, _T("Sector"));
    }

  void Visit(const LineSectorZone& oz) 
    {
      hide_all();
      WndFrame* wp = ((WndFrame *)wf->FindByName(_T("frmOZLine")));
      if (wp) {
        wp->show();
      }
      WndProperty* wv;
      wv = ((WndProperty*)wf->FindByName(_T("prpOZLineLength")));
      if (wv) {
        wv->GetDataField()->SetAsFloat(Units::ToUserDistance(oz.getLength()));
        wv->GetDataField()->SetUnits(Units::GetDistanceName());
        wv->RefreshDisplay();
      }
      _stprintf(text, _T("Line"));
    }

  void Visit(const CylinderZone& oz) 
    {
      hide_all();
      WndFrame* wp = ((WndFrame *)wf->FindByName(_T("frmOZCylinder")));
      if (wp) {
        wp->show();
      }
      WndProperty* wv;
      wv = ((WndProperty*)wf->FindByName(_T("prpOZCylinderRadius")));
      if (wv) {
        wv->GetDataField()->SetAsFloat(Units::ToUserDistance(oz.getRadius()));
        wv->GetDataField()->SetUnits(Units::GetDistanceName());
        wv->RefreshDisplay();
      }
      _stprintf(text, _T("Cylinder"));
    }
private:
  void hide_all() {
    WndFrame* wp;
    wp = ((WndFrame *)wf->FindByName(_T("frmOZFAISector")));
    if (wp) {
      wp->hide();
    }
    wp = ((WndFrame *)wf->FindByName(_T("frmOZLine")));
    if (wp) {
      wp->hide();
    }
    wp = ((WndFrame *)wf->FindByName(_T("frmOZSector")));
    if (wp) {
      wp->hide();
    }
    wp = ((WndFrame *)wf->FindByName(_T("frmOZCylinder")));
    if (wp) {
      wp->hide();
    }
  }
  TCHAR *text;
};

class TPLabelTaskPoint:
  public TaskPointConstVisitor
{
public:
  TPLabelTaskPoint(TCHAR* buff):
    text(buff)
  {
    text[0] = NULL;
  }
  void Visit(const UnorderedTaskPoint& tp) {
  }
  void Visit(const StartPoint& tp) {    
    _stprintf(text, _T("Start point: %s"), tp.get_waypoint().Name.c_str());
  }
  void Visit(const FinishPoint& tp) {
    _stprintf(text, _T("Finish point: %s"), tp.get_waypoint().Name.c_str());
  }
  void Visit(const AATPoint& tp) {
    _stprintf(text, _T("Assigned area point: %s"), tp.get_waypoint().Name.c_str());
  }
  void Visit(const ASTPoint& tp) {
    _stprintf(text, _T("Task point: %s"), tp.get_waypoint().Name.c_str());
  }
  TCHAR* text;
};


static void
RefreshView()
{
  wTaskView->invalidate();

  TCHAR buf[100];
  LabelObservationZone ozv(buf);
  OrderedTaskPoint* tp = ordered_task->get_tp(active_index);

  if (tp) {
    tp->CAccept_oz(ozv);
  }

  TPLabelTaskPoint tpv(buf);
  tp->CAccept(tpv);
  wf->SetCaption(tpv.text);
}

static void
OnTaskPaint(WindowControl *Sender, Canvas &canvas)
{
  RECT rc = Sender->get_client_rect();

  Chart chart(canvas, rc);

  OrderedTaskPoint* tp = ordered_task->get_tp(active_index);

  if (!tp) {
    chart.DrawNoData();
    return;
  }

  BufferCanvas buffer;
  BufferCanvas stencil;

  buffer.set(canvas);
  stencil.set(canvas);

  ChartProjection proj(rc, *tp, tp->get_location());

  MapDrawHelper helper(canvas, buffer, stencil, proj, rc,
                       XCSoarInterface::SettingsMap());
  RenderObservationZone ozv(helper);
  RenderTaskPoint tpv(helper, ozv, false, XCSoarInterface::Basic().Location);
  ::RenderTask dv(tpv);
  ordered_task->CAccept(dv); 
}


static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnCloseClicked),
  DeclareCallBackEntry(OnTaskPaint),
  DeclareCallBackEntry(NULL)
};

bool
dlgTaskPointShowModal(SingleWindow &parent, OrderedTask** task, const unsigned index)
{
  ordered_task = *task;
  parent_window = &parent;
  task_modified = false;
  active_index = index;

  wf = NULL;

  if (!Layout::landscape) {
    wf = dlgLoadFromXML(CallBackTable,
                        _T("dlgTaskPoint_L.xml"),
                        parent,
                        _T("IDR_XML_TASKPOINT_L"));
  } else {
    wf = dlgLoadFromXML(CallBackTable,
                        _T("dlgTaskPoint.xml"),
                        parent,
                        _T("IDR_XML_TASKPOINT"));
  }

  wTaskView = (WndFrame*)wf->FindByName(_T("frmTaskView"));
  assert(wTaskView!=NULL);

  RefreshView();

  if (!wf) return false;
  assert(wf!=NULL);
  wf->ShowModal();
  delete wf;
  wf = NULL;

  if (*task != ordered_task) {
    *task = ordered_task;
    return true;
  } else {
    return task_modified;
  }
}
