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
#ifdef WII
  mutex_t g_mutex;
#else
  CRITICAL_SECTION m_cs;
#endif
};

#endif

