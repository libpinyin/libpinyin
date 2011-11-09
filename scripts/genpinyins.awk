#!/usr/bin/awk
	{ if (length($2) == len) pinyins[$1] += $4 }

END {
    for (pinyin in pinyins) {
        print pinyin, pinyins[pinyin]
    }
}
