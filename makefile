BUILD_DIR = ./build
TOOLS_DIR = ./tools
DISK_DIR = ./disk
ENTRY_POINT = 0xc0001500
ENTRY_FUNCTION = main
ASM = $(TOOLS_DIR)/nasm
C = gcc
LINK = ld
INCLUDE_DIR = .
ASM_FLAGS = -f coff
C_FLAGS = -m32 -c -I $(INCLUDE_DIR)
LINK_FLAGS_KERNEL = -mi386pe -Ttext $(ENTRY_POINT) -e $(ENTRY_FUNCTION) -o $(BUILD_DIR)/kernel.bin
OBJS_KERNEL = $(BUILD_DIR)/main.o $(BUILD_DIR)/init.o $(BUILD_DIR)/interrupt.o \
	   $(BUILD_DIR)/kernel.o $(BUILD_DIR)/debug.o $(BUILD_DIR)/bitmap.o \
	   $(BUILD_DIR)/memory.o $(BUILD_DIR)/thread.o $(BUILD_DIR)/bitmap.o \
	   $(BUILD_DIR)/sync.o $(BUILD_DIR)/switch.o $(BUILD_DIR)/timer.o \
	   $(BUILD_DIR)/console.o $(BUILD_DIR)/keyboard.o $(BUILD_DIR)/ioqueue.o \
	   $(BUILD_DIR)/process.o $(BUILD_DIR)/tss.o $(BUILD_DIR)/string.o \
	   $(BUILD_DIR)/stdio.o \
	   $(BUILD_DIR)/kernel.o $(BUILD_DIR)/debug.o $(BUILD_DIR)/bitmap.o \
	   $(BUILD_DIR)/print.o $(BUILD_DIR)/list.o $(BUILD_DIR)/syscall.o \
	   $(BUILD_DIR)/syscall-init.o $(BUILD_DIR)/stdio-kernel.o \
	   $(BUILD_DIR)/ide.o


# C编译
$(BUILD_DIR)/main.o: kernel/main.c kernel/init.h lib/kernel/print.h \
				lib/stdint.h kernel/interrupt.h \
				device/timer.h
		$(C) $(C_FLAGS) $< -o $@

$(BUILD_DIR)/init.o: kernel/init.c kernel/init.h \
				lib/kernel/print.h lib/stdint.h \
				kernel/memory.h kernel/interrupt.h \
				device/timer.h device/console.h \
				device/keyboard.h thread/thread.h \
				userprog/tss.h userprog/syscall-init.h
		$(C) $(C_FLAGS) $< -o $@

$(BUILD_DIR)/interrupt.o: kernel/interrupt.c kernel/interrupt.h \
				kernel/global.h lib/stdint.h \
				lib/kernel/print.h lib/kernel/io.h
		$(C) $(C_FLAGS) $< -o $@

$(BUILD_DIR)/timer.o: device/timer.c device/timer.h lib/stdint.h \
				lib/kernel/io.h lib/kernel/print.h
		$(C) $(C_FLAGS) $< -o $@

$(BUILD_DIR)/ide.o: device/ide.c device/ide.h lib/stdint.h \
				device/timer.h kernel/interrupt.h kernel/memory.h \
				lib/kernel/stdio-kernel.h lib/stdio.h lib/kernel/io.h
		$(C) $(C_FLAGS) $< -o $@

$(BUILD_DIR)/debug.o: kernel/debug.c kernel/debug.h \
				lib/kernel/print.h lib/stdint.h kernel/interrupt.h
		$(C) $(C_FLAGS) $< -o $@

$(BUILD_DIR)/string.o: lib/string.c lib/string.h lib/stdint.h kernel/global.h \
				lib/stdint.h kernel/debug.h
		$(C) $(C_FLAGS) $< -o $@

$(BUILD_DIR)/stdio.o: lib/stdio.c lib/stdio.h lib/stdint.h kernel/interrupt.h \
    	kernel/global.h lib/string.h lib/user/syscall.h lib/kernel/print.h \
		lib/user/syscall.h
		$(C) $(C_FLAGS) $< -o $@


$(BUILD_DIR)/bitmap.o: lib/kernel/bitmap.c lib/kernel/bitmap.h \
				kernel/global.h lib/stdint.h lib/string.h lib/stdint.h \
				lib/kernel/print.h kernel/interrupt.h kernel/debug.h
		$(C) $(C_FLAGS) $< -o $@

$(BUILD_DIR)/memory.o: kernel/memory.c kernel/memory.h lib/stdint.h lib/kernel/bitmap.h \
				kernel/global.h kernel/debug.h lib/kernel/print.h \
				lib/kernel/io.h kernel/interrupt.h lib/string.h lib/stdint.h
		$(C) $(C_FLAGS) $< -o $@

