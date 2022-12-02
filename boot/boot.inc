;----------Loader----------
LOADER_BASE_ADDR equ 0x900
LOADER_ENTRY equ LOADER_BASE_ADDR + 0x300
LOADER_START_SECTOR equ 0x2

PAGE_DIR_TABLE_POS equ 0x100000

;----------kernel----------
KERNEL_BIN_BASE_ADDR equ 0x70000
KERNEL_ENTRY_POINT equ 0xc0001500
KERNEL_START_SECTOR equ 0x9

;----------gtd描述符属性----------
DESC_G_4K equ (1b << 23)
DESC_D_32 equ (1b << 22)
DESC_L equ (0b << 21)      ; 64位代码标记
DESC_AVL equ (0b << 20)        ; CPU未使用
DESC_LIMIT_CODE2 equ (0xf << 16)
DESC_LIMIT_DATA2 equ (0xf << 16)
DESC_LIMIT_VIDEO2 equ (0x0 << 16)
DESC_P equ (1b << 15)
DESC_DPL_0 equ (0x0 << 13)
DESC_DPL_1 equ (0x1 << 13)
DESC_DPL_2 equ (0x2 << 13)
DESC_DPL_3 equ (0x3 << 13)
DESC_S_CODE equ (1b << 12)
DESC_S_DATA equ (1b << 12)
DESC_S_SYS equ (0b << 12)
DESC_TYPE_CODE equ (0x8 << 8)
DESC_TYPE_DATA equ (0x2 << 8)

DESC_CODE_HIGH4 equ ((0x00 << 24) | DESC_G_4K | DESC_D_32 | DESC_L | DESC_AVL | DESC_LIMIT_CODE2 | DESC_P | DESC_DPL_0 | DESC_S_CODE | DESC_TYPE_CODE | 0x00)
DESC_DATA_HIGH4 equ ((0x00 << 24) | DESC_G_4K | DESC_D_32 | DESC_L | DESC_AVL | DESC_LIMIT_DATA2 | DESC_P | DESC_DPL_0 | DESC_S_DATA | DESC_TYPE_DATA | 0x00)
DESC_VIDEO_HIGH4 equ ((0x00 << 24) | DESC_G_4K | DESC_D_32 | DESC_L | DESC_AVL | DESC_LIMIT_VIDEO2 | DESC_P | DESC_DPL_0 | DESC_S_DATA | DESC_TYPE_DATA | 0x0b)

;----------段选择子属性----------
RPL0 equ 00b
RPL1 equ 01b
RPL2 equ 10b
RPL3 equ 11b
TI_GDT equ (0b << 2)
TI_LDT equ (1b << 2)

;----------页表项属性----------
PG_P equ 1b
PG_RW_R equ (0b << 1)
PG_RW_W equ (1b << 1)
PG_US_S equ (0b << 2)
PG_US_U equ (1b << 2)