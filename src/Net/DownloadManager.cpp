/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2012 The XCSoar Project
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

#include "Net/DownloadManager.hpp"

#ifdef ANDROID

#include "Android/DownloadManager.hpp"
#include "Android/Main.hpp"

static AndroidDownloadManager *download_manager;

bool
Net::DownloadManager::Initialise()
{
  assert(download_manager == NULL);

  if (!AndroidDownloadManager::Initialise(Java::GetEnv()))
    return false;

  download_manager = AndroidDownloadManager::Create(Java::GetEnv(), *context);
  return download_manager != NULL;
}

void
Net::DownloadManager::BeginDeinitialise()
{
}

void
Net::DownloadManager::Deinitialise()
{
  delete download_manager;
  download_manager = NULL;
}

bool
Net::DownloadManager::IsAvailable()
{
  return download_manager != NULL;
}

void
Net::DownloadManager::AddListener(DownloadListener &listener)
{
  if (download_manager != NULL)
    download_manager->AddListener(listener);
}

void
Net::DownloadManager::RemoveListener(DownloadListener &listener)
{
  if (download_manager != NULL)
    download_manager->RemoveListener(listener);
}

void
Net::DownloadManager::Enqueue(const char *uri, const TCHAR *relative_path)
{
  assert(download_manager != NULL);

  download_manager->Enqueue(Java::GetEnv(), uri, relative_path);
}

#else /* !ANDROID */

#include "ToFile.hpp"
#include "Session.hpp"
#include "Thread/StandbyThread.hpp"
#include "Util/tstring.hpp"
#include "Operation/Operation.hpp"
#include "LocalPath.hpp"
#include "OS/FileUtil.hpp"

#include <list>
#include <algorithm>

#include <windef.h> /* for MAX_PATH */

class DownloadManagerThread : protected StandbyThread,
                              private QuietOperationEnvironment {
  struct Item {
    std::string uri;
    tstring path_relative;

    Item(const char *_uri, const TCHAR *_path_relative)
      :uri(_uri), path_relative(_path_relative) {}
  };

  std::list<Item> queue;

  std::list<Net::DownloadListener *> listeners;

public:
  void StopAsync() {
    ScopeLock protect(mutex);
    StandbyThread::StopAsync();
  }

  void WaitStopped() {
    ScopeLock protect(mutex);
    StandbyThread::WaitStopped();
  }

  void AddListener(Net::DownloadListener &listener) {
    ScopeLock protect(mutex);

    assert(std::find(listeners.begin(), listeners.end(),
                     &listener) == listeners.end());

    listeners.push_back(&listener);
  }

  void RemoveListener(Net::DownloadListener &listener) {
    ScopeLock protect(mutex);

    auto i = std::find(listeners.begin(), listeners.end(), &listener);
    assert(i != listeners.end());
    listeners.erase(i);
  }

  void Enqueue(const char *uri, const TCHAR *path_relative) {
    ScopeLock protect(mutex);
    queue.push_back(Item(uri, path_relative));

    if (!IsBusy())
      Trigger();
  }

protected:
  /* methods from class StandbyThread */
  virtual void Tick();

private:
  /* methods from class OperationEnvironment */
  virtual bool IsCancelled() const {
    return StandbyThread::IsStopped();
  }
};

void
DownloadManagerThread::Tick()
{
  Net::Session session;

  while (!queue.empty() && !IsCancelled()) {
    Item item(std::move(queue.front()));
    queue.pop_front();
    mutex.Unlock();

    TCHAR path[MAX_PATH];
    LocalPath(path, item.path_relative.c_str());

    TCHAR tmp[MAX_PATH];
    _tcscpy(tmp, path);
    _tcscat(tmp, _T(".tmp"));
    File::Delete(tmp);
    bool success =
      DownloadToFile(session, item.uri.c_str(), tmp, NULL, *this) &&
      File::Replace(tmp, path);

    mutex.Lock();
    for (auto i = listeners.begin(), end = listeners.end(); i != end; ++i)
      (*i)->OnDownloadComplete(item.path_relative.c_str(), success);
  }
}

static DownloadManagerThread *thread;

bool
Net::DownloadManager::Initialise()
{
  assert(thread == NULL);

  thread = new DownloadManagerThread();
  return true;
}

void
Net::DownloadManager::BeginDeinitialise()
{
  assert(thread != NULL);

  thread->StopAsync();
}

#if defined(__clang__) || GCC_VERSION >= 40700
/* no, DownloadManagerThread really doesn't need a virtual
   destructor */
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
#endif

void
Net::DownloadManager::Deinitialise()
{
  assert(thread != NULL);

  thread->WaitStopped();
  delete thread;
}

bool
Net::DownloadManager::IsAvailable()
{
  assert(thread != NULL);

  return true;
}

void
Net::DownloadManager::AddListener(DownloadListener &listener)
{
  assert(thread != NULL);

  thread->AddListener(listener);
}

void
Net::DownloadManager::RemoveListener(DownloadListener &listener)
{
  assert(thread != NULL);

  thread->RemoveListener(listener);
}

void
Net::DownloadManager::Enqueue(const char *uri, const TCHAR *relative_path)
{
  assert(thread != NULL);

  thread->Enqueue(uri, relative_path);
}

#endif