$(BUILD_DIR)/thread.o: thread/thread.c thread/thread.h lib/stdint.h \
				kernel/global.h lib/string.h lib/stdint.h kernel/debug.h \
				kernel/interrupt.h lib/kernel/print.h kernel/memory.h \
				lib/kernel/bitmap.h userprog/process.h thread/thread.h
		$(C) $(C_FLAGS) $< -o $@

$(BUILD_DIR)/list.o: lib/kernel/list.c lib/kernel/list.h kernel/global.h \
				lib/stdint.h kernel/interrupt.h 
		$(C) $(C_FLAGS) $< -o $@

$(BUILD_DIR)/stdio-kernel.o: lib/kernel/stdio-kernel.c lib/kernel/stdio-kernel.h lib/stdint.h \
				lib/stdio.h lib/stdint.h device/console.h 
		$(C) $(C_FLAGS) $< -o $@

$(BUILD_DIR)/console.o: device/console.c device/console.h lib/stdint.h \
				kernel/interrupt.h
		$(C) $(C_FLAGS) $< -o $@

$(BUILD_DIR)/sync.o: thread/sync.c thread/sync.h lib/kernel/list.h \
				kernel/global.h lib/stdint.h thread/thread.h lib/string.h \
				kernel/debug.h kernel/interrupt.h
		$(C) $(C_FLAGS) $< -o $@

$(BUILD_DIR)/keyboard.o: device/keyboard.c device/keyboard.h lib/kernel/print.h \
				lib/stdint.h kernel/interrupt.h lib/kernel/io.h device/ioqueue.h \
				thread/thread.h lib/kernel/list.h kernel/global.h thread/sync.h
		$(C) $(C_FLAGS) $< -o $@

$(BUILD_DIR)/ioqueue.o: device/ioqueue.c device/ioqueue.h lib/stdint.h \
				thread/thread.h lib/kernel/list.h kernel/global.h thread/sync.h \
				kernel/interrupt.h kernel/debug.h
		$(C) $(C_FLAGS) $< -o $@

$(BUILD_DIR)/tss.o: userprog/tss.c userprog/tss.h thread/thread.h lib/stdint.h \
				lib/kernel/list.h kernel/global.h lib/string.h lib/stdint.h \
				lib/kernel/print.h
		$(C) $(C_FLAGS) $< -o $@

$(BUILD_DIR)/process.o: userprog/process.c userprog/process.h thread/thread.h \
    	lib/stdint.h lib/kernel/list.h kernel/global.h kernel/debug.h \
     	kernel/memory.h lib/kernel/bitmap.h userprog/tss.h kernel/interrupt.h \
      	lib/string.h
		$(C) $(C_FLAGS) $< -o $@

$(BUILD_DIR)/syscall.o: lib/user/syscall.c lib/user/syscall.h lib/stdint.h
		$(C) $(C_FLAGS) $< -o $@

$(BUILD_DIR)/syscall-init.o: userprog/syscall-init.c userprog/syscall-init.h \
    	lib/stdint.h lib/user/syscall.h lib/kernel/print.h thread/thread.h \
     	lib/kernel/list.h kernel/global.h lib/kernel/bitmap.h kernel/memory.h \
	device/console.h
		$(C) $(C_FLAGS) $< -o $@



# ASM编译
$(BUILD_DIR)/kernel.o: kernel/kernel.asm
		$(ASM) $(ASM_FLAGS) $< -o $@

$(BUILD_DIR)/print.o: lib/kernel/print.asm

		$(ASM) $(ASM_FLAGS) $< -o $@
$(BUILD_DIR)/switch.o: thread/switch.asm
		$(ASM) $(ASM_FLAGS) $< -o $@

# 目标文件链接
$(BUILD_DIR)/kernel.bin: $(OBJS_KERNEL)
		$(LINK) $(LINK_FLAGS_KERNEL) $^ -o $@
	
# 命令定义
.PHONY: build hd clean all

hd:
		$(TOOLS_DIR)/filecp.exe $(DISK_DIR)/master.vhd 0 ./build/mbr.bin
		$(TOOLS_DIR)/filecp.exe $(DISK_DIR)/master.vhd 1024 $(BUILD_DIR)/loader.bin
		$(TOOLS_DIR)/filecp.exe $(DISK_DIR)/master.vhd 4608 $(BUILD_DIR)/kernel.bin

clean:
		del $(BUILD_DIR)/*.o
		del $(BUILD_DIR)/*.bin
		del $(BUILD_DIR)/*.lst
		del $(BUILD_DIR)/disk.vhd.lock

build: $(BUILD_DIR)/kernel.bin


all: build hd