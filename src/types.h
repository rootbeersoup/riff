#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdlib.h>

#include "str.h"

#define TYPE_INT 1 // 0001
#define TYPE_FLT 2 // 0010
#define TYPE_STR 4 // 0100
#define TYPE_ARR 8 // 1000

#define IS_FLT(x) (x.type == TYPE_FLT)
#define IS_INT(x) (x.type == TYPE_INT)
#define IS_STR(x) (x.type == TYPE_STR)
#define IS_ARR(x) (x.type == TYPE_ARR)

#define IS_NUM(x) (x.type & (TYPE_INT | TYPE_FLT))

typedef double  flt_t;
typedef int64_t int_t;

// Struct for constant values
typedef struct {
    uint8_t type;
    union {
        flt_t  f;
        int_t  i;
        str_t *s;
        // Add array type
    } as;
} value_t;

#endif
