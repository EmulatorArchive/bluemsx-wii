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
#ifndef _H_MUTEX_
#define _H_MUTEX_

#ifdef WII
#include <gccore.h>
#else
#include <windows.h>
#endif

class CMutex
{
public:
  CMutex();
  ~CMutex();
  void Lock(void);
  void Unlock(void);
private:
  int m_lock_count;
#ifdef WII
  mutex_t m_mutex;
#else
  CRITICAL_SECTION m_cs;
#endif
};

#endif

