/*
 * SUNSTONE SAGA - Game Boy Advance edition (v2)
 * An original retro RPG. Single-file source; build with devkitARM:
 *   arm-none-eabi-gcc -mthumb -mthumb-interwork -mcpu=arm7tdmi -O2 -specs=gba.specs main.c -o sunstone_saga.elf
 *   arm-none-eabi-objcopy -O binary sunstone_saga.elf sunstone_saga.gba
 *   gbafix sunstone_saga.gba -tSUNSTONE -cSUNS -m01
 * Controls: D-pad move, A talk/confirm, B cancel, START menu, SELECT sound on/off.
 */
#include <stdarg.h>
#include <string.h>

typedef unsigned char u8; typedef unsigned short u16; typedef unsigned int u32;
typedef signed char s8; typedef signed short s16; typedef signed int s32;

/* ================= PLATFORM ================= */
#ifdef HOST
u8 host_io[0x400]; u16 host_vram[0xC000]; u16 host_pal[512]; u16 host_oam[512]; u8 host_sram[0x10000];
u16 host_keys=0x03FF;
static u8 ew_anim[24*2*256]; static u32 ew_gbuf[64*64]; static u8 ew_q[48*176];
#define IO16(o) (*(volatile u16*)(host_io+(o)))
#define VRAM16 ((volatile u16*)host_vram)
#define PALBG ((volatile u16*)host_pal)
#define PALOBJ ((volatile u16*)(host_pal+256))
#define OAM16 ((volatile u16*)host_oam)
#define SRAM8 ((volatile u8*)host_sram)
#define KEYREG host_keys
#define ANIMBUF ew_anim
#define GBUF ew_gbuf
#define QBUF ew_q
#else
#define IO16(o) (*(volatile u16*)(0x04000000+(o)))
#define VRAM16 ((volatile u16*)0x06000000)
#define PALBG ((volatile u16*)0x05000000)
#define PALOBJ ((volatile u16*)0x05000200)
#define OAM16 ((volatile u16*)0x07000000)
#define SRAM8 ((volatile u8*)0x0E000000)
#define KEYREG IO16(0x130)
#define ANIMBUF ((u8*)0x02020000)
#define GBUF ((u32*)0x02028000)
#define QBUF ((u8*)0x02030000)
#endif
#define REG_DISPCNT IO16(0x00)
#define REG_VCOUNT  IO16(0x06)
#define REG_BG0CNT  IO16(0x08)
#define REG_BG1CNT  IO16(0x0A)
#define REG_BG2CNT  IO16(0x0C)
#define REG_BG0HOFS IO16(0x10)
#define REG_BG0VOFS IO16(0x12)
#define REG_BG1HOFS IO16(0x14)
#define REG_BG1VOFS IO16(0x16)
#define REG_BG2HOFS IO16(0x18)
#define REG_BG2VOFS IO16(0x1A)
#define REG_WIN0H   IO16(0x40)
#define REG_WIN0V   IO16(0x44)
#define REG_WININ   IO16(0x48)
#define REG_WINOUT  IO16(0x4A)
#define REG_MOSAIC  IO16(0x4C)
#define REG_BLDCNT  IO16(0x50)
#define REG_BLDY    IO16(0x54)
#define REG_SND1SWEEP IO16(0x60)
#define REG_SND1CNT IO16(0x62)
#define REG_SND1FREQ IO16(0x64)
#define REG_SND2L   IO16(0x68)
#define REG_SND2H   IO16(0x6C)
#define REG_SNDCNT_L IO16(0x80)
#define REG_SNDCNT_H IO16(0x82)
#define REG_SNDCNT_X IO16(0x84)
#define REG_WAITCNT IO16(0x204)

/* Tells emulators (Delta, mGBA...) this cart uses battery SRAM saves. */
__attribute__((used,aligned(4))) const char SAVE_TAG[]="SRAM_V113";

static void vsync(void){
#ifndef HOST
  while(REG_VCOUNT>=160); while(REG_VCOUNT<160);
#endif
}

/* ================= DATA (generated) ================= */
static const char* const WORLD_ROWS[30]={
"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~",
"~~~^^^^^^^^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~",
"~~^^TTT...^^~~~~~~~~~~~~~^^^^^^^^^~~~~~~",
"~~^TT.......T~~~~~~~~~~~^^hhhhhhhh^^~~~~",
"~~^T....A....T~~~~~~~~~^hh.........h^~~~",
"~~^T..........T~~~~~~~^h......B.....h~~~",
"~~~T...........======...............h~~~",
"~~~TT.........T~~~~~~~h.............hh~~",
"~~~~T.........TT~~~~~~hh....TTT.....h~~~",
"~~~~TT...TT....T~~~~~~~h...TTTTT....h~~~",
"~~~~~T..TTTT...T~~~~~~~hh..TTTTT...hh~~~",
"~~~~~T...TT....hh~~~~~~~h...TTT....h~~~~",
"~~~~~hh........h^~~~~~~~hh.........h~~~~",
"~~~~~~h.........^~~~~~~~~h.........hh~~~",
"~~~~~~hh.....::::^~~~~~~^h..........h~~~",
"~~~~~~~h....:::::^~~~~~^^hh.........h~~~",
"~~~~~~~^h...:::::^~~~~~^C.h.........h~~~",
"~~~~~~~^^hh......^~~~~~~^^h.........hh~~",
"~~~~~~~~^^h......^~~~~~~~^h....TT....h~~",
"~~~~~~~~~^^......^~~~~~~~~h...TTTT...h~~",
"~~~~~~~~~~^^....^^~~~~~~~~hh...TT....h~~",
"~~~~~~~~~~~^^^^^^~~~~~~~~~~h.........h~~",
"~~~~~~~~~~~~~~~~~~~~~~~~~~~hh.......hh~~",
"~~~~~~~~~~~~~~~~~~~~~~~~~~~~h.......h~~~",
"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~###~~~~~~",
"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~...~~~~~~",
"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^...^~~~~~",
"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^^.K.^^~~~~",
"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^^^^^~~~~~",
"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"};
static const char* const CAVE_ROWS[15]={
"####################",
"#S..x.......#......#",
"##.##.#####.#.####.#",
"#..#..#...#.x.#....#",
"#.##.##.#.#...#.x###",
"#....#..#.#####....#",
"####.#.##.....#.##.#",
"#....#..#####.#..#.#",
"#.#####.x...#.##.#.#",
"#.....#.#.#.#....#.#",
"#####.#...x.######.#",
"#.....#####........#",
"#.#.x.....#.######.#",
"#...###.#...#....#.#",
"####################"};
static const char* const KEEP_ROWS[14]={
"###############",
"#####t...t#####",
"######.R.######",
"#....t.R.t....#",
"#.##.#.R.#.##.#",
"#.##...R...##.#",
"#......R......#",
"####.##R##.####",
"#......R......#",
"#.##.#.R.#.##.#",
"#....t.R.t....#",
"######.R.######",
"######.S.######",
"###############"};
#define WORLD_W 40
#define WORLD_H 30
#define TOWN_W 16
#define TOWN_H 12
#define CAVE_W 20
#define CAVE_H 15
#define KEEP_W 15
#define KEEP_H 14
static const char* const D_MUSH[16]={"................",".....kkkkkk.....","...kkrrwwrrrkk..","..krrrrwwrrrrrk.",".krwwrrrrrrwwrrk",".krwwrrrrrrwwrrk",".kkrrrrrrrrrrrkk","..kkkkkkkkkkkkk.","....kssssssk....","....ksessesk....","....kssmmssk....","....kssssssk....","...kksssssskk...","..kggkkkkkkggk..","..kkk......kkk..","................"};
static const char* const D_BAT[16]={"................","................","k..............k","kk....k..k....kk","krk..krkkrk..krk","krrk.krrrrk.krrk","krrrkrwrrwrkrrrk","krrrrrrrrrrrrrrk","krrrrrkmmkrrrrrk",".krrrkrrrrkrrrk.","..krk.krrk.krk..","...kk..kk..kk...","................","................","................","................"};
static const char* const D_BEETLE[16]={"................","................","......k..k......",".......kk.......","....kkkkkkkk....","...kwekkkkewk...","..kkkkkkkkkkkk..",".krrrrrkkrrrrrk.","krrlrrrkkrrrlrrk","krllrrrkkrrrllrk","krrrrrrkkrrrrrrk","krrrrrrkkrrrrrrk",".krrrrrkkrrrrrk.","k.kkkkkkkkkkkk.k",".k..k......k..k.","................"};
static const char* const D_WISP[16]={"................","......k.........",".....krk....k...",".....krrk..krk..","....krllrkkrrk..","...krlllllrrrk..","..krlllllllllrk.","..krllelllellrk.","..krllelllellrk.","..krlllllllllrk.","..krlllkkklllrk.","...krlllllllrk..","....krrrrrrrk...",".....krk.krk....","......k...k.....","................"};
static const char* const D_KNIGHT[16]={"......kkkk......",".....kmmmmk.....","....kmmmmmmk....","....kmkkkkmk....","....kmkeekmk....","....kmmmmmmk....","..kkkkmmmmkkkk..",".kmmmkkmmkkmmmk.","kmmmkmmmmmmkmmmk","kmlmkmmllmmkmlmk","kmmmkmmmmmmkmmmk",".kkkkmmmmmmkkkk.","....kmmkkmmk....","....kmmkkmmk....","...kmmmkkmmmk...","...kkkkkkkkkk..."};
static const char* const D_KING[16]={"....y..yy..y....","....yyyyyyyy....","...kyyryyryyk...","..kppppppppppk..",".kppkkkkkkkkppk.",".kpkkekkkkekkpk.",".kpkkkkkkkkkkpk.",".kppkkkkkkkkppk.","kpppppkkkkpppppk","kppdppppppppdppk","kppdppyyyyppdppk","kppdppppppppdppk","kppdppppppppdppk","kpdppppppppppdpk","kkpppppppppppkk.",".kkkkkkkkkkkkkk."};

