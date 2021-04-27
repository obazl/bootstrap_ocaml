#include <assert.h>
#include <errno.h>
#include <libgen.h>             /* for basename() */
#ifdef LINUX                    /* FIXME */
#include <linux/limits.h>
#else // FIXME: macos test
#include <limits.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>

#include "utarray.h"
#include "uthash.h"

/* #include "enums.h" */
#include "utils.h"
/* #include "omp.h" */

int errnum;
int rc;

#if INTERFACE

/* struct package_s { */
/*     char name[512]; */
/*     struct fileset_s *filesets; */
/*     /\* struct fileset_s *coq_filesets; *\/ */
/*     /\* struct fileset_s *ocaml_filesets; *\/ */
/*     int has_types;              /\* used by emit to decide what rules to load *\/ */
/*     UT_hash_handle hh;         /\* makes this structure hashable *\/ */
/* } ; */

#endif

/* #if INTERFACE */
/* enum extension { */
/*     NOEXT, BAZEL, */
/*     ML, MLI, MLG, MLL, MLY, MLLIB, MLPACK, */
/*     CMI, CMX, CMO, CMA, CMXA, CMXS, */
/*     VO, VOS, VOK, V, */
/*     GLOB, */
/*     A, O */
/* }; */
/* #endif */

int strsort(const void *_a, const void *_b)
{
    const char *a = *(const char* const *)_a;
    const char *b = *(const char* const *)_b;
    /* printf("strsort: %s =? %s\n", a, b); */
    return strcmp(a,b);
}

EXPORT char* mystrcat( char* dest, char* src )
{
     while (*dest) dest++;
     while ( (*dest++ = *src++) );
     return --dest;
}


