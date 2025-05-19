#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef RLIM_SUPPORTED
#include <sys/resource.h>
#endif

#include "test.h"

int t_memfill()
{
	int r = 0;
	/* alloc mmap space with PROT_NONE */
	if (t_vmfill(0,0,0) < 0) {
		t_error("vmfill failed: %s\n", strerror(errno));
		r = -1;
	}

#ifdef RLIM_SUPPORTED
	/* limit brk space */
	if (t_setrlim(RLIMIT_DATA, 0) < 0)
		r = -1;
#endif

	if (!r)
		/* use up libc reserves if any */
		while (malloc(1));
	return r;
}
