# -*- coding: utf-8 -*-
# vim:set et sts=4 sw=4:
#
# libpinyin - Library to deal with pinyin.
#
# Copyright (c) 2007-2008 Peng Huang <shawn.p.huang@gmail.com>
# Copyright (C) 2011 Peng Wu <alexepico@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

N_ = lambda x : x
PINYIN_DICT = {
    "a" : 1, "ai" : 2, "an" : 3, "ang" : 4, "ao" : 5,
    "ba" : 6, "bai" : 7, "ban" : 8, "bang" : 9, "bao" : 10,
    "bei" : 11, "ben" : 12, "beng" : 13, "bi" : 14, "bian" : 15,
    "biao" : 16, "bie" : 17, "bin" : 18, "bing" : 19, "bo" : 20,
    "bu" : 21, "ca" : 22, "cai" : 23, "can" : 24, "cang" : 25,
    "cao" : 26, "ce" : 27, "cen" : 28, "ceng" : 29, "ci" : 30,
    "cong" : 31, "cou" : 32, "cu" : 33, "cuan" : 34, "cui" : 35,
    "cun" : 36, "cuo" : 37, "cha" : 38, "chai" : 39, "chan" : 40,
    "chang" : 41, "chao" : 42, "che" : 43, "chen" : 44, "cheng" : 45,
    "chi" : 46, "chong" : 47, "chou" : 48, "chu" : 49, "chuai" : 50,
    "chuan" : 51, "chuang" : 52, "chui" : 53, "chun" : 54, "chuo" : 55,
    "da" : 56, "dai" : 57, "dan" : 58, "dang" : 59, "dao" : 60,
    "de" : 61, "dei" : 62,
    # "den" : 63,
    "deng" : 64, "di" : 65,
    "dia" : 66, "dian" : 67, "diao" : 68, "die" : 69, "ding" : 70,
    "diu" : 71, "dong" : 72, "dou" : 73, "du" : 74, "duan" : 75,
    "dui" : 76, "dun" : 77, "duo" : 78, "e" : 79, "ei" : 80,
    "en" : 81, "er" : 82, "fa" : 83, "fan" : 84, "fang" : 85,
    "fei" : 86, "fen" : 87, "feng" : 88, "fo" : 89, "fou" : 90,
    "fu" : 91, "ga" : 92, "gai" : 93, "gan" : 94, "gang" : 95,
    "gao" : 96, "ge" : 97, "gei" : 98, "gen" : 99, "geng" : 100,
    "gong" : 101, "gou" : 102, "gu" : 103, "gua" : 104, "guai" : 105,
    "guan" : 106, "guang" : 107, "gui" : 108, "gun" : 109, "guo" : 110,
    "ha" : 111, "hai" : 112, "han" : 113, "hang" : 114, "hao" : 115,
    "he" : 116, "hei" : 117, "hen" : 118, "heng" : 119, "hong" : 120,
    "hou" : 121, "hu" : 122, "hua" : 123, "huai" : 124, "huan" : 125,
    "huang" : 126, "hui" : 127, "hun" : 128, "huo" : 129, "ji" : 130,
    "jia" : 131, "jian" : 132, "jiang" : 133, "jiao" : 134, "jie" : 135,
    "jin" : 136, "jing" : 137, "jiong" : 138, "jiu" : 139, "ju" : 140,
    "juan" : 141, "jue" : 142, "jun" : 143, "ka" : 144, "kai" : 145,
    "kan" : 146, "kang" : 147, "kao" : 148, "ke" : 149,
    # "kei" : 150,
    "ken" : 151, "keng" : 152, "kong" : 153, "kou" : 154, "ku" : 155,
    "kua" : 156, "kuai" : 157, "kuan" : 158, "kuang" : 159, "kui" : 160,
    "kun" : 161, "kuo" : 162, "la" : 163, "lai" : 164, "lan" : 165,
    "lang" : 166, "lao" : 167, "le" : 168, "lei" : 169, "leng" : 170,
    "li" : 171, "lia" : 172, "lian" : 173, "liang" : 174, "liao" : 175,
    "lie" : 176, "lin" : 177, "ling" : 178, "liu" : 179,
    "lo" : 180,
    "long" : 181, "lou" : 182, "lu" : 183, "luan" : 184,
    # "lue" : 185,
    "lun" : 186, "luo" : 187, "lv" : 188, "lve" : 189,
    "ma" : 190,
    "mai" : 191, "man" : 192, "mang" : 193, "mao" : 194, "me" : 195,
    "mei" : 196, "men" : 197, "meng" : 198, "mi" : 199, "mian" : 200,
    "miao" : 201, "mie" : 202, "min" : 203, "ming" : 204, "miu" : 205,
    "mo" : 206, "mou" : 207, "mu" : 208, "na" : 209, "nai" : 210,
    "nan" : 211, "nang" : 212, "nao" : 213, "ne" : 214, "nei" : 215,
    "nen" : 216, "neng" : 217, "ni" : 218, "nian" : 219, "niang" : 220,
    "niao" : 221, "nie" : 222, "nin" : 223, "ning" : 224, "niu" : 225,
    "ng" : 226,
    "nong" : 227, "nou" : 228, "nu" : 229, "nuan" : 230,
    # "nue" : 231,
    "nuo" : 232, "nv" : 233, "nve" : 234,
    "o" : 235,
    "ou" : 236, "pa" : 237, "pai" : 238, "pan" : 239, "pang" : 240,
    "pao" : 241, "pei" : 242, "pen" : 243, "peng" : 244, "pi" : 245,
    "pian" : 246, "piao" : 247, "pie" : 248, "pin" : 249, "ping" : 250,
    "po" : 251, "pou" : 252, "pu" : 253, "qi" : 254, "qia" : 255,
    "qian" : 256, "qiang" : 257, "qiao" : 258, "qie" : 259, "qin" : 260,
    "qing" : 261, "qiong" : 262, "qiu" : 263, "qu" : 264, "quan" : 265,
    "que" : 266, "qun" : 267, "ran" : 268, "rang" : 269, "rao" : 270,
    "re" : 271, "ren" : 272, "reng" : 273, "ri" : 274, "rong" : 275,
    "rou" : 276, "ru" : 277, "ruan" : 278, "rui" : 279, "run" : 280,
    "ruo" : 281, "sa" : 282, "sai" : 283, "san" : 284, "sang" : 285,
    "sao" : 286, "se" : 287, "sen" : 288, "seng" : 289, "si" : 290,
    "song" : 291, "sou" : 292, "su" : 293, "suan" : 294, "sui" : 295,
    "sun" : 296, "suo" : 297, "sha" : 298, "shai" : 299, "shan" : 300,
    "shang" : 301, "shao" : 302, "she" : 303, "shei" : 304, "shen" : 305,
    "sheng" : 306, "shi" : 307, "shou" : 308, "shu" : 309, "shua" : 310,
    "shuai" : 311, "shuan" : 312, "shuang" : 313, "shui" : 314, "shun" : 315,
    "shuo" : 316, "ta" : 317, "tai" : 318, "tan" : 319, "tang" : 320,
    "tao" : 321, "te" : 322,
    # "tei" : 323,
    "teng" : 324, "ti" : 325,
    "tian" : 326, "tiao" : 327, "tie" : 328, "ting" : 329, "tong" : 330,
    "tou" : 331, "tu" : 332, "tuan" : 333, "tui" : 334, "tun" : 335,
    "tuo" : 336, "wa" : 337, "wai" : 338, "wan" : 339, "wang" : 340,
    "wei" : 341, "wen" : 342, "weng" : 343, "wo" : 344, "wu" : 345,
    "xi" : 346, "xia" : 347, "xian" : 348, "xiang" : 349, "xiao" : 350,
    "xie" : 351, "xin" : 352, "xing" : 353, "xiong" : 354, "xiu" : 355,
    "xu" : 356, "xuan" : 357, "xue" : 358, "xun" : 359, "ya" : 360,
    "yan" : 361, "yang" : 362, "yao" : 363, "ye" : 364, "yi" : 365,
    "yin" : 366, "ying" : 367, "yo" : 368, "yong" : 369, "you" : 370,
    "yu" : 371, "yuan" : 372, "yue" : 373, "yun" : 374, "za" : 375,
    "zai" : 376, "zan" : 377, "zang" : 378, "zao" : 379, "ze" : 380,
    "zei" : 381, "zen" : 382, "zeng" : 383, "zi" : 384, "zong" : 385,
    "zou" : 386, "zu" : 387, "zuan" : 388, "zui" : 389, "zun" : 390,
    "zuo" : 391, "zha" : 392, "zhai" : 393, "zhan" : 394, "zhang" : 395,
    "zhao" : 396, "zhe" : 397, "zhen" : 398, "zheng" : 399, "zhi" : 400,
    "zhong" : 401, "zhou" : 402, "zhu" : 403, "zhua" : 404, "zhuai" : 405,
    "zhuan" : 406, "zhuang" : 407, "zhui" : 408, "zhun" : 409, "zhuo" : 410,
    # some weird pinyins
    #~ "eng" : 411, "chua" : 412, "fe" : 413, "fiao" : 414, "liong" : 415
}

