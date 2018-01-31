#!/usr/bin/env python3

"""
Python3 script to fetch pinyin based on raw txt file. Results will be saved in the same dir with same filename with suffix "dict_".

format of the txt file should be 
"词语 freq # comments"
"词语 freq" or 
"词语"

However only only the fist two parts (i.e.m 词语 and freq) will be retained in the result.

Author: Kevin Suo <suokunlong@126.com>
This script is written specially for libpinyin, but may be used for any other suitable purposes.

Licensed under GNU Public License v3 or above.
"""

import os
import urllib.request
import urllib.parse
from time import sleep

raw_dir = r"/run/media/suokunlong/SSD-Data/soft/libpinyin-dict/raw"

def fetch_pinyin(words, sep="'"):
    """Fetch pinyin from https://zhongwenzhuanpinyin.51240.com/
    inputs: words = [['中国'], 
                     ['美国', '49.9999'], 
                     ['德国', '97.0', '# this is a comment']]
    output: dicts = [['中国', "zhong'guo", None, None], 
                     ['美国', "mei'guo", '49.9999', None], 
                     ['德国', "de'guo", '97.0', '# this is a comment']]
    """
    print("process words: ", words)
    url = "https://zhongwenzhuanpinyin.51240.com/web_system/51240_com_www/system/file/zhongwenzhuanpinyin/data/?ajaxtimestamp=1516872009015"
    headers = {
        'Accept': '*/*',
        'Accept-Encoding': 'utf-8, deflate, sdch',
        'Accept-Language': 'zh-CN,zh;q=0.8',
        'Proxy-Connection': 'keep-alive',
        'User-Agent': '"Mozilla/5.0 (X11; Fedora; Linux x86_64; rv:57.0) Gecko/20100101 Firefox/57.0"'
    }
    words_str = ""
    for word in words:
        words_str = words_str + word[0] + ","
    post_values = {
        'zwzyp_zhongwen': words_str,
        'zwzyp_shengdiao': "0",
        'zwzyp_wenzi': "0",
        'zwzyp_jiange': "1",
        'zwzyp_duozhongduyin': "0"}
    #print("post values are: ", post_values)
    data=urllib.parse.urlencode(post_values).encode('utf-8')
    request = urllib.request.Request(url, headers=headers, data=data)
    html = urllib.request.urlopen(request).read()
    html = html.decode()
    html = html.replace("""<div align="center"><textarea name="zhongwen" style="font-size: 18px;border: 1px solid #d9d9d9;height: 155px;width: 95%;" readonly="readonly">""","")
    html = html.replace(""" , </textarea></div>""","")
    pinyins = html.split(" , ")
    #print(pinyins)
    
    dicts = []
    for i in range(len(words)):
        l = len(words[i])
        chars = words[i][0]
        pinyin = pinyins[i].replace(" ", sep)
        if l == 1:
            freq = None
            other = None
        elif 1 < l <= 3:
            freq = words[i][1]
            other = None
            if l == 3:
                other = words[i][2]    
        else:
            raise Exception("lenth of word list > 3: ", words[i])
            
        dict = [chars, pinyin, freq, other]
        dicts.append(dict)
        
    # avoid flood to the server.
    sleep(1)
    
    return dicts

def get_words(lines):
    """
    input:  lines = ["中国", 
                     "美国 49.9999", 
                     "德国 97.0 # this is a comment"]
    return: words = [['中国'], 
                     ['美国', '49.9999'], 
                     ['德国', '97.0', '# this is a comment']]
    """
    words = []
    
    for line in lines:
        word = line.split(" ", maxsplit=2) 
        chars = word[0]
        if not is_chinese(chars):
            raise Exception("get_words_error", "Non-chinese word chars found in line: ", line)

        if len(word) >= 2:           
            freq = word[1]
            if not freq.replace(".", "").isdigit():
                raise Exception("get_words_error", "non-numeric freq chars found in line: ", line)
        
        words.append(word)
        
    return words

