#!/usr/bin/env python3
"""Asset generator for The Shattered Crown (GBA).
All art is authored here as text and converted to GBA 4bpp tiles.
Run:  python3 gen_assets.py   -> writes assets.c / assets.h
Only needed when changing art or maps; the generated files are committed."""
import random
from font5x8 import G as FONT

def rgb15(r, g, b):
    return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)

# ----------------------------------------------------------------- palettes
UI_BASE = [(0,0,0),(248,248,248),(40,40,72),(32,56,128),(232,232,248),
           (152,160,200),(16,16,40),(248,216,96),(0,0,0)] + [(0,0,0)]*7
UI_GREY = list(UI_BASE); UI_GREY[1] = (128,136,168)
UI_GOLD = list(UI_BASE); UI_GOLD[1] = (248,216,96)
UI_NARR = list(UI_BASE); UI_NARR[3] = (0,0,0); UI_NARR[2] = (48,48,72)

EXT = "kGglDdWwTBRrSsY"
EXT_COL = [(24,20,32),(88,168,64),(56,120,48),(136,208,88),(208,168,104),
           (160,120,72),(48,104,200),(120,176,240),(24,80,40),(120,72,40),
           (184,56,48),(120,32,32),(232,224,200),(152,140,120),(248,208,64)]
INT = "kFfWwCcRrYGgBbS"
INT_COL = [(24,20,32),(176,120,64),(136,88,48),(224,208,176),(176,160,128),
           (152,96,56),(104,56,32),(176,56,56),(128,32,40),(240,200,72),
           (72,160,64),(40,104,40),(72,112,200),(48,72,144),(248,248,240)]

def pal16(chars, cols):
    out = [(0,0,0)] + cols
    out += [(0,0,0)] * (16 - len(out))
    return out, {c: i + 1 for i, c in enumerate(chars)}

EXT_PAL, EXT_IDX = pal16(EXT, EXT_COL)
INT_PAL, INT_IDX = pal16(INT, INT_COL)

# sprite palettes: index meaning shared by both sprite sheets
SPR = "ksSHhTtCbyae"
def spal(hair, hair_l, tunic, tunic_d, pants, belt=(216,176,64), bag=(200,160,96), skin=(248,200,160), skin_d=(208,144,112)):
    cols = [(32,24,40), skin, skin_d, hair, hair_l, tunic, tunic_d, pants,
            (64,48,40), belt, bag, (248,248,248)]
    return pal16(SPR, cols)
OBJ_PALS = [
    spal((112,64,32),(168,104,56),(40,144,160),(24,96,112),(96,64,48)),       # 0 Rowan
    spal((72,48,32),(112,80,48),(96,160,72),(56,112,48),(112,88,64)),         # 1 villager
    spal((192,192,200),(240,240,248),(120,80,160),(80,48,112),(80,64,96)),    # 2 elder
    spal((216,112,40),(248,160,72),(232,184,64),(184,128,40),(96,72,56)),     # 3 child
    spal((120,136,160),(184,200,216),(72,96,144),(48,64,104),(64,64,80)),     # 4 guard
    spal((40,32,32),(80,64,56),(232,224,208),(176,160,144),(120,72,48)),      # 5 innkeeper
    spal((176,72,48),(216,120,80),(200,88,120),(144,56,88),(104,72,72)),      # 6 villager B
]
SPR_IDX = OBJ_PALS[0][1]

# ------------------------------------------------------------- tile encoding
def enc_tile(px):  # px: 8x8 list of ints -> 8 u32 words
    out = []
    for y in range(8):
        w = 0
        for x in range(8):
            w |= (px[y][x] & 15) << (4 * x)
        out.append(w)
    return out