PINYIN_LIST = PINYIN_DICT.keys ()


SHENGMU_DICT = {
    "b" : 1, "p" : 2, "m" : 3, "f" : 4, "d" : 5,
    "t" : 6, "n" : 7, "l" : 8, "g" : 9, "k" : 10, "h" : 11,
    "j" : 12, "q" : 13, "x" : 14, "zh" : 15, "ch" : 16, "sh" : 17,
    "r" : 18, "z" : 19, "c" : 20, "s" : 21, "y" : 22, "w" : 23
}

SHENGMU_LIST = SHENGMU_DICT.keys ()


YUNMU_DICT = {
    "a" : 1, "ai" : 2, "an" : 3, "ang" : 4, "ao" : 5,
    "e" : 6, "ei" : 7, "en" : 8, "eng" : 9, "er" : 10,
    "i" : 11, "ia" : 12, "ian" : 13, "iang" : 14, "iao" : 15,
    "ie" : 16, "in" : 17, "ing" : 18, "iong" : 19, "iu" : 20,
    "o" : 21, "ong" : 22, "ou" : 23, "u" : 24, "ua" : 25,
    "uai" : 26, "uan" : 27, "uang" : 28, "ue" : 29, "ui" : 30,
    "un" : 31, "uo" : 32, "v" : 33, "ve" : 34
}

