
#ifndef __MSTAR__ABLMAPTLB_H__
#define __MSTAR__ABLMAPTLB_H__

#define ABLMAP_FLAG_DST_PREMULTIPLY      BIT(0)
#define ABLMAP_FLAG_SRC_PREMULTCOLOR     BIT(1)
#define ABLMAP_FLAG_SRC_PREMULTIPLY      BIT(2)
#define ABLMAP_FLAG_BLEND_ALPHACHANNEL   BIT(3)
#define ABLMAP_FLAG_BLEND_COLORALPHA     BIT(4)
#define ABLMAP_FLAG_DEMULTIPLY            BIT(5)
#define ABLMAP_FLAG_MAX                    BIT(6)

#define MSTAR_ABLMAPTBL_SIZE        33
#define MSTAR_ABLMAPFLAGTBL_SIZE   64
#define MSTAR_ABLBLDOP_SIZE     12

#define ABLMAP_ENTRY_IDX0      BIT(0)
#define ABLMAP_ENTRY_IDX1      BIT(1)
#define ABLMAP_ENTRY_IDX2      BIT(2)
#define ABLMAP_ENTRY_IDX3      BIT(3)
#define ABLMAP_ENTRY_IDX4      BIT(4)
#define ABLMAP_ENTRY_IDX5      BIT(5)
#define ABLMAP_ENTRY_IDX6      BIT(6)
#define ABLMAP_ENTRY_IDX7      BIT(7)
#define ABLMAP_ENTRY_IDX8      BIT(8)
#define ABLMAP_ENTRY_IDX9      BIT(9)
#define ABLMAP_ENTRY_IDX10     BIT(10)
#define ABLMAP_ENTRY_IDX11     BIT(11)
#define ABLMAP_ENTRY_IDX12     BIT(12)
#define ABLMAP_ENTRY_IDX13     BIT(13)
#define ABLMAP_ENTRY_IDX14     BIT(14)
#define ABLMAP_ENTRY_IDX15     BIT(15)
#define ABLMAP_ENTRY_IDX16     BIT(16)
#define ABLMAP_ENTRY_IDX17     BIT(17)
#define ABLMAP_ENTRY_IDX18     BIT(18)
#define ABLMAP_ENTRY_IDX19     BIT(19)
#define ABLMAP_ENTRY_IDX20     BIT(20)
#define ABLMAP_ENTRY_IDX21     BIT(21)
#define ABLMAP_ENTRY_IDX22     BIT(22)
#define ABLMAP_ENTRY_IDX23     BIT(23)
#define ABLMAP_ENTRY_IDX24     BIT(24)
#define ABLMAP_ENTRY_IDX25     BIT(25)
#define ABLMAP_ENTRY_IDX26     BIT(26)
#define ABLMAP_ENTRY_IDX27     BIT(27)
#define ABLMAP_ENTRY_IDX28     BIT(28)
#define ABLMAP_ENTRY_IDX29     BIT(29)
#define ABLMAP_ENTRY_IDX30     BIT(30)
#define ABLMAP_ENTRY_IDX31     BIT(31)

#define ABLMAP_ENTRY_IDX32     BIT(0)

typedef struct
{
    u32 u32HWBlendOp;
    u32 u32AlphaSrcFrom;
    u8 u8ConstAlpha;
    bool bTblConstAlpha;
}MSTARAblMapTblEntry;

typedef struct
{
    u32 u32FlagLo;
    u32 u32FlagHi;
}MSTARAblMapFlag;

