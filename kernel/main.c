#include "kernel/init.h"
#include "kernel/debug.h"
#include "device/console.h"
#include "thread/thread.h"
#include "userprog/process.h"
#include "device/ioqueue.h"
#include "device/keyboard.h"

#include "userprog/syscall-init.h"
#include "lib/user/syscall.h"
#include "lib/stdio.h"
#include "kernel/memory.h"


// 要让_main作为第一个函数实现
void kThread_a(void* arg);
void kThread_b(void* arg);

void uProg_a(void);
void uProg_b(void);

int test_var_a = 0, test_var_b = 0;

// nm指令可以查看程序符号

int _main(void) {
    PutStr("I am kernel\n");
    InitAll();

    // ThreadStart("kThread_a", 31, kThread_a, " B_");
    // ThreadStart("kThread_b", 31, kThread_b, " A_");

    ProcessExecute(uProg_a, "uProg_a");
    // ProcessExecute(uProg_b, "uProg_b");

    IntrEnable();
    while(1);
    return 0;
}


void uProg_a(void) {
    printf(" I am %s, my pid:%d%c", "prog_a", GetPid(), '\n');

     void* addr1 = malloc(256);
     void* addr2 = malloc(255);
     void* addr3 = malloc(254);
     printf(" prog_a malloc addr:0x%x,0x%x,0x%x\n", \
    (int)addr1, (int)addr2, (int)addr3);
    
     int cpu_delay = 100000;
     while(cpu_delay-- > 0);
     free(addr1);
     free(addr2);
     free(addr3);
     while(1);


    // test_var_a = GetPid();
    // PutStr("uProg_a"); PutStr("\n");
    while(1) {
        // test_var_a++;
    }
}

void uProg_b(void) {
    // test_var_b = GetPid();
    printf(" I am %s, my pid:%d%c", "prog_b", GetPid(), '\n');

     void* addr1 = malloc(256);
     printf("cnmd");
     void* addr2 = malloc(255);
     void* addr3 = malloc(254);
     printf(" prog_b malloc addr:0x%x,0x%x,0x%x\n", \
     (int)addr1, (int)addr2, (int)addr3);
    
     int cpu_delay = 100000;
     while(cpu_delay-- > 0);
     free(addr1);
     free(addr2);
     free(addr3);
     while(1);

    while(1) {
        // test_var_b++;
    }
}



void kThread_a(void* arg) {
    int max = 1000;
    // while (max-- > 0) {
    //     void* addr = MemAllocPage(kPfKernel, 10);
    //     PutInt(addr); PutChar('1 ');
    //     void* addr2 = MemAllocPage(kPfKernel, 1);
    //     PutInt(addr2); PutChar('2 ');
    //     MemFreePage(kPfKernel, addr, 10);
    //     MemFreePage(kPfKernel, addr2, 1);
    // }
    // while(1);
    // return;
 char* para = arg;
 void* addr1;
 void* addr2;
 void* addr3;
 void* addr4;
 void* addr5;
 void* addr6;
 void* addr7;
 ConsolePutStr(" thread_a start\n");
 max = 100;
 while (max-- > 0) {
	 int size = 128;
     
	 addr1 = SysMalloc(size);
     // PutStr("malloc1  "); PutInt(addr1); PutChar('\n');
	 size *= 2;
	 addr2 = SysMalloc(size);
     // PutStr("malloc2  "); PutInt(addr2); PutChar('\n');
	 size *= 2;
	 addr3 = SysMalloc(size);
     // PutStr("malloc3  "); PutInt(addr3); PutChar('\n');
	 SysFree(addr1);
     // PutStr("free1  "); PutInt(addr1); PutChar('\n');
      
	 addr4 = SysMalloc(size);
     // PutStr("malloc4  "); PutInt(addr4); PutChar('\n');
	 size *= 2; size *= 2; size *= 2; size *= 2;
	 size *= 2; size *= 2; size *= 2;
    
    // 这里的调用分配的内存导致覆盖了11的页面
	 addr5 = SysMalloc(size);
     // PutStr("size  "); PutInt(size); PutChar('\n');
     // PutStr("malloc5  "); PutInt(addr5); PutChar('\n');

	 addr6 = SysMalloc(size);
     // PutStr("malloc6  "); PutInt(addr6); PutChar('\n');
	 SysFree(addr5);
     // PutStr("free5  "); PutInt(addr5); PutChar('\n');
	 size *= 2;
	 addr7 = SysMalloc(size);
     // PutStr("malloc7  "); PutInt(addr7); PutChar('\n');
	 SysFree(addr6);
     // PutStr("free6  "); PutInt(addr6); PutChar('\n');
	 SysFree(addr7);
     // PutStr("free7  "); PutInt(addr7); PutChar('\n');
	 SysFree(addr2);
     // PutStr("free2  "); PutInt(addr2); PutChar('\n');
	 SysFree(addr3);
     // PutStr("free3  "); PutInt(addr3); PutChar('\n');
	 SysFree(addr4);
     // PutStr("free4  "); PutInt(addr4); PutChar('\n');
	PutInt(max); PutChar(' ');
    }
 ConsolePutStr(" thread_a end\n");
 while (1);
}


void kThread_b(void* arg) {
char* para = arg;
	 void* addr1;
	 void* addr2;
	 void* addr3;
	 void* addr4;
	 void* addr5;
	 void* addr6;
	 void* addr7;
	 void* addr8;
	 void* addr9;
	 int max = 100;
	 ConsolePutStr(" thread_b start\n");
	 while (max-- > 0) {
		int size = 9;
		addr1 = SysMalloc(size);
		size *= 2;
		addr2 = SysMalloc(size);
		size *= 2;
		SysFree(addr2);
		addr3 = SysMalloc(size);
		SysFree(addr1);
		addr4 = SysMalloc(size);
		addr5 = SysMalloc(size);
		addr6 = SysMalloc(size);
		SysFree(addr5);
		size *= 2;
		addr7 = SysMalloc(size);
		SysFree(addr6);
		SysFree(addr7);
		SysFree(addr3);
		SysFree(addr4);
		
		size *= 2; size *= 2; size *= 2;
		 addr1 = SysMalloc(size);
		 addr2 = SysMalloc(size);
		 addr3 = SysMalloc(size);
		 addr4 = SysMalloc(size);
		 addr5 = SysMalloc(size);
		 addr6 = SysMalloc(size);
		 addr7 = SysMalloc(size);
		 addr8 = SysMalloc(size);
		 addr9 = SysMalloc(size);
		 SysFree(addr1);
		 SysFree(addr2);
		 SysFree(addr3);
		 SysFree(addr4);
		 SysFree(addr5);
		 SysFree(addr6);
		 SysFree(addr7);
		 SysFree(addr8);
		 SysFree(addr9);
		
	}
	 ConsolePutStr(" thread_b end\n");
	 while (1);
}