#include "keyboard.h"

#include "lib/kernel/print.h"
#include "lib/kernel/io.h"
#include "kernel/interrupt.h"
#include "kernel/global.h"
#include "device/ioqueue.h"

#define KBD_BUF_PORT 0x60       // 8042输入输出缓冲区寄存器端口

// 转义字符定义控制字符
#define ESC '\x1b'
#define BACKSPACE '\b'
#define TAB '\t'
#define ENTRY '\r'
#define DELETE '\x7f'

// 不可见字符定义为0
#define CHAR_INVISIBLE 0
#define CTRL_L_CHAR CHAR_INVISIBLE
#define CTRL_R_CHAR CHAR_INVISIBLE
#define SHIFT_L_CHAR CHAR_INVISIBLE
#define SHIFT_R_CHAR CHAR_INVISIBLE
#define ALT_L_CHAR CHAR_INVISIBLE
#define CAPS_LOCK_CHAR CHAR_INVISIBLE

// 控制字符的通码和断码
#define SHIFT_l_MAKE 0x2a
#define SHIFT_R_MAKE 0x36
#define ALT_l_MAKE 0x38
#define ALT_R_MAKE 0xe038
#define ALT_R_BREAK 0x30b8
#define CTRL_L_MAKE 0x1d
#define CTRL_R_MAKE 0xe01d
#define CTRL_R_BREAK 0xe09d
#define CAPS_LOCK_MAKE 0x3a

static bool gsCtrlStatus, gsShiftStatus, gsAltStatus, gsCapsLockStatus, gsExtScancode;

static char gsKeymap[][2] = {
    {0, 0},     // 0x00
    {ESC, ESC},     // 0x01
    {'1', '!'},     // 0x02
    {'2', '@'},     // 0x03
    {'3', '#'},     // 0x04
    {'4', '$'},     // 0x05
    {'5', '%'},     // 0x06
    {'6', '^'},     // 0x07
    {'7', '&'},     // 0x08
    {'8', '*'},     // 0x09
    {'9', '('},     // 0x0a
    {'0', ')'},     // 0x0b
    {'-', '_'},     // 0x0c
    {'=', '+'},     // 0x0d
    {BACKSPACE, BACKSPACE},     // 0x0e
    {TAB, TAB},     // 0x0f
    {'q', 'Q'},     // 0x10
    {'w', 'W'},     // 0x11
    {'e', 'E'},     // 0x12
    {'r', 'R'},     // 0x13
    {'t', 'T'},     // 0x14
    {'y', 'Y'},     // 0x15
    {'u', 'U'},     // 0x16
    {'i', 'I'},     // 0x17
    {'o', 'O'},     // 0x18
    {'p', 'P'},     // 0x19
    {'[', '{'},     // 0x1a
    {']', '}'},     // 0x1b
    {ENTRY, ENTRY},     // 0x1c
    {CTRL_L_CHAR, CTRL_L_CHAR},     // 0x1d
    {'a', 'A'},     // 0x1e
    {'s', 'S'},     // 0x1f
    {'d', 'D'},     // 0x20
    {'f', 'F'},     // 0x21
    {'g', 'G'},     // 0x22
    {'h', 'H'},     // 0x23
    {'j', 'J'},     // 0x24
    {'k', 'K'},     // 0x25
    {'l', 'L'},     // 0x26
    {';', ':'},     // 0x27
    {'\'', '"'},     // 0x28
    {'`', '~'},     // 0x29
    {SHIFT_L_CHAR, SHIFT_L_CHAR},     // 0x2a
    {'\\', '|'},     // 0x2b
    {'z', 'Z'},     // 0x2c
    {'x', 'X'},     // 0x2d
    {'c', 'C'},     // 0x2e
    {'v', 'V'},     // 0x2f
    {'b', 'B'},     // 0x30
    {'n', 'N'},     // 0x31
    {'m', 'M'},     // 0x32
    {',', '<'},     // 0x33
    {'.', '>'},     // 0x34
    {'/', '?'},     // 0x35
    {SHIFT_R_CHAR, SHIFT_R_CHAR},     // 0x36
    {'*', '*'},     // 0x37
    {ALT_L_CHAR, ALT_L_CHAR},     // 0x38
    {' ', ' '},     // 0x39
    {CAPS_LOCK_CHAR, CAPS_LOCK_CHAR},     // 0x3a
    
};

IoQueue gKbdBuf;

static void IntrKeyboardHandler(void) {
    bool ctrlDownLast = gsCtrlStatus;
    bool shiftDownLast = gsShiftStatus;
    bool capsLockLast = gsCapsLockStatus;
    
    bool breakCode;

    uint8_t scancode = inb(KBD_BUF_PORT);
    
    // 若扫描码以e0开头，表示此键会产生多个扫描码，结束此次中断处理函数，等待下一个扫描码
    if (scancode == 0xe0) {
        gsExtScancode = true;
        return;
    }

    // 合并上次的0xe0扫描码
    if (gsExtScancode) {
        scancode = 0xe000 | scancode;
        gsExtScancode = false;
    }

    breakCode = (scancode & 0x0080) != 0;
    if (breakCode) {
        // 是断码(按键弹起产生的扫描码)
        uint16_t makeCode = (scancode &= 0xff9f);

        if (makeCode == CTRL_L_MAKE || makeCode == SHIFT_R_CHAR) {
            gsCtrlStatus = false;
        } else if (makeCode == ALT_l_MAKE || makeCode == SHIFT_R_MAKE) {
            gsShiftStatus = false;
        } else if (makeCode == ALT_l_MAKE || makeCode == ALT_R_MAKE) {
            gsAltStatus = false;
        }

        return;
    }
    else if (scancode > 0x00 && scancode < 0x3b || scancode == ALT_l_MAKE || scancode == CTRL_R_MAKE) {
        // 是通码(按键按下产生的扫描码)
        bool shift = false;
        if (scancode < 0x0e || scancode == 0x29 || scancode == 0x1a || scancode == 0x1b || scancode == 0x2b || scancode == 0x27 || scancode == 0x28 || scancode == 0x33 || scancode == 0x34 || scancode == 0x35) {
            if (shiftDownLast) {
                shift = true;
            }
        } else {
            if (shiftDownLast && capsLockLast) {
                shift = false;
            } else if (shiftDownLast || capsLockLast) {
                shift = true;
            } else {
                shift = false;
            }
        }
        uint8_t index = (scancode &= 0x00ff);
        char curChar = gsKeymap[index][shift];
        if (curChar) {
            if (!IoQueueFull(&gKbdBuf)) {
                // PutChar(curChar);
                IoQueuePutChar(&gKbdBuf, curChar);
            }
            return;
        }

        if (scancode == CTRL_L_MAKE || scancode == CTRL_R_MAKE) {
            gsCtrlStatus = true;
        } else  if (scancode == SHIFT_l_MAKE || scancode == SHIFT_R_MAKE) {
            
        }

    } else {
        PutStr("unknown key\n");
    }

}

void KeyBoardInit(void) {
    PutStr("KeyboardInit start\n");
    IoQueueInit(&gKbdBuf);
    IntrRegisterHandler(0x21, IntrKeyboardHandler);
    PutStr("KeyboardInit done\n");
}