MSTARAblMapTblEntry MSTARAblMapTbl[MSTAR_ABLMAPTBL_SIZE] =
{
    // hw_bld_op---------hw_alpha_op----------------------hw_constant_op---bTblConstAlpha
    {  0xFFFFFFFF,      0xFFFFFFFF,                  0x00,           true           },
    {  COEF_ONE,          ABL_FROM_ASRC,                  0x00,           false          },
    {  COEF_ONE,          ABL_FROM_CONST,                 0x00,           false          },
    {  COEF_ONE,          ABL_FROM_ROP8_IN,               0xFF,           true           },
    {  COEF_ONE,          ABL_FROM_ROP8_IN,               0x00,           false          },
    {  COEF_ONE,          ABL_FROM_ROP8_SRC,              0x00,           false          },
    {  COEF_ONE,          ABL_FROM_ROP8_SRCOUT,           0x00,           false          },
    {  COEF_ONE,          ABL_FROM_ROP8_SRCOUT,           0xFF,           true           },
    {  COEF_ZERO,         ABL_FROM_ADST,                  0x00,           false          },
    {  COEF_ZERO,         ABL_FROM_ROP8_IN,               0xFF,           true           },
    {  COEF_ZERO,         ABL_FROM_ROP8_IN,               0x00,           false          },
    {  COEF_ZERO,         ABL_FROM_ROP8_DSTOUT,           0xFF,           true           },
    {  COEF_ZERO,         ABL_FROM_ROP8_DSTOUT,           0x00,           false          },
    {  COEF_ASRC,         ABL_FROM_ROP8_OVER,             0xFF,           true           },
    {  COEF_ASRC,         ABL_FROM_ROP8_INV_SRC_ATOP_DST, 0xFF,           true           },
    {  COEF_CONST,        ABL_FROM_ADST,                  0x00,           false          },
    {  COEF_1_ASRC,       ABL_FROM_ROP8_INV_DST_ATOP_SRC, 0xFF,           true           },
    {  COEF_1_ADST,       ABL_FROM_CONST,                 0x00,           false          },
    {  COEF_1_ADST,       ABL_FROM_ROP8_OVER,             0xFF,           true           },
    {  COEF_1_ADST,       ABL_FROM_ROP8_OVER,             0x00,           false          },
    {  COEF_CONST_SRC,    ABL_FROM_ASRC,                  0x00,           false          },
    {  COEF_CONST_SRC,    ABL_FROM_CONST,                 0x00,           false          },
    {  COEF_CONST_SRC ,   ABL_FROM_ROP8_IN,               0x00,           false          },
    {  COEF_CONST_SRC,    ABL_FROM_ROP8_SRC,              0x00,           true           },
    {  COEF_CONST_SRC,    ABL_FROM_ROP8_SRC,              0x00,           false          },
    {  COEF_CONST_SRC ,   ABL_FROM_ROP8_SRCOUT,           0x00,           false          },
    {  COEF_SRC_XOR_DST,  ABL_FROM_ROP8_SRC_XOR_DST,      0xFF,           true           },
    {  COEF_SRC_XOR_DST,  ABL_FROM_ROP8_SRC_XOR_DST,      0x00,           false          },
    {  COEF_SRC_ATOP_DST, ABL_FROM_ADST,                  0x00,           false          },
    {  COEF_DST_ATOP_SRC, ABL_FROM_ROP8_SRC,              0x00,           false          },
    {  COEF_DST_ATOP_SRC, ABL_FROM_ASRC,                  0xFF,           true           },
    {  COEF_ROP8_SRCOVER, ABL_FROM_ROP8_OVER,             0x00,           false          },
    {  COEF_ROP8_DSTOVER, ABL_FROM_ROP8_OVER,             0x00,           false          },
};

MSTARAblMapFlag MSTARAblMapFlagTbl[MSTAR_ABLMAPFLAGTBL_SIZE] =
{
    { ABLMAP_ENTRY_IDX0,    0x0 },
    { ABLMAP_ENTRY_IDX0,    0x0 },
    { ABLMAP_ENTRY_IDX20,   0x0 },
    { ABLMAP_ENTRY_IDX20,   0x0 },
    { 0x0, 0x0 },
    { 0x0, 0x0 },
    { 0x0, 0x0 },
    { 0x0, 0x0 },
    { ABLMAP_ENTRY_IDX1|ABLMAP_ENTRY_IDX14|ABLMAP_ENTRY_IDX16|ABLMAP_ENTRY_IDX8|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX1|ABLMAP_ENTRY_IDX18|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX8|ABLMAP_ENTRY_IDX20|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX20|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX8|ABLMAP_ENTRY_IDX13|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX23|ABLMAP_ENTRY_IDX26|ABLMAP_ENTRY_IDX28|ABLMAP_ENTRY_IDX30, 0x0 },
    { ABLMAP_ENTRY_IDX8|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX2|ABLMAP_ENTRY_IDX8|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX2|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX8|ABLMAP_ENTRY_IDX21|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX21|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX8|ABLMAP_ENTRY_IDX21|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX21|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX8|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX5|ABLMAP_ENTRY_IDX8|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX5|ABLMAP_ENTRY_IDX19|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX8|ABLMAP_ENTRY_IDX23|ABLMAP_ENTRY_IDX24, 0x0 },
    { ABLMAP_ENTRY_IDX23|ABLMAP_ENTRY_IDX24, 0x0 },
    { ABLMAP_ENTRY_IDX8|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX23|ABLMAP_ENTRY_IDX27|ABLMAP_ENTRY_IDX28|ABLMAP_ENTRY_IDX29, 0x0 },
    { ABLMAP_ENTRY_IDX8|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX23, 0x0 },
    { 0x0, 0x0 },
    { 0x0, 0x0 },
    { 0x0, 0x0 },
    { 0x0, 0x0 },
    { ABLMAP_ENTRY_IDX0, 0x0 },
    { ABLMAP_ENTRY_IDX0, 0x0 },
    { ABLMAP_ENTRY_IDX20, 0x0 },
    { ABLMAP_ENTRY_IDX20, 0x0 },
    { ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX8|ABLMAP_ENTRY_IDX9|ABLMAP_ENTRY_IDX11|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX8|ABLMAP_ENTRY_IDX9|ABLMAP_ENTRY_IDX11|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX0|ABLMAP_ENTRY_IDX3|ABLMAP_ENTRY_IDX7|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX0|ABLMAP_ENTRY_IDX3|ABLMAP_ENTRY_IDX7|ABLMAP_ENTRY_IDX8|ABLMAP_ENTRY_IDX9|ABLMAP_ENTRY_IDX11|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX20|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX8|ABLMAP_ENTRY_IDX9|ABLMAP_ENTRY_IDX11|ABLMAP_ENTRY_IDX20|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX8|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX2|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX2|ABLMAP_ENTRY_IDX8|ABLMAP_ENTRY_IDX15|ABLMAP_ENTRY_IDX17|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX2|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX2|ABLMAP_ENTRY_IDX8|ABLMAP_ENTRY_IDX15|ABLMAP_ENTRY_IDX17|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX21|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX21|ABLMAP_ENTRY_IDX8|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX8|ABLMAP_ENTRY_IDX10|ABLMAP_ENTRY_IDX12|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX8|ABLMAP_ENTRY_IDX10|ABLMAP_ENTRY_IDX12|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX4|ABLMAP_ENTRY_IDX5|ABLMAP_ENTRY_IDX6|ABLMAP_ENTRY_IDX23, 0x0 },
    { ABLMAP_ENTRY_IDX4|ABLMAP_ENTRY_IDX5|ABLMAP_ENTRY_IDX6|ABLMAP_ENTRY_IDX8|ABLMAP_ENTRY_IDX10|ABLMAP_ENTRY_IDX12|ABLMAP_ENTRY_IDX23|ABLMAP_ENTRY_IDX31, ABLMAP_ENTRY_IDX32 },
    { ABLMAP_ENTRY_IDX22|ABLMAP_ENTRY_IDX23|ABLMAP_ENTRY_IDX24|ABLMAP_ENTRY_IDX25, 0x0 },
    { ABLMAP_ENTRY_IDX8|ABLMAP_ENTRY_IDX10|ABLMAP_ENTRY_IDX12|ABLMAP_ENTRY_IDX22|ABLMAP_ENTRY_IDX23|ABLMAP_ENTRY_IDX24|ABLMAP_ENTRY_IDX25, 0x0 },
};

