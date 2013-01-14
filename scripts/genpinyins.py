#!/usr/bin/python3
import os
from operator import itemgetter

pinyin_dict = {}


def strip_tone(old_pinyin_str):
    oldpinyins = old_pinyin_str.split("'")
    newpinyins = []

    for pinyin in oldpinyins:
        if pinyin[-1].isdigit():
            pinyin = pinyin[:-1]
        newpinyins.append(pinyin)

    new_pinyin_str = "'".join(newpinyins)
    return new_pinyin_str


def add_pinyin_dict(pinyin, freq):
    if 0 == freq:
        return
    if not pinyin in pinyin_dict:
        pinyin_dict[pinyin] = freq
    else:
        pinyin_dict[pinyin] += freq


def load_phrase(filename):
    phrasefile = open(filename, "r")
    for line in phrasefile.readlines():
        line = line.rstrip(os.linesep)
        (pinyin, word, token, freq) = line.split(None, 3)
        pinyin = strip_tone(pinyin)
        freq = int(freq)

        if len(word) in [1, 2]:
            add_pinyin_dict(pinyin, freq)

    phrasefile.close()

load_phrase("../data/gb_char.table")
load_phrase("../data/gbk_char.table")


def save_pinyin(filename):
    pinyinfile = open(filename, "w")
    for pinyin, freq in pinyin_dict.items():
        freq = str(freq)
        line = "\t".join((pinyin, freq))
        pinyinfile.writelines([line, os.linesep])
    pinyinfile.close()


if __name__ == "__main__":
    save_pinyin("pinyins.txt")
