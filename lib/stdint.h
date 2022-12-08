#ifndef LIB_STDINT_H_
#define LIB_STDINT_H_

typedef signed char int8;
typedef signed short int16;
typedef signed int int32;
typedef signed long long int64;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef int8 bool;
#define false 0
#define true 1

#define NULL ((void*)0)

#endif // LIB_STDINT_H_