MSTARAblMapFlag MSTARAblMapBldopTbl[MSTAR_ABLBLDOP_SIZE][MSTAR_ABLBLDOP_SIZE] =
{
    { {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0} },
    { {0x0, 0x0}, {ABLMAP_ENTRY_IDX23, 0x0}, {ABLMAP_ENTRY_IDX8, 0x0}, {0x0, 0x0}, {ABLMAP_ENTRY_IDX11|ABLMAP_ENTRY_IDX12, 0x0}, {ABLMAP_ENTRY_IDX9|ABLMAP_ENTRY_IDX10, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0} },
    { {0x0, 0x0}, {ABLMAP_ENTRY_IDX0|ABLMAP_ENTRY_IDX1|ABLMAP_ENTRY_IDX2|ABLMAP_ENTRY_IDX5|ABLMAP_ENTRY_IDX20|ABLMAP_ENTRY_IDX21|ABLMAP_ENTRY_IDX24, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {ABLMAP_ENTRY_IDX13|ABLMAP_ENTRY_IDX31, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0} },
    { {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0} },
    { {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0} },
    { {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {ABLMAP_ENTRY_IDX14, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0} },
    { {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {ABLMAP_ENTRY_IDX16, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0} },
    { {0x0, 0x0}, {ABLMAP_ENTRY_IDX3|ABLMAP_ENTRY_IDX4|ABLMAP_ENTRY_IDX22, 0x0}, {ABLMAP_ENTRY_IDX18|ABLMAP_ENTRY_IDX19, ABLMAP_ENTRY_IDX32}, {0x0, 0x0}, {0x0, 0x0}, {ABLMAP_ENTRY_IDX17|ABLMAP_ENTRY_IDX30, 0x0}, {ABLMAP_ENTRY_IDX15|ABLMAP_ENTRY_IDX26|ABLMAP_ENTRY_IDX28, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0} },
    { {0x0, 0x0}, {ABLMAP_ENTRY_IDX6|ABLMAP_ENTRY_IDX7|ABLMAP_ENTRY_IDX25, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {ABLMAP_ENTRY_IDX29, 0x0}, {ABLMAP_ENTRY_IDX27, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0} },
    { {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {ABLMAP_ENTRY_IDX8, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0} },
    { {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {ABLMAP_ENTRY_IDX1|ABLMAP_ENTRY_IDX2|ABLMAP_ENTRY_IDX5|ABLMAP_ENTRY_IDX20|ABLMAP_ENTRY_IDX21|ABLMAP_ENTRY_IDX24, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0} },
    { {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0}, {0x0, 0x0} },
};

#endif //__MSTAR__ABLMAPTLB_H__

