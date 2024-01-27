#ifndef __TERA_ASSERT_H
#define __TERA_ASSERT_H

#include <stdio.h>
#include <stdlib.h>

#define assert(expression, ...) ;

#if 0
#define assert(expression, ...) \
  if (!(expression)) { \
    fprintf(stderr, __VA_ARGS__); \
    abort(); \
  }
#endif

#endif //__TERA_ASSERT_H
