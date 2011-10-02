/***************************************************************
 *
 * Copyright (C) 2008-2011 Tim Brugman
 *
 * This file may be licensed under the terms of of the
 * GNU General Public License Version 2 (the ``GPL'').
 *
 * Software distributed under the License is distributed
 * on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
 * express or implied. See the GPL for the specific language
 * governing rights and limitations.
 *
 * You should have received a copy of the GPL along with this
 * program. If not, go to http://www.gnu.org/licenses/gpl.html
 * or write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 ***************************************************************/

#include <assert.h>
#include "Mutex.h"

CMutex::CMutex()
{
  m_lock_count = 0;
#ifdef WII
  LWP_MutexInit(&m_mutex, true);
#else
  assert( ((int)(&m_cs) & 0x3) == 0 ); // Check alignment
  InitializeCriticalSection(&m_cs);
#endif
}

CMutex::~CMutex()
{
#ifdef WII
  LWP_MutexDestroy(m_mutex);
#else
  DeleteCriticalSection(&m_cs);
#endif
}

void CMutex::Lock(void)
{
#ifdef WII
  LWP_MutexLock(m_mutex);
#else
  EnterCriticalSection(&m_cs);
#endif
  m_lock_count++;
}

void CMutex::Unlock(void)
{
  assert( m_lock_count > 0 );
  m_lock_count--;
#ifdef WII
  LWP_MutexUnlock(m_mutex);
#else
  LeaveCriticalSection(&m_cs);
#endif
}