def is_chinese(uchar):
    """判断一个unicode是否是汉字
    http://blog.csdn.net/qinbaby/article/details/23201883
    """
    if uchar >= u'\u4e00' and uchar<=u'\u9fa5':
        return True
    else:
        return False

def check_line(line):
    """
    """
    line = line.strip()
    if line.startswith("#") or line == "":
        pass
    else:
        word = line.split(" ", maxsplit=2) 
        chars = word[0]
        
        if not is_chinese(chars):
            print("Line error: ", "Non-chinese word chars found in line: ", line)
            return False
            
        if len(word) >= 2:           
            freq = word[1]
            if not freq.replace(".", "").isdigit():
                print("get_words_error", "non-numeric freq chars found in line: ", line) 
                return False
           
    return True

        
def check_raw_file(raw_file):
    """
    """
    with open(raw_file,"r") as f_in:
        lines = f_in.readlines()
        errors = []
        for line in lines:
            if (check_line(line)):
                pass
            else:
                errors.append(line)
    
    if len(errors) > 0:
        print("check raw file erros are: ", errors, "\n", raw_file)
        return False
    else:
        return True

def check_raw_files(raw_dir):
    """
    """
    for file_name in os.listdir(raw_dir):
        # print("checking raw file: ", file_name)
        if (file_name.startswith("dict_")):
            pass
        else:
            raw_file = os.path.join(raw_dir, file_name)
            if check_raw_file(raw_file) == False:
                raise Exception("Raw File Check Failed: ", raw_file)

def process_raw_file(raw_file, result_file):
    """ 1. 一行一行读取
        2. 碰到comment行或空行，如果lines_current里有要索取的词，则执行索取操作，写入索取结果，清零lines_current, 然后再写入comment或空行。
        3. 每max_lines_per_fetch行时，强制索取。
        4. 到结尾时，索取剩余的lines_current中的词。
        检索结果被存为以"dict_"为前缀的新文件。
    """
    with open(result_file,"w") as f_out:
    
        with open(raw_file,"r") as f_in:
            lines = f_in.readlines()
            max_lines_per_fetch = 200
            i = 1
            lines_current = []
            
            for line in lines:
                line = line.strip()
                if line.startswith("#") or line == "":
                    if len(lines_current) == 0:    # there is no words to fetch
                        f_out.write(line+"\n")
                    else:
                        words = get_words(lines_current)
                        dicts = fetch_pinyin(words)
                        write_dicts(f_out, dicts)
                        lines_current.clear()
                        # remember to write the comment or blank line after fetch
                        f_out.write(line+"\n")
                else:
                    lines_current.append(line)
                    if len(lines_current) >= max_lines_per_fetch or i==len(lines):
                        words = get_words(lines_current)
                        dicts = fetch_pinyin(words)
                        write_dicts(f_out, dicts)
                        lines_current.clear()
                i += 1

def write_dicts(f_out, dicts):
    """
    """
    for dict in dicts:
        chars = dict[0]
        pinyin = dict[1]
        freq = dict[2]
        # "other" is disregarded in below process.
        other = dict[3]
        if freq == None:
            f_out.write(dict[0]+" "+dict[1]+"\n")
        else:
            f_out.write(chars+" "+pinyin+" "+freq+"\n")    

def process_raw_files(raw_dir):
    """
    """
    # check for errors in raw files before do anything else
    check_raw_files(raw_dir)
    
    for file_name in os.listdir(raw_dir):
        if (file_name.startswith("dict_")):
            print("ignored: ", file_name)
            pass
        else:
            print("Catch: ", file_name)
            raw_file = os.path.join(raw_dir, file_name)
            result_file = os.path.join(raw_dir, "dict_"+file_name)
            process_raw_file(raw_file, result_file)

process_raw_files(raw_dir)