def split8(img, w, h):  # img[h][w] -> tiles row-major
    tiles = []
    for ty in range(h // 8):
        for tx in range(w // 8):
            tiles.append([img[ty*8+y][tx*8:tx*8+8] for y in range(8)])
    return tiles

# ------------------------------------------------------------- UI / font
ui_tiles = [[[0]*8 for _ in range(8)] for _ in range(128)]   # index == ascii
def font_tile(rows, fg=1, bg=3, sh=2):
    t = [[bg]*8 for _ in range(8)]
    ink = [[rows[y][x] == '#' for x in range(5)] for y in range(8)]
    for y in range(8):
        for x in range(5):
            if ink[y][x]:
                if y + 1 < 8 and x + 2 < 8: t[y+1][x+2] = sh
    for y in range(8):
        for x in range(5):
            if ink[y][x]: t[y][x+1] = fg
    return t
for ch, rows in FONT.items():
    ui_tiles[ord(ch)] = font_tile(rows)
# window frame: 1 TL 2 T 3 TR 4 L 5 R 6 BL 7 B 8 BR
edge = [6, 4, 5]  # outer -> inner
def frame_tile(top, left, bottom, right):
    t = [[3]*8 for _ in range(8)]
    for y in range(8):
        for x in range(8):
            d = []
            if top: d.append(y)
            if bottom: d.append(7 - y)
            if left: d.append(x)
            if right: d.append(7 - x)
            if d:
                m = min(d)
                if m < 3: t[y][x] = edge[m]
    # round corners
    def clr(x, y): t[y][x] = 0
    if top and left: clr(0,0); clr(1,0); clr(0,1)
    if top and right: clr(7,0); clr(6,0); clr(7,1)
    if bottom and left: clr(0,7); clr(1,7); clr(0,6)
    if bottom and right: clr(7,7); clr(6,7); clr(7,6)
    return t
ui_tiles[1] = frame_tile(1,1,0,0); ui_tiles[2] = frame_tile(1,0,0,0)
ui_tiles[3] = frame_tile(1,0,0,1); ui_tiles[4] = frame_tile(0,1,0,0)
ui_tiles[5] = frame_tile(0,0,0,1); ui_tiles[6] = frame_tile(0,1,1,0)
ui_tiles[7] = frame_tile(0,0,1,0); ui_tiles[8] = frame_tile(0,0,1,1)
# 9: "more" arrow (gold)
arrow = ["......","......","#####.",".###..","..#...","......"]
t = [[3]*8 for _ in range(8)]
for y, r in enumerate(arrow):
    for x, c in enumerate(r):
        if c == '#': t[y+1][x+2] = 7
ui_tiles[9] = t
# 10: solid black (for full-screen fades / backgrounds)
ui_tiles[10] = [[8]*8 for _ in range(8)]
# 11..15: HP/MP bar segments (fill 0..4 px pairs) -- reserved for later builds
for i in range(5):
    t = [[3]*8 for _ in range(8)]
    for x in range(8):
        t[3][x] = 6; t[5][x] = 6
        t[4][x] = 7 if x < i * 2 else 6
    ui_tiles[11 + i] = t

# ------------------------------------------------------------- metatiles
def canvas(fill): return [[fill]*16 for _ in range(16)]
def overlay(cv, art, idx):
    for y, row in enumerate(art):
        for x, c in enumerate(row):
            if c != '.': cv[y][x] = idx[c]
    return cv
def speckle(cv, idx, seed, pairs):
    r = random.Random(seed)
    for ch, n in pairs:
        for _ in range(n):
            cv[r.randrange(16)][r.randrange(16)] = idx[ch]
    return cv
E = EXT_IDX; I = INT_IDX
def grass(seed=1): return speckle(canvas(E['G']), E, seed, [('g', 10), ('l', 6)])

MT = []   # (name, 16x16 pixels, palbank, flags)
F_SOLID, F_COUNTER, F_WATER = 1, 2, 4
PB_EXT, PB_INT, PB_TITLE = 4, 5, 6
def mt(name, px, pb, flags=0): MT.append((name, px, pb, flags))

mt('VOID', [[I['k']]*16 for _ in range(16)], PB_INT, F_SOLID)
mt('GRASS', grass(1), PB_EXT)
mt('GRASS2', overlay(grass(2), [
 "................","...l......l.....","..lgl....lgl....","..g.g....g.g....",
 "................","................","........l.......",".......lgl......",
 ".......g.g......","................","..l.............",".lgl........l...",
 ".g.g.......lgl..","............g.g.","................","................"], E), PB_EXT)
mt('FLOWERS', overlay(grass(3), [
 "................","..S.............",".SYS.......R....","..S.......RYR...",
 "...g.......R....","...g.......g....","................","......S.........",
 ".....SYS........","......S...S.....","......g..SYS....","..R.......S.....",
 ".RYR......g.....","..R.............","..g.............","................"], E), PB_EXT)
mt('PATH', speckle(canvas(E['D']), E, 4, [('d', 14), ('s', 4), ('S', 3)]), PB_EXT)
w = canvas(E['W'])
for (x, y) in [(2,3),(3,3),(4,3),(10,6),(11,6),(12,6),(5,11),(6,11),(7,11),(13,13),(14,13)]:
    w[y][x] = E['w']
mt('WATER', w, PB_EXT, F_SOLID | F_WATER)
mt('TREE', overlay(grass(5), [
 "....kkkkkkk.....","..kkglllggkk....",".kgglllllgggk...",".kgllllgggggTk..",
 "kgglllgggggTTTk.","kggggggggggTgTk.","kTggggggTgggTTk.","kTTgggTgggTTTTk.",
 ".kTTTgggTTTTTk..",".kkTTTTTTTTTkk..","...kkTkBBkTkk...",".....kBBBk......",
 ".....kBdBk......","....kBBBBBk.....",".....kkkkk......","................"], E), PB_EXT, F_SOLID)
roof = canvas(E['R'])
for y in range(16):
    for x in range(16):
        if y % 4 == 3: roof[y][x] = E['r']
        elif (x + (y // 4) * 4) % 8 == 0: roof[y][x] = E['r']
mt('ROOF', roof, PB_EXT, F_SOLID)
rt = [r[:] for r in roof]
for x in range(16): rt[0][x] = E['k']; rt[1][x] = E['r']; rt[2][x] = E['R'] if x % 2 else E['r']
mt('ROOF_TOP', rt, PB_EXT, F_SOLID)
def wall_base():
    c = canvas(E['S'])
    for x in range(16):
        c[0][x] = E['k']; c[1][x] = E['r']; c[2][x] = E['B']
        c[13][x] = E['B']; c[14][x] = E['s']; c[15][x] = E['s']
    for y in range(3, 13):
        c[y][7] = E['B']; c[y][15] = E['s']
    return c
mt('WALL', wall_base(), PB_EXT, F_SOLID)
mt('WINDOW', overlay(wall_base(), [
 "................","................","................","...BBBBBBBBBB...",
 "...BwwwwBwwwB...","...BwWWWBwWWB...","...BwWWWBwWWB...","...BBBBBBBBBB...",
 "...BwWWWBwWWB...","...BwWWWBwWWB...","...BBBBBBBBBB...","..ssssssssssss..",
 "................","................","................","................"], E), PB_EXT, F_SOLID)
door_art = [
 "................","................","................","..kkkkkkkkkkkk..",
 "..kBBdBBBBdBBk..","..kBBdBBBBdBBk..","..kBBdBBBBdBBk..","..kBBdBBBBdBBk..",
 "..kBBdBBBBdBYk..","..kBBdBBBBdBBk..","..kBBdBBBBdBBk..","..kBBdBBBBdBBk..",
 "..kBBdBBBBdBBk..","..kBBdBBBBdBBk..","..kddddddddddk..","..kkkkkkkkkkkk.."]
mt('DOOR', overlay(wall_base(), door_art, E), PB_EXT)
mt('DOOR_LOCKED', overlay(wall_base(), door_art, E), PB_EXT, F_SOLID)
mt('FENCE', overlay(grass(6), [
 "................","................","................","..kk........kk..",
 ".kBBk......kBBk.","kBBBBkkkkkkBBBBk","kddddddddddddddk","kBBBBkkkkkkBBBBk",
 ".kBBk......kBBk.","kkBBkkkkkkkkBBkk","kddddddddddddddk","kkBBkkkkkkkkBBkk",
 ".kBBk......kBBk.",".kddk......kddk.","..kk........kk..","................"], E), PB_EXT, F_SOLID)
mt('SIGN', overlay(grass(7), [
 "................","................","..kkkkkkkkkkkk..",".kBBBBBBBBBBBBk.",
 ".kBddddddddddBk.",".kBBBBBBBBBBBBk.",".kBddddddBddBBk.",".kBBBBBBBBBBBBk.",
 "..kkkkkBBkkkkk..","......kBdk......","......kBdk......","......kBdk......",
 ".....kkBdkk.....","....g.kkkk.g....","................","................"], E), PB_EXT, F_SOLID)
mt('RUBBLE', overlay(speckle(canvas(E['D']), E, 8, [('d', 20), ('k', 6)]), [
 "................","..kkk.......ss..",".kBBBk.....sSSs.",".kdBBBkk...sssk.",
 "..kkdBBBk...kk..","....kkdBBk......","......kkdBk.kk..",".ss.....kkkkSSk.",
 "sSSs........ssk.","sssk..kkkkk.....",".kk..kBBBBBk....",".....kddddddk...","......kkkkkk..s.",
 "..s........ksSs.",".sSs.......ksss.","..k.........kk.."], E), PB_EXT, F_SOLID)
mt('STONE', overlay(grass(9), [
 "................","................","....kkkkkk......","...kSSSSssk.....",
 "..kSSSSSssssk...","..kSSSSssssssk..",".kSSSsssssssssk.",".kSSssssssssssk.",
 ".ksssssssssssdk.","..kssssssssddk..","...kkddddddkk...",".....kkkkkk.....",
 "................","................","................","................"], E), PB_EXT, F_SOLID)
mt('WELL', overlay(grass(10), [
 "................","....kkkkkkkk....","..kksSSSSSSskk..",".ksSSssssssSSsk.",
 ".kSskkkkkkkksSk.","kSsk.WWWWWW.ksSk","kSskWWwwWWWWksSk","kSskWWWWWwwWksSk",
 "kSskWWWWWWWWksSk","kSsk.WWWWWW.ksSk",".kSskkkkkkkksSk.",".ksSSssssssSSsk.",
 "..kksSSSSSSskk..","....kkkkkkkk....","................","................"], E), PB_EXT, F_SOLID)
# interior
def floor():
    c = canvas(I['F'])
    for y in range(16):
        for x in range(16):
            if y % 4 == 3: c[y][x] = I['f']
            elif x == (5 if (y // 4) % 2 else 13): c[y][x] = I['f']
    return c
mt('FLOOR', floor(), PB_INT)
beam = canvas(I['c'])
for x in range(16): beam[15][x] = I['k']; beam[0][x] = I['C']
mt('IWALL_TOP', beam, PB_INT, F_SOLID)
iw = canvas(I['W'])
for x in range(16):
    for y in (0, 1): iw[y][x] = I['w']
    for y in range(11, 16): iw[y][x] = I['C'] if y > 11 else I['c']
    iw[13][x] = I['c'] if x % 4 == 0 else iw[13][x]
mt('IWALL', iw, PB_INT, F_SOLID)
mt('SHELF', overlay([r[:] for r in iw], [
 "................","................","...R..Y...B..G..","..RRr.YY.BBbGGg.",
 "..RRr.YY.BBbGGg.","kccccccccccccccc","cccccccccccccccc","..Y..B..R..S....",
 ".YYYBBb.RRrSSS..",".YYYBBb.RRrSSS..","kccccccccccccccc"], I), PB_INT, F_SOLID)
ctr = canvas(I['C'])
for x in range(16):
    ctr[0][x] = I['Y']; ctr[1][x] = I['C']
    for y in range(10, 16): ctr[y][x] = I['c']
    ctr[10][x] = I['k']; ctr[15][x] = I['k']
mt('COUNTER', ctr, PB_INT, F_SOLID | F_COUNTER)
mt('BED', overlay(floor(), [
 ".kkkkkkkkkkkkkk.",".kcccccccccccck.",".kcSSSSSSSSSSck.",".kcSSSSSSSSSSck.",
 ".kcSSSSSSSSSSck.",".kBBBBBBBBBBBBk.",".kBbBBBBBBBBbBk.",".kBBBBBBBBBBBBk.",
 ".kBBBBBBBBBBBBk.",".kBbBBBBBBBBbBk.",".kBBBBBBBBBBBBk.",".kBBBBBBBBBBBBk.",
 ".kbbbbbbbbbbbbk.",".kcccccccccccck.",".kkkkkkkkkkkkkk.","................"], I), PB_INT, F_SOLID)
mt('TABLE', overlay(floor(), [
 "................","....kkkkkkkk....","..kkCCCCCCCCkk..",".kCCCCCYCCCCCCk.",
 ".kCCCCYSYCCCCCk.","kCCCCCkYkCCCCCCk","kCCCCCCCCCCCSCCk","kCCCCCCCCCCSSSCk",
 "kcCCCCCCCCCCSCck",".kcCCCCCCCCCCck.",".kkccccccccccck.","..kkkcccccckkk..",
 "....kkkkkkkk....","................","................","................"], I), PB_INT, F_SOLID)
rug = canvas(I['R'])
for y in range(16):
    for x in range(16):
        if (x + y) % 8 == 0 or (x - y) % 8 == 0: rug[y][x] = I['r']
mt('RUG', rug, PB_INT)
mt('EXIT_MAT', overlay(floor(), [
 "................","................","................","................",
 "................","................","..kkkkkkkkkkkk..","..kYYYYYYYYYYk..",
 "..kYrrrrrrrrYk..","..kYrYYYYYYrYk..","..kYrrrrrrrrYk..","..kYYYYYYYYYYk..",
 "..kkkkkkkkkkkk..","................","................","................"], I), PB_INT)
mt('PLANT', overlay(floor(), [
 "................","....k.k..kk.....","...kGk.kkGGk....","..kGGGkGGgGk....",
 ".kGgGGGGGgGGk...",".kGGgGGgGGGgk...","..kGGGgGGGgk....","...kkGGgGGkk....",
 "....kkcckkk.....","....kCCCCCk.....","...kCCCCCCCk....","...kcCCCCCck....",
 "....kcccccck....",".....kkkkkk.....","................","................"], I), PB_INT, F_SOLID)

MT_ID = {m[0]: i for i, m in enumerate(MT)}
assert len(MT) <= 64

# build deduplicated BG tile set for map metatiles (charblock 1)
map_tiles = [[[0]*8 for _ in range(8)]]   # tile 0 = backdrop
tile_lookup = {}
mt_se = []
for name, px, pb, fl in MT:
    se = []
    for (ox, oy) in [(0,0),(8,0),(0,8),(8,8)]:
        t = [px[oy+y][ox:ox+8] for y in range(8)]
        key = (pb, tuple(tuple(r) for r in t))
        if key not in tile_lookup:
            tile_lookup[key] = len(map_tiles); map_tiles.append(t)
        se.append(tile_lookup[key] | (pb << 12))
    mt_se.append(se)

# ------------------------------------------------------------- maps
EXT_KEY = {'T':'TREE','.':'GRASS',',':'GRASS2','*':'FLOWERS','=':'PATH','~':'WATER',
           '^':'ROOF_TOP','R':'ROOF','H':'WALL','O':'WINDOW','D':'DOOR','L':'DOOR_LOCKED',
           '#':'FENCE','S':'SIGN','x':'RUBBLE','o':'STONE','U':'WELL'}
BRINDLEFORD = [
 "TTTTTTTTTTTTTT=TTTTTTTTTTTTTTT",
 "TT,....*....TT=TT....,....*.TT",
 "T..^^^^^^...,===...,.^^^^...,T",
 "T..RRRRRR....===.....RRRR....T",
 "T.*HOHHDHS...===.....HOLH..*.T",
 "T.....,=.....===.......=.....T",
 "T,....=====================..T",
 "T............=U=...........,.T",
 "T..,.......*.===..*..........T",
 "T.^^^^^......===....^^^^^..*.T",
 "T.RRRRR.....,===....RRRRR....T",
 "T.HOLOH......===....HOLOH....T",
 "T..S=........===......=......T",
 "T...========================.T",
 "T*.,.........===.....#x#xx#..T",
 "T~~~.....,...===.....xx=.....T",
 "T~~~~..*.....===..,..#xxx..#.T",
 "T~~~~~...o...S==..........*..T",
 "TT~~~~,.........,....o.....,TT",
 "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTT",
]
INN = [
 "TTTTTTTTTTTTTTT",
 "TWQWWWWWWWQWWWT",
 "T...pB.B..p...T",
 "TCCCCC.......pT",
 "T......rrr....T",
 "T..b...rrr.b..T",
 "T......rrr....T",
 "Tp............T",
 "T.............T",
 "TTTTTTTMTTTTTTT",
]
INT_KEY = {'T':'IWALL_TOP','W':'IWALL','Q':'SHELF','.':'FLOOR','C':'COUNTER','B':'BED',
           'b':'TABLE','r':'RUG','M':'EXIT_MAT','p':'PLANT'}
def build_map(rows, key):
    h = len(rows); w = len(rows[0])
    for r in rows: assert len(r) == w, (r, len(r), w)
    return w, h, [MT_ID[key[c]] for r in rows for c in r]
MAPS = {'brindleford': build_map(BRINDLEFORD, EXT_KEY), 'inn': build_map(INN, INT_KEY)}

# ------------------------------------------------------------- sprites
ROWAN = {
'down': [
 "................","......kkkk......","....kkhhhhkk....","...khhHHHHhhk...",
 "..khHHHHHHHHhk..","..kHHHHHHHHHHk..","..kHHsHHHHsHHk..","..kHsssssssSHk..",
 "...kskssssksk...","...kSsssssssk...","....kSssssSk....","....kkSSSSkk....",
 "...kTaTTTTTTk...","..kTTTaTTTTTTk..",".kstTTTaTTTTtsk.",".kSktTTTaTTTkSk.",
 "..kkyyyyyaaakk..","...kTTTTTaaak..."],
'up': [
 "................","......kkkk......","....kkhhhhkk....","...khhhhhhhhk...",
 "..khHhhhhhhHhk..","..kHHHHHHHHHHk..","..kHHHHHHHHHHk..","..kHHHHHHHHHHk..",
 "...kHHHHHHHHk...","...kHHHHHHHHk...","....kHHHHHHk....","....kkSSSSkk....",
 "...kTTTTTTaTk...","..kTTTTTTaTTTk..",".kstTTTTaTTTtsk.",".kSktTTaTTTTkSk.",
 "..kkaaayyyyykk..","...kaaaTTTTTk..."],
'side': [
 "................","......kkkk......","....kkhhhhk.....","...khhHHHHhk....",
 "..khHHHHHHHhk...","..kHHHHHHHHHk...","..kHHHHHsHHHk...","..kHHHHsssssk...",
 "...kHHHsssksk...","...kHHHssssssk..","....kHHSsssk....",".....kkSSkk.....",
 "....kTTTTTk.....","...kaTTTTTTk....","...kaTTTsTTk....","...kaaTTSTk.....",
 "...kaayyyyk.....","....kTTTTTk....."],
}
TOWN = {
'down': [
 "................","................",".....kkkkkk.....","....khhhhhhk....",
 "...khHHHHHHhk...","...kHHHHHHHHk...","...kHsHHHHsHk...","...kssssssssk...",
 "...kskssssksk...","...kSssssssSk...","....kSssssSk....","....kkTTTTkk....",
 "...kTTTTTTTTk...","..ksTTTTTTTTsk..","..kSkTTTTTTkSk..","...kyyyyyyyyk...",
 "...kTTTTTTTTk...","...ktTTTTTTtk..."],
'up': [
 "................","................",".....kkkkkk.....","....khhhhhhk....",
 "...khhhhhhhhk...","...kHHHHHHHHk...","...kHHHHHHHHk...","...kHHHHHHHHk...",
 "...kHHHHHHHHk...","....kHHHHHHk....","....kkHHHHkk....","....kkTTTTkk....",
 "...kTTTTTTTTk...","..ksTTTTTTTTsk..","..kSkTTTTTTkSk..","...kyyyyyyyyk...",
 "...kTTTTTTTTk...","...ktTTTTTTtk..."],
'side': [
 "................","................",".....kkkkkk.....","....khhhhhhk....",
 "...khHHHHHHhk...","...kHHHHHHHHk...","...kHHHHHsssk...","...kHHHHssksk...",
 "...kHHHHsssssk..","....kHHSssssk...",".....kSSsssk....","....kkTTTTkk....",
 "....kTTTTTTk....","....kTTTsTTk....","....kTTTSkTk....","....kyyyyyyk....",
 "....kTTTTTTk....","....ktTTTTtk...."],
}
LEGS_FRONT = {
 'stand': ["...kCCCCCCCCk...","...kCCCkkCCCk...","...kCCk..kCCk...","...kbbk..kbbk...","..kbbbk..kbbbk..","..kkkk....kkkk.."],
 'A':     ["...kCCCCCCCCk...","...kCCCkkCCCk...","...kCCk..kCCk...","...kbbk..kbbk...","..kbbbk...kkk...","..kkkk.........."],
 'B':     ["...kCCCCCCCCk...","...kCCCkkCCCk...","...kCCk..kCCk...","...kbbk..kbbk...","...kkk...kbbbk..","..........kkkk.."],
}
LEGS_SIDE = {
 'stand': ["....kCCCCCk.....","....kCCCCCk.....","....kCCkCCk.....","....kbbkbbk.....","....kbbkbbbk....",".....kkkkkkk...."],
 'A':     ["....kCCCCCk.....","...kCCCkCCCk....","..kCCk...kCCk...","..kbbk...kbbk...",".kbbk....kbbbk..",".kkk......kkkk.."],
 'B':     ["....kCCCCCk.....","....kCCCCCk.....","....kCCkCCk.....","...kbbkkbbk.....","...kbbbkbbbk....","....kkkkkkkk...."],
}
def sprite_frame(top, legs):
    rows = ["................"] * 8 + top + legs   # 8 blank rows -> 16x32
    assert len(rows) == 32, len(rows)
    return [[SPR_IDX[c] if c != '.' else 0 for c in r] for r in rows]
def sheet(art):
    frames = []
    for d in ('down', 'up', 'side'):
        legs = LEGS_SIDE if d == 'side' else LEGS_FRONT
        for f in ('stand', 'A', 'B'):
            frames.append(sprite_frame(art[d], legs[f]))
    return frames
SHEETS = [sheet(ROWAN), sheet(TOWN)]
obj_tiles = []
for sh in SHEETS:
    for fr in sh:
        obj_tiles += split8(fr, 16, 32)

# ------------------------------------------------------------- title image
from PIL import Image, ImageDraw
TITLE_COL = [(0,0,0),(8,8,24),(16,16,48),(32,24,72),(56,40,96),(88,56,112),
             (248,248,255),(160,176,232),(12,12,28),(28,28,52),(248,208,72),
             (200,144,40),(128,80,24),(248,240,200),(96,200,232),(40,24,40)]
img = Image.new('P', (240, 160), 1)
d = ImageDraw.Draw(img)
bands = [(0,1),(40,2),(75,3),(105,4),(128,5)]
for i, (y0, c) in enumerate(bands):
    y1 = bands[i+1][0] if i + 1 < len(bands) else 160
    d.rectangle([0, y0, 239, y1], fill=c)
    if i > 0:
        pc = bands[i-1][1]
        for y in range(y0, y0 + 4):
            for x in range(240):
                if (x + y) % 2 == 0 and (y - y0) < 2 or ((x + y) % 4 == 0 and (y - y0) < 4):
                    img.putpixel((x, y), pc)
rnd = random.Random(42)
for _ in range(70):
    x, y = rnd.randrange(240), rnd.randrange(110)
    img.putpixel((x, y), 7 if rnd.random() < 0.7 else 6)
for (x, y) in [(30,20),(205,14),(180,52),(20,70),(222,90)]:
    for dx, dy in [(0,0),(1,0),(-1,0),(0,1),(0,-1)]:
        img.putpixel((x+dx, y+dy), 6 if (dx, dy) == (0,0) else 7)
# far mountains + near hills + castle
d.polygon([(0,120),(30,96),(55,112),(90,88),(125,110),(160,92),(200,114),(240,98),(240,160),(0,160)], fill=9)
d.polygon([(0,140),(40,124),(80,134),(120,126),(170,136),(210,122),(240,130),(240,160),(0,160)], fill=8)
cx = 176
d.rectangle([cx-18, 112, cx+18, 130], fill=8)
d.rectangle([cx-4, 92, cx+4, 112], fill=8)          # beacon tower
d.polygon([(cx-6,92),(cx,84),(cx+6,92)], fill=8)
for tx in (cx-18, cx+12):
    d.rectangle([tx, 104, tx+6, 112], fill=8)
    d.polygon([(tx-1,104),(tx+3,98),(tx+7,104)], fill=8)
img.putpixel((cx, 88), 9)                           # the dark beacon
# crown
CROWN = [
 "..........k.............k............k..........",
 ".........kYk...........kYk..........kYk.........",
 ".........kYk...........kYk..........kYk.........",
 "........kYYYk.........kYYYk........kYYYk........",
 "........kYYYk.........kYYYk........kYYYk........",
 ".......kYYyYYk.......kYYyYYk......kYYyYYk.......",
 "..k....kYYyYYk...k...kYYyYYk...k..kYYyYYk....k..",
 ".kYk..kYYyyyYYk.kYk.kYYyyyYYk.kYk.kYYyyyYYk.kYk.",
 ".kYYkkYYyyyyyYYkYYYkYYyyyyyYYkYYYkYYyyyyyYYkkYYk",
 ".kYYYYYyyyyyyyYYYyYYYyyyyyyyYYYyYYYyyyyyyyYYYYYk",
 ".kYYyYYyyyyyyyyYyyyYyyyyyCyyyYyyyYyyyyyyyyYYyYYk",
 ".kYyyyyyyyyyyyyyyyyyyyyyCCyyyyyyyyyyyyyyyyyyyYk.",
 "..kyyyyyyyyyyyyyyyyyyyyCCyyyyyyyyyyyyyyyyyyyyk..",
 "..kkkkkkkkkkkkkkkkkkkkCCkkkkkkkkkkkkkkkkkkkkkk..",
 "..kHHHHHHHkHHHHHHHkHHHCCHHHHkHHHHHHHkHHHHHHHk...",
 "..kHHgHHHkcckHHHHkcckCCcckHHHkcckHHHHkcckHHHk...",
 "..kHgggHkcwwck..kcwCCwwck..kcwwck..kcwwckHHHk...",
 "..kHHgHHkcwwckHHkcCCwwwckHHkcwwckHHkcwwckHHHk...",
 "..kHHHHHHkcckHHHHkCCcckHHHHHkcckHHHHkcckHHHHk...",
 "..kHHHHHHHkHHHHHHCCkHHHHHHHHHHkHHHHHHHkHHHHHk...",
 "..kkkkkkkkkkkkkkCCkkkkkkkkkkkkkkkkkkkkkkkkkkk...",
]
CMAP = {'k':15,'Y':13,'y':10,'H':11,'h':12,'c':12,'w':14,'g':14,'C':1}
ox, oy = 120 - 48, 4
for y, row in enumerate(CROWN):
    for x, c in enumerate(row):
        if c != '.':
            for sy in range(2):
                for sx in range(2):
                    img.putpixel((ox + x*2 + sx, oy + y*2 + sy), CMAP[c])
# glow pixels from the crack (the seal light leaking)
for (x, y) in [(22,-2),(26,-4),(19,-5),(28,23),(23,24),(16,23)]:
    img.putpixel((ox + x*2, oy + y*2), 14)
def logo_text(text, x0, y0, scale, fg, shade, outline):
    w = len(text) * 6 * scale
    x0 = x0 if x0 >= 0 else (240 - w) // 2
    pix = set()
    for i, ch in enumerate(text):
        rows = FONT[ch]
        for yy in range(8):
            for xx in range(5):
                if rows[yy][xx] == '#':
                    for sy in range(scale):
                        for sx in range(scale):
                            pix.add((x0 + (i*6 + xx) * scale + sx, y0 + yy * scale + sy))
    for (x, y) in pix:
        for dx in (-1, 0, 1):
            for dy in (-1, 0, 1, 2):
                if (x+dx, y+dy) not in pix: img.putpixel((x+dx, y+dy), outline)
    for (x, y) in pix:
        img.putpixel((x, y), shade if (y - y0) % (8*scale) >= 4*scale else fg)
logo_text("THE", -1, 52, 1, 6, 7, 15)
logo_text("SHATTERED", -1, 63, 2, 13, 10, 15)
logo_text("CROWN", -1, 81, 2, 13, 10, 15)
img.putpalette([c for col in TITLE_COL for c in col])
img.resize((480, 320), Image.NEAREST).convert('RGB').save('preview_title.png')
tpx = [[img.getpixel((x, y)) for x in range(240)] for y in range(160)]
title_tiles = [[[0]*8 for _ in range(8)]]
tlook = {tuple(tuple(r) for r in title_tiles[0]): 0}
title_map = []
for t in split8(tpx, 240, 160):
    k = tuple(tuple(r) for r in t)
    if k not in tlook:
        tlook[k] = len(title_tiles); title_tiles.append(t)
    title_map.append(tlook[k] | (PB_TITLE << 12))
assert len(title_tiles) < 1100, len(title_tiles)

# ------------------------------------------------------------- previews
def pal_rgb(p): return [c for col in p for c in col]
def render_map(name, pal_list):
    w, h, cells = MAPS[name]
    im = Image.new('RGB', (w*16, h*16))
    for i, m in enumerate(cells):
        px = MT[m][1]; pb = MT[m][2]
        pal = EXT_PAL if pb == PB_EXT else INT_PAL
        for y in range(16):
            for x in range(16):
                im.putpixel(((i % w)*16 + x, (i // w)*16 + y), pal[px[y][x]] if px[y][x] else (0,0,0))
    im.resize((w*32, h*32), Image.NEAREST).save(f'preview_{name}.png')
render_map('brindleford', None); render_map('inn', None)
im = Image.new('RGB', (16*9, 32*len(SHEETS)), (90, 160, 90))
for s, sh in enumerate(SHEETS):
    pal = OBJ_PALS[0 if s == 0 else 1][0]
    for f, fr in enumerate(sh):
        for y in range(32):
            for x in range(16):
                if fr[y][x]: im.putpixel((f*16 + x, s*32 + y), pal[fr[y][x]])
im.resize((im.width*4, im.height*4), Image.NEAREST).save('preview_sprites.png')

# ------------------------------------------------------------- emit C
def c_words(name, tiles, ctype='u32'):
    words = [w for t in tiles for w in enc_tile(t)]
    body = ",\n".join("  " + ",".join(f"0x{w:08X}" for w in words[i:i+8]) for i in range(0, len(words), 8))
    return f"const {ctype} {name}[{len(words)}] __attribute__((aligned(4))) = {{\n{body}\n}};\n"
def c_u16(name, vals):
    body = ",\n".join("  " + ",".join(f"0x{v:04X}" for v in vals[i:i+12]) for i in range(0, len(vals), 12))
    return f"const u16 {name}[{len(vals)}] __attribute__((aligned(4))) = {{\n{body}\n}};\n"
def c_u8(name, vals):
    body = ",\n".join("  " + ",".join(f"{v}" for v in vals[i:i+30]) for i in range(0, len(vals), 30))
    return f"const u8 {name}[{len(vals)}] = {{\n{body}\n}};\n"
def pal_vals(p): return [rgb15(*c) for c in p]

out = ["/* GENERATED by gen_assets.py -- do not edit by hand. */", '#include "assets.h"', ""]
out.append(c_words("gfx_ui_tiles", ui_tiles))
out.append(c_words("gfx_map_tiles", map_tiles))
out.append(c_words("gfx_obj_tiles", obj_tiles))
out.append(c_words("gfx_title_tiles", title_tiles))
out.append(c_u16("gfx_title_map", title_map))
out.append(c_u16("pal_ui", pal_vals(UI_BASE) + pal_vals(UI_GREY) + pal_vals(UI_GOLD) + pal_vals(UI_NARR)))
out.append(c_u16("pal_ext", pal_vals(EXT_PAL)))
out.append(c_u16("pal_int", pal_vals(INT_PAL)))
out.append(c_u16("pal_title", pal_vals(TITLE_COL)))
out.append(c_u16("pal_obj", [v for p in OBJ_PALS for v in pal_vals(p[0])]))
out.append(c_u16("metatile_se", [v for se in mt_se for v in se]))
out.append(c_u8("metatile_flags", [m[3] for m in MT]))
for name, (w, h, cells) in MAPS.items():
    out.append(c_u8(f"maptiles_{name}", cells))
open("assets.c", "w").write("\n".join(out))

hdr = ["/* GENERATED by gen_assets.py -- do not edit by hand. */",
       "#ifndef ASSETS_H", "#define ASSETS_H", '#include "gba.h"', ""]
hdr.append(f"extern const u32 gfx_ui_tiles[{len(ui_tiles)*8}];")
hdr.append(f"extern const u32 gfx_map_tiles[{len(map_tiles)*8}];")
hdr.append(f"extern const u32 gfx_obj_tiles[{len(obj_tiles)*8}];")
hdr.append(f"extern const u32 gfx_title_tiles[{len(title_tiles)*8}];")
hdr.append("extern const u16 gfx_title_map[600];")
hdr.append("extern const u16 pal_ui[64], pal_ext[16], pal_int[16], pal_title[16];")
hdr.append(f"extern const u16 pal_obj[{16*len(OBJ_PALS)}];")
hdr.append(f"extern const u16 metatile_se[{len(MT)*4}];")
hdr.append(f"extern const u8 metatile_flags[{len(MT)}];")
hdr.append(f"#define UI_TILE_COUNT {len(ui_tiles)}")
hdr.append(f"#define MAP_TILE_COUNT {len(map_tiles)}")
hdr.append(f"#define OBJ_TILE_COUNT {len(obj_tiles)}")
hdr.append(f"#define TITLE_TILE_COUNT {len(title_tiles)}")
hdr.append(f"#define OBJ_PAL_COUNT {len(OBJ_PALS)}")
hdr.append("#define MTF_SOLID 1\n#define MTF_COUNTER 2\n#define MTF_WATER 4")
hdr.append("#define PB_UI 0\n#define PB_UI_GREY 1\n#define PB_UI_GOLD 2\n#define PB_UI_NARR 3")
hdr.append(f"#define PB_EXT {PB_EXT}\n#define PB_INT {PB_INT}\n#define PB_TITLE {PB_TITLE}")
hdr.append("#define SPRITE_FRAMES_PER_SHEET 9\n#define SPRITE_TILES_PER_FRAME 8")
hdr.append("enum { SHEET_ROWAN, SHEET_TOWN };")
hdr.append("enum { OPAL_ROWAN, OPAL_VILLAGER, OPAL_ELDER, OPAL_CHILD, OPAL_GUARD, OPAL_INNKEEP, OPAL_VILLAGER_B };")
hdr.append("enum {\n" + ",\n".join(f"  MT_{m[0]}" for m in MT) + ",\n  MT_COUNT\n};")
for name, (w, h, cells) in MAPS.items():
    hdr.append(f"#define MAPW_{name.upper()} {w}\n#define MAPH_{name.upper()} {h}")
    hdr.append(f"extern const u8 maptiles_{name}[{w*h}];")
hdr.append("#endif")
open("assets.h", "w").write("\n".join(hdr) + "\n")
print("ui", len(ui_tiles), "map", len(map_tiles), "obj", len(obj_tiles), "title", len(title_tiles), "metatiles", len(MT))