static const char* const TOWN_ROWS[12]={
"TTTTTTT,,TTTTTTT",
"T......,,......T",
"T.####.,,.####.T",
"T.#bb#.,,.#bb#.T",
"T.#cc#.,,.#cc#.T",
"T.#bb#.,,.#bb#.T",
"T.#Id#.,,.#dS#.T",
"T....l.,,.l....T",
",,,,,,,,,,,,,,,,",
"T..f...,,...f..T",
"To.....,,......T",
"TTTTTTT,,TTTTTTT"};
#define TOWN_W 16
#define TOWN_H 12
static const u8 FONT[96][7]={
{0,0,0,0,0,0,0},
{4,4,4,4,4,0,4},
{10,10,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{4,4,8,0,0,0,0},
{2,4,8,8,8,4,2},
{8,4,2,2,2,4,8},
{0,0,0,0,0,0,0},
{0,4,4,31,4,4,0},
{0,0,0,0,0,4,8},
{0,0,0,14,0,0,0},
{0,0,0,0,0,0,4},
{1,1,2,4,8,16,16},
{14,17,19,21,25,17,14},
{4,12,4,4,4,4,14},
{14,17,1,2,4,8,31},
{31,2,4,2,1,17,14},
{2,6,10,18,31,2,2},
{31,16,30,1,1,17,14},
{6,8,16,30,17,17,14},
{31,1,2,4,8,8,8},
{14,17,17,14,17,17,14},
{14,17,17,15,1,2,12},
{0,4,0,0,0,4,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{14,17,1,2,4,0,4},
{0,0,0,0,0,0,0},
{14,17,17,31,17,17,17},
{30,17,17,30,17,17,30},
{14,17,16,16,16,17,14},
{30,17,17,17,17,17,30},
{31,16,16,30,16,16,31},
{31,16,16,30,16,16,16},
{14,17,16,23,17,17,15},
{17,17,17,31,17,17,17},
{14,4,4,4,4,4,14},
{7,2,2,2,2,18,12},
{17,18,20,24,20,18,17},
{16,16,16,16,16,16,31},
{17,27,21,21,17,17,17},
{17,25,21,19,17,17,17},
{14,17,17,17,17,17,14},
{30,17,17,30,16,16,16},
{14,17,17,17,21,18,13},
{30,17,17,30,20,18,17},
{15,16,16,14,1,1,30},
{31,4,4,4,4,4,4},
{17,17,17,17,17,17,14},
{17,17,17,17,17,10,4},
{17,17,17,21,21,27,17},
{17,17,10,4,10,17,17},
{17,17,10,4,4,4,4},
{31,1,2,4,8,16,31},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,0,0,0,0,0},
{0,0,8,21,2,0,0},
{0,0,0,0,0,0,0}};


/* ================= UTIL ================= */
static u32 rngs=0x2545F491u;
static u32 rnd32(void){u32 x=rngs;x^=x<<13;x^=x>>17;x^=x<<5;rngs=x;return x;}
static int rnd(int a,int b){return a+(int)(rnd32()%(u32)(b-a+1));}
static int chance(int permille){return (int)(rnd32()%1000)<permille;}
static int imax(int a,int b){return a>b?a:b;}
static int imin(int a,int b){return a<b?a:b;}
static int iabs(int a){return a<0?-a:a;}
static int isqrt(int v){int r=0;while((r+1)*(r+1)<=v)r++;return r;}
static int isin(int a){a&=1023;int s=1;if(a>=512){a-=512;s=-1;}int t=a*(512-a);return s*(256*16*t)/(5*262144-4*t);}
static u32 hash2(int x,int y){u32 h=(u32)x*73856093u^(u32)y*19349663u;h^=h>>13;h*=0x5bd1e995u;h^=h>>15;return h;}
static int vfmt(char*out,int max,const char*f,va_list ap){
  int n=0;
  while(*f&&n<max-1){
    if(*f=='%'&&f[1]){f++;
      if(*f=='s'){const char*s=va_arg(ap,const char*);while(*s&&n<max-1)out[n++]=*s++;}
      else if(*f=='d'){int v=va_arg(ap,int);char tmp[12];int k=0;unsigned u;if(v<0){out[n++]='-';u=(unsigned)(-v);}else u=(unsigned)v;do{tmp[k++]=(char)('0'+u%10);u/=10;}while(u);while(k&&n<max-1)out[n++]=tmp[--k];}
      else out[n++]=*f;
      f++;
    }else out[n++]=*f++;
  }
  out[n]=0;return n;
}
static void sfmt(char*out,int max,const char*f,...){va_list ap;va_start(ap,f);vfmt(out,max,f,ap);va_end(ap);}
static void padTo(char*dst,const char*s,int n){int l=(int)strlen(s);strcpy(dst,s);int sp=imax(1,n-l);for(int i=0;i<sp;i++)dst[l+i]=' ';dst[l+sp]=0;}

/* ================= PALETTE ================= */
#define NONE 0xFFFFFFFFu
static u16 bgPal[256]; static int bgN=16;
static u16 objPal[256]; static int objN=16;
static u8 palDirty=1;
static u16 rgb15(u32 c){return (u16)(((c>>19)&31)|(((c>>11)&31)<<5)|(((c>>3)&31)<<10));}
static int colDist(u16 a,u16 b){int dr=(a&31)-(b&31),dg=((a>>5)&31)-((b>>5)&31),db=((a>>10)&31)-((b>>10)&31);return dr*dr+dg*dg+db*db;}
static int nearest(const u16*pal,int from,int n,u16 v){int best=from,bd=1<<30;for(int i=from;i<n;i++){int d=colDist(pal[i],v);if(d<bd){bd=d;best=i;}}return best;}
static u32 lastBgC=NONE; static u8 lastBgI=0;
static u8 bgIdx(u32 c){
  if(c==lastBgC)return lastBgI;
  u16 v=rgb15(c);int i;
  for(i=16;i<bgN;i++)if(bgPal[i]==v)break;
  if(i==bgN){if(bgN<256){bgPal[bgN++]=v;palDirty=1;}else i=nearest(bgPal,16,256,v);}
  lastBgC=c;lastBgI=(u8)i;return (u8)i;
}
static u32 lastObC=NONE; static u8 lastObI=0;
static u8 objIdx(u32 c){
  if(c==lastObC)return lastObI;
  u16 v=rgb15(c);int i;
  for(i=16;i<objN;i++)if(objPal[i]==v)break;
  if(i==objN){if(objN<256){objPal[objN++]=v;palDirty=1;}else i=nearest(objPal,16,256,v);}
  lastObC=c;lastObI=(u8)i;return (u8)i;
}

/* ================= GAME TYPES ================= */
enum{MODE_TITLE,MODE_MAP,MODE_BATTLE,MODE_END};
enum{MAP_WORLD,MAP_BROOK,MAP_SALT,MAP_CAVE,MAP_KEEP};
enum{SET_WORLD,SET_TOWN,SET_CAVE,SET_KEEP};
enum{D_DOWN,D_UP,D_LEFT,D_RIGHT};
static const s8 DX[4]={0,0,-1,1}, DY[4]={1,-1,0,0};
static const u8 OPP[4]={D_UP,D_DOWN,D_RIGHT,D_LEFT};
enum{PL_HERO,PL_ELDER,PL_VILLAGER,PL_KID,PL_MERCHANT,PL_INN,PL_GUARD,PL_SAILOR,PL_SAGE,PL_WOMAN,PL_COUNT};
static const u32 PALG[PL_COUNT]={0x2f8f5b,0xe6e6ef,0xc0603a,0xe8b84a,0x3a6ad0,0x9a4ab0,0x8a8aa0,0xf0f0f0,0x4a3a8a,0xd88ac0};
static const u32 PALB[PL_COUNT]={0x8a5a30,0x6a5a9a,0x3a6aa0,0xc04a5a,0x5a8a3a,0xd0b070,0x5a5a70,0x2a4a8a,0x8a6ac0,0x4a8a8a};
enum{T_INN_B,T_SHOP_B,T_ELDER,T_GUARD_B,T_VILL_B,T_KID,T_INN_S,T_SHOP_S,T_SAGE,T_SAILOR,T_WOMAN,T_OLD};
typedef struct{s8 x,y,hx,hy;u8 pal,talk,wander,dir;s16 wt;}Npc;
typedef struct{u8 id;s8 x,y;u8 give;s16 gold;}Chest;
enum{GIVE_NONE,GIVE_CHARM,GIVE_SUNSTEEL};
typedef struct{const char*const*rows;u8 w,h,set;char fill;const char*title;Npc*npcs;u8 nn;const Chest*chests;u8 nc;u8 enc,dark;s8 bx,by;}Map;

static Npc brookNpcs[6]={{4,3,4,3,PL_INN,T_INN_B,0,0,0},{11,3,11,3,PL_MERCHANT,T_SHOP_B,0,0,0},{9,9,9,9,PL_ELDER,T_ELDER,0,0,0},
  {9,1,9,1,PL_GUARD,T_GUARD_B,0,0,0},{3,9,3,9,PL_VILLAGER,T_VILL_B,1,0,0},{12,10,12,10,PL_KID,T_KID,1,0,0}};
static Npc saltNpcs[6]={{4,3,4,3,PL_INN,T_INN_S,0,0,0},{11,3,11,3,PL_MERCHANT,T_SHOP_S,0,0,0},{9,9,9,9,PL_SAGE,T_SAGE,0,0,0},
  {3,10,3,10,PL_SAILOR,T_SAILOR,0,0,0},{12,9,12,9,PL_WOMAN,T_WOMAN,1,0,0},{6,1,6,1,PL_ELDER,T_OLD,0,0,0}};
static const Chest caveChests[2]={{0,1,11,GIVE_CHARM,0},{1,18,13,GIVE_NONE,150}};
static const Chest keepChests[2]={{2,1,3,GIVE_SUNSTEEL,0},{3,13,3,GIVE_NONE,400}};
static const Map MAPS[5]={
  {WORLD_ROWS,WORLD_W,WORLD_H,SET_WORLD,'~',0,0,0,0,0,1,0,-1,-1},
  {TOWN_ROWS,TOWN_W,TOWN_H,SET_TOWN,'T',"Brookhollow",brookNpcs,6,0,0,0,0,-1,-1},
  {TOWN_ROWS,TOWN_W,TOWN_H,SET_TOWN,'T',"Saltmere",saltNpcs,6,0,0,0,0,-1,-1},
  {CAVE_ROWS,CAVE_W,CAVE_H,SET_CAVE,'#',"Sunken Grotto",0,0,caveChests,2,1,1,-1,-1},
  {KEEP_ROWS,KEEP_W,KEEP_H,SET_KEEP,'#',"Hollow Keep",0,0,keepChests,2,1,0,7,1},
};
/* metatile chars per set. World extras: 'g' flowers, 'q' tufts, \1..\17 shorelines (mask) */
static const char*const SETCHARS[4]={".T^h:~=ABCK#gq\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17",".fT,#bdcIlSo","#.Sx","#.RSt"};
static const char*const ANIMCHARS[4]={"~=\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17","lo","x","t"};
static const char*const WALK[4]={".Th:=ABCK",".,bdf",".S",".RS"};

/* ---- items, gear, spells, monsters ---- */
typedef struct{const char*name;s16 val;s16 price;}Gear;
static const Gear WEAPONS[6]={{"Wood Stick",2,0},{"Oak Club",5,60},{"Bronze Sword",10,180},{"Iron Blade",16,450},{"Steel Saber",24,1100},{"Sunsteel Sword",34,1600}};
static const Gear ARMORS[5]={{"Traveler Cloth",2,0},{"Leather Vest",5,50},{"Scale Mail",10,170},{"Chain Mail",16,420},{"Plate Armor",24,1050}};
enum{W_STICK,W_CLUB,W_BRONZE,W_IRON,W_STEEL,W_SUN};
enum{GD_HERB=0,GD_FEATHER=1,GD_W=10,GD_A=20};
static const s8 SHOPS[2][6]={{GD_HERB,GD_FEATHER,GD_W+1,GD_W+2,GD_A+1,GD_A+2},{GD_HERB,GD_FEATHER,GD_W+3,GD_W+4,GD_A+3,GD_A+4}};
static const char*goodName(int g){if(g==GD_HERB)return"Herb";if(g==GD_FEATHER)return"Homing Feather";if(g>=GD_A)return ARMORS[g-GD_A].name;return WEAPONS[g-GD_W].name;}
static int goodPrice(int g){if(g==GD_HERB)return 10;if(g==GD_FEATHER)return 25;if(g>=GD_A)return ARMORS[g-GD_A].price;return WEAPONS[g-GD_W].price;}
enum{SP_HEAL,SP_DMG,SP_WARP};
typedef struct{const char*name;u8 lv,mp,type;s16 mn,mx;u8 battle,fx;}Spell;
enum{FX_NONE,FX_SLASH,FX_CRIT,FX_SPARK,FX_BLAZE,FX_HEAL,FX_MHEAL,FX_CLAW,FX_EBOLT,FX_EFIRE,FX_ESPORE,FX_GUARD};
static const Spell SPELLS[5]={{"Mend",3,4,SP_HEAL,22,32,1,FX_HEAL},{"Spark",4,2,SP_DMG,8,13,1,FX_SPARK},{"Return",7,6,SP_WARP,0,0,0,0},{"Blaze",10,5,SP_DMG,28,38,1,FX_BLAZE},{"Restore",13,8,SP_HEAL,80,100,1,FX_HEAL}};
enum{DS_MUSH,DS_BAT,DS_BEETLE,DS_WISP,DS_KNIGHT,DS_KING};
static const char*const*const DESIGNS[6]={D_MUSH,D_BAT,D_BEETLE,D_WISP,D_KNIGHT,D_KING};
typedef struct{const char*name;u8 design;const char*pc;u32 col[6];s16 hp,atk,def,agi,xp,gold;u8 boss;u16 spP;u8 spMin,spMax;const char*spMsg;u8 spFx;u16 healP,guardP;}Mon;
enum{M_MOSS,M_BOGB,M_EMBAT,M_THORNB,M_GLOOM,M_RUST,M_DUSKB,M_BLIGHT,M_GROTTO,M_IRONB,M_HGUARD,M_SHADE,M_GRAVE,M_KING};
static const Mon MONS[14]={
  {"Mossling",DS_MUSH,"rwsemg",{0x4f9a3a,0xd8f0a8,0xf0dcb0,0x1a1420,0xa04040,0x2f6a2a},7,6,3,3,2,3,0,0,0,0,0,0,0,0},
  {"Bog Beetle",DS_BEETLE,"rlwe",{0x6a4a2a,0x9a7a4a,0xf0f0f0,0xc03030},9,8,5,4,3,4,0,0,0,0,0,0,0,0},
  {"Ember Bat",DS_BAT,"rwm",{0xd0602a,0xffe060,0x801818},10,10,5,9,4,6,0,0,0,0,0,0,0,0},
  {"Thorn Beetle",DS_BEETLE,"rlwe",{0x2a6a3a,0x5aa06a,0xf0f0f0,0xe0a020},18,15,11,6,8,12,0,0,0,0,0,0,0,150},
  {"Gloom Wisp",DS_WISP,"rle",{0x3a5ac0,0x9ad0ff,0x10183a},16,13,8,11,9,14,0,300,6,10,"hurls a spark of lightning!",FX_EBOLT,0,0},
  {"Rust Knight",DS_KNIGHT,"mle",{0x9a5a3a,0xd09060,0xffd040},35,26,20,9,20,30,0,0,0,0,0,0,0,200},
  {"Dusk Bat",DS_BAT,"rwm",{0x5a3a8a,0xff6080,0x2a1040},24,22,14,20,15,20,0,0,0,0,0,0,0,0},
  {"Blight Cap",DS_MUSH,"rwsemg",{0x7a3a9a,0xe0b0f0,0xd8c8b0,0x1a1420,0x5a1a3a,0x4a2a5a},30,23,16,8,17,25,0,250,10,15,"spews choking spores!",FX_ESPORE,0,0},
  {"Grotto Wisp",DS_WISP,"rle",{0x2a9a8a,0xb0ffe8,0x0a2a28},28,24,15,15,22,28,0,300,12,18,"hurls a spark of lightning!",FX_EBOLT,250,0},
  {"Iron Beetle",DS_BEETLE,"rlwe",{0x5a6070,0x9aa0b0,0xf0f0f0,0xff5040},40,29,32,6,28,36,0,0,0,0,0,0,0,200},
  {"Hollow Guard",DS_KNIGHT,"mle",{0x4a3a6a,0x8a7ab0,0xff4060},70,44,36,18,50,60,0,0,0,0,0,0,0,250},
  {"Shade Wisp",DS_WISP,"rle",{0x9a1a3a,0xff9a7a,0x2a0610},55,40,28,24,45,55,0,350,20,28,"casts a blazing flame!",FX_EFIRE,300,0},
  {"Gravewing",DS_BAT,"rwm",{0x2a2030,0xff3030,0x801010},50,42,26,32,42,50,0,0,0,0,0,0,0,0},
  {"Hollow King",DS_KING,"yrpde",{0xe0b040,0xd02040,0x3a2458,0x24163a,0xff3050},210,60,44,30,0,0,1,300,26,36,"breathes cold fire!",FX_EFIRE,150,0},
};
static const u8 Z1[]={M_MOSS,M_MOSS,M_BOGB,M_EMBAT}, Z2[]={M_BOGB,M_EMBAT,M_THORNB,M_GLOOM,M_THORNB},
  Z3[]={M_THORNB,M_RUST,M_DUSKB,M_BLIGHT,M_GLOOM}, Z4[]={M_RUST,M_DUSKB,M_HGUARD,M_BLIGHT},
  ZC[]={M_GROTTO,M_IRONB,M_RUST,M_BLIGHT}, ZK[]={M_HGUARD,M_SHADE,M_GRAVE,M_IRONB};
static const u16 XPT[21]={0,0,7,20,45,85,150,250,400,600,850,1150,1500,1950,2500,3200,4000,5000,6200,7600,9200};

/* ================= STATE ================= */
typedef struct{u8 lv;u16 xp;s16 hp,mp;s32 gold;u8 weapon,armor,herb,feather,charm;u8 map;s16 x,y;u8 dir,home;u8 fCharm,fThorns,fBoss;u8 chests[4];}Player;
enum{AN_IDLE,AN_HIT,AN_LUNGE,AN_DIE,AN_GONE};
typedef struct{u8 on,mon,boss,defend,guard;s16 hp,maxhp;int intro,anim,animT,fx,fxT,fxArg;}Battle;
static Player P; static Battle B;
static u8 moving,mt; static s16 mfx,mfy; static int idle;
static int mode=MODE_TITLE, frameT=0, titleSel=0, hasSaveFlag=0, shake=0, grace=0, endT=0, endDone=0, uiFlash=0;
static const char*bannerText=0; static int bannerT=0;
static u8 soundOn=1;
static struct{u8 a,b,start,select,held,prev,dirPress,rep;}inp;
typedef struct{u8 on;int t,dur,mid;s16 a,b,c,d;}Fade;
static Fade fade;
static int posA[2],posB[2],posC[2],posK[2],caveIn[2];
typedef struct{u8 on,green;s16 val,x,y;int t;}Popup;
static Popup pops[2];

typedef struct{s16 maxhp,maxmp,str,agi;}Stats;
static Stats statsFor(int l){Stats s;s.maxhp=(s16)(15+(l-1)*7+(l-1)*(l-1)*15/100);s.maxmp=(s16)(l<3?0:6+(l-3)*5);s.str=(s16)(4+(l-1)*26/10);s.agi=(s16)(4+(l-1)*22/10);return s;}
static int atkV(void){return statsFor(P.lv).str+WEAPONS[P.weapon].val;}
static int defV(void){return statsFor(P.lv).agi/2+ARMORS[P.armor].val;}
static int agiV(void){return statsFor(P.lv).agi;}
static int dmgCalc(int a,int d){int b100=a*50-d*25;if(b100<100)return chance(500)?1:0;return imax(1,b100*(80+rnd(0,40))/10000);}
static int healAmt(int n){return imax(0,imin(n,statsFor(P.lv).maxhp-P.hp));}
static int lowHP(void){return P.hp<statsFor(P.lv).maxhp/4;}

/* ================= SOUND: effects (ch2) + music (ch1) ================= */
typedef struct{u16 f;u8 len;}Note;
enum{SFX_BLIP,SFX_ERR,SFX_HIT,SFX_MISS,SFX_ZAP,SFX_HURT,SFX_RUN,SFX_ENC,SFX_WIN,SFX_LVL,SFX_HEAL,SFX_CHEST,SFX_INN,SFX_DIE,SFX_BUY,SFX_FIRE,SFX_CRIT,SFX_GUARD};
static const Note N_BLIP[]={{990,3},{0,0}},N_ERR[]={{140,8},{0,0}},N_HIT[]={{300,3},{180,3},{90,3},{0,0}},N_MISS[]={{200,3},{0,0}},
  N_ZAP[]={{1400,3},{900,3},{1600,3},{500,4},{0,0}},N_HURT[]={{160,4},{110,4},{70,4},{0,0}},N_RUN[]={{700,3},{500,3},{350,3},{0,0}},
  N_ENC[]={{392,3},{523,3},{659,3},{784,3},{1046,5},{0,0}},N_WIN[]={{523,5},{659,5},{784,5},{1046,9},{0,0}},
  N_LVL[]={{523,7},{523,7},{784,7},{1046,12},{0,0}},N_HEAL[]={{660,4},{880,4},{1175,6},{0,0}},N_CHEST[]={{440,4},{554,4},{659,4},{880,8},{0,0}},
  N_INN[]={{523,10},{659,10},{784,10},{659,10},{784,10},{1046,16},{0,0}},N_DIE[]={{330,13},{262,13},{196,13},{131,20},{0,0}},N_BUY[]={{1320,3},{1760,5},{0,0}},
  N_FIRE[]={{220,3},{180,3},{240,3},{150,3},{200,3},{120,5},{0,0}},N_CRIT[]={{1200,2},{300,3},{1000,2},{150,5},{0,0}},N_GUARD[]={{600,3},{600,3},{800,6},{0,0}};
static const Note*const SFXTAB[]={N_BLIP,N_ERR,N_HIT,N_MISS,N_ZAP,N_HURT,N_RUN,N_ENC,N_WIN,N_LVL,N_HEAL,N_CHEST,N_INN,N_DIE,N_BUY,N_FIRE,N_CRIT,N_GUARD};
static const Note*sfxCur=0; static int sfxT=0;
static void playNote(void){
  sfxT=sfxCur->len;
  if(!sfxCur->f){REG_SND2L=0;REG_SND2H=0x8000;return;}
  int rate=2048-131072/sfxCur->f; if(rate<0)rate=0;
  REG_SND2L=(u16)(0x0080|((sfxCur->len>8?4:2)<<8)|(12<<12));
  REG_SND2H=(u16)(rate|0x8000);
}
static void sfx(int id){if(!soundOn)return;sfxCur=SFXTAB[id];playNote();}
static void sfxTick(void){if(!sfxCur)return;if(--sfxT>0)return;sfxCur++;if(!sfxCur->len){sfxCur=0;return;}playNote();}

/* Original melodies. Notes in Hz. */
enum{C3=131,D3=147,E3=165,F3=175,G3=196,A3=220,B3=247,C4=262,D4=294,E4=330,F4=349,G4=392,A4=440,BB4=466,B4=494,C5=523,D5=587,E5=659,F5=698,G5=784,A5=880,R0=0};
static const Note MU_TITLE[]={{C5,4},{G4,4},{E5,4},{D5,4},{C5,4},{A4,4},{G4,8},{F4,4},{A4,4},{C5,4},{B4,4},{C5,8},{R0,8},
  {E5,4},{D5,4},{C5,4},{G4,4},{A4,4},{B4,4},{C5,8},{D5,4},{B4,4},{G4,4},{B4,4},{C5,12},{R0,4},{0,0}};
static const Note MU_FIELD[]={{E4,2},{G4,2},{C5,4},{B4,2},{A4,2},{G4,4},{A4,2},{G4,2},{E4,2},{D4,2},{E4,4},{R0,4},
  {F4,2},{A4,2},{D5,4},{C5,2},{B4,2},{A4,4},{G4,2},{E4,2},{F4,2},{D4,2},{C4,4},{R0,4},
  {C4,2},{E4,2},{G4,2},{C5,2},{B4,4},{G4,4},{A4,2},{C5,2},{B4,2},{A4,2},{G4,6},{R0,2},
  {F4,2},{E4,2},{D4,2},{F4,2},{E4,2},{D4,2},{C4,2},{E4,2},{D4,4},{G3,4},{C4,6},{R0,2},{0,0}};
static const Note MU_TOWN[]={{F4,3},{A4,3},{C5,6},{C5,3},{A4,3},{F4,6},{G4,3},{A4,3},{BB4,3},{A4,3},{G4,6},
  {F4,3},{A4,3},{C5,3},{D5,3},{C5,6},{A4,3},{G4,3},{F4,9},{R0,3},{0,0}};
static const Note MU_CAVE[]={{A3,4},{C4,4},{E4,4},{D4,4},{C4,4},{B3,4},{A3,8},{E4,4},{F4,4},{E4,4},{D4,4},{C4,8},{B3,8},
  {A3,4},{E3,4},{A3,4},{B3,4},{C4,4},{D4,4},{E4,8},{D4,4},{C4,4},{B3,4},{G3,4},{A3,12},{R0,4},{0,0}};
static const Note MU_BATTLE[]={{A4,1},{A4,1},{C5,1},{A4,1},{E5,2},{D5,1},{C5,1},{B4,1},{B4,1},{D5,1},{B4,1},{E5,2},{C5,1},{B4,1},
  {A4,1},{A4,1},{C5,1},{E5,1},{A5,2},{G5,1},{F5,1},{E5,1},{D5,1},{C5,1},{B4,1},{A4,2},{E4,2},
  {F4,1},{A4,1},{C5,1},{F5,1},{E5,2},{C5,2},{D5,1},{B4,1},{G4,1},{B4,1},{E5,2},{R0,2},{0,0}};
static const Note MU_BOSS[]={{D4,2},{D4,1},{F4,1},{A4,2},{G4,2},{F4,1},{E4,1},{D4,2},{C4,2},{D4,2},{A3,2},
  {D4,1},{F4,1},{A4,1},{C5,1},{D5,4},{C5,2},{A4,2},{F4,2},{E4,2},{D4,4},
  {BB4,2},{A4,2},{G4,2},{F4,2},{E4,2},{F4,2},{G4,2},{A4,4},{D4,2},{E4,2},{F4,2},{E4,2},{D4,6},{R0,2},{0,0}};
enum{MUS_TITLE,MUS_FIELD,MUS_TOWN,MUS_CAVE,MUS_BATTLE,MUS_BOSS};
static const Note*const MUSICS[]={MU_TITLE,MU_FIELD,MU_TOWN,MU_CAVE,MU_BATTLE,MU_BOSS};
static const u8 MUSUNIT[]={9,8,9,10,7,9};
static int musId=-1,musPos=0,musT=0;
static void musicStop(void){REG_SND1CNT=0;REG_SND1FREQ=0x8000;}
static void musicSet(int id){if(id==musId)return;musId=id;musPos=0;musT=0;musicStop();}
static void musicTick(void){
  if(musId<0||!soundOn)return;
  if(musT>0){musT--;return;}
  const Note*n=&MUSICS[musId][musPos];
  if(!n->len){musPos=0;n=&MUSICS[musId][0];}
  musT=n->len*MUSUNIT[musId]-1;
  if(n->f){int rate=2048-131072/n->f;REG_SND1SWEEP=0x0008;REG_SND1CNT=(u16)(0x0040|(3<<8)|(7<<12));REG_SND1FREQ=(u16)(rate|0x8000);}
  else musicStop();
  musPos++;
}

/* ================= SAVE ================= */
#define SAVE_MAGIC 0x53534731u
typedef struct{u32 magic;Player p;u32 sum;}SaveData;
static u32 sumOf(const u8*b,int n){u32 s=0x1234;for(int i=0;i<n;i++)s=s*31+b[i];return s;}
static int doSave(void){
  SaveData d;memset(&d,0,sizeof d);d.magic=SAVE_MAGIC;d.p=P;d.sum=sumOf((const u8*)&d.p,sizeof(Player));
  const u8*b=(const u8*)&d;for(int i=0;i<(int)sizeof d;i++)SRAM8[i]=b[i];
  for(int i=0;i<(int)sizeof d;i++)if(SRAM8[i]!=b[i])return 0;
  return 1;
}
static int loadSave(Player*out){
  SaveData d;u8*b=(u8*)&d;for(int i=0;i<(int)sizeof d;i++)b[i]=SRAM8[i];
  if(d.magic!=SAVE_MAGIC||d.sum!=sumOf((const u8*)&d.p,sizeof(Player)))return 0;
  if(d.p.map>MAP_KEEP||d.p.lv<1||d.p.lv>20)return 0;
  *out=d.p;return 1;
}

/* ================= MESSAGE QUEUE ================= */
enum{I_SAY,I_ACT,I_ASK};
typedef struct{u8 type,code;s16 a,b;char t[168];}Item;
#define QN 48
#define q ((Item*)QBUF)
static int qh=0,qn=0;
static Item*qpush(void){Item*it=&q[(qh+qn)%QN];if(qn<QN)qn++;return it;}
static Item*qpushFront(void){qh=(qh+QN-1)%QN;if(qn<QN)qn++;return &q[qh];}
static void say(const char*s){Item*it=qpush();it->type=I_SAY;strncpy(it->t,s,167);it->t[167]=0;}
static void sayf(const char*f,...){Item*it=qpush();it->type=I_SAY;va_list ap;va_start(ap,f);vfmt(it->t,168,f,ap);va_end(ap);}
static void act(int code,int a,int b){Item*it=qpush();it->type=I_ACT;it->code=(u8)code;it->a=(s16)a;it->b=(s16)b;}
static void ask(int menu,int a,int b){Item*it=qpush();it->type=I_ASK;it->code=(u8)menu;it->a=(s16)a;it->b=(s16)b;}

#define LW 27
typedef struct{u8 active,typing,waiting;char lines[4][32];int nlines;char nl[4][32];int nn;int chars,total;}Box;
static Box box;
typedef struct{u8 menu;s16 a,b;char o[8][24];s16 v[8];u8 n,sel;s16 cancel;u8 hasCancel,top,battle,status,gold;}Choice;
static Choice ch; static u8 choiceOn=0;
static void clearBox(void){box.nlines=0;box.nn=0;box.typing=0;box.waiting=0;box.chars=0;box.total=0;}
static int busy(void){return qn>0||choiceOn||box.typing||box.waiting;}

static int wrapText(const char*t,char out[][32],int maxl){
  int n=0;char line[64];int ll=0;const char*p=t;
  for(;;){
    const char*e=p;while(*e&&*e!=' '&&*e!='\n')e++;
    int wl=(int)(e-p);if(wl>LW)wl=LW;
    if(ll==0){memcpy(line,p,wl);ll=wl;}
    else if(ll+1+wl<=LW){line[ll++]=' ';memcpy(line+ll,p,wl);ll+=wl;}
    else{if(n<maxl){memcpy(out[n],line,ll);out[n][ll]=0;n++;}memcpy(line,p,wl);ll=wl;}
    if(*e==' '){p=e+1;continue;}
    if(n<maxl){memcpy(out[n],line,ll);out[n][ll]=0;n++;}
    ll=0;
    if(*e=='\n'){p=e+1;continue;}
    break;
  }
  return n;
}
static void startText(const char*t){
  static char tmp[16][32];
  int n=wrapText(t,tmp,16);
  if(n>4){
    Item*it=qpushFront();it->type=I_SAY;it->t[0]=0;
    for(int i=4;i<n;i++){strcat(it->t,tmp[i]);if(i<n-1)strcat(it->t,"\n");}
    n=4;
  }
  box.active=1;
  int keep=4-n;
  if(box.nlines>keep){int drop=box.nlines-keep;for(int i=0;i<keep;i++)strcpy(box.lines[i],box.lines[i+drop]);box.nlines=keep;}
  box.total=0;for(int i=0;i<n;i++){strcpy(box.nl[i],tmp[i]);box.total+=(int)strlen(tmp[i]);}
  box.nn=n;box.chars=0;box.typing=1;box.waiting=0;
}
static void mergeBox(void){
  for(int i=0;i<box.nn;i++){
    if(box.nlines==4){for(int k=0;k<3;k++)strcpy(box.lines[k],box.lines[k+1]);box.nlines=3;}
    strcpy(box.lines[box.nlines++],box.nl[i]);
  }
  box.nn=0;box.chars=0;box.total=0;
}

static void doAct(int code,int a,int b);
static void buildChoice(int menu,int a,int b);
static void onChoice(int menu,int v,int a,int b);
static void startFade(int mid,int dur,int a,int b,int c,int d);

static int battleBlocking(void){
  if(mode!=MODE_BATTLE||!B.on)return 0;
  if(B.intro>0||B.fx)return 1;
  if(B.anim==AN_LUNGE||B.anim==AN_DIE)return 1;
  return 0;
}
static void pump(void){
  while(!fade.on&&!choiceOn&&!box.typing&&!box.waiting&&qn){
    if(battleBlocking())return;
    Item it=q[qh];qh=(qh+1)%QN;qn--;
    if(it.type==I_ACT){doAct(it.code,it.a,it.b);continue;}
    if(it.type==I_ASK){buildChoice(it.code,it.a,it.b);choiceOn=1;break;}
    startText(it.t);break;
  }
}
static void handleUI(void){
  if(choiceOn){
    int n=ch.n;
    if(inp.dirPress==D_UP+1){ch.sel=(u8)((ch.sel+n-1)%n);sfx(SFX_BLIP);}
    else if(inp.dirPress==D_DOWN+1){ch.sel=(u8)((ch.sel+1)%n);sfx(SFX_BLIP);}
    else if(inp.a){choiceOn=0;sfx(SFX_BLIP);onChoice(ch.menu,ch.v[ch.sel],ch.a,ch.b);}
    else if(inp.b&&ch.hasCancel){choiceOn=0;sfx(SFX_BLIP);onChoice(ch.menu,ch.cancel,ch.a,ch.b);}
    return;
  }
  if(box.typing){if(inp.a||inp.b)box.chars=box.total;return;}
  if(box.waiting&&(inp.a||inp.b)){box.waiting=0;mergeBox();}
}

/* ================= GRAPHICS GENERATION ================= */
static u32 pix[256];
static void R(u32 c,int x,int y,int w,int h){for(int j=y;j<y+h;j++)if(j>=0&&j<16)for(int i=x;i<x+w;i++)if(i>=0&&i<16)pix[j*16+i]=c;}
static void pts(u32 c,const s8*xy,int n){for(int i=0;i<n;i++)R(c,xy[i*2],xy[i*2+1],1,1);}
static void grass2(u32 b,u32 s){static const s8 g[]={2,3,9,1,13,6,5,10,11,12,1,13,7,6,14,14};R(b,0,0,16,16);for(int i=0;i<8;i++)R(s,g[i*2],g[i*2+1],1,2);}
static void grass(void){grass2(0x4a9a3c,0x3b8231);}
static void tree(int x,int y){R(0x0e2a14,x+1,y+8,7,1);R(0x5a3a1e,x+3,y+6,2,3);R(0x3a2412,x+4,y+6,1,3);R(0x1d5a2a,x+1,y,6,7);R(0x1d5a2a,x,y+1,8,5);R(0x2f7a37,x+1,y+1,4,3);R(0x4aa050,x+2,y+1,2,1);R(0x154420,x+5,y+4,2,2);}
static void mountain(void){grass();for(int y=1;y<16;y++){int hw=imax(1,((y-1)*55+50)/100);R(0x837c70,8-hw,y,hw*2,1);R(0x625c52,8,y,hw,1);R(0x9a9488,8-hw,y,1,1);if(y<5)R(0xf4f4f4,8-hw,y,hw*2-(y>2?1:0),1);}R(0x625c52,5,9,2,1);R(0x4e483f,10,12,2,1);}
static void water(int f){int o=f?2:0;R(0x2b5db3,0,0,16,16);R(0x3366c0,0,(4+f)%16,16,1);R(0x5b8ee0,1+o,3,4,1);R(0x5b8ee0,9-o,8,5,1);R(0x5b8ee0,3+o,13,4,1);R(0x3f70c8,11-o,1,3,1);R(0x3f70c8,10+o,12,3,1);R(0x8ab4f0,2+o,3,1,1);}
static void house(int x,int y,u32 roof){R(roof,x+2,y,3,1);R(roof,x+1,y+1,5,1);R(roof,x,y+2,7,1);R(0xecdfba,x+1,y+3,5,4);R(0x5a3a1e,x+3,y+5,1,2);R(0x7ab0e0,x+1,y+4,1,1);R(0x7ab0e0,x+5,y+4,1,1);R(0x6a5a4a,x+1,y+7,6,1);}
static void hump(int cx,int cy){static const int ws[4]={2,4,6,8};for(int i=0;i<4;i++)R(0x2f6e28,cx-ws[i]/2-1,cy-3+i,ws[i]+2,1);for(int i=0;i<4;i++)R(0x76c657,cx-ws[i]/2,cy-2+i,ws[i],1);R(0x9ad878,cx-1,cy-1,2,1);}
static void stairs(u32 b){R(b,0,0,16,16);R(0x141010,2,2,12,12);R(0x8e877c,3,3,10,3);R(0xb0a99c,3,3,10,1);R(0x8e877c,4,7,8,3);R(0xb0a99c,4,7,8,1);R(0x8e877c,5,11,6,3);R(0xb0a99c,5,11,6,1);}
static void bricks(u32 base,u32 mortar,u32 hi){R(base,0,0,16,16);for(int r=0;r<4;r++){R(mortar,0,r*4+3,16,1);int o=r%2?0:4;R(mortar,o,r*4,1,3);R(mortar,o+8,r*4,1,3);if(hi!=NONE)R(hi,o+1,r*4,6,1);}}
static void shore(int mask,int f){
  water(f);u32 sand=0xd9c88c,foam=0xeaf4ff;
  if(mask&1){R(sand,0,0,16,2);for(int x=0;x<16;x++)if((x+f*2)%4!=3)R(foam,x,2,1,1);}
  if(mask&4){R(sand,0,14,16,2);for(int x=0;x<16;x++)if((x+f*2)%4!=3)R(foam,x,13,1,1);}
  if(mask&8){R(sand,0,0,2,16);for(int y=0;y<16;y++)if((y+f*2)%4!=3)R(foam,2,y,1,1);}
  if(mask&2){R(sand,14,0,2,16);for(int y=0;y<16;y++)if((y+f*2)%4!=3)R(foam,13,y,1,1);}
}
static void makeTile(int set,char c,int f){
  for(int i=0;i<256;i++)pix[i]=0x000000;
  if(set==SET_WORLD){
    if(c>=1&&c<=15){shore(c,f);return;}
    switch(c){
      case '.':grass();break;
      case 'g':grass();R(0xf05a6a,3,4,2,2);R(0xffe066,10,9,2,2);R(0xffffff,12,3,1,1);R(0xf05a6a,6,12,1,1);R(0x2f6a28,3,6,2,1);R(0x2f6a28,10,11,2,1);break;
      case 'q':grass();R(0x6fbf52,3,6,1,3);R(0x6fbf52,5,5,1,4);R(0x6fbf52,7,6,1,3);R(0x6fbf52,10,10,1,3);R(0x6fbf52,12,9,1,4);break;
      case 'T':grass2(0x3f8a36,0x3b8231);tree(0,0);tree(8,6);break;
      case '^':mountain();break;
      case 'h':grass();hump(5,9);hump(11,13);break;
      case ':':{static const s8 d[]={2,2,7,5,12,3,4,11,10,9,14,13,6,14};R(0xd9c27c,0,0,16,16);pts(0xbba15e,d,7);R(0xe8d494,3,8,4,1);R(0xe8d494,9,13,4,1);}break;
      case '~':water(f);break;
      case '=':water(f);R(0x9a6a3a,0,3,16,10);for(int x=3;x<16;x+=4)R(0x6a4422,x,4,1,8);R(0x5a3a1e,0,3,16,1);R(0x5a3a1e,0,12,16,1);R(0xb88a52,0,4,16,1);break;
      case 'A':grass();house(1,8,0xb5402f);house(8,3,0xb5402f);break;
      case 'B':grass();house(1,3,0x3a6ab5);house(8,8,0x3a6ab5);break;
      case 'C':mountain();R(0x0b0808,6,10,5,6);R(0x0b0808,7,9,3,1);R(0x3a3028,5,10,1,6);break;
      case 'K':R(0x2a2236,0,0,16,16);R(0x6c5c80,4,4,8,12);R(0x6c5c80,3,2,2,3);R(0x6c5c80,7,2,2,3);R(0x6c5c80,11,2,2,3);R(0x4e4060,10,4,2,12);R(0xff4a6a,7,6,2,3);R(0x0e0a14,6,12,4,4);R(0x8a7aa0,4,4,1,12);break;
      case '#':R(0x22122a,0,0,16,16);for(int i=0;i<16;i++){R(0x7a3a8a,i,(i*5)%16,1,1);R(0x7a3a8a,(i*3+5)%16,i,1,1);R(0x5a2a6a,i,(i*5+1)%16,1,1);R(0xb26ac4,(i*7+2)%16,(i*11)%16,1,1);}break;
    }
  }else if(set==SET_TOWN){
    switch(c){
      case '.':grass2(0x5aa84a,0x4a9440);break;
      case 'f':grass2(0x5aa84a,0x4a9440);R(0xf05a6a,3,4,2,2);R(0xffe066,10,9,2,2);R(0xf05a6a,6,12,2,2);R(0xffffff,12,3,2,2);R(0xffe066,2,11,1,1);break;
      case 'T':grass2(0x5aa84a,0x4a9440);tree(4,3);break;
      case ',':R(0xb9ab8b,0,0,16,16);for(int r=0;r<4;r++){R(0x9a8c6e,0,r*4+3,16,1);int o=r%2?3:7;R(0x9a8c6e,o,r*4,1,3);R(0x9a8c6e,o+8,r*4,1,3);R(0xcabc9c,o+1,r*4,2,1);}break;
      case '#':bricks(0x8c7c6a,0x5e5246,0xa89a86);break;
      case 'b':R(0xb07c4a,0,0,16,16);for(int r=0;r<4;r++){R(0x8c5e34,0,r*4+3,16,1);R(0x8c5e34,(r*5)%16,r*4,1,3);}break;
      case 'd':R(0x5a3c22,0,0,16,16);R(0x7a5434,2,0,12,16);R(0x3a2414,2,0,12,2);R(0xe0b040,11,8,1,2);break;
      case 'c':R(0xb07c4a,0,0,16,16);R(0x7a4a26,0,2,16,12);R(0xa86a3a,0,2,16,3);R(0x4a2a14,0,13,16,1);break;
      case 'I':bricks(0x8c7c6a,0x5e5246,0xa89a86);R(0x3a2414,2,1,12,1);R(0x3a2414,3,1,1,3);R(0x3a2414,12,1,1,3);R(0x3a2414,2,4,12,9);R(0xe8d0a0,3,5,10,7);
        R(0x3a6ab5,5,8,6,2);R(0xffffff,5,7,2,1);R(0x6a4422,4,10,1,1);R(0x6a4422,11,10,1,1);break;
      case 'S':bricks(0x8c7c6a,0x5e5246,0xa89a86);R(0x3a2414,2,1,12,1);R(0x3a2414,3,1,1,3);R(0x3a2414,12,1,1,3);R(0x3a2414,2,4,12,9);R(0xe8d0a0,3,5,10,7);
        R(0xe0b040,6,6,4,5);R(0xe0b040,5,7,6,3);R(0xfff0a0,6,7,1,2);R(0xa07a20,9,8,1,2);break;
      case 'l':grass2(0x5aa84a,0x4a9440);R(0x22303a,7,15,4,1);R(0x2a2a3a,7,5,2,10);R(0x2a2a3a,5,1,6,5);R(f?0xffc850:0xffe890,6,2,4,3);R(0x2a2a3a,7,0,2,1);R(0x3a8a3a,5,15,6,1);break;
      case 'o':grass2(0x5aa84a,0x4a9440);R(0x5a3a1e,2,1,1,6);R(0x5a3a1e,13,1,1,6);R(0xb5402f,1,0,14,2);R(0x8c8c96,2,6,12,8);R(0x6c6c78,2,13,12,1);
        R(0x2b5db3,4,8,8,4);R(f?0x8ab4f0:0x5b8ee0,5+f*3,9,2,1);R(0xa8a8b4,2,6,12,1);break;
    }
  }else if(set==SET_CAVE){
    switch(c){
      case '#':R(0x2a201c,0,0,16,16);R(0x43352c,2,2,5,4);R(0x43352c,9,8,5,5);R(0x43352c,1,11,4,3);R(0x57463a,3,2,2,1);R(0x57463a,10,8,2,1);R(0x18120f,0,15,16,1);break;
      case '.':{static const s8 d[]={3,2,11,4,6,9,13,12,2,13,9,14};R(0x57514b,0,0,16,16);pts(0x47423d,d,6);}break;
      case 'S':stairs(0x57514b);break;
      case 'x':R(0x2a201c,0,0,16,16);R(0x43352c,1,9,6,5);R(0x2a8aa8,4,4,3,9);R(0x60e0ff,5,3,2,9);R(0x2a8aa8,9,6,3,7);R(0x60e0ff,10,5,1,7);R(0x2a8aa8,12,9,2,4);
        R(f?0xffffff:0xb0f4ff,5+f*5,4+f*2,1,1);R(0x18120f,0,15,16,1);break;
    }
  }else{
    switch(c){
      case '#':bricks(0x3a2a4c,0x271b36,0x46365a);break;
      case '.':R(0x4c3e5e,0,0,16,16);R(0x3e3150,0,0,16,1);R(0x3e3150,0,0,1,16);R(0x5a4c6e,1,1,1,1);break;
      case 'R':{static const s8 d[]={5,3,10,7,6,12};R(0x8e1f30,0,0,16,16);R(0xc9a24a,1,0,1,16);R(0xc9a24a,14,0,1,16);pts(0x6e1424,d,3);}break;
      case 'S':stairs(0x4c3e5e);break;
      case 't':bricks(0x3a2a4c,0x271b36,0x46365a);R(0x5a4a3a,6,9,4,1);R(0x5a4a3a,7,9,2,5);R(0xff6020,6,4-f,4,5+f);R(0xffb040,7,5-f,2,3+f);R(0xfff0a0,7,7,2,1);break;
    }
  }
}
static void put8(volatile u16*dst,const u8*t){for(int i=0;i<32;i++)dst[i]=(u16)(t[2*i]|(t[2*i+1]<<8));}
static void metaToBytes(u8 out[256],int obj){
  for(int qd=0;qd<4;qd++){int ox=(qd&1)*8,oy=(qd>>1)*8;
    for(int y=0;y<8;y++)for(int x=0;x<8;x++){u32 c=pix[(oy+y)*16+ox+x];out[qd*64+y*8+x]=(c==NONE)?0:(obj?objIdx(c):bgIdx(c));}}
}
static void uploadBytes(volatile u16*base,int ti,const u8*b,int ntiles){for(int k=0;k<ntiles;k++)put8(base+(ti+k)*32,b+k*64);}
#define CB0 (VRAM16)
#define CB1 (VRAM16+0x2000)
#define CB2 (VRAM16+0x4000)
#define OBJV (VRAM16+0x8000)

/* generic sprite buffer (up to 64x64) uploaded as 8bpp 1D tiles */
static int GW=16,GH=16;
static void gclear(int w,int h){GW=w;GH=h;for(int i=0;i<w*h;i++)GBUF[i]=NONE;}
static void gp(int x,int y,u32 c){if(x>=0&&y>=0&&x<GW&&y<GH)GBUF[y*GW+x]=c;}
static void gr(int x,int y,int w,int h,u32 c){for(int j=0;j<h;j++)for(int i=0;i<w;i++)gp(x+i,y+j,c);}
static void gline(int x0,int y0,int x1,int y1,int th,u32 c){
  int dx=x1-x0,dy=y1-y0,n=imax(iabs(dx),iabs(dy));if(!n)n=1;
  for(int i=0;i<=n;i++){int x=x0+dx*i/n,y=y0+dy*i/n;gr(x-th/2,y-th/2,th,th,c);}
}
static void gupload(int slot){
  u8 t[64];int tw=GW/8,th=GH/8;
  for(int ty=0;ty<th;ty++)for(int tx=0;tx<tw;tx++){
    for(int y=0;y<8;y++)for(int x=0;x<8;x++){u32 c=GBUF[(ty*8+y)*GW+tx*8+x];t[y*8+x]=c==NONE?0:objIdx(c);}
    put8(OBJV+slot*128+(ty*tw+tx)*32,t);
  }
}

/* ---- UI tiles in charblock 1 ---- */
enum{TL_GLYPH=1,TL_B_TL=97,TL_B_T,TL_B_TR,TL_B_L,TL_B_R,TL_B_BL,TL_B_B,TL_B_BR,TL_CURSOR,TL_ARROW,TL_GLYPH2=107,TL_B2_TL=203};
static void glyphTile(u8*t,int g,u8 fg){for(int y=0;y<8;y++)for(int x=0;x<8;x++){int on=0;if(y<7&&x>=1&&x<=5)on=(FONT[g][y]>>(4-(x-1)))&1;t[y*8+x]=on?fg:1;}}
static void borderTiles(int base,u8 fg){
  u8 t[64];
  for(int k=0;k<8;k++){
    for(int y=0;y<8;y++)for(int x=0;x<8;x++){int w=0;
      switch(k){case 0:w=(y==2&&x>=2)||(x==2&&y>=2);break;case 1:w=(y==2);break;case 2:w=(y==2&&x<=5)||(x==5&&y>=2);break;
        case 3:w=(x==2);break;case 4:w=(x==5);break;case 5:w=(y==5&&x>=2)||(x==2&&y<=5);break;case 6:w=(y==5);break;case 7:w=(y==5&&x<=5)||(x==5&&y<=5);break;}
      t[y*8+x]=w?fg:1;}
    put8(CB1+(base+k)*32,t);
  }
}
static void uploadUI(void){
  u8 t[64];
  memset(t,0,64);put8(CB1,t);
  for(int g=0;g<96;g++){glyphTile(t,g,2);put8(CB1+(TL_GLYPH+g)*32,t);glyphTile(t,g,4);put8(CB1+(TL_GLYPH2+g)*32,t);}
  borderTiles(TL_B_TL,2);borderTiles(TL_B2_TL,4);
  for(int i=0;i<64;i++)t[i]=1;
  for(int i=0;i<4;i++)for(int y=i;y<=7-i;y++)t[y*8+1+i]=2;
  put8(CB1+TL_CURSOR*32,t);
  for(int i=0;i<64;i++)t[i]=1;
  for(int x=1;x<=5;x++)t[3*8+x]=2;
  for(int x=2;x<=4;x++)t[4*8+x]=2;
  t[5*8+3]=2;
  put8(CB1+TL_ARROW*32,t);
}

/* ---- people sprites ---- */
static void heroRows(int dir,int f,char r[16][17]){
  int n=0;
#define PUTR(s) do{memcpy(r[n],(s),16);r[n][16]=0;n++;}while(0)
  if(dir==D_LEFT||dir==D_RIGHT){
    static const char*const S[13]={"......kkkk......",".....kggggkk....","....kggggggk....","....kggggkkkk...","....kgggkssssk..","....kggksssesk..",
      "....kgggksssk...",".....kggkkkk....",".....kgbbbbk....","....kggbyybk....","....kgkbbbbsk...",".....kkbbbbk....",".....kbbbbbk...."};
    for(int i=0;i<13;i++)PUTR(S[i]);
    if(f==0){PUTR(".....kddkddk....");PUTR("....kddk.kddk...");PUTR("....kkkk.kkkk...");}
    else{PUTR(".....kdddddk....");PUTR("......kddk......");PUTR("......kkkk......");}
    if(dir==D_LEFT)for(int i=0;i<16;i++)for(int a=0,b=15;a<b;a++,b--){char c=r[i][a];r[i][a]=r[i][b];r[i][b]=c;}
    return;
  }
  PUTR("......kkkk......");PUTR("....kkggggkk....");PUTR("...kggggggggk...");
  if(dir==D_UP){for(int i=0;i<4;i++)PUTR("..kggggggggggk..");PUTR("...kggggggggk...");}
  else{PUTR("..kggkkkkkkggk..");PUTR("..kgksssssskgk..");PUTR("..kgksesseskgk..");PUTR("..kgksssssskgk..");PUTR("...kgkkkkkkgk...");}
  PUTR("..kggbbbbbbggk..");PUTR(".kgggbbyybbgggk.");PUTR(".kskgbbbbbbgksk.");PUTR("..kkgbbbbbbgkk..");PUTR("...kbbbbbbbbk...");
  if(f==0){PUTR("...kddkkkkddk...");PUTR("...kddk..kddk...");PUTR("...kkkk..kkkk...");}
  else{PUTR("...kddkkkkddk...");PUTR("..kddk....kddk..");PUTR("..kkkk....kkkk..");}
#undef PUTR
}
static u32 personColor(int p,char c){
  switch(c){case 'k':case 'e':return 0x17121f;case 's':return 0xf3c9a0;case 'y':return p==PL_GUARD?0xc0c0d0:0xe8c547;case 'd':return 0x3a2f52;case 'g':return PALG[p];case 'b':return PALB[p];}
  return NONE;
}
static void uploadPerson(int p,int slotBase){
  char r[16][17];u8 b[256];
  for(int d=0;d<4;d++)for(int f=0;f<2;f++){
    heroRows(d,f,r);
    for(int y=0;y<16;y++)for(int x=0;x<16;x++)pix[y*16+x]=personColor(p,r[y][x]);
    metaToBytes(b,1);uploadBytes(OBJV,(slotBase+d*2+f)*4,b,4);
  }
}
static u32 monColor(int m,char c){
  if(c=='k')return 0x120e18;
  const Mon*M=&MONS[m];
  for(int i=0;M->pc[i];i++)if(M->pc[i]==c)return M->col[i];
  return NONE;
}
#define SLOT_HERO 0
#define SLOT_NPC 8
#define SLOT_CHEST 56
#define SLOT_CHESTO 57
#define SLOT_BOSS 58
#define SLOT_SLASH 60
#define SLOT_BOLT 72
#define SLOT_FLAME 76
#define SLOT_STAR 78
#define SLOT_MON 80
#define SLOT_CLAW 96
#define SLOT_SHIELD 100
#define SLOT_DIGIT 104
static void uploadChest(int open,int slot){
  gclear(16,16);
  if(open){gr(1,1,14,4,0x2a1810);gr(2,2,12,2,0xa8682e);gr(1,5,14,10,0x2a1810);gr(2,6,12,2,0x120a06);gr(2,8,12,6,0x8a5428);gr(2,8,12,1,0xe0b040);}
  else{gr(1,4,14,11,0x2a1810);gr(2,5,12,3,0xa8682e);gr(3,5,10,1,0xc8884e);gr(2,8,12,6,0x8a5428);gr(2,8,12,1,0xe0b040);gr(7,7,2,4,0xe0b040);gr(7,9,2,1,0x2a1810);}
  gupload(slot);
}
static void uploadMonSmall(int m,int slot){
  const char*const*d=DESIGNS[MONS[m].design];gclear(16,16);
  for(int y=0;y<16;y++)for(int x=0;x<16;x++)gp(x,y,monColor(m,d[y][x]));
  gupload(slot);
}
static void uploadMonBig(int m){
  const char*const*d=DESIGNS[MONS[m].design];int sc=MONS[m].boss?4:3;int ox=(64-16*sc)/2,oy=64-16*sc;
  gclear(64,64);
  for(int y=0;y<16*sc;y++)for(int x=0;x<16*sc;x++)gp(ox+x,oy+y,monColor(m,d[y/sc][x/sc]));
  gupload(SLOT_MON);
}
static void uploadEffects(void){
  /* slash: 3 frames 32x32 */
  for(int f=0;f<3;f++){
    gclear(32,32);int len=f==0?14:28;
    for(int t=0;t<=len;t++){int x=29-t,y=2+t+(t*(28-t))/60;
      if(f<2){gr(x-1,y-1,4,4,0x7ac8ff);gr(x,y,2,2,0xffffff);}else if(t%2==0)gp(x,y,0xa0d8ff);}
    gupload(SLOT_SLASH+f*4);
  }
  /* lightning bolt 16x32, 2 frames */
  static const s8 bx[2][6]={{8,4,11,5,10,7},{8,12,5,11,6,9}};
  for(int f=0;f<2;f++){
    gclear(16,32);
    for(int i=0;i<5;i++)gline(bx[f][i],i*7,bx[f][i+1],(i+1)*7,3,0xfff070);
    for(int i=0;i<5;i++)gline(bx[f][i],i*7,bx[f][i+1],(i+1)*7,1,0xffffff);
    gupload(SLOT_BOLT+f*2);
  }
  /* flame 16x16, 2 frames */
  for(int f=0;f<2;f++){
    gclear(16,16);
    for(int y=2;y<16;y++){int w=imin(7,(y-1)*7/10+(y>11?0:0));if(y>12)w=7-(y-12)*2;int cx=8+((y<6)?(f?1:-1):0);
      for(int x=-w;x<=w;x++){int ax=iabs(x);u32 c=ax>w-2?0xd02810:ax>w-4?0xff8020:0xfff070;if(y<5&&ax>=w-1)c=0xd02810;gp(cx+x,y,c);}}
    gupload(SLOT_FLAME+f);
  }
  /* star sparkle 16x16, 2 frames */
  for(int f=0;f<2;f++){
    gclear(16,16);int r=f?4:7;
    for(int i=-r;i<=r;i++){u32 c=iabs(i)<2?0xffffff:0x9fffc0;gp(8+i,8,c);gp(8,8+i,c);}
    if(!f){gp(6,6,0xd0fff0);gp(10,6,0xd0fff0);gp(6,10,0xd0fff0);gp(10,10,0xd0fff0);}
    gupload(SLOT_STAR+f);
  }
  /* claw marks 32x32 */
  gclear(32,32);
  for(int k=0;k<3;k++){gline(22+k*4,2,6+k*4,28,3,0xd02030);gline(22+k*4,2,6+k*4,28,1,0xffd0d0);}
  gupload(SLOT_CLAW);
  /* shield glint 32x32 */
  gclear(32,32);
  for(int y=4;y<28;y++){int w=y<16?10:10-(y-16)*10/12;gr(16-w,y,w*2,1,0x8a9ab0);gr(16-w+2,y,w*2-4,1,0xc8d4e8);}
  gr(15,6,2,20,0xffffff);gr(8,14,16,2,0xffffff);
  gupload(SLOT_SHIELD);
  /* digits 8x8: white set 0-9, green set 10-19 */
  for(int s=0;s<2;s++)for(int d=0;d<10;d++){
    gclear(8,8);u32 fg=s?0x70ff70:0xffffff,ol=s?0x0a3010:0x200810;int g='0'-32+d;
    for(int y=0;y<7;y++)for(int x=0;x<5;x++)if((FONT[g][y]>>(4-x))&1){for(int oy=-1;oy<=1;oy++)for(int ox=-1;ox<=1;ox++)if(GBUF[imax(0,imin(7,y+oy))*8+imax(0,imin(7,x+1+ox))]==NONE)gp(x+1+ox,y+oy,ol);}
    for(int y=0;y<7;y++)for(int x=0;x<5;x++)if((FONT[g][y]>>(4-x))&1)gp(x+1,y,fg);
    u8 t[64];for(int i=0;i<64;i++)t[i]=GBUF[i]==NONE?0:objIdx(GBUF[i]);
    put8(OBJV+SLOT_DIGIT*128+(s*10+d)*32,t);
  }
}

/* ---- map tileset ---- */
static int curSet=-1,curSpriteMap=-1; static u8 gfxOK=0;
static u8 charMeta[128];
static int animMeta[24],animN=0,animFrame=0,metaFlower=0,metaTuft=0,metaShore=0;
static u8 npcSlotOf[PL_COUNT];
static void loadSet(int set){
  const char*cs=SETCHARS[set];u8 b[256];
  memset(charMeta,0,sizeof charMeta);animN=0;
  for(int i=0;cs[i];i++){
    charMeta[(int)(u8)cs[i]]=(u8)i;makeTile(set,cs[i],0);metaToBytes(b,0);uploadBytes(CB0,i*4,b,4);
    if(strchr(ANIMCHARS[set],cs[i])){
      memcpy(ANIMBUF+animN*512,b,256);makeTile(set,cs[i],1);metaToBytes(ANIMBUF+animN*512+256,0);animMeta[animN++]=i;
    }
  }
  metaFlower=charMeta['g'];metaTuft=charMeta['q'];metaShore=charMeta[1];
  curSet=set;animFrame=0;
}
static void loadMapSprites(int m){
  const Map*M=&MAPS[m];int k=0;
  memset(npcSlotOf,0,sizeof npcSlotOf);
  for(int i=0;i<M->nn;i++){int p=M->npcs[i].pal;int found=0;for(int j=0;j<i;j++)if(M->npcs[j].pal==p)found=1;
    if(!found&&k<6){npcSlotOf[p]=(u8)(SLOT_NPC+k*8);uploadPerson(p,SLOT_NPC+k*8);k++;}}
  curSpriteMap=m;
}
static void reloadGfx(void){
  uploadUI();uploadPerson(PL_HERO,SLOT_HERO);uploadChest(0,SLOT_CHEST);uploadChest(1,SLOT_CHESTO);uploadMonSmall(M_KING,SLOT_BOSS);uploadEffects();
  curSet=-1;curSpriteMap=-1;gfxOK=1;
}

/* ---- battle backdrops (painted into BG2) ---- */
enum{BD_PLAINS,BD_FOREST,BD_HILLS,BD_SAND,BD_THORN,BD_CAVE,BD_KEEP};
static char tileAt(int m,int x,int y);
static int bdStyle(void){
  if(P.map==MAP_CAVE)return BD_CAVE;if(P.map==MAP_KEEP)return BD_KEEP;
  switch(tileAt(P.map,P.x,P.y)){case 'T':return BD_FOREST;case 'h':return BD_HILLS;case ':':return BD_SAND;case '#':return BD_THORN;}
  if(P.y>=24)return BD_THORN;return BD_PLAINS;
}
static u32 skyAt(int y,int day){
  static const u32 duskSky[7]={0x140c2a,0x22123a,0x34184a,0x4a1e52,0x6a2a58,0x8a3a5a,0xa04a5a};
  static const u32 daySky[7]={0x3a6ad0,0x4a7ad8,0x5a8ae0,0x6a9ae6,0x7aaaea,0x8abaf0,0x9acaf4};
  int b=imin(6,imax(0,y*7/52));return day?daySky[b]:duskSky[b];
}
static u32 bdPixel(int st,int x,int y){
  int day=P.fBoss;
  if(st==BD_CAVE){
    int g=x/5,u=x%5;int len=6+(int)(hash2(g,7)%22)-iabs(u-2)*4;
    if(y>=66){if(y==66)return 0x4a3e34;return (hash2(x/2,y/2)%7==0)?0x46392f:0x3a302a;}
    if(y<len)return u==1?0x5a4a3e:0x4a3a30;
    if((x>=10&&x<=14&&y>=56-(x==12?6:2))||(x>=48&&x<=51&&y>=58-(x==49?5:1)))return (x==12||x==49)?0xb0ffff:0x40c0e0;
    return (hash2(x/4,y/4)%5==0)?0x34281f:0x2a201c;
  }
  if(st==BD_KEEP){
    if(y>=64){if(x>=24&&x<=39)return (x==24||x==39)?0xc9a24a:0x8e1f30;return (((x/8)+(y/6))&1)?0x4c3e5e:0x40344f;}
    if((x>=4&&x<=13)||(x>=50&&x<=59)){int lx=(x<32)?x-4:x-50;if(y>=6&&y<=9)return 0x7a6a8a;if(lx==0||lx==9)return 0x4a3a5a;if(lx==2)return 0x6a5a7a;return 0x5a4a6a;}
    if(x>=25&&x<=38&&y>=6&&y<=40+((x%4<2)?0:3)){if(x==25||x==38||y==6)return 0xc9a24a;int dx=iabs(x-31),dy=iabs(y-20);if(dx+dy<=4)return dx+dy<=1?0xfff0a0:0xc9a24a;return 0x9a1a2a;}
    int row=y/6;int off=(row&1)?6:0;if(y%6==5||(x+off)%12==0)return 0x271b36;return 0x3a2a4c;
  }
  u32 sky=skyAt(y,day);
  if(st==BD_THORN){
    sky=y<52?(y<26?0x1a0a26:0x3a1442):0x2a1a30;
    int top=44+(int)(hash2(x/2,3)%22);if(y>=top){if(y==top)return 0x7a3a8a;return 0x1a0e20;}
    if(y<30&&hash2(x,y)%97==0)return 0xffd0f0;
    return sky;
  }
  /* sky details */
  if(y<52){
    int c1=(x-14)*(x-14)*4+(y-12)*(y-12)*36,c2=(x-46)*(x-46)*4+(y-22)*(y-22)*36;
    if(c1<=36*16||c2<=36*20)return day?0xf4f8ff:0xb06a8a;
    if(!day&&y<34&&hash2(x,y)%61==0)return 0xfff0d0;
  }
  if(st==BD_PLAINS){
    int mh=18+(8*isin(x*40))/256+(4*isin(x*97+200))/256;
    if(y<52){if(y>=52-mh){if(y<52-mh+2&&mh>21)return 0xe8e8f0;return day?0x6a86c0:0x3a2a5a;}return sky;}
    if(iabs(x-32)<(y-52)/2+2)return iabs(x-32)>=(y-52)/2?(day?0xa89060:0x4a3a3a):(day?0xc9b07a:0x6a5a4a);
    if(y<56)return day?0x3f8a36:0x284a2a;
    if((x*7+y*13)%17==0)return day?0x3b8231:0x223f24;
    return day?0x4a9a3c:0x2e5a30;
  }
  if(st==BD_FOREST){
    int u=x%10;int th=22+((x/10)*7)%9;int top=52-th+iabs(u-5)*3;
    if(y>=58){if((x*5+y*11)%13==0)return day?0x2a5a22:0x1a3018;return day?0x3a6a2a:0x24401f;}
    if(y>=top)return u==4?(day?0x2f7a37:0x203e26):(day?0x1d5a2a:0x162e1c);
    return sky;
  }
  if(st==BD_HILLS){
    int h1=52-(10+(8*isin(x*24))/256),h2=62-(6*isin(x*32+300))/256;
    if(y>=h2){if(y==h2)return day?0x76c657:0x3a6a3a;return day?0x4a9a3c:0x284a2a;}
    if(y>=h1)return day?0x5aa84a:0x305a34;
    return sky;
  }
  /* sand */
  {int d1=50-(6*isin(x*20))/256,d2=62-(5*isin(x*28+500))/256;
   if(y>=d2)return ((x+y*3)%11==0)?(day?0xd0b070:0x8a6a50):(day?0xe8c87c:0x9a7a58);
   if(y>=d1)return day?0xd9b86c:0x8a6a50;
   return sky;}
}
#define BD_TX 10  /* inner backdrop tile x (8 tiles wide) */
static void genBattleBG(void){
  int st=bdStyle();u8 t[64];
  memset(t,0,64);put8(CB2,t);
  for(int ty=0;ty<11;ty++)for(int tx=0;tx<8;tx++){
    for(int y=0;y<8;y++)for(int x=0;x<8;x++)t[y*8+x]=bgIdx(bdPixel(st,tx*8+x,ty*8+y));
    put8(CB2+(1+ty*8+tx)*32,t);
  }
  for(int i=0;i<1024;i++)VRAM16[31*0x400+i]=0;
  for(int ty=0;ty<11;ty++)for(int tx=0;tx<8;tx++)VRAM16[31*0x400+(2+ty)*32+BD_TX+tx]=(u16)(1+ty*8+tx);
}

/* ---- palette upload with dusk tint ---- */
static void applyPalette(void){
  int tint=(mode==MODE_MAP||mode==MODE_BATTLE)&&P.map==MAP_WORLD&&!P.fBoss;
  bgPal[4]=rgb15(uiFlash>0?0xff4a4a:0xffa040);
  for(int i=0;i<256;i++){
    u16 v=bgPal[i];
    if(tint&&i>=16&&i<bgN){int r=v&31,g=(v>>5)&31,b=(v>>10)&31;r+=(8-r)*22/100;g+=(3-g)*22/100;b+=(13-b)*22/100;v=(u16)(r|(g<<5)|(b<<10));}
    PALBG[i]=v;
    u16 o=objPal[i];
    if(tint&&i>=16&&i<objN&&mode==MODE_MAP){int r=o&31,g=(o>>5)&31,b=(o>>10)&31;r+=(8-r)*22/100;g+=(3-g)*22/100;b+=(13-b)*22/100;o=(u16)(r|(g<<5)|(b<<10));}
    PALOBJ[i]=o;
  }
  palDirty=0;
}

/* ================= MODE 4 (title / ending) ================= */
static void m4_hline(int x,int y,int w,u8 c){
  if(y<0||y>=160)return;
  if(x<0){w+=x;x=0;}
  if(x+w>240)w=240-x;
  if(w<=0)return;
  volatile u16*row=&VRAM16[y*120];int i=x,e=x+w;
  if(i&1){row[i>>1]=(u16)((row[i>>1]&0xFF)|(c<<8));i++;}
  u16 cc=(u16)(c|(c<<8));for(;i+1<e;i+=2)row[i>>1]=cc;
  if(i<e)row[i>>1]=(u16)((row[i>>1]&0xFF00)|c);
}
static void m4_rect(int x,int y,int w,int h,u8 c){for(int j=0;j<h;j++)m4_hline(x,y+j,w,c);}
static void m4_disc(int cx,int cy,int r,u8 c){for(int dy=-r;dy<=r;dy++){int w=isqrt(r*r-dy*dy);m4_hline(cx-w,cy+dy,w*2,c);}}
static int glyphOf(char c){if(c>='a'&&c<='z')c=(char)(c-32);if(c<32||c>126)c=' ';return c-32;}
static void m4_text(const char*s,int x,int y,int sc,u8 c){
  for(;*s;s++,x+=8*sc){int g=glyphOf(*s);for(int r=0;r<7;r++)for(int k=0;k<5;k++)if((FONT[g][r]>>(4-k))&1)m4_rect(x+(1+k)*sc,y+r*sc,sc,sc,c);}
}
static void m4_textc(const char*s,int cx,int y,int sc,u8 c){m4_text(s,cx-(int)strlen(s)*4*sc,y,sc,c);}
static void m4_panel(int x,int y,int w,int h){u8 k=1,wt=2;m4_rect(x,y,w,h,k);m4_rect(x+2,y+2,w-4,1,wt);m4_rect(x+2,y+h-3,w-4,1,wt);m4_rect(x+2,y+2,1,h-4,wt);m4_rect(x+w-3,y+2,1,h-4,wt);}
static void m4_cursor(int x,int y){for(int i=0;i<4;i++)m4_rect(x+i,y+i,1,8-2*i,2);}
static void m4_mountains(u32 col,int base,int amp){u8 c=bgIdx(col);for(int x=0;x<240;x++){int h=amp+(10*isin(x*733/100))/256+(6*isin(x*2118/100+163))/256;m4_rect(x,base-h,1,160-(base-h),c);}}
static void drawTitleMenu(void){
  m4_panel(72,104,96,36);m4_text("NEW GAME",92,110,1,2);m4_text("CONTINUE",92,122,1,hasSaveFlag?2:3);m4_cursor(80,110+titleSel*12);
}
static void drawTitleScene(void){
  static const u32 sky[8]={0x07051a,0x0d0826,0x150c33,0x1f1040,0x2c144a,0x3c1a50,0x512254,0x6a2c56};
  for(int i=0;i<8;i++)m4_rect(0,i*14,240,14,bgIdx(sky[i]));
  m4_rect(0,112,240,48,bgIdx(0x6a2c56));
  u8 st1=bgIdx(0xffffff),st2=bgIdx(0xa89cd8);
  for(int i=0;i<44;i++)m4_rect((i*53+17)%240,(i*37+11)%92,1,1,i%3==0?st1:st2);
  m4_disc(120,112,34,bgIdx(0x8a3a3a));m4_disc(120,112,30,bgIdx(0xffcf5a));m4_disc(120,112,26,bgIdx(0xffe590));m4_disc(133,104,27,bgIdx(0x1a0e30));
  m4_mountains(0x24183a,118,20);
  m4_mountains(0x0c0818,128,14);
  /* castle silhouette on the hill */
  {u8 k=bgIdx(0x0c0818),w=bgIdx(0xff6a5a);m4_rect(176,92,22,26,k);m4_rect(172,86,6,32,k);m4_rect(196,86,6,32,k);m4_rect(184,80,6,14,k);
   for(int i=0;i<3;i++){m4_rect(172+i*12,82,4,4,k);}m4_rect(186,98,2,3,w);m4_rect(178,104,2,2,w);}
  m4_textc("SUNSTONE",122,26,2,bgIdx(0x3a1a10));m4_textc("SUNSTONE",120,24,2,bgIdx(0xffd25a));
  m4_textc("~ SAGA ~",120,46,1,bgIdx(0xf4e6ff));
  drawTitleMenu();
  m4_textc("AN ORIGINAL RETRO RPG",120,148,1,bgIdx(0x9a8cc0));
}
static const char*const END_LINES[10]={"The Hollow King has fallen,","and the Sunstone blazes.","","Light pours over the land.","The long dusk is over.","","Brookhollow wakes to its","first sunrise in an age.","","THE END"};
static void drawEndScene(void){
  static const u32 b[8]={0x2a1a4a,0x4a2a5a,0x7a3a5a,0xb85a4a,0xe88a4a,0xf8b858,0xffd27a,0xffe39a};
  for(int i=0;i<8;i++)m4_rect(0,i*17,240,17,bgIdx(b[i]));
  m4_disc(120,118,26,bgIdx(0xfff8d0));m4_disc(120,118,20,bgIdx(0xfff2b0));
  m4_mountains(0x22402a,140,10);m4_rect(0,140,240,20,bgIdx(0x1a3220));
}
static void drawEndLine(int i){
  const char*l=END_LINES[i];if(!l[0])return;
  m4_textc(l,121,9+i*11,1,bgIdx(0x2a1020));m4_textc(l,120,8+i*11,1,i==9?bgIdx(0xfff2b0):bgIdx(0xffffff));
}
static void enterMode4(void){
  REG_DISPCNT=0x0404;REG_BG2CNT=0;REG_BLDCNT=0;REG_MOSAIC=0;gfxOK=0;
  for(int i=0;i<128;i++)OAM16[i*4]=0x0200;
  for(int i=0;i<240*160/2;i++)VRAM16[i]=0;
}

/* ================= MAP LOGIC ================= */
static char tileAt(int m,int x,int y){
  const Map*M=&MAPS[m];if(x<0||y<0||x>=M->w||y>=M->h)return M->fill;
  char c=M->rows[y][x];if(c=='X'||(c=='B'&&M->set==SET_KEEP))c='.';return c;
}
static int isLand(int x,int y){char c=tileAt(MAP_WORLD,x,y);return c!='~'&&c!='=';}
static int metaAt(int mx,int my){
  char c=tileAt(P.map,mx,my);
  if(curSet==SET_WORLD){
    if(c=='.'){u32 h=hash2(mx,my)%13;if(h==0)return metaFlower;if(h==1)return metaTuft;}
    else if(c=='~'){int m=0;if(isLand(mx,my-1))m|=1;if(isLand(mx+1,my))m|=2;if(isLand(mx,my+1))m|=4;if(isLand(mx-1,my))m|=8;if(m)return metaShore+m-1;}
  }
  return charMeta[(int)(u8)c];
}
static void findPos(char c,int out[2]){const Map*M=&MAPS[MAP_WORLD];for(int y=0;y<M->h;y++)for(int x=0;x<M->w;x++)if(M->rows[y][x]==c){out[0]=x;out[1]=y;return;}}
enum{E_NONE,E_NPC,E_CHEST,E_BOSS};
static int entityAt(int m,int x,int y,int*idx){
  const Map*M=&MAPS[m];
  for(int i=0;i<M->nn;i++)if(M->npcs[i].x==x&&M->npcs[i].y==y){*idx=i;return E_NPC;}
  for(int i=0;i<M->nc;i++)if(M->chests[i].x==x&&M->chests[i].y==y){*idx=i;return E_CHEST;}
  if(M->bx>=0&&!P.fBoss&&M->bx==x&&M->by==y){*idx=0;return E_BOSS;}
  return E_NONE;
}
static int walkable(int set,char t){if(set==SET_WORLD&&t=='#')return P.fCharm;return t!=0&&strchr(WALK[set],t)!=0;}
static const u8*zoneList(int*n){
  if(P.map==MAP_CAVE){*n=sizeof ZC;return ZC;}
  if(P.map==MAP_KEEP){*n=sizeof ZK;return ZK;}
  int x=P.x,y=P.y;
  if(y>=24){*n=sizeof Z4;return Z4;}
  if(x<=17){if(y<=10){*n=sizeof Z1;return Z1;}*n=sizeof Z2;return Z2;}
  if(y<=12){*n=sizeof Z2;return Z2;}
  if(y<=23){*n=sizeof Z3;return Z3;}
  *n=sizeof Z4;return Z4;
}
static void startBattle(int m);
static int frameTCount(void);
enum{MID_ENTER,MID_WARPHOME,MID_FULLHEAL,MID_LOSE,MID_ENDING,MID_NEWGAME,MID_CONTINUE,MID_POSTEND};
static void enterMap(int m,int x,int y,int dir){startFade(MID_ENTER,24,m,x,y,dir);}
static void openMenu(void);
static void openChest(int i);
static void npcTalk(int id);
static void bossTalk(void);
#define STEP 10
static void onStep(void){
  const Map*M=&MAPS[P.map];char t=tileAt(P.map,P.x,P.y);
  if(P.map==MAP_WORLD){
    if(t=='A'){enterMap(MAP_BROOK,7,10,D_UP);return;}
    if(t=='B'){enterMap(MAP_SALT,7,10,D_UP);return;}
    if(t=='C'){enterMap(MAP_CAVE,caveIn[0],caveIn[1],D_RIGHT);return;}
    if(t=='K'){enterMap(MAP_KEEP,7,11,D_UP);return;}
    if(t=='#'&&!P.fThorns){P.fThorns=1;clearBox();act(0,SFX_HEAL,0);say("The Star Charm flares with light!");say("The black thorns shrink back, opening a path.");return;}
  }
  if(t=='S'&&(P.map==MAP_CAVE||P.map==MAP_KEEP)){int*p=P.map==MAP_CAVE?posC:posK;enterMap(MAP_WORLD,p[0],p[1],D_DOWN);return;}
  if(M->enc){
    if(grace>0){grace--;return;}
    int r=0;
    if(M->set==SET_WORLD){switch(t){case '.':r=45;break;case 'T':r=77;break;case 'h':r=91;break;case ':':r=63;break;case '#':r=111;break;}}
    else r=M->set==SET_CAVE?67:83;
    if(chance(r)){int n;const u8*l=zoneList(&n);startBattle(l[rnd(0,n-1)]);}
  }
}
static void tryMove(int d){
  P.dir=(u8)d;int nx=P.x+DX[d],ny=P.y+DY[d];const Map*M=&MAPS[P.map];
  if(nx<0||ny<0||nx>=M->w||ny>=M->h){if(M->set==SET_TOWN){int*p=P.map==MAP_BROOK?posA:posB;enterMap(MAP_WORLD,p[0],p[1],D_DOWN);}return;}
  char t=tileAt(P.map,nx,ny);int idx;
  if(P.map==MAP_WORLD&&t=='#'&&!P.fCharm){if(inp.dirPress){clearBox();say("Black thorns, thick as iron, bar the way.");}return;}
  if(!walkable(M->set,t))return;
  if(entityAt(P.map,nx,ny,&idx))return;
  moving=1;mt=0;mfx=P.x;mfy=P.y;P.x=(s16)nx;P.y=(s16)ny;
}
static void interact(void){
  int fx=P.x+DX[P.dir],fy=P.y+DY[P.dir];const Map*M=&MAPS[P.map];int idx=0;
  int e=entityAt(P.map,fx,fy,&idx);
  if(!e&&M->set==SET_TOWN&&tileAt(P.map,fx,fy)=='c')e=entityAt(P.map,fx+DX[P.dir],fy+DY[P.dir],&idx);
  if(e){
    clearBox();
    if(e==E_NPC){M->npcs[idx].dir=OPP[P.dir];npcTalk(M->npcs[idx].talk);}
    else if(e==E_CHEST)openChest(idx);
    else bossTalk();
    return;
  }
  if(M->set==SET_TOWN){char c=tileAt(P.map,fx,fy);
    if(c=='I'){clearBox();say("The sign reads: INN - Rest and be restored.");return;}
    if(c=='S'){clearBox();say("The sign reads: GOODS - Arms, armor and supplies.");return;}
    if(c=='o'){clearBox();say("A deep stone well. The water is cool and clear.");return;}}
  openMenu();
}
static void updateMap(void){
  if(moving){mt++;if(mt<STEP)return;moving=0;onStep();if(fade.on||busy()||mode!=MODE_MAP)return;}
  if(inp.a){interact();return;}
  if(inp.start){openMenu();return;}
  if(inp.held)tryMove(inp.held-1);
}
static void wanderNPCs(void){
  const Map*M=&MAPS[P.map];if(!M->nn||busy())return;
  for(int i=0;i<M->nn;i++){
    Npc*n=&M->npcs[i];if(!n->wander)continue;
    if(--n->wt>0)continue;
    n->wt=(s16)rnd(60,150);
    int d=rnd(0,3);n->dir=(u8)d;int x=n->x+DX[d],y=n->y+DY[d],idx;
    if(x<1||y<1||x>=M->w-1||y>=M->h-1)continue;
    char t=tileAt(P.map,x,y);if(t!='.'&&t!=','&&t!='f')continue;
    if(iabs(x-n->hx)>2||iabs(y-n->hy)>2)continue;
    if(x==P.x&&y==P.y)continue;
    if(moving&&x==mfx&&y==mfy)continue;
    if(entityAt(P.map,x,y,&idx))continue;
    n->x=(s8)x;n->y=(s8)y;
  }
}

/* ================= ACTIONS / MENUS ================= */
enum{A_SFX,A_CLEARLINES,A_HEALHP,A_GOLD,A_CHARM,A_SUNSTEEL,A_WARPHOME,A_INNREST,A_SAVEMSG,A_STARTBATTLE,A_BATTLEOPEN,
  A_CMDMENU,A_AFTERPLAYER,A_AFTERENEMY,A_MONHURT,A_PHURT,A_GAIN,A_ENDBATTLE,A_BDEAD,A_BOSSDONE,A_LOSEFADE,A_LOSEMSG,
  A_FX,A_LUNGE,A_MONHEAL,A_GUARD};
enum{MN_MAIN,MN_FSPELL,MN_FITEM,MN_INN,MN_SHOP,MN_SHOPYN,MN_BOSS,MN_CMD,MN_BSPELL,MN_BITEM};
static void showStatus(void){
  Stats s=statsFor(P.lv);int nxt=P.lv<20?XPT[P.lv+1]-P.xp:0;
  act(A_CLEARLINES,0,0);
  sayf("Wren   LV %d\nHP %d/%d  MP %d/%d\nATK %d  DEF %d\nGOLD %d",P.lv,P.hp,s.maxhp,P.mp,s.maxmp,atkV(),defV(),(int)P.gold);
  act(A_CLEARLINES,0,0);
  sayf("EXP %d  NEXT %d\nWPN %s\nARM %s",P.xp,nxt,WEAPONS[P.weapon].name,ARMORS[P.armor].name);
}
static void openMenu(void){sfx(SFX_BLIP);ask(MN_MAIN,0,0);}
static void fieldSpell(void){
  int any=0;for(int i=0;i<5;i++)if(P.lv>=SPELLS[i].lv)any=1;
  if(!any){say("You haven't learned any spells yet.");return;}
  ask(MN_FSPELL,0,0);
}
static void fieldItem(void){
  if(!P.herb&&!P.feather&&!P.charm){say("Your pack is empty.");return;}
  ask(MN_FITEM,0,0);
}
static void inn(int cost,int town){sayf("Welcome, traveler! A night's rest is %d gold. Will you stay?",cost);ask(MN_INN,cost,town);}
static void shop(int sid){say("Welcome! Take a look at my wares.");ask(MN_SHOP,sid,0);}
static void openChest(int i){
  const Chest*c=&MAPS[P.map].chests[i];
  if(P.chests[c->id]){say("The chest is empty.");return;}
  P.chests[c->id]=1;act(A_SFX,SFX_CHEST,0);
  say("Wren opens the chest...");
  if(c->gold){sayf("Found %d gold!",c->gold);act(A_GOLD,c->gold,0);}
  else if(c->give==GIVE_CHARM){say("Found the Star Charm!");say("It glows softly, as if eager to part something.");act(A_CHARM,0,0);}
  else{say("Found the Sunsteel Sword!");say("Wren equips it. The blade hums with warm light.");act(A_SUNSTEEL,0,0);}
}
static void bossTalk(void){
  say("HOLLOW KING: So, a little warden climbs my stair.");
  say("The Sunstone is mine, and the dusk is forever.");
  say("Kneel... or burn in cold fire!");
  ask(MN_BOSS,0,0);
}
static void npcTalk(int id){
  switch(id){
    case T_INN_B:inn(6,MAP_BROOK);break;
    case T_SHOP_B:shop(0);break;
    case T_ELDER:
      if(P.fBoss)say("Dawn at last! Brookhollow will sing of you for an age.");
      else if(P.fCharm){say("The Star Charm! Then go, Wren.");say("The Hollow Keep stands beyond the black thorns, far to the southeast.");}
      else{say("Wren, since the Sunstone was stolen, the sun has not risen.");
        say("The Hollow King hoards it in his keep, behind a wall of black thorns.");
        say("Old tales speak of a Star Charm that parts the thorns. It was lost in the Sunken Grotto, east across the bridge.");}
      break;
    case T_GUARD_B:say("East over the bridge lies Saltmere. Their smiths forge far finer blades.");break;
    case T_VILL_B:say("Hurt? Rest at the inn. And always keep a Herb or two in your pack.");break;
    case T_KID:say(P.fBoss?"The sun! I can see my shadow again!":"Mosslings are slow. I could beat one! ...Probably.");break;
    case T_INN_S:inn(15,MAP_SALT);break;
    case T_SHOP_S:shop(1);break;
    case T_SAGE:say("The Sunken Grotto is cut into the cliffs south of here, on the western shore.");say("Its tunnels are dark. You will only see the ground close around you.");break;
    case T_SAILOR:say(P.fBoss?"The sea is warm again. Time to mend my nets!":"No ship has sailed since the dusk fell. The sea has gone cold and still.");break;
    case T_WOMAN:say("Gloom Wisps hurl lightning. In battle, DEFEND halves the harm you take.");break;
    case T_OLD:say("The Hollow King breathes cold fire. Learn strong healing before you face him.");say("A soldier who fled his keep swore he hid a fine sword in a side room there.");break;
  }
}

/* ---- battle ---- */
static void popup(int val,int x,int y,int green){int k=pops[0].on&&!pops[1].on?1:0;if(pops[0].on&&pops[1].on)k=pops[0].t>pops[1].t?0:1;pops[k].on=1;pops[k].val=(s16)val;pops[k].x=(s16)x;pops[k].y=(s16)y;pops[k].t=0;pops[k].green=(u8)green;}
static const char*art(const char*n){char c=n[0];return (c=='A'||c=='E'||c=='I'||c=='O'||c=='U')?"An":"A";}
static void startBattle(int m){
  const Mon*M=&MONS[m];int hp=M->hp+(M->boss?0:rnd(0,M->hp/5));
  memset(&B,0,sizeof B);
  B.on=1;B.mon=(u8)m;B.hp=(s16)hp;B.maxhp=(s16)hp;B.intro=44;B.boss=M->boss;B.anim=AN_IDLE;
  mode=MODE_BATTLE;sfx(SFX_ENC);clearBox();box.active=1;
  genBattleBG();uploadMonBig(m);palDirty=1;
  if(M->boss)say("The Hollow King rises from his throne!");else sayf("%s %s draws near!",art(M->name),M->name);
  act(A_BATTLEOPEN,0,0);
}
static void cmdMenu(void){B.defend=0;ask(MN_CMD,0,0);}
static void afterPlayer(void){act(A_AFTERPLAYER,0,0);}
static void enemyTurn(void){
  const Mon*M=&MONS[B.mon];
  if(M->healP&&B.hp<B.maxhp/2&&chance(M->healP)){
    int h=imin(B.maxhp-B.hp,rnd(20,30)*(M->boss?2:1));
    sayf("The %s glows with a dark light!",M->name);act(A_FX,FX_MHEAL,0);act(A_MONHEAL,h,0);sayf("The %s recovers %d HP.",M->name,h);return;
  }
  if(M->guardP&&!B.guard&&chance(M->guardP)){sayf("The %s raises its shield!",M->name);act(A_FX,FX_GUARD,0);act(A_GUARD,0,0);return;}
  if(M->spP&&chance(M->spP)){
    int d=rnd(M->spMin,M->spMax);if(B.defend)d/=2;
    sayf("The %s %s",M->name,M->spMsg);act(A_FX,M->spFx,0);act(A_PHURT,d,0);sayf("Wren takes %d damage.",d);
  }else{
    sayf("The %s attacks!",M->name);act(A_LUNGE,0,0);
    if(chance(31)){act(A_SFX,SFX_MISS,0);say("Wren dodges the blow!");return;}
    int d=dmgCalc(M->atk,defV());if(B.defend)d/=2;
    act(A_FX,FX_CLAW,0);act(A_PHURT,d,0);
    if(d>0)sayf("Wren takes %d damage.",d);else say("Wren takes no damage.");
  }
}
static void doFight(void){
  const Mon*M=&MONS[B.mon];say("Wren attacks!");
  if(!B.boss&&chance(25)){act(A_FX,FX_SLASH,0);act(A_SFX,SFX_MISS,0);sayf("The %s dodges swiftly!",M->name);afterPlayer();return;}
  int crit=!B.boss&&chance(42);
  int d=crit?atkV()*(90+rnd(0,30))/100:dmgCalc(atkV(),M->def);
  if(B.guard&&!crit){d/=2;B.guard=0;}
  if(crit){act(A_FX,FX_CRIT,0);say("A critical blow!");}
  else act(A_FX,FX_SLASH,0);
  act(A_MONHURT,d,0);
  if(d>0)sayf("The %s takes %d damage.",M->name,d);else sayf("The %s takes no damage.",M->name);
  afterPlayer();
}
static void doDefend(void){B.defend=1;act(A_FX,FX_GUARD,1);say("Wren raises a guard and braces for the attack!");afterPlayer();}
static void doSpellMenu(void){
  int any=0;for(int i=0;i<5;i++)if(P.lv>=SPELLS[i].lv&&SPELLS[i].battle)any=1;
  if(!any){say("You haven't learned any spells yet.");cmdMenu();return;}
  ask(MN_BSPELL,0,0);
}
static void doItemMenu(void){if(!P.herb){say("You have no Herbs.");cmdMenu();return;}ask(MN_BITEM,0,0);}
static void doRun(void){
  say("Wren tries to run...");
  if(B.boss){say("But the Hollow King bars the way!");afterPlayer();return;}
  int p=imax(250,imin(900,550+(agiV()-MONS[B.mon].agi)*20));
  if(chance(p)){act(A_SFX,SFX_RUN,0);say("...and gets away!");act(A_ENDBATTLE,0,0);}
  else{say("...but is cut off!");afterPlayer();}
}
static void levelUps(void){
  while(P.lv<20&&P.xp>=XPT[P.lv+1]){
    Stats a=statsFor(P.lv);P.lv++;Stats b=statsFor(P.lv);
    P.hp=(s16)(P.hp+b.maxhp-a.maxhp);P.mp=(s16)(P.mp+b.maxmp-a.maxmp);
    act(A_SFX,SFX_LVL,0);sayf("Wren reached level %d!",P.lv);
    for(int i=0;i<5;i++)if(SPELLS[i].lv==P.lv)sayf("Wren learned %s!",SPELLS[i].name);
  }
}
static void win(void){
  const Mon*M=&MONS[B.mon];
  act(A_BDEAD,0,0);
  if(B.boss){say("The Hollow King crumbles into dust!");say("From the ashes rises a blinding light... the Sunstone!");act(A_BOSSDONE,0,0);return;}
  int g=imax(1,M->gold*(80+rnd(0,40))/100);
  sayf("The %s is defeated!",M->name);
  sayf("Gained %d EXP and %d gold.",M->xp,g);
  act(A_GAIN,M->xp,g);
}
static void lose(void){act(A_SFX,SFX_DIE,0);say("Wren has fallen...");act(A_LOSEFADE,0,0);act(A_LOSEMSG,0,0);}
static void endBattle(void){B.on=0;mode=MODE_MAP;clearBox();box.active=0;grace=2;palDirty=1;pops[0].on=pops[1].on=0;}
static const u8 FXDUR[]={0,12,26,24,32,30,30,12,20,32,32,18};

static void doAct(int code,int a,int b){
  switch(code){
    case A_SFX:sfx(a);break;
    case A_CLEARLINES:box.nlines=0;break;
    case A_HEALHP:P.hp=(s16)(P.hp+a);sfx(SFX_HEAL);if(mode==MODE_BATTLE)popup(a,36,54,1);break;
    case A_GOLD:P.gold+=a;break;
    case A_CHARM:P.fCharm=1;P.charm=1;break;
    case A_SUNSTEEL:P.weapon=W_SUN;break;
    case A_WARPHOME:startFade(MID_WARPHOME,40,0,0,0,0);break;
    case A_INNREST:P.gold-=a;P.home=(u8)b;sfx(SFX_INN);startFade(MID_FULLHEAL,90,0,0,0,0);break;
    case A_SAVEMSG:if(a)say(doSave()?"(Your journey has been saved.)":"(The save failed.)");else say(doSave()?"Your journey has been saved.":"The save failed.");break;
    case A_STARTBATTLE:startBattle(a);break;
    case A_BATTLEOPEN:{
      const Mon*M=&MONS[B.mon];
      if(!B.boss&&M->agi*10>agiV()*13&&chance(300)){sayf("The %s strikes before you can act!",M->name);enemyTurn();act(A_AFTERENEMY,0,0);}
      else cmdMenu();
    }break;
    case A_CMDMENU:cmdMenu();break;
    case A_AFTERPLAYER:if(B.hp<=0){win();break;}enemyTurn();act(A_AFTERENEMY,0,0);break;
    case A_AFTERENEMY:if(P.hp<=0)lose();else cmdMenu();break;
    case A_MONHURT:B.hp=(s16)(B.hp-a);if(a>0){B.anim=AN_HIT;B.animT=0;sfx(b?SFX_ZAP:SFX_HIT);popup(a,112,38,0);}else sfx(SFX_MISS);break;
    case A_PHURT:P.hp=(s16)imax(0,P.hp-a);if(a>0){shake=12;uiFlash=14;palDirty=1;sfx(SFX_HURT);popup(a,36,54,0);}else sfx(SFX_MISS);break;
    case A_GAIN:P.xp=(u16)imin(60000,P.xp+a);P.gold=imin(99999,P.gold+b);levelUps();act(A_ENDBATTLE,0,0);break;
    case A_ENDBATTLE:endBattle();break;
    case A_BDEAD:B.anim=AN_DIE;B.animT=0;sfx(SFX_WIN);break;
    case A_BOSSDONE:P.fBoss=1;startFade(MID_ENDING,70,0,0,0,0);break;
    case A_LOSEFADE:startFade(MID_LOSE,70,0,0,0,0);break;
    case A_LOSEMSG:sayf("Wren wakes at the %s inn.",MAPS[P.home].title);say("Half of your gold has been lost.");break;
    case A_FX:B.fx=a;B.fxT=0;B.fxArg=b;
      if(a==FX_SPARK||a==FX_EBOLT)sfx(SFX_ZAP);else if(a==FX_BLAZE||a==FX_EFIRE)sfx(SFX_FIRE);else if(a==FX_CRIT)sfx(SFX_CRIT);
      else if(a==FX_HEAL||a==FX_MHEAL||a==FX_ESPORE)sfx(SFX_HEAL);else if(a==FX_GUARD)sfx(SFX_GUARD);break;
    case A_LUNGE:B.anim=AN_LUNGE;B.animT=0;break;
    case A_MONHEAL:B.hp=(s16)imin(B.maxhp,B.hp+a);popup(a,112,38,1);break;
    case A_GUARD:B.guard=1;break;
  }
}
static void addOpt(const char*s,int v){if(ch.n<8){strncpy(ch.o[ch.n],s,23);ch.o[ch.n][23]=0;ch.v[ch.n]=(s16)v;ch.n++;}}
static void buildChoice(int menu,int a,int b){
  char buf[32];
  memset(&ch,0,sizeof ch);ch.menu=(u8)menu;ch.a=(s16)a;ch.b=(s16)b;
  switch(menu){
    case MN_MAIN:addOpt("Status",0);addOpt("Spell",1);addOpt("Item",2);addOpt("Save",3);addOpt("Close",4);ch.hasCancel=1;ch.cancel=4;ch.top=1;ch.status=1;break;
    case MN_FSPELL:case MN_BSPELL:
      for(int i=0;i<5;i++)if(P.lv>=SPELLS[i].lv&&(menu==MN_FSPELL||SPELLS[i].battle)){char nm[12];padTo(nm,SPELLS[i].name,8);sfmt(buf,32,"%s%d",nm,SPELLS[i].mp);addOpt(buf,i);}
      ch.hasCancel=1;ch.cancel=-1;if(menu==MN_FSPELL){ch.top=1;ch.status=1;}else ch.battle=1;break;
    case MN_FITEM:
      if(P.herb){char nm[20];padTo(nm,"Herb",15);sfmt(buf,32,"%sx%d",nm,P.herb);addOpt(buf,0);}
      if(P.feather){char nm[20];padTo(nm,"Homing Feather",15);sfmt(buf,32,"%sx%d",nm,P.feather);addOpt(buf,1);}
      if(P.charm)addOpt("Star Charm",2);
      ch.hasCancel=1;ch.cancel=-1;ch.top=1;ch.status=1;break;
    case MN_INN:addOpt("Stay",0);addOpt("Leave",1);ch.hasCancel=1;ch.cancel=1;ch.gold=1;break;
    case MN_SHOP:
      for(int i=0;i<6;i++){int g=SHOPS[a][i];char nm[20];padTo(nm,goodName(g),15);sfmt(buf,32,"%s%d",nm,goodPrice(g));addOpt(buf,g);}
      addOpt("Leave",-1);ch.hasCancel=1;ch.cancel=-1;ch.gold=1;break;
    case MN_SHOPYN:addOpt("Yes",0);addOpt("No",1);ch.hasCancel=1;ch.cancel=1;ch.gold=1;break;
    case MN_BOSS:addOpt("Fight",0);addOpt("Retreat",1);ch.hasCancel=1;ch.cancel=1;break;
    case MN_CMD:addOpt("Fight",0);addOpt("Spell",1);addOpt("Defend",2);addOpt("Item",3);addOpt("Run",4);ch.battle=1;break;
    case MN_BITEM:sfmt(buf,32,"Herb x%d",P.herb);addOpt(buf,0);ch.hasCancel=1;ch.cancel=-1;ch.battle=1;break;
  }
}
static void onChoice(int menu,int v,int a,int b){
  switch(menu){
    case MN_MAIN:
      if(v==0)showStatus();else if(v==1)fieldSpell();else if(v==2)fieldItem();else if(v==3)act(A_SAVEMSG,0,0);
      break;
    case MN_FSPELL:{
      if(v<0)break;
      const Spell*s=&SPELLS[v];
      if(s->type==SP_DMG){sayf("%s can only be cast in battle.",s->name);break;}
      if(P.mp<s->mp){say("Not enough MP!");break;}
      P.mp=(s16)(P.mp-s->mp);sayf("Wren casts %s!",s->name);
      if(s->type==SP_HEAL){int h=healAmt(rnd(s->mn,s->mx));act(A_HEALHP,h,0);sayf("Recovered %d HP.",h);}
      else act(A_WARPHOME,0,0);
    }break;
    case MN_FITEM:
      if(v<0)break;
      if(v==0){int h=healAmt(rnd(25,35));P.herb--;say("Wren eats a Herb.");act(A_HEALHP,h,0);sayf("Recovered %d HP.",h);}
      else if(v==1){P.feather--;say("Wren tosses the feather skyward...");act(A_WARPHOME,0,0);}
      else say("The Star Charm glows warmly in your hand.");
      break;
    case MN_INN:
      if(v!=0){say("Safe travels, friend.");break;}
      if(P.gold<a){say("Sorry, you don't have enough gold.");break;}
      act(A_INNREST,a,b);say("Good morning! You look well rested.");act(A_SAVEMSG,1,0);
      break;
    case MN_SHOP:{
      if(v<0){say("Come again!");break;}
      int g=v,price=goodPrice(g);
      if(g<GD_W){
        u8*cnt=g==GD_HERB?&P.herb:&P.feather;
        if(P.gold<price){say("You can't afford that.");ask(MN_SHOP,a,0);break;}
        if(*cnt>=9){say("Your pack can't hold any more.");ask(MN_SHOP,a,0);break;}
        P.gold-=price;(*cnt)++;sfx(SFX_BUY);sayf("One %s. Anything else?",goodName(g));ask(MN_SHOP,a,0);break;
      }
      int isW=g<GD_A;const Gear*cur=isW?&WEAPONS[P.weapon]:&ARMORS[P.armor];const Gear*it=isW?&WEAPONS[g-GD_W]:&ARMORS[g-GD_A];
      if(cur==it){say("You already have that!");ask(MN_SHOP,a,0);break;}
      if(it->val<cur->val){sayf("That's weaker than your %s!",cur->name);ask(MN_SHOP,a,0);break;}
      int refund=cur->price/2;
      if(P.gold+refund<it->price){say("You can't afford that.");ask(MN_SHOP,a,0);break;}
      if(refund>0)sayf("I'll take your %s for %d gold. Deal?",cur->name,refund);else sayf("The %s? Deal?",it->name);
      ask(MN_SHOPYN,a,g);
    }break;
    case MN_SHOPYN:
      if(v==0){int g=b,isW=g<GD_A;const Gear*cur=isW?&WEAPONS[P.weapon]:&ARMORS[P.armor];const Gear*it=isW?&WEAPONS[g-GD_W]:&ARMORS[g-GD_A];
        P.gold+=cur->price/2-it->price;if(isW)P.weapon=(u8)(g-GD_W);else P.armor=(u8)(g-GD_A);sfx(SFX_BUY);sayf("Wren equips the %s!",it->name);}
      ask(MN_SHOP,a,0);
      break;
    case MN_BOSS:if(v==0)act(A_STARTBATTLE,M_KING,0);else say("HOLLOW KING: Run, then. The dark will wait for you.");break;
    case MN_CMD:if(v==0)doFight();else if(v==1)doSpellMenu();else if(v==2)doDefend();else if(v==3)doItemMenu();else doRun();break;
    case MN_BSPELL:{
      if(v<0){cmdMenu();break;}
      const Spell*s=&SPELLS[v];
      if(P.mp<s->mp){say("Not enough MP!");cmdMenu();break;}
      P.mp=(s16)(P.mp-s->mp);sayf("Wren casts %s!",s->name);act(A_FX,s->fx,0);
      if(s->type==SP_HEAL){int h=healAmt(rnd(s->mn,s->mx));act(A_HEALHP,h,0);sayf("Wren recovers %d HP.",h);}
      else{int d=rnd(s->mn,s->mx);if(B.boss)d=d*6/10;act(A_MONHURT,d,1);sayf("The %s takes %d damage.",MONS[B.mon].name,d);}
      afterPlayer();
    }break;
    case MN_BITEM:{
      if(v<0){cmdMenu();break;}
      int h=healAmt(rnd(25,35));P.herb--;say("Wren eats a Herb.");act(A_FX,FX_HEAL,0);act(A_HEALHP,h,0);sayf("Recovered %d HP.",h);afterPlayer();
    }break;
  }
}

/* ================= FADES & FLOW ================= */
static void startFade(int mid,int dur,int a,int b,int c,int d){fade.on=1;fade.t=0;fade.dur=dur;fade.mid=mid;fade.a=(s16)a;fade.b=(s16)b;fade.c=(s16)c;fade.d=(s16)d;}
static void newPlayer(void){
  memset(&P,0,sizeof P);P.lv=1;P.hp=15;P.mp=0;P.gold=30;P.weapon=W_STICK;P.armor=0;P.herb=2;P.map=MAP_BROOK;P.x=7;P.y=9;P.dir=D_UP;P.home=MAP_BROOK;
}
static void fullHeal(void){Stats s=statsFor(P.lv);P.hp=s.maxhp;P.mp=s.maxmp;}
static void showBanner(void){bannerText=MAPS[P.map].title;bannerT=bannerText?110:0;}
static void doMid(void){
  switch(fade.mid){
    case MID_ENTER:P.map=(u8)fade.a;P.x=fade.b;P.y=fade.c;P.dir=(u8)fade.d;moving=0;grace=3;showBanner();palDirty=1;break;
    case MID_WARPHOME:P.map=P.home;P.x=7;P.y=10;P.dir=D_UP;moving=0;grace=3;showBanner();palDirty=1;break;
    case MID_FULLHEAL:fullHeal();break;
    case MID_LOSE:B.on=0;mode=MODE_MAP;fullHeal();P.gold/=2;P.map=P.home;P.x=7;P.y=10;P.dir=D_UP;moving=0;clearBox();box.active=0;grace=3;palDirty=1;pops[0].on=pops[1].on=0;break;
    case MID_ENDING:B.on=0;mode=MODE_END;endT=0;endDone=0;clearBox();box.active=0;enterMode4();drawEndScene();palDirty=1;break;
    case MID_NEWGAME:newPlayer();mode=MODE_MAP;clearBox();grace=3;palDirty=1;
      say("Since the Sunstone was stolen, an endless dusk has gripped the land.");
      say("You are Wren, a young warden of Brookhollow.");
      say("Talk to the Elder in the square. (A: talk, START: menu)");break;
    case MID_CONTINUE:mode=MODE_MAP;clearBox();grace=3;moving=0;showBanner();palDirty=1;break;
    case MID_POSTEND:mode=MODE_MAP;P.map=MAP_BROOK;P.x=7;P.y=9;P.dir=D_DOWN;moving=0;fullHeal();P.home=MAP_BROOK;doSave();
      clearBox();say("Thank you for playing Sunstone Saga!");say("The world is yours to roam in the new light.");palDirty=1;break;
  }
}
static void updateTitle(void){
  if(inp.dirPress==D_UP+1||inp.dirPress==D_DOWN+1){titleSel^=1;sfx(SFX_BLIP);drawTitleMenu();}
  if(inp.a||inp.start){
    rngs^=(u32)frameTCount()*2654435761u;
    if(titleSel==0){sfx(SFX_BLIP);startFade(MID_NEWGAME,40,0,0,0,0);}
    else{Player t;if(loadSave(&t)){sfx(SFX_BLIP);P=t;startFade(MID_CONTINUE,40,0,0,0,0);}else sfx(SFX_ERR);}
  }
}
static void updateEnd(void){
  endT++;
  for(int i=0;i<10;i++)if(endT==i*45+1)drawEndLine(i);
  int done=endT>10*45+40;
  if(done){if((endT/24)%2==0)m4_textc("PRESS A",120,146,1,2);else m4_rect(80,146,80,8,bgIdx(0x1a3220));}
  if(!endDone&&done&&(inp.a||inp.start)){endDone=1;startFade(MID_POSTEND,50,0,0,0,0);}
}

/* ================= INPUT ================= */
static u16 prevKeys=0;
static void readInput(void){
  u16 k=(u16)(~KEYREG)&0x3FF;u16 pressed=(u16)(k&~prevKeys);prevKeys=k;
  if(pressed&0x001)inp.a=1;
  if(pressed&0x002)inp.b=1;
  if(pressed&0x008)inp.start=1;
  if(pressed&0x004)inp.select=1;
  u8 held=0;
  if(k&0x040)held=D_UP+1;else if(k&0x080)held=D_DOWN+1;else if(k&0x020)held=D_LEFT+1;else if(k&0x010)held=D_RIGHT+1;
  if(held!=inp.prev){inp.dirPress=held;inp.rep=15;inp.prev=held;}
  else if(held&&--inp.rep<=0){inp.dirPress=held;inp.rep=5;}
  inp.held=held;
}

/* ================= UPDATE ================= */
static void update(void){
  frameT++;readInput();sfxTick();musicTick();
  if(inp.select){soundOn^=1;if(!soundOn){REG_SND2L=0;REG_SND2H=0x8000;sfxCur=0;musicStop();}else{musId=-1;sfx(SFX_BLIP);}}
  if(shake>0)shake--;
  if(bannerT>0)bannerT--;
  if(uiFlash>0){uiFlash--;if(uiFlash==0)palDirty=1;}
  for(int i=0;i<2;i++)if(pops[i].on&&++pops[i].t>50)pops[i].on=0;
  if(B.on){
    if(B.intro>0)B.intro--;
    if(B.fx){if(++B.fxT>=FXDUR[B.fx])B.fx=0;}
    if(B.anim!=AN_IDLE&&B.anim!=AN_GONE){B.animT++;
      if(B.anim==AN_HIT&&B.animT>=14)B.anim=AN_IDLE;
      if(B.anim==AN_LUNGE&&B.animT>=16)B.anim=AN_IDLE;
      if(B.anim==AN_DIE&&B.animT>=26)B.anim=AN_GONE;}
  }
  if(fade.on){fade.t++;if(fade.t==fade.dur/2)doMid();if(fade.t>=fade.dur)fade.on=0;}
  else if(mode==MODE_TITLE)updateTitle();
  else if(mode==MODE_END)updateEnd();
  else{
    if(mode==MODE_MAP)wanderNPCs();
    if(busy())handleUI();else if(mode==MODE_MAP)updateMap();
    pump();
  }
  if(box.typing){box.chars+=2;if(box.chars>=box.total){box.chars=box.total;box.typing=0;if(qn&&q[qh].type==I_ASK){mergeBox();pump();}else box.waiting=1;}}
  if(mode==MODE_MAP&&!busy()&&!fade.on){box.active=0;box.nlines=0;}
  if(mode==MODE_MAP&&!busy()&&!moving&&!inp.held&&!fade.on)idle++;else idle=0;
  /* background music follows the scene */
  if(soundOn){int m=MUS_TITLE;
    if(mode==MODE_BATTLE)m=B.boss?MUS_BOSS:MUS_BATTLE;
    else if(mode==MODE_MAP)m=P.map==MAP_WORLD?MUS_FIELD:(P.map==MAP_BROOK||P.map==MAP_SALT)?MUS_TOWN:MUS_CAVE;
    musicSet(m);}
  inp.a=inp.b=inp.start=inp.select=0;inp.dirPress=0;
}

/* ================= RENDER (mode 0) ================= */
static u16 bg1[32*32];
static void T1(int x,int y,int t){if(x>=0&&x<32&&y>=0&&y<32)bg1[y*32+x]=(u16)t;}
static void panelA(int x,int y,int w,int h,int alt){
  int b=alt?TL_B2_TL:TL_B_TL;
  for(int j=0;j<h;j++)for(int i=0;i<w;i++){int t;
    if(j==0)t=i==0?b:i==w-1?b+2:b+1;else if(j==h-1)t=i==0?b+5:i==w-1?b+7:b+6;else t=i==0?b+3:i==w-1?b+4:TL_GLYPH;
    T1(x+i,y+j,t);}
}
static void panel(int x,int y,int w,int h){panelA(x,y,w,h,0);}
static void textA(int x,int y,const char*s,int alt){for(;*s;s++,x++)T1(x,y,(alt?TL_GLYPH2:TL_GLYPH)+glyphOf(*s));}
static void text(int x,int y,const char*s){textA(x,y,s,0);}
static void textN(int x,int y,const char*s,int n){for(int i=0;i<n&&s[i];i++)T1(x+i,y,TL_GLYPH+glyphOf(s[i]));}
static void drawStat(void){
  char b[16];int alt=lowHP()||uiFlash>0;panelA(0,0,9,7,alt);textA(1,1,"Wren",alt);
  sfmt(b,16,"LV %d",P.lv);textA(1,2,b,alt);sfmt(b,16,"HP %d",P.hp);textA(1,3,b,alt);sfmt(b,16,"MP %d",P.mp);textA(1,4,b,alt);sfmt(b,16,"G %d",(int)P.gold);textA(1,5,b,alt);
}
static void drawBox(void){
  panel(0,14,30,6);
  const char*ls[8];int lens[8];int n=0;
  for(int i=0;i<box.nlines;i++){ls[n]=box.lines[i];lens[n]=99;n++;}
  int rem=box.chars;
  for(int i=0;i<box.nn;i++){ls[n]=box.nl[i];lens[n]=imax(0,rem);rem-=(int)strlen(box.nl[i]);n++;}
  int s=imax(0,n-4);
  for(int i=s;i<n;i++)textN(1,15+(i-s),ls[i],lens[i]);
  if(box.waiting&&(frameT/16)%2==0)T1(28,18,TL_ARROW);
}
static void drawChoice(void){
  int ml=0;for(int i=0;i<ch.n;i++)ml=imax(ml,(int)strlen(ch.o[i]));
  int w=ml+3,h=ch.n+2,x=30-w,y=(ch.top||ch.battle)?0:14-h;
  panel(x,y,w,h);for(int i=0;i<ch.n;i++)text(x+2,y+1+i,ch.o[i]);
  if((frameT/20)%3!=2||!ch.battle)T1(x+1,y+1+ch.sel,TL_CURSOR);
}
static void drawBattleUI(void){
  char b[16];const Mon*M=&MONS[B.mon];
  panel(BD_TX-1,1,10,13);
  for(int i=BD_TX;i<BD_TX+8;i++)for(int j=2;j<=12;j++)T1(i,j,0);
  int alt=lowHP()||uiFlash>0;
  panelA(0,0,9,6,alt);textA(1,1,"Wren",alt);sfmt(b,16,"HP %d",P.hp);textA(1,2,b,alt);sfmt(b,16,"MP %d",P.mp);textA(1,3,b,alt);sfmt(b,16,"LV %d",P.lv);textA(1,4,b,alt);
  if(B.anim!=AN_GONE){
    char w[2][12];int nw=0,ml=0;const char*p=M->name;
    while(*p&&nw<2){int l=0;while(*p&&*p!=' '&&l<11)w[nw][l++]=*p++;w[nw][l]=0;ml=imax(ml,l);nw++;while(*p==' ')p++;}
    int pw=ml+2,px=30-pw;panel(px,8,pw,nw+2);for(int i=0;i<nw;i++)text(px+1,9+i,w[i]);
  }
}
/* OAM helpers */
static void objHide(int i){OAM16[i*4]=0x0200;}
static void objSet(int i,int x,int y,int shape,int size,int unit,int prio,int hflip,int w,int h){
  if(x<=-w||x>=240||y<=-h||y>=160){objHide(i);return;}
  OAM16[i*4+0]=(u16)((y&255)|0x2000|(shape<<14));
  OAM16[i*4+1]=(u16)((x&511)|(size<<14)|(hflip?0x1000:0));
  OAM16[i*4+2]=(u16)(unit|(prio<<10));
}
static void playerPix(int*px,int*py){
  if(moving){*px=(mfx*16)+((P.x-mfx)*16*mt)/STEP;*py=(mfy*16)+((P.y-mfy)*16*mt)/STEP;}else{*px=P.x*16;*py=P.y*16;}
}
static u16 mosaicV=0;
static void setMosaic(u16 v){mosaicV=v;REG_MOSAIC=v;}
#define MON_CX 112
#define MON_OAM 39
static int drawEffects(int o){
  int t=B.fxT;
  switch(B.fx){
    case FX_SLASH:objSet(o++,MON_CX-16,50,0,2,(SLOT_SLASH+imin(2,t/4)*4)*8,0,0,32,32);break;
    case FX_CRIT:if(t<12)objSet(o++,MON_CX-20,46,0,2,(SLOT_SLASH+imin(2,t/4)*4)*8,0,0,32,32);
      else objSet(o++,MON_CX-12,54,0,2,(SLOT_SLASH+imin(2,(t-12)/4)*4)*8,0,1,32,32);break;
    case FX_SPARK:if((t/2)%3!=2){int f=(t/2)&1;objSet(o++,MON_CX-8,18,2,2,(SLOT_BOLT+f*2)*8,0,0,16,32);objSet(o++,MON_CX-8,50,2,2,(SLOT_BOLT+(1-f)*2)*8,0,0,16,32);}break;
    case FX_EBOLT:if((t/2)%3!=2){int f=(t/2)&1;objSet(o++,MON_CX-8,64,2,2,(SLOT_BOLT+f*2)*8,0,0,16,32);objSet(o++,MON_CX-8,96,2,2,(SLOT_BOLT+(1-f)*2)*8,0,0,16,32);}break;
    case FX_BLAZE:for(int i=0;i<6;i++){int y=92-((t*3+i*11)%48);int x=MON_CX-8+((i*13)%5-2)*9;objSet(o++,x,y,0,1,(SLOT_FLAME+((t/3+i)&1))*8,0,0,16,16);}break;
    case FX_EFIRE:for(int i=0;i<7;i++){int y=56+imax(0,t*3-i*5);int x=16+i*32-(t%4);if(y<150)objSet(o++,x,y,0,1,(SLOT_FLAME+((t/3+i)&1))*8,0,0,16,16);}break;
    case FX_HEAL:for(int i=0;i<5;i++){int y=44-((t*2+i*9)%40);int x=4+(i*17)%56;objSet(o++,x,y,0,1,(SLOT_STAR+((t/4+i)&1))*8,0,0,16,16);}break;
    case FX_MHEAL:for(int i=0;i<5;i++){int y=96-((t*2+i*9)%56);int x=MON_CX-24+(i*13)%48;objSet(o++,x,y,0,1,(SLOT_STAR+((t/4+i)&1))*8,0,0,16,16);}break;
    case FX_ESPORE:for(int i=0;i<7;i++){int y=50+((t*3+i*13)%90);int x=20+(i*31)%200;objSet(o++,x,y,0,1,(SLOT_STAR+((t/4+i)&1))*8,0,0,16,16);}break;
    case FX_CLAW:if(t<10)objSet(o++,MON_CX-16,74,0,2,SLOT_CLAW*8,0,(t/2)&1,32,32);break;
    case FX_GUARD:if((t/2)&1){if(B.fxArg)objSet(o++,20,30,0,2,SLOT_SHIELD*8,0,0,32,32);else objSet(o++,MON_CX-16,56,0,2,SLOT_SHIELD*8,0,0,32,32);}break;
  }
  return o;
}
static int drawPopups(int o){
  for(int k=0;k<2;k++){Popup*p=&pops[k];if(!p->on)continue;
    int t=p->t;int dy=t<6?-2*t:t<10?-12+(t-6)*2:-4;
    char b[8];sfmt(b,8,"%d",p->val);int n=(int)strlen(b);int x0=p->x-n*7/2;
    for(int i=0;i<n;i++)objSet(o++,x0+i*7,p->y+dy,0,0,(SLOT_DIGIT*8)+((p->green?10:0)+(b[i]-'0'))*2,0,0,8,8);
  }
  return o;
}
static void drawMonster(void){
  int sx=256,sy=256,ox=0,oy=0,vis=1,mos=0,t=frameT;
  sy=256-5+(5*isin(t*12))/256;sx=256+3-(3*isin(t*12))/256;
  switch(B.anim){
    case AN_HIT:ox=(B.animT&2)?3:-3;vis=((B.animT>>1)&1)==0;break;
    case AN_LUNGE:{int k=B.animT<8?B.animT:16-B.animT;sx=sy=256+k*9;oy=k*2;}break;
    case AN_DIE:sy=256-B.animT*8;sx=256+B.animT*4;mos=imin(15,B.animT*15/20);if(B.animT>=22)vis=0;break;
    case AN_GONE:vis=0;break;
  }
  if(B.intro>0){mos=imin(15,B.intro*15/24);}
  if(!vis||sy<16||B.intro>=24){objHide(MON_OAM);setMosaic((u16)(mosaicV&0x00FF));return;}
  int cx=MON_CX+ox,cy=100-(32*sy)/256+oy;
  OAM16[MON_OAM*4]=(u16)(((cy-64)&255)|0x0100|0x0200|0x2000|(mos?0x1000:0));
  OAM16[MON_OAM*4+1]=(u16)(((cx-64)&511)|(3<<14));
  OAM16[MON_OAM*4+2]=(u16)((SLOT_MON*8)|(1<<10));
  OAM16[3]=(u16)(65536/sx);OAM16[7]=0;OAM16[11]=0;OAM16[15]=(u16)(65536/sy);
  setMosaic((u16)((mosaicV&0x00FF)|(mos<<8)|(mos<<12)));
}
static void renderMode0(void){
  if(!gfxOK)reloadGfx();
  if(curSet!=MAPS[P.map].set)loadSet(MAPS[P.map].set);
  if(curSpriteMap!=P.map)loadMapSprites(P.map);
  int sx=0,sy=0;if(shake>0){sx=rnd(-2,2);sy=rnd(-2,2);}
  int px,py;playerPix(&px,&py);int cx=px-112,cy=py-72;
  /* BG0 metatiles */
  int mx0=cx>>4,my0=cy>>4;
  for(int my=my0;my<=my0+10;my++)for(int mx=mx0;mx<=mx0+15;mx++){
    int base=metaAt(mx,my)*4;
    int tx=(mx*2)&63,ty=(my*2)&31;
    for(int k=0;k<4;k++){int x=tx+(k&1),y=ty+(k>>1);int sb=28+(x>>5);VRAM16[sb*0x400+y*32+(x&31)]=(u16)(base+k);}
  }
  REG_BG0HOFS=(u16)(cx-sx);REG_BG0VOFS=(u16)(cy-sy);
  int af=(frameT/30)&1;
  if(af!=animFrame){animFrame=af;for(int i=0;i<animN;i++)uploadBytes(CB0,animMeta[i]*4,ANIMBUF+i*512+af*256,4);}
  const Map*M=&MAPS[P.map];int used=0;
  int inBattle=(mode==MODE_BATTLE&&B.on);
  int showBattle=inBattle&&B.intro<24;
  if(!inBattle){
    struct{int y,x,slot;}ents[16];int ne=0;int an=(frameT/16)&1;
    for(int i=0;i<M->nc;i++){ents[ne].y=M->chests[i].y*16;ents[ne].x=M->chests[i].x*16;ents[ne].slot=P.chests[M->chests[i].id]?SLOT_CHESTO:SLOT_CHEST;ne++;}
    for(int i=0;i<M->nn;i++){Npc*n=&M->npcs[i];ents[ne].y=n->y*16;ents[ne].x=n->x*16;ents[ne].slot=npcSlotOf[n->pal]+n->dir*2+an;ne++;}
    if(M->bx>=0&&!P.fBoss){ents[ne].y=M->by*16;ents[ne].x=M->bx*16;ents[ne].slot=SLOT_BOSS;ne++;}
    ents[ne].y=py;ents[ne].x=px;ents[ne].slot=SLOT_HERO+P.dir*2+(moving?an:((frameT/32)&1));ne++;
    for(int i=1;i<ne;i++){int j=i;while(j>0&&ents[j].y>ents[j-1].y){int ty=ents[j].y,tx=ents[j].x,ts=ents[j].slot;ents[j]=ents[j-1];ents[j-1].y=ty;ents[j-1].x=tx;ents[j-1].slot=ts;j--;}}
    for(int i=0;i<ne;i++)objSet(used++,ents[i].x-cx+sx,ents[i].y-cy+sy,0,1,ents[i].slot*8,1,0,16,16);
    setMosaic(0);
  }else{
    if(showBattle){drawMonster();used=drawPopups(0);used=drawEffects(used);}
    else{objHide(MON_OAM);used=0;}
    int lvl=B.intro>=24?imin(15,(44-B.intro)):15;
    setMosaic((u16)((mosaicV&0xFF00)|lvl|(lvl<<4)));
  }
  for(int i=used;i<(inBattle?MON_OAM:40);i++)objHide(i);
  /* BG1 UI */
  memset(bg1,0,sizeof bg1);
  if(showBattle)drawBattleUI();
  if(mode==MODE_MAP&&!busy()&&!fade.on){
    if(bannerText&&bannerT>0){int w=(int)strlen(bannerText)+2;int x=(30-w)/2;panel(x,16,w,3);text(x+1,17,bannerText);}
    if(idle>60)drawStat();
  }
  if(choiceOn){if(ch.status)drawStat();else if(ch.gold){char b[16];panel(0,0,11,3);sfmt(b,16,"G %d",(int)P.gold);text(1,1,b);}}
  if(box.active&&!(inBattle&&!showBattle))drawBox();
  if(choiceOn)drawChoice();
  for(int i=0;i<32*32;i++)VRAM16[30*0x400+i]=bg1[i];
  REG_BG1HOFS=(u16)(-sx);REG_BG1VOFS=(u16)(-sy);
  REG_BG2HOFS=(u16)(-sx);REG_BG2VOFS=(u16)(-sy);
  REG_BG0CNT=(u16)(2|(0<<2)|0x80|(28<<8)|(1<<14)|(inBattle?0x40:0));
  u16 dc=0x0040|0x0100|0x0200|0x1000;
  if(showBattle)dc|=0x0400;
  if(M->dark&&!inBattle){dc|=0x2000;REG_WIN0H=(u16)((80<<8)|160);REG_WIN0V=(u16)((40<<8)|120);REG_WININ=0x17;REG_WINOUT=0x02;}
  REG_DISPCNT=dc;
}
static void renderBlend(void){
  int fxFlash=0;
  if(mode==MODE_BATTLE&&B.on&&B.fx){int t=B.fxT;
    if((B.fx==FX_SPARK||B.fx==FX_EBOLT)&&t%4<2)fxFlash=10;
    if((B.fx==FX_BLAZE||B.fx==FX_EFIRE)&&t%6<2)fxFlash=6;
    if(B.fx==FX_CRIT&&t>=12&&t<15)fxFlash=14;}
  if(fade.on){int h=fade.dur/2;int lv=16*(h-iabs(fade.t-h))*115/(h*100);if(lv>16)lv=16;REG_BLDCNT=0x3F|(3<<6);REG_BLDY=(u16)lv;}
  else if(mode==MODE_BATTLE&&B.on&&B.intro>=24&&((B.intro>>2)&1)==0){REG_BLDCNT=0x3F|(2<<6);REG_BLDY=12;}
  else if(fxFlash){REG_BLDCNT=0x3F|(2<<6);REG_BLDY=(u16)fxFlash;}
  else if(mode==MODE_BATTLE){REG_BLDCNT=0x01|(3<<6);REG_BLDY=12;}
  else REG_BLDCNT=0;
}
static void render(void){
  if(palDirty)applyPalette();
  if(mode==MODE_MAP||mode==MODE_BATTLE)renderMode0();
  renderBlend();
}
static int frameTCount(void){return frameT;}

/* ================= INIT / MAIN ================= */
static void gameInit(void){
  volatile const char*keep=SAVE_TAG;(void)keep;
  REG_WAITCNT=0x4317;
  REG_SNDCNT_X=0x80;REG_SNDCNT_L=0xFF77;REG_SNDCNT_H=0x0002;
  bgPal[0]=0;bgPal[1]=rgb15(0x0b0914);bgPal[2]=rgb15(0xf4f0ff);bgPal[3]=rgb15(0x6a6080);bgPal[4]=rgb15(0xffa040);
  objPal[0]=0;
  REG_BG0CNT=(u16)(2|(0<<2)|0x80|(28<<8)|(1<<14));
  REG_BG1CNT=(u16)(0|(1<<2)|0x80|(30<<8));
  findPos('A',posA);findPos('B',posB);findPos('C',posC);findPos('K',posK);
  {const Map*C=&MAPS[MAP_CAVE];for(int y=0;y<C->h;y++)for(int x=0;x<C->w;x++)if(C->rows[y][x]=='S'){caveIn[0]=x+1;caveIn[1]=y;}}
  Player t;hasSaveFlag=loadSave(&t);titleSel=hasSaveFlag?1:0;
  newPlayer();
  mode=MODE_TITLE;enterMode4();drawTitleScene();applyPalette();
}
static void gameFrame(void){
  update();vsync();
  /* BG2CNT must be (re)set after leaving bitmap mode */
  if(mode==MODE_MAP||mode==MODE_BATTLE)REG_BG2CNT=(u16)(1|(2<<2)|0x80|(31<<8));
  render();
}

#ifndef HOST
int main(void){gameInit();for(;;)gameFrame();return 0;}
#endif
