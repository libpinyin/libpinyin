# -*- coding: utf-8 -*-
# vim:set et sts=4 sw=4:
#
# libpinyin - Library to deal with pinyin.
#
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


CHEWING_INITIAL_LIST = [
    'CHEWING_ZERO_INITIAL',    #Zero Initial
    'CHEWING_B',               #"ㄅ"
    'CHEWING_C',               #"ㄘ"
    'CHEWING_CH',              #"ㄔ"
    'CHEWING_D',               #"ㄉ"
    'CHEWING_F',               #"ㄈ"
    'CHEWING_H',               #"ㄏ"
    'CHEWING_G',               #"ㄍ"
    'CHEWING_K',               #"ㄎ"
    'CHEWING_J',               #"ㄐ"
    'CHEWING_M',               #"ㄇ"
    'CHEWING_N',               #"ㄋ"
    'CHEWING_L',               #"ㄌ"
    'CHEWING_R',               #"ㄖ"
    'CHEWING_P',               #"ㄆ"
    'CHEWING_Q',               #"ㄑ"
    'CHEWING_S',               #"ㄙ"
    'CHEWING_SH',              #"ㄕ"
    'CHEWING_T',               #"ㄊ"
    'PINYIN_W',                #Invalid Chewing
    'CHEWING_X',               #"ㄒ"
    'PINYIN_Y',                #Invalid Chewing
    'CHEWING_Z',               #"ㄗ"
    'CHEWING_ZH'               #"ㄓ"
]


CHEWING_MIDDLE_LIST = [
    'CHEWING_ZERO_MIDDLE',     #Zero Middle
    'CHEWING_I',               #"ㄧ"
    'CHEWING_U',               #"ㄨ"
    'CHEWING_V'                #"ㄩ"
]


CHEWING_FINAL_LIST = [
    'CHEWING_ZERO_FINAL',    #Zero Final
    'CHEWING_A',             #"ㄚ"
    'CHEWING_AI',            #"ㄞ"
    'CHEWING_AN',            #"ㄢ"
    'CHEWING_ANG',           #"ㄤ"
    'CHEWING_AO',            #"ㄠ"
    'CHEWING_E',             #"ㄝ" and "ㄜ"
    'INVALID_EA',            #Invalid Pinyin/Chewing
    'CHEWING_EI',            #"ㄟ"
    'CHEWING_EN',            #"ㄣ"
    'CHEWING_ENG',           #"ㄥ"
    'CHEWING_ER',            #"ㄦ"
    'CHEWING_NG',            #"ㄫ"
    'CHEWING_O',             #"ㄛ"
    'PINYIN_ONG',            #"ueng"
    'CHEWING_OU',            #"ㄡ"
    'PINYIN_IN',             #"ien"
    'PINYIN_ING'             #"ieng"
]


CHEWING_TONE_LIST = [
    'CHEWING_ZERO_TONE',     #Zero Tone
    'CHEWING_1',             #" "
    'CHEWING_2',             #'ˊ'
    'CHEWING_3',             #'ˇ'
    'CHEWING_4',             #'ˋ'
    'CHEWING_5'              #'˙'
]


def gen_entries(items, last_enum, num_enum):
    entries = []
    for enum, item in enumerate(items, start=0):
        entry = '{0} = {1}'.format(item, enum)
        entries.append(entry)

    #last enum
    entry = last_enum + ' = ' + items[-1]
    entries.append(entry)

    #num enum
    entry = num_enum
    entries.append(entry)

    return ",\n".join(entries)


def gen_initials():
    return gen_entries(CHEWING_INITIAL_LIST, 'CHEWING_LAST_INITIAL',
                       'CHEWING_NUMBER_OF_INITIALS = CHEWING_LAST_INITIAL + 1')


def gen_middles():
    return gen_entries(CHEWING_MIDDLE_LIST, 'CHEWING_LAST_MIDDLE',
                       'CHEWING_NUMBER_OF_MIDDLES = CHEWING_LAST_MIDDLE + 1')


def gen_finals():
    return gen_entries(CHEWING_FINAL_LIST, 'CHEWING_LAST_FINAL',
                       'CHEWING_NUMBER_OF_FINALS = CHEWING_LAST_FINAL + 1')


def gen_tones():
    return gen_entries(CHEWING_TONE_LIST, 'CHEWING_LAST_TONE',
                       'CHEWING_NUMBER_OF_TONES = CHEWING_LAST_TONE + 1')


def gen_table_index(content_table):
    entries = []
    for i in range(0, len(CHEWING_INITIAL_LIST)):
        initial = CHEWING_INITIAL_LIST[i]
        for m in range(0, len(CHEWING_MIDDLE_LIST)):
            middle = CHEWING_MIDDLE_LIST[m]
            for f in range(0, len(CHEWING_FINAL_LIST)):
                final = CHEWING_FINAL_LIST[f]
                chewingkey = 'ChewingKey({0}, {1}, {2})'.format(initial, middle, final)
                index = -1
                try:
                    index = [x[2] for x in content_table].index(chewingkey)
                except ValueError:
                    pass

                entry = '{0:<7} /* {1} */'.format(index, chewingkey)
                entries.append(entry)
    return ",\n".join(entries)


### main function ###
if __name__ == "__main__":
    print(gen_initials() + gen_middles() + gen_finals() + gen_tones())
