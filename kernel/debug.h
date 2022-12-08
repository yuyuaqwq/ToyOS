#ifndef KERNEL_DEBUG_H_
#define KERNEL_DEBUG_H_

void PanicSpin(char* fileName, int line, const char* func, const char* condition);

#define PANIC(...) PanicSpin(__FILE__, __LINE__, __func__, __VA_ARGS__)

#ifdef NDEBUG
    #define ASSERT(CONDITION) ((void)0)
#else
#define ASSERT(CONDITION) \
if (CONDITION) {} else { PANIC(#CONDITION); }
#endif // NDEBUG

#endif // KERNEL_DEBUG_H_