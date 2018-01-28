#pragma once

#define JH256_H0_0 _mm_set_epi64x(0xc145b29c7bbecd92, 0xebd3202c41a398eb)
#define JH256_H0_1 _mm_set_epi64x(0x38a507ed6820026, 0xfac7d4609151931c)
#define JH256_H0_2 _mm_set_epi64x(0x77941ad4481afbe0, 0x45b92677269e23a4)
#define JH256_H0_3 _mm_set_epi64x(0xa82fff0f4224f056, 0x7a176b0226abb5cd)
#define JH256_H0_4 _mm_set_epi64x(0x62e27df70849141d, 0x754d2e7f8996a371)
#define JH256_H0_5 _mm_set_epi64x(0x6c29804757b6d587, 0x948f2476f7957627)
#define JH256_H0_6 _mm_set_epi64x(0xf7a0557c6508451, 0x6c0d8eac2d275e5c)
#define JH256_H0_7 _mm_set_epi64x(0x69d71cd313abe389, 0xea12247067d3e47b)

#define JH256_E8_ROUNDCONSTANT_00_EVN                                          \
  _mm_set_epi64x(0x571523b70a15847b, 0x67f815dfa2ded572)
#define JH256_E8_ROUNDCONSTANT_00_ODD                                          \
  _mm_set_epi64x(0x402bd1c3c54f9f4e, 0xf6875a4d90d6ab81)
#define JH256_E8_ROUNDCONSTANT_01_EVN                                          \
  _mm_set_epi64x(0x9a99b26699d2c503, 0x9cfa455ce03a98ea)
#define JH256_E8_ROUNDCONSTANT_01_ODD                                          \
  _mm_set_epi64x(0x31a2db881a1456b5, 0x8a53bbf2b4960266)
#define JH256_E8_ROUNDCONSTANT_02_EVN                                          \
  _mm_set_epi64x(0x1044c1870ab23f40, 0xdb0e199a5c5aa303)
#define JH256_E8_ROUNDCONSTANT_02_ODD                                          \
  _mm_set_epi64x(0xdccde75eadeb336f, 0x1d959e848019051c)
#define JH256_E8_ROUNDCONSTANT_03_EVN                                          \
  _mm_set_epi64x(0xd027bbf7156578dc, 0x416bbf029213ba10)
#define JH256_E8_ROUNDCONSTANT_03_ODD                                          \
  _mm_set_epi64x(0xd3910041d2bf1a3f, 0x5078aa3739812c0a)
#define JH256_E8_ROUNDCONSTANT_04_EVN                                          \
  _mm_set_epi64x(0xce97c0929c9f62dd, 0x907eccf60d5a2d42)
#define JH256_E8_ROUNDCONSTANT_04_ODD                                          \
  _mm_set_epi64x(0x23fcc663d665dfd1, 0xac442bc70ba75c18)
#define JH256_E8_ROUNDCONSTANT_05_EVN                                          \
  _mm_set_epi64x(0xa8ec6c447e450521, 0x1ab8e09e036c6e97)
#define JH256_E8_ROUNDCONSTANT_05_ODD                                          \
  _mm_set_epi64x(0x97818394b29796fd, 0xfa618e5dbb03f1ee)
#define JH256_E8_ROUNDCONSTANT_06_EVN                                          \
  _mm_set_epi64x(0x956a9ffb2d8d672a, 0x2f3003db37858e4a)
#define JH256_E8_ROUNDCONSTANT_06_ODD                                          \
  _mm_set_epi64x(0x14427fc04672c78a, 0x6c69b8f88173fe8a)
#define JH256_E8_ROUNDCONSTANT_07_EVN                                          \
  _mm_set_epi64x(0x80bb118fa76f4475, 0xc45ec7bd8f15f4c5)
#define JH256_E8_ROUNDCONSTANT_07_ODD                                          \
  _mm_set_epi64x(0xf4a3a6981e00b882, 0xbc88e4aeb775de52)
#define JH256_E8_ROUNDCONSTANT_08_EVN                                          \
  _mm_set_epi64x(0x89f9b7d524565faa, 0x1563a3a9338ff48e)
