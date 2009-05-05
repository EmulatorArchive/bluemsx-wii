#ifndef _VERSION_H
#define _VERSION_H

#include "revision.inc"

#if REVISION_NUMBER==999
#define VERSION_AS_STRING "V1.0.0"
#else
#define VERSION_AS_STRING "Build "REVISION_NUMBER_STRING
#endif

#endif
