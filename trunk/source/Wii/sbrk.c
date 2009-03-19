#include <gccore.h>
#include <ogc/machine/processor.h>
#include <_ansi.h>
#include <_syslist.h>
#include <stdlib.h>
#include <unistd.h>
#include <reent.h>
#include <errno.h>

void* _DEFUN(__libogc_sbrk_r,(ptr,incr),
			 struct _reent *ptr _AND
					 ptrdiff_t incr)
{
	u32 level;
	char *heap_end = 0;
	char *prev_heap = 0;

	_CPU_ISR_Disable(level);
	heap_end = (char*)SYS_GetArena2Lo();

	if((heap_end+incr)>(char*)SYS_GetArena2Hi()) {

		ptr->_errno = ENOMEM;
		prev_heap = (char *)-1;

	} else {

		prev_heap = heap_end;
		SYS_SetArena2Lo((void*)(heap_end+incr));
	}
	_CPU_ISR_Restore(level);

	return (void*)prev_heap;
}