YUNMU_LIST = YUNMU_DICT.keys ()


MOHU_SHENGMU = {
    "z"     : ("z", "zh"),
    "zh" : ("z", "zh"),
    "c"     : ("c", "ch"),
    "ch" : ("c", "ch"),
    "s"     : ("s", "sh"),
    "sh" : ("s", "sh"),
    "l"     : ("l", "n"),
    "n"  : ("l", "n")
}

MOHU_YUNMU = {
    "an"  : ("an", "ang"),
    "ang" : ("an", "ang"),
    "en"  : ("en", "eng"),
    "eng" : ("en", "eng"),
    "in"  : ("in", "ing"),
    "ing" : ("in", "ing")
}

MSPY_SHUANGPIN_SHENGMU_DICT = {
    "b" : "b", "c" : "c", "d" : "d", "f" : "f", "g" : "g",
    "h" : "h", "i" : "ch","j" : "j", "k" : "k", "l" : "l",
    "m" : "m", "n" : "n", "o" : "'", "p" : "p", "q" : "q",
    "r" : "r", "s" : "s", "t" : "t", "u" : "sh","v" : "zh",
    "w" : "w", "x" : "x", "y" : "y", "z" : "z"
}

MSPY_SHUANGPIN_YUNMU_DICT = {
    "a" : ("a",),
    "b" : ("ou",),
    "c" : ("iao",),
    "d" : ("uang", "iang"),
    "e" : ("e",),
    "f" : ("en",),
    "g" : ("eng", "ng"),
    "h" : ("ang",),
    "i" : ("i",),
    "j" : ("an",),
    "k" : ("ao",),
    "l" : ("ai",),
    "m" : ("ian",),
    "n" : ("in",),
    "o" : ("uo", "o"),
    "p" : ("un",),
    "q" : ("iu",),
    "r" : ("uan", "er"),
    "s" : ("ong", "iong"),
    "t" : ("ue",),
    "u" : ("u",),
    "v" : ("ui","ue"),
    "w" : ("ia","ua"),
    "x" : ("ie",),
    "y" : ("uai", "v"),
    "z" : ("ei",),
    ";" : ("ing",)
}