#define JH256_E8_ROUNDCONSTANT_08_ODD                                          \
  _mm_set_epi64x(0x362c42065ae9ca36, 0xfde05a7c20edf1b6)
#define JH256_E8_ROUNDCONSTANT_09_EVN                                          \
  _mm_set_epi64x(0xa74b9a7374f93a53, 0x3d98fe4e433529ce)
#define JH256_E8_ROUNDCONSTANT_09_ODD                                          \
  _mm_set_epi64x(0x9f5ad8af81ad9d0e, 0x86814e6f591ff5d0)
#define JH256_E8_ROUNDCONSTANT_10_EVN                                          \
  _mm_set_epi64x(0x2717b96ebe280b8b, 0x6a6234ee670605a7)
#define JH256_E8_ROUNDCONSTANT_10_ODD                                          \
  _mm_set_epi64x(0x7b487ec66f7ea0e0, 0x3f1080c626077447)
#define JH256_E8_ROUNDCONSTANT_11_EVN                                          \
  _mm_set_epi64x(0x9ef18e979fe7e391, 0xc0a4f84aa50a550d)
#define JH256_E8_ROUNDCONSTANT_11_ODD                                          \
  _mm_set_epi64x(0x62b0e5f3415a9e7e, 0xd48d605081727686)
#define JH256_E8_ROUNDCONSTANT_12_EVN                                          \
  _mm_set_epi64x(0x84c9f4ce001ae4e3, 0x7a205440ec1f9ffc)
#define JH256_E8_ROUNDCONSTANT_12_ODD                                          \
  _mm_set_epi64x(0xa554c324117e2e55, 0xd895fa9df594d74f)
#define JH256_E8_ROUNDCONSTANT_13_EVN                                          \
  _mm_set_epi64x(0xb2c4a50fe27ff578, 0x286efebd2872df5b)
#define JH256_E8_ROUNDCONSTANT_13_ODD                                          \
  _mm_set_epi64x(0x7f5928eb85937e44, 0x2ed349eeef7c8905)
#define JH256_E8_ROUNDCONSTANT_14_EVN                                          \
  _mm_set_epi64x(0x65e4d61df128865e, 0x4a3124b337695f70)
#define JH256_E8_ROUNDCONSTANT_14_ODD                                          \
  _mm_set_epi64x(0x8a87d423e843fe74, 0xe720b95104771bc7)
#define JH256_E8_ROUNDCONSTANT_15_EVN                                          \
  _mm_set_epi64x(0xc1d9309b097acbdd, 0xf2947692a3e8297d)
#define JH256_E8_ROUNDCONSTANT_15_ODD                                          \
  _mm_set_epi64x(0xbf829cf24f4924da, 0xe01bdc5bfb301b1d)
#define JH256_E8_ROUNDCONSTANT_16_EVN                                          \
  _mm_set_epi64x(0x48bcf8de0544320d, 0xffbf70b431bae7a4)
#define JH256_E8_ROUNDCONSTANT_16_ODD                                          \
  _mm_set_epi64x(0xa08b29e0c1c39f45, 0x39d3bb5332fcae3b)
#define JH256_E8_ROUNDCONSTANT_17_EVN                                          \
  _mm_set_epi64x(0x34f1904212347094, 0xf09aef7fd05c9e5)
#define JH256_E8_ROUNDCONSTANT_17_ODD                                          \
  _mm_set_epi64x(0x4a982f4f368e3be9, 0x95ed44e301b771a2)
#define JH256_E8_ROUNDCONSTANT_18_EVN                                          \
  _mm_set_epi64x(0xffaf52874b44c147, 0x15f66ca0631d4088)
#define JH256_E8_ROUNDCONSTANT_18_ODD                                          \
  _mm_set_epi64x(0xe68c6eccc5b67046, 0x30c60ae2f14abb7e)
#define JH256_E8_ROUNDCONSTANT_19_EVN                                          \
  _mm_set_epi64x(0xae183ec84b849dda, 0xca4fbd56a4d5a4)
