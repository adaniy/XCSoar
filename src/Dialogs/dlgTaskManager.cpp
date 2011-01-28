/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2010 The XCSoar Project
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

#include "Dialogs/dlgTaskManager.hpp"
#include "Dialogs/Internal.hpp"
#include "Dialogs/dlgTaskHelpers.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Key.h"
#include "Components.hpp"
#include "Gauge/TaskView.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Screen/Fonts.hpp"
#include "TaskStore.hpp"
#include "LocalPath.hpp"
#include "OS/FileUtil.hpp"
#include "Logger/Logger.hpp"

#include "Form/TabBar.hpp"
#include "Form/Panel.hpp"
#include <assert.h>
#include <stdio.h>

static SingleWindow *parent_window;
static WndForm *wf = NULL;

static TabBarControl* wTabBar = NULL;
static OrderedTask* active_task = NULL;
static bool task_modified = false;

bool
dlgTaskManager::CommitTaskChanges()
{
  if (!task_modified)
    return true;

  if (!active_task->task_size() || active_task->check_task()) {
    active_task->check_duplicate_waypoints(way_points);
    protected_task_manager->task_commit(*active_task);
    protected_task_manager->task_save_default();

    task_modified = false;
    return true;
  }

  MessageBoxX(getTaskValidationErrors(
    active_task->get_factory().getValidationErrors()),
    _("Validation Errors"), MB_ICONEXCLAMATION);

  return (MessageBoxX(_("Task not valid. Changes will be lost.\nContinue?"),
                      _("Task Manager"), MB_YESNO | MB_ICONQUESTION) == IDYES);
}

bool
dlgTaskManager::OnClose()
{
  if (CommitTaskChanges()) {
    wf->SetModalResult(mrOK);
    return true;
  }

  return false;
}

void
dlgTaskManagerShowModal(SingleWindow &parent)
{
  dlgTaskManager::dlgTaskManagerShowModal(parent);
}

CallBackTableEntry dlgTaskManager::CallBackTable[] = {
  DeclareCallBackEntry(dlgTaskEdit::OnMoveUpClicked),
  DeclareCallBackEntry(dlgTaskEdit::OnMoveDownClicked),
  DeclareCallBackEntry(dlgTaskEdit::OnEditTurnpointClicked),
  DeclareCallBackEntry(dlgTaskEdit::OnNewClicked),
  DeclareCallBackEntry(dlgTaskEdit::OnTaskPaint),
  DeclareCallBackEntry(dlgTaskEdit::OnTabPreShow),

  DeclareCallBackEntry(dlgTaskProperties::OnTypeClicked),

  DeclareCallBackEntry(pnlTaskList::OnLoadSaveClicked),
  DeclareCallBackEntry(pnlTaskList::OnDeleteClicked),
  DeclareCallBackEntry(pnlTaskList::OnDeclareClicked),
  DeclareCallBackEntry(pnlTaskList::OnRenameClicked),
  DeclareCallBackEntry(pnlTaskList::OnTaskPaint),
  DeclareCallBackEntry(pnlTaskList::OnTabPreShow),

  DeclareCallBackEntry(dlgTaskManagerClose::OnCloseClicked),
  DeclareCallBackEntry(dlgTaskManagerClose::OnRevertClicked),

  DeclareCallBackEntry(NULL)
};

void
dlgTaskManager::RevertTask()
{
  // create new task first to guarantee pointers are different
  OrderedTask* temptask = protected_task_manager->task_clone();
  delete active_task;
  active_task = temptask;
  task_modified = false;
}

void
dlgTaskManager::dlgTaskManagerShowModal(SingleWindow &parent)
{
  if (protected_task_manager == NULL)
    return;

  parent_window = &parent;

  wf = LoadDialog(CallBackTable, parent,
                  Layout::landscape ?
                  _T("IDR_XML_TASKMANAGER_L") : _T("IDR_XML_TASKMANAGER"));

  assert(wf != NULL);

  active_task = protected_task_manager->task_clone();
  task_modified = false;

  // Load tabs
  wTabBar = (TabBarControl*)wf->FindByName(_T("TabBar"));
  assert(wTabBar != NULL);


  Window* wEdit =
    dlgTaskEdit::Load(parent, wTabBar, wf, &active_task, &task_modified);
  assert(wEdit);
  wTabBar->AddClient(wEdit, _T("Turn points"), false, NULL,
                     dlgTaskEdit::OnTabPreShow);


  Window* wProps =
    dlgTaskProperties::Load(parent, wTabBar, wf, &active_task, &task_modified);
  assert(wProps);
  wTabBar->AddClient(wProps, _T("Proper ties"), false,
                     dlgTaskProperties::OnTabPreHide,
                     dlgTaskProperties::OnTabPreShow);

  // ToDo: fix the label word wrap on PDAs to "Properties" wraps to two lines nicely

  Window* wLst =
    pnlTaskList::Load(parent, wTabBar, wf, &active_task, &task_modified);
  assert(wLst);
  wTabBar->AddClient(wLst, _T("Browse declare"), false, NULL,
                     pnlTaskList::OnTabPreShow);

  Window* wClose =
    dlgTaskManagerClose::Load(parent, wTabBar, wf, &active_task, &task_modified);
  assert(wClose);
  wTabBar->AddClient(wClose, _T("Close"), false, NULL,
                     dlgTaskManagerClose::OnTabPreShow);

  wTabBar->SetCurrentPage(0);

  wf->ShowModal();

  pnlTaskList::DestroyTab();

  delete wf;
  delete active_task;
}
