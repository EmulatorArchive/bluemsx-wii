#include "Mutex.h"

#ifndef WII
#include <assert.h>
#endif

CMutex::CMutex()
{
#ifdef WII
  LWP_MutexInit(&g_mutex, true);
#else
  assert( ((int)(&m_cs) & 0x3) == 0 ); // Check alignment
  InitializeCriticalSection(&m_cs);
#endif
}

CMutex::~CMutex()
{
#ifdef WII
  LWP_MutexDestroy(g_mutex);
#else
  DeleteCriticalSection(&m_cs);
#endif
}

void CMutex::Lock(void)
{
#ifdef WII
  LWP_MutexLock(g_mutex);
#else
  EnterCriticalSection(&m_cs);
#endif
}

void CMutex::Unlock(void)
{
#ifdef WII
  LWP_MutexUnlock(g_mutex);
#else
  LeaveCriticalSection(&m_cs);
#endif
}

