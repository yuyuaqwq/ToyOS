#ifndef LIB_STDINT_H_
#define LIB_STDINT_H_

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef int8_t bool;
#define false 0
#define true 1

#define NULL ((void*)0)

#endif // LIB_STDINT_H_