#define JH256_E8_ROUNDCONSTANT_19_ODD                                          \
  _mm_set_epi64x(0x67255c1468cea6e8, 0xadd1643045ce5773)
#define JH256_E8_ROUNDCONSTANT_20_EVN                                          \
  _mm_set_epi64x(0x9a99949a5806e933, 0x16e10ecbf28cdaa3)
#define JH256_E8_ROUNDCONSTANT_20_ODD                                          \
  _mm_set_epi64x(0x1885d1a07facced1, 0x7b846fc220b2601f)
#define JH256_E8_ROUNDCONSTANT_21_EVN                                          \
  _mm_set_epi64x(0x46b4a5aac01c9a50, 0xd319dd8da15b5932)
#define JH256_E8_ROUNDCONSTANT_21_ODD                                          \
  _mm_set_epi64x(0x7eee560bab19caf6, 0xba6b04e467633d9f)
#define JH256_E8_ROUNDCONSTANT_22_EVN                                          \
  _mm_set_epi64x(0xee51363b35f7bde9, 0x742128a9ea79b11f)
#define JH256_E8_ROUNDCONSTANT_22_ODD                                          \
  _mm_set_epi64x(0x1707da3fec2463a, 0x76d350755aac571d)
#define JH256_E8_ROUNDCONSTANT_23_EVN                                          \
  _mm_set_epi64x(0x79676b9e20eced78, 0x42d8a498afc135f7)
#define JH256_E8_ROUNDCONSTANT_23_ODD                                          \
  _mm_set_epi64x(0x832c83324d3bc3fa, 0xa8db3aea15638341)
#define JH256_E8_ROUNDCONSTANT_24_EVN                                          \
  _mm_set_epi64x(0x9a762db734f04059, 0xf347271c1f3b40a7)
#define JH256_E8_ROUNDCONSTANT_24_ODD                                          \
  _mm_set_epi64x(0xef5957dc398dfdb8, 0xfd4f21d26c4e3ee7)
#define JH256_E8_ROUNDCONSTANT_25_EVN                                          \
  _mm_set_epi64x(0xd70f36849d7a25b, 0xdaeb492b490c9b8d)
#define JH256_E8_ROUNDCONSTANT_25_ODD                                          \
  _mm_set_epi64x(0x658ef8e4f0e9a5f5, 0x84558d7ad0ae3b7d)
#define JH256_E8_ROUNDCONSTANT_26_EVN                                          \
  _mm_set_epi64x(0x5aec3e759e07a80c, 0x533b1036f4a2b8a0)
#define JH256_E8_ROUNDCONSTANT_26_ODD                                          \
  _mm_set_epi64x(0x4cbcbaf8555cb05b, 0x4f88e85692946891)
#define JH256_E8_ROUNDCONSTANT_27_EVN                                          \
  _mm_set_epi64x(0x5d1c6b72d6f4da75, 0x7b9487f3993bbbe3)
#define JH256_E8_ROUNDCONSTANT_27_ODD                                          \
  _mm_set_epi64x(0x71db28b850a5346c, 0x6db334dc28acae64)
#define JH256_E8_ROUNDCONSTANT_28_EVN                                          \
  _mm_set_epi64x(0xfc75dd593364dbe3, 0x2a518d10f2e261f8)
#define JH256_E8_ROUNDCONSTANT_28_ODD                                          \
  _mm_set_epi64x(0xb043e8023cd1bb67, 0xa23fce43f1bcac1c)
#define JH256_E8_ROUNDCONSTANT_29_EVN                                          \
  _mm_set_epi64x(0x5c5316b44d19347f, 0x75a12988ca5b0a33)
#define JH256_E8_ROUNDCONSTANT_29_ODD                                          \
  _mm_set_epi64x(0x3fafeeb6d7757479, 0x1e4d790ec3943b92)
#define JH256_E8_ROUNDCONSTANT_30_EVN                                          \
  _mm_set_epi64x(0x5127234c097ef45c, 0x21391abef7d4a8ea)
#define JH256_E8_ROUNDCONSTANT_30_ODD                                          \
  _mm_set_epi64x(0xadd5a66d4a17a344, 0xd23c32ba5324a326)