ZRM_SHUANGPIN_SHENGMU_DICT = {
    "b" : "b", "c" : "c", "d" : "d", "f" : "f", "g" : "g",
    "h" : "h", "i" : "ch","j" : "j", "k" : "k", "l" : "l",
    "m" : "m", "n" : "n", "o" : "'", "p" : "p", "q" : "q",
    "r" : "r", "s" : "s", "t" : "t", "u" : "sh","v" : "zh",
    "w" : "w", "x" : "x", "y" : "y", "z" : "z"
}

ZRM_SHUANGPIN_YUNMU_DICT = {
    "a" : ("a",),
    "b" : ("ou",),
    "c" : ("iao",),
    "d" : ("uang", "iang"),
    "e" : ("e",),
    "f" : ("en",),
    "g" : ("eng", "ng"),
    "h" : ("ang",),
    "i" : ("i",),
    "j" : ("an",),
    "k" : ("ao",),
    "l" : ("ai",),
    "m" : ("ian",),
    "n" : ("in",),
    "o" : ("uo", "o"),
    "p" : ("un",),
    "q" : ("iu",),
    "r" : ("uan", "er"),
    "s" : ("ong", "iong"),
    "t" : ("ue",),
    "u" : ("u",),
    "v" : ("ui","v"),
    "w" : ("ia","ua"),
    "x" : ("ie",),
    "y" : ("uai", "ing"),
    "z" : ("ei",),
}

ABC_SHUANGPIN_SHENGMU_DICT = {
    "a" : "zh", "b" : "b", "c" : "c", "d" : "d", "e":"ch", "f" : "f", "g" : "g",
    "h" : "h", "j" : "j", "k" : "k", "l" : "l",
    "m" : "m", "n" : "n", "o" : "'", "p" : "p", "q" : "q",
    "r" : "r", "s" : "s", "t" : "t", "v" : "sh",
    "w" : "w", "x" : "x", "y" : "y", "z" : "z"
}

ABC_SHUANGPIN_YUNMU_DICT = {
    "a" : ("a",),
    "b" : ("ou",),
    "c" : ("in","uai"),
    "d" : ("ia", "ua"),
    "e" : ("e",),
    "f" : ("en",),
    "g" : ("eng", "ng"),
    "h" : ("ang",),
    "i" : ("i",),
    "j" : ("an",),
    "k" : ("ao",),
    "l" : ("ai",),
    "m" : ("ue","ui"),
    "n" : ("un",),
    "o" : ("uo", "o"),
    "p" : ("uan",),
    "q" : ("ei",),
    "r" : ("er", "iu"),
    "s" : ("ong", "iong"),
    "t" : ("iang","uang"),
    "u" : ("u",),
    "v" : ("v","ue"),
    "w" : ("ian",),
    "x" : ("ie",),
    "y" : ("ing",),
    "z" : ("iao",),
}

ZGPY_SHUANGPIN_SHENGMU_DICT = {
    "a" : "ch", "b" : "b", "c" : "c", "d" : "d", "f" : "f", "g" : "g",
    "h" : "h", "i" : "sh","j" : "j", "k" : "k", "l" : "l",
    "m" : "m", "n" : "n", "o" : "'", "p" : "p", "q" : "q",
    "r" : "r", "s" : "s", "t" : "t", "u" : "zh",
    "w" : "w", "x" : "x", "y" : "y", "z" : "z"
}

ZGPY_SHUANGPIN_YUNMU_DICT = {
    "a" : ("a", ),
    "b" : ("iao", ),
    "d" : ("ie", ),
    "e" : ("e", ),
    "f" : ("ian", ),
    "g" : ("iang", "uang"),
    "h" : ("ong", "iong"),
    "i" : ("i", ),
    "j" : ("er", "iu"),
    "k" : ("ei", ),
    "l" : ("uan", ),
    "m" : ("un", ),
    "n" : ("ue", "ui"),
    "o" : ("uo", "o"),
    "p" : ("ai", ),
    "q" : ("ao", ),
    "r" : ("an", ),
    "s" : ("ang", ),
    "t" : ("eng", "ng"),
    "u" : ("u", ),
    "v" : ("v", ),
    "w" : ("en", ),
    "x" : ("ia", "ua"),
    "y" : ("in", "uai"),
    "z" : ("ou" ,),
    ";" : ("ing", )
}

