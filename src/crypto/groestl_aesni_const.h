#pragma once

#define GROESTL256_ALL_1B                                                      \
  _mm_set_epi32(0x1b1b1b1b, 0x1b1b1b1b, 0x1b1b1b1b, 0x1b1b1b1b)

#define GROESTL256_TRANSP_MASK                                                 \
  _mm_set_epi32(0x0f070b03, 0x0e060a02, 0x0d050901, 0x0c040800)

#define GROESTL256_ROUND_CONST_L0_0                                            \
  _mm_set_epi32(0xffffffff, 0xffffffff, 0x70605040, 0x30201000)
#define GROESTL256_ROUND_CONST_L0_1                                            \
  _mm_set_epi32(0xffffffff, 0xffffffff, 0x71615141, 0x31211101)
#define GROESTL256_ROUND_CONST_L0_2                                            \
  _mm_set_epi32(0xffffffff, 0xffffffff, 0x72625242, 0x32221202)
#define GROESTL256_ROUND_CONST_L0_3                                            \
  _mm_set_epi32(0xffffffff, 0xffffffff, 0x73635343, 0x33231303)
#define GROESTL256_ROUND_CONST_L0_4                                            \
  _mm_set_epi32(0xffffffff, 0xffffffff, 0x74645444, 0x34241404)
#define GROESTL256_ROUND_CONST_L0_5                                            \
  _mm_set_epi32(0xffffffff, 0xffffffff, 0x75655545, 0x35251505)
#define GROESTL256_ROUND_CONST_L0_6                                            \
  _mm_set_epi32(0xffffffff, 0xffffffff, 0x76665646, 0x36261606)
#define GROESTL256_ROUND_CONST_L0_7                                            \
  _mm_set_epi32(0xffffffff, 0xffffffff, 0x77675747, 0x37271707)
#define GROESTL256_ROUND_CONST_L0_8                                            \
  _mm_set_epi32(0xffffffff, 0xffffffff, 0x78685848, 0x38281808)
#define GROESTL256_ROUND_CONST_L0_9                                            \
  _mm_set_epi32(0xffffffff, 0xffffffff, 0x79695949, 0x39291909)

#define GROESTL256_ROUND_CONST_L7_0                                            \
  _mm_set_epi32(0x8f9fafbf, 0xcfdfefff, 0x00000000, 0x00000000)
#define GROESTL256_ROUND_CONST_L7_1                                            \
  _mm_set_epi32(0x8e9eaebe, 0xcedeeefe, 0x00000000, 0x00000000)
#define GROESTL256_ROUND_CONST_L7_2                                            \
  _mm_set_epi32(0x8d9dadbd, 0xcdddedfd, 0x00000000, 0x00000000)
#define GROESTL256_ROUND_CONST_L7_3                                            \
  _mm_set_epi32(0x8c9cacbc, 0xccdcecfc, 0x00000000, 0x00000000)
#define GROESTL256_ROUND_CONST_L7_4                                            \
  _mm_set_epi32(0x8b9babbb, 0xcbdbebfb, 0x00000000, 0x00000000)
#define GROESTL256_ROUND_CONST_L7_5                                            \
  _mm_set_epi32(0x8a9aaaba, 0xcadaeafa, 0x00000000, 0x00000000)
#define GROESTL256_ROUND_CONST_L7_6                                            \
  _mm_set_epi32(0x8999a9b9, 0xc9d9e9f9, 0x00000000, 0x00000000)
#define GROESTL256_ROUND_CONST_L7_7                                            \
  _mm_set_epi32(0x8898a8b8, 0xc8d8e8f8, 0x00000000, 0x00000000)
#define GROESTL256_ROUND_CONST_L7_8                                            \
  _mm_set_epi32(0x8797a7b7, 0xc7d7e7f7, 0x00000000, 0x00000000)
#define GROESTL256_ROUND_CONST_L7_9                                            \
  _mm_set_epi32(0x8696a6b6, 0xc6d6e6f6, 0x00000000, 0x00000000)

#define GROESTL256_ROUND_CONST_Lx                                              \
  _mm_set_epi32(0xffffffff, 0xffffffff, 0x00000000, 0x00000000)

#define GROESTL256_SUBSH_MASK_0                                                \
  _mm_set_epi32(0x03060a0d, 0x08020509, 0x0c0f0104, 0x070b0e00)
#define GROESTL256_SUBSH_MASK_1                                                \
  _mm_set_epi32(0x04070c0f, 0x0a03060b, 0x0e090205, 0x000d0801)
#define GROESTL256_SUBSH_MASK_2                                                \
  _mm_set_epi32(0x05000e09, 0x0c04070d, 0x080b0306, 0x010f0a02)
#define GROESTL256_SUBSH_MASK_3                                                \
  _mm_set_epi32(0x0601080b, 0x0e05000f, 0x0a0d0407, 0x02090c03)
#define GROESTL256_SUBSH_MASK_4                                                \
  _mm_set_epi32(0x0702090c, 0x0f060108, 0x0b0e0500, 0x030a0d04)
#define GROESTL256_SUBSH_MASK_5                                                \
  _mm_set_epi32(0x00030b0e, 0x0907020a, 0x0d080601, 0x040c0f05)
#define GROESTL256_SUBSH_MASK_6                                                \
  _mm_set_epi32(0x01040d08, 0x0b00030c, 0x0f0a0702, 0x050e0906)
#define GROESTL256_SUBSH_MASK_7                                                \
  _mm_set_epi32(0x02050f0a, 0x0d01040e, 0x090c0003, 0x06080b07)