#define JH256_E8_ROUNDCONSTANT_31_EVN                                          \
  _mm_set_epi64x(0x563c6b91983d5983, 0x8c9f2afa63e1db5)
#define JH256_E8_ROUNDCONSTANT_31_ODD                                          \
  _mm_set_epi64x(0xf6c76e08cc3ee246, 0x4d608672a17cf84c)
#define JH256_E8_ROUNDCONSTANT_32_EVN                                          \
  _mm_set_epi64x(0x2ae6c4efa566d62b, 0x5e76bcb1b333982f)
#define JH256_E8_ROUNDCONSTANT_32_ODD                                          \
  _mm_set_epi64x(0x6321efbc1582ee74, 0x36d4c1bee8b6f406)
#define JH256_E8_ROUNDCONSTANT_33_EVN                                          \
  _mm_set_epi64x(0x26585806c45a7da7, 0x69c953f40d4ec1fd)
#define JH256_E8_ROUNDCONSTANT_33_ODD                                          \
  _mm_set_epi64x(0x3f9d63283daf907e, 0x16fae0061614c17e)
#define JH256_E8_ROUNDCONSTANT_34_EVN                                          \
  _mm_set_epi64x(0x300cd4b730ceaa5f, 0xcd29b00e3f2c9d2)
#define JH256_E8_ROUNDCONSTANT_34_ODD                                          \
  _mm_set_epi64x(0x9af8cee3d830eb0d, 0x9832e0f216512a74)
#define JH256_E8_ROUNDCONSTANT_35_EVN                                          \
  _mm_set_epi64x(0xd36886046ee651ff, 0x9279f1b57b9ec54b)
#define JH256_E8_ROUNDCONSTANT_35_ODD                                          \
  _mm_set_epi64x(0x5750a17f3a6e6cc, 0x316796e6574d239b)
#define JH256_E8_ROUNDCONSTANT_36_EVN                                          \
  _mm_set_epi64x(0x62a205f88452173c, 0xce6c3213d98176b1)
#define JH256_E8_ROUNDCONSTANT_36_ODD                                          \
  _mm_set_epi64x(0x486a9323825446ff, 0x47154778b3cb2bf4)
#define JH256_E8_ROUNDCONSTANT_37_EVN                                          \
  _mm_set_epi64x(0x8e5086fc897cfcf2, 0x65655e4e0758df38)
#define JH256_E8_ROUNDCONSTANT_37_ODD                                          \
  _mm_set_epi64x(0x4e477830a20940f0, 0x86ca0bd0442e7031)
#define JH256_E8_ROUNDCONSTANT_38_EVN                                          \
  _mm_set_epi64x(0xbd3a2ce437e95ef7, 0x8338f7d139eea065)
#define JH256_E8_ROUNDCONSTANT_38_ODD                                          \
  _mm_set_epi64x(0xe7de9fefd1ed44a3, 0x6ff8130126b29721)
#define JH256_E8_ROUNDCONSTANT_39_EVN                                          \
  _mm_set_epi64x(0xbe42dc12f6f7853c, 0xd992257615dfa08b)
#define JH256_E8_ROUNDCONSTANT_39_ODD                                          \
  _mm_set_epi64x(0xdea83eaada7d8d53, 0x7eb027ab7ceca7d8)
#define JH256_E8_ROUNDCONSTANT_40_EVN                                          \
  _mm_set_epi64x(0xf908731afd43f65a, 0xd86902bd93ce25aa)
#define JH256_E8_ROUNDCONSTANT_40_ODD                                          \
  _mm_set_epi64x(0x6a21fd4c33664d97, 0xa5194a17daef5fc0)
#define JH256_E8_ROUNDCONSTANT_41_EVN                                          \
  _mm_set_epi64x(0x9b54cdedbb0f1eea, 0x701541db3198b435)
#define JH256_E8_ROUNDCONSTANT_41_ODD                                          \
  _mm_set_epi64x(0xe26f4791bf9d75f6, 0x72409751a163d09a)