PYJJ_SHUANGPIN_SHENGMU_DICT = {
    "a" : "'", "b" : "b", "c" : "c", "d" : "d", "f" : "f", "g" : "g",
    "h" : "h", "i" : "sh","j" : "j", "k" : "k", "l" : "l",
    "m" : "m", "n" : "n", "o" : "'", "p" : "p", "q" : "q",
    "r" : "r", "s" : "s", "t" : "t", "u" : "ch","v" : "zh",
    "w" : "w", "x" : "x", "y" : "y", "z" : "z"
}

PYJJ_SHUANGPIN_YUNMU_DICT = {
    "a" : ("a",),
    "b" : ("ia","ua"),
    "c" : ("uan",),
    "d" : ("ao", ),
    "e" : ("e",),
    "f" : ("an",),
    "g" : ("ang",),
    "h" : ("iang","uang"),
    "i" : ("i",),
    "j" : ("ian",),
    "k" : ("iao",),
    "l" : ("in",),
    "m" : ("ie",),
    "n" : ("iu",),
    "o" : ("uo", "o"),
    "p" : ("ou",),
    "q" : ("er","ing"),
    "r" : ("en", ),
    "s" : ("ai", ),
    "t" : ("eng", "ng"),
    "u" : ("u",),
    "v" : ("v","ui"),
    "w" : ("ei",),
    "x" : ("uai","ue"),
    "y" : ("ong","iong"),
    "z" : ("un",),
}

XHE_SHUANGPIN_SHENGMU_DICT = {
    "b" : "b", "c" : "c", "d" : "d", "f" : "f", "g" : "g",
    "h" : "h", "i" : "ch", "j" : "j", "k" : "k", "l" : "l",
    "m" : "m", "n" : "n", "o" : "'", "p" : "p", "q" : "q",
    "r" : "r", "s" : "s", "t" : "t", "u" : "sh", "v" : "zh",
    "w" : "w", "x" : "x", "y" : "y", "z" : "z",
    "a" : "'", "e" : "'"
}

XHE_SHUANGPIN_YUNMU_DICT = {
    "a" : ("a",),
    "b" : ("in",),
    "c" : ("ao",),
    "d" : ("ai",),
    "e" : ("e",),
    "f" : ("en",),
    "g" : ("eng", "ng"),
    "h" : ("ang",),
    "i" : ("i",),
    "j" : ("an",),
    "k" : ("uai", "ing"),
    "l" : ("iang", "uang"),
    "m" : ("ian",),
    "n" : ("iao",),
    "o" : ("uo", "o"),
    "p" : ("ie",),
    "q" : ("iu",),
    "r" : ("uan", "er"),
    "s" : ("ong", "iong"),
    "t" : ("ue",),
    "u" : ("u",),
    "v" : ("v", "ui"),
    "w" : ("ei",),
    "x" : ("ia", "ua"),
    "y" : ("un",),
    "z" : ("ou",),
}

SHUANGPIN_SCHEMAS = {
    N_("MSPY") : (MSPY_SHUANGPIN_SHENGMU_DICT, MSPY_SHUANGPIN_YUNMU_DICT),
    N_("ZRM")  : (ZRM_SHUANGPIN_SHENGMU_DICT,  ZRM_SHUANGPIN_YUNMU_DICT),
    N_("ABC")  : (ABC_SHUANGPIN_SHENGMU_DICT,  ABC_SHUANGPIN_YUNMU_DICT),
    N_("ZGPY") : (ZGPY_SHUANGPIN_SHENGMU_DICT, ZGPY_SHUANGPIN_YUNMU_DICT),
    N_("PYJJ") : (PYJJ_SHUANGPIN_SHENGMU_DICT, PYJJ_SHUANGPIN_YUNMU_DICT),
    N_("XHE")  : (XHE_SHUANGPIN_SHENGMU_DICT,  XHE_SHUANGPIN_YUNMU_DICT),
}

