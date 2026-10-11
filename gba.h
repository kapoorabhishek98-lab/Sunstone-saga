/* gba.h - minimal GBA hardware definitions (no external libraries needed).
 * Build with -DHOST to compile against RAM buffers for desktop testing. */
#ifndef GBA_H
#define GBA_H
#include <stdint.h>

typedef uint8_t  u8;  typedef uint16_t u16; typedef uint32_t u32;
typedef int8_t   s8;  typedef int16_t  s16; typedef int32_t  s32;
typedef volatile u16 vu16; typedef volatile u32 vu32;

#ifdef HOST
extern u8  host_io[0x400];
extern u16 host_pal[0x200];
extern u16 host_vram[0xC000];
extern u16 host_oam[0x200];
void host_vsync(void);
#define IO_BASE   ((uintptr_t)host_io)
#define PAL_BASE  ((uintptr_t)host_pal)
#define VRAM_BASE ((uintptr_t)host_vram)
#define OAM_BASE  ((uintptr_t)host_oam)
#else
#define IO_BASE   0x04000000u
#define PAL_BASE  0x05000000u
#define VRAM_BASE 0x06000000u
#define OAM_BASE  0x07000000u
#endif

#define REG16(o) (*(vu16 *)(IO_BASE + (o)))
#define REG_DISPCNT    REG16(0x000)
#define REG_VCOUNT     REG16(0x006)
#define REG_BG0CNT     REG16(0x008)
#define REG_BG1CNT     REG16(0x00A)
#define REG_BG0HOFS    REG16(0x010)
#define REG_BG0VOFS    REG16(0x012)
#define REG_BG1HOFS    REG16(0x014)
#define REG_BG1VOFS    REG16(0x016)
#define REG_BLDCNT     REG16(0x050)
#define REG_BLDY       REG16(0x054)
#define REG_SND1SWEEP  REG16(0x060)
#define REG_SND1CNT    REG16(0x062)
#define REG_SND1FREQ   REG16(0x064)
#define REG_SND2CNT    REG16(0x068)
#define REG_SND2FREQ   REG16(0x06C)
#define REG_SNDDMGCNT  REG16(0x080)
#define REG_SNDDSCNT   REG16(0x082)
#define REG_SNDSTAT    REG16(0x084)
#define REG_KEYINPUT   REG16(0x130)

#define BG_PAL   ((u16 *)(PAL_BASE))
#define OBJ_PAL  ((u16 *)(PAL_BASE + 0x200))
#define CHARBLOCK(n)   ((void *)(VRAM_BASE + (n) * 0x4000))
#define SCREENBLOCK(n) ((u16 *)(VRAM_BASE + (n) * 0x800))
#define OBJ_VRAM ((void *)(VRAM_BASE + 0x10000))
#define OAM_MEM  ((void *)(OAM_BASE))

#define DCNT_MODE0    0x0000
#define DCNT_BLANK    0x0080
#define DCNT_OBJ_1D   0x0040
#define DCNT_BG0      0x0100
#define DCNT_BG1      0x0200
#define DCNT_OBJ      0x1000

#define BG_PRIO(n) (n)
#define BG_CBB(n)  ((n) << 2)
#define BG_SBB(n)  ((n) << 8)
#define BG_SIZE_32x32 0x0000
#define BG_SIZE_64x64 0xC000

#define KEY_A      0x0001
#define KEY_B      0x0002
#define KEY_SELECT 0x0004
#define KEY_START  0x0008
#define KEY_RIGHT  0x0010
#define KEY_LEFT   0x0020
#define KEY_UP     0x0040
#define KEY_DOWN   0x0080
#define KEY_R      0x0100
#define KEY_L      0x0200
#define KEY_DIRS   0x00F0
#define KEY_MASK   0x03FF

#define ATTR0_HIDE 0x0200
#define ATTR0_TALL 0x8000
#define ATTR1_HFLIP 0x1000
#define ATTR1_SIZE_16x32 0x8000   /* tall shape + size 2 */

typedef struct { u16 attr0, attr1, attr2, fill; } OamEntry;

#endif
