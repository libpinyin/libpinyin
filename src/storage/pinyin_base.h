/* 
 *  libpinyin
 *  Library to deal with pinyin.
 *  
 *  Copyright (C) 2002,2003,2006 James Su
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/** @file pinyin_base.h
 *  @brief the definitions of pinyin related classes and structs.
 */

#ifndef PINYIN_BASE_H
#define PINYIN_BASE_H

#include <string.h>
#include <glib.h>
#include "pinyin_custom.h"

namespace pinyin{

// Predefinition of some classes and structs
struct PinyinKey;

class PinyinValidator;
class PinyinParser;

struct PinyinKeyPos{
    int    m_pos;
    size_t m_len;
    PinyinKeyPos(){
	m_pos = 0;
	m_len = 0;
    }
    void set_pos(int pos){
	m_pos = pos;
    }
    void set_length(size_t len){
	m_len = len;
    }
    int get_pos(){
	return m_pos;
    }
    int get_end_pos(){
	return m_pos + m_len;
    }
    size_t get_length(){
	return m_len;
    }
};

typedef GArray* PinyinKeyVector; /* Array of PinyinKey */
typedef GArray* PinyinKeyPosVector; /* Array of PinyinKeyPos */


/**
 * @brief enums of pinyin initial element.
 *
 * A pinyin key can be divided into three tokens:
 * Initial -- such as B P M F D T N L  etc.
 * Final   -- such as A O E I U V etc.
 * Tone    -- can be 1, 2, 3, 4 and 5.
 */
enum PinyinInitial
{
    PINYIN_ZeroInitial = 0,    /**< zero initial. indicates invaild initial */
    PINYIN_Bo  = 1,
    PINYIN_Ci  = 2,
    PINYIN_Chi = 3,
    PINYIN_De  = 4,
    PINYIN_Fo  = 5,
    PINYIN_He  = 6,
    PINYIN_Ge  = 7,
    PINYIN_Ke  = 8,
    PINYIN_Ji  = 9,
    PINYIN_Mo  =10,
    PINYIN_Ne  =11,
    PINYIN_Le  =12,
    PINYIN_Ri  =13,
    PINYIN_Po  =14,
    PINYIN_Qi  =15,
    PINYIN_Si  =16,
    PINYIN_Shi =17,
    PINYIN_Te  =18,
    PINYIN_Wu  =19,
    PINYIN_Xi  =20,
    PINYIN_Yi  =21,
    PINYIN_Zi  =22,
    PINYIN_Zhi =23,
    PINYIN_LastInitial = PINYIN_Zhi,    /**< the last initial */
    PINYIN_Number_Of_Initials = PINYIN_LastInitial + 1
};

/**
 * @brief enums of pinyin final element.
 */
enum PinyinFinal
{
    PINYIN_ZeroFinal = 0,    /**< zero final. indicates invalid final */
    PINYIN_A    = 1,
    PINYIN_Ai   = 2,
    PINYIN_An   = 3,
    PINYIN_Ang  = 4,
    PINYIN_Ao   = 5,
    PINYIN_E    = 6,
    PINYIN_Ea   = 7,
    PINYIN_Ei   = 8,
    PINYIN_En   = 9,
    PINYIN_Eng  =10,
    PINYIN_Er   =11,
    PINYIN_I    =12,
    PINYIN_Ia   =13,
    PINYIN_Ian  =14,
    PINYIN_Iang =15,
    PINYIN_Iao  =16,
    PINYIN_Ie   =17,
    PINYIN_In   =18,
    PINYIN_Ing  =19,
    PINYIN_Iong =20,
    PINYIN_Iu   =21,
    PINYIN_Ng   =22,
    PINYIN_O    =23,
    PINYIN_Ong  =24,
    PINYIN_Ou   =25,
    PINYIN_U    =26,
    PINYIN_Ua   =27,
    PINYIN_Uai  =28,
    PINYIN_Uan  =29,
    PINYIN_Uang =30,
    PINYIN_Ue   =31,
    PINYIN_Ueng =32,
    PINYIN_Ui   =33,
    PINYIN_Un   =34,
    PINYIN_Uo   =35,
    PINYIN_V    =36,
    PINYIN_Van  =37,
    PINYIN_Ve   =38,
    PINYIN_Vn   =39,
    PINYIN_LastFinal = PINYIN_Vn,    /**< the last final */
    PINYIN_Number_Of_Finals = PINYIN_LastFinal + 1
};

/**
 * @brief enums of pinyin tone element.
 */
enum PinyinTone
{
    PINYIN_ZeroTone = 0,    /**< zero tone. this will be matched with all other tones. */
    PINYIN_First  = 1,
    PINYIN_Second = 2,
    PINYIN_Third  = 3,
    PINYIN_Fourth = 4,
    PINYIN_Fifth  = 5,
    PINYIN_LastTone = PINYIN_Fifth, /**< the last tone */
    PINYIN_Number_Of_Tones = PINYIN_LastTone + 1
};

/**
 * @brief enums of Shuang Pin Schemes.
 */
enum PinyinShuangPinScheme
{
#if 0
    SHUANG_PIN_STONE      = 0,
#endif
    SHUANG_PIN_ZRM        = 1,
    SHUANG_PIN_MS         = 2,
    SHUANG_PIN_ZIGUANG    = 3,
    SHUANG_PIN_ABC        = 4,
#if 0
    SHUANG_PIN_LIUSHI     = 5,
#endif
    SHUANG_PIN_PYJJ       = 6,
    SHUANG_PIN_XHE        = 7,
    SHUANG_PIN_CUSTOMIZED = 30,        /* for user's keyboard */
    SHUANG_PIN_DEFAULT    = SHUANG_PIN_MS
};

/**
 * @brief enums of ZhuYin Schemes.
 */
enum PinyinZhuYinScheme
{
    ZHUYIN_ZHUYIN   = 0,
    ZHUYIN_STANDARD = 1,
    ZHUYIN_HSU      = 2,
    ZHUYIN_IBM      = 3,
    ZHUYIN_GIN_YIEH = 4,
    ZHUYIN_ET       = 5,
    ZHUYIN_ET26     = 6,
    ZHUYIN_DEFAULT  = ZHUYIN_STANDARD
};

/**
 * @brief Pinyin key class.
 * 
 * A pinyin key is a composed element of an initial, a final and a tone,
 * which represents one or several Chinese ideographs
 *
 * The position and length information for the portion of string, from which
 * the PinyinKey is parsed, are also stored in this structure.
 */
struct PinyinKey
{
    friend class PinyinBitmapIndexLevel;
    friend inline int pinyin_exact_compare(const PinyinKey key_lhs[], 
					   const PinyinKey key_rhs[],
					   int word_length);
    friend inline int pinyin_compare_with_ambiguities
    (const PinyinCustomSettings &custom,
     const PinyinKey* key_lhs,
     const PinyinKey* key_rhs,
     int word_length);
    friend inline void compute_lower_value(const PinyinCustomSettings &custom,
					   PinyinKey in_keys[], 
					   PinyinKey out_keys[], 
					   int word_length);
    friend inline void compute_upper_value(const PinyinCustomSettings &custom,
					   PinyinKey in_keys[], 
					   PinyinKey out_keys[], 
					   int word_length);
    
private:
    guint16 m_initial : 5;   /**< pinyin initial */
    guint16 m_final   : 6;   /**< pinyin final */
    guint16 m_tone    : 3;   /**< pinyin tone */
public:
    /**
     * @brief Minimal numerical value of a PinyinKey
     * @sa get_value();
     */
    static const guint16 min_value;

    /**
     * @brief Maximal numerical value of a PinyinKey
     * @sa get_value();
     */
    static const guint16 max_value;

public:
    /**
     * Constructor.
     *
     * The default constructor of class PinyinKey.
     */
    PinyinKey (PinyinInitial initial = PINYIN_ZeroInitial,
               PinyinFinal   final   = PINYIN_ZeroFinal,
               PinyinTone    tone    = PINYIN_ZeroTone)
        : m_initial (initial), m_final (final), m_tone (tone)
    {
    }

    /**
     * Constructor.
     *
     * Construct a PinyinKey object from a key string, with
     * specified validator.
     *
     * @sa PinyinValidator
     */
    PinyinKey (const PinyinValidator &validator, const char *str, int len = -1)
    {
        set (validator, str, len);
    }

    PinyinKey (guint16 value)
    {
        set (value);
    }
    /**
     * Clear the PinyinKey object.
     */

    void clear ()
    {
        m_initial = PINYIN_ZeroInitial;
        m_final   = PINYIN_ZeroFinal;
        m_tone    = PINYIN_ZeroTone;
    }

    /**
     * Read PinyinKey value from a key string.
     * 
     * @param validator a PinyinValidator object to validate the key.
     * @param key a Latin string including one or more pinyin keys.
     * @return the number of characters used by this pinyin key.
     */ 
    int set (const PinyinValidator &validator, const char *str, int len = -1);

    /**
     * Set PinyinKey's value to initial, final and tone.
     */
    void set (PinyinInitial initial = PINYIN_ZeroInitial,
              PinyinFinal final     = PINYIN_ZeroFinal,
              PinyinTone tone       = PINYIN_ZeroTone)
    {
        m_initial = initial;
        m_final   = final;
        m_tone    = tone;
    }

    /**
     * @brief Set this PinyinKey from its numerical value.
     */
    void set (guint16 value)
    {
        m_tone = value % PINYIN_Number_Of_Tones;
        value /= PINYIN_Number_Of_Tones;
        m_final = value % PINYIN_Number_Of_Finals;
        m_initial = value / PINYIN_Number_Of_Finals;
    }

    /**
     * @brief Get numerical value of this PinyinKey
     */
    guint16 get_value () const
    {
        return (m_initial * PINYIN_Number_Of_Finals + m_final) * PINYIN_Number_Of_Tones + m_tone;
    }

    /**
     * Set PinyinKey's initial value to initial.
     */
    void set_initial (PinyinInitial initial = PINYIN_ZeroInitial)
    {
        m_initial = initial;
    }

    /**
     * Set PinyinKey's final value to final.
     */
    void set_final (PinyinFinal final = PINYIN_ZeroFinal)
    {
        m_final = final;
    }

    /**
     * Set PinyinKey's tone value to tone.
     */
    void set_tone (PinyinTone tone = PINYIN_ZeroTone)
    {
        m_tone = tone;
    }

    /**
     * Get initial value of this key.
     */
    PinyinInitial get_initial () const
    {
        return static_cast<PinyinInitial>(m_initial);
    }

    /**
     * Get final value of this key.
     */
    PinyinFinal get_final () const
    {
        return static_cast<PinyinFinal>(m_final);
    }

    /**
     * Get tone value of this key.
     */
    PinyinTone get_tone () const
    {
        return static_cast<PinyinTone>(m_tone);
    }

    /**
     * Get Latin name of this key's initial.
     */
    const char* get_initial_string () const;

    /**
     * Get Chinese ZhuYin name of this key's initial, in UTF-8 encoding.
     */
    const char* get_initial_zhuyin_string () const;

    /**
     * Get Latin name of this key's final.
     */
    const char* get_final_string () const;

    /**
     * Get Chinese ZhuYin name of this key's final, in UTF-8 encoding.
     */
    const char* get_final_zhuyin_string () const;

    /**
     * Get Latin name of this key's tone.
     */
    const char* get_tone_string () const;

    /**
     * Get Chinese ZhuYin name of this key's tone, in UTF-8 encoding.
     */
    const char* get_tone_zhuyin_string () const;

    /**
     * Get Latin name of this key.
     */
    const char * get_key_string () const;

    /**
     * Get Chinese ZhuYin name of this key, in UTF-8 encoding.
     */
    const char * get_key_zhuyin_string () const;

    /**
     * Check if this key is empty.
     */
    bool is_empty () const
    {
        return  m_initial == PINYIN_ZeroInitial && m_final == PINYIN_ZeroFinal && m_tone == PINYIN_ZeroTone;
    }

    /**
     * Check if this key has both initial, final and tone.
     */
    bool is_complete () const
    {
        return m_initial != PINYIN_ZeroInitial && m_final != PINYIN_ZeroFinal && m_tone != PINYIN_ZeroTone;
    }

    bool operator == (PinyinKey rhs) const
    {
        return m_initial == rhs.m_initial && m_final == rhs.m_final && m_tone == rhs.m_tone;
    }

    bool operator != (PinyinKey rhs) const
    {
        return m_initial != rhs.m_initial || m_final != rhs.m_final || m_tone != rhs.m_tone;
    }

    bool operator < (PinyinKey rhs) const
    {
        if (m_initial < rhs.m_initial) return true;
        if (m_initial > rhs.m_initial) return false;
        if (m_final < rhs.m_final) return true;
        if (m_final > rhs.m_final) return false;
        return m_tone < rhs.m_tone;
    }

    bool operator > (PinyinKey rhs) const
    {
        if (m_initial > rhs.m_initial) return true;
        if (m_initial < rhs.m_initial) return false;
        if (m_final > rhs.m_final) return true;
        if (m_final < rhs.m_final) return false;
        return m_tone > rhs.m_tone;
    }
};

/**
 * NULL Validator of PinyinKey object.
 *
 * This class is for validating a PinyinKey object.
 */
class PinyinValidator
{
public:
    /**
     * Overloaded operator () function to validate a pinyin key.
     *
     * @param key The key to be validated.
     * @return true if the key is valid.
     */
    virtual bool operator () (PinyinKey key) const = 0;
};

class PinyinLargeTable;
/**
 * Validator of PinyinKey object.
 *
 * This class is for validating a PinyinKey object.
 */
class BitmapPinyinValidator:public PinyinValidator
{
    char m_bitmap [(PINYIN_Number_Of_Initials * PINYIN_Number_Of_Finals * PINYIN_Number_Of_Tones + 7) / 8];

public:
    BitmapPinyinValidator (const PinyinLargeTable *table = 0);

    /**
     * initialize the validator with specified custom settings
     * and PinyinLargeTable.
     */
    void initialize (const PinyinLargeTable *table = 0);

    /**
     * Overloaded operator () function to validate a pinyin key.
     *
     * @param key The key to be validated.
     * @return true if the key is valid.
     */
    virtual bool operator () (PinyinKey key) const;
};

/**
 * NULL Validator of PinyinKey object.
 *
 * This class is for validating a PinyinKey object.
 */
class NullPinyinValidator:public PinyinValidator
{
public:
    /**
     * Overloaded operator () function to validate a pinyin key.
     *
     * @param key The key to be validated.
     * @return true if the key is valid.
     */
    virtual bool operator () (PinyinKey key) const{
	return true;
    }
};

/**
 * @brief Class to translate string into PinyinKey.
 */
class PinyinParser
{
public: 
    virtual ~PinyinParser ();

    /**
     * @brief Translate only one PinyinKey from a string.
     *
     * @param validator PinyinValidator object to valid result.
     * @param key Stores result PinyinKey.
     * @param str Input string in UTF-8 encoding, in most case this string is just a plain ASCII string,
     *            but for ZhuYin Parser works in ZHUYIN_ZHUYIN scheme,
     *            it's an UTF-8 string which contains ZhuYin chars.
     * @param len The length of str, in number of chars rather than bytes.
     *
     * @return the number of chars were actually used.
     */
    virtual int parse_one_key (const PinyinValidator &validator, PinyinKey &key, const char *str, int len) const = 0;

    /**
     * @brief Handy wrapper function of parse_one_key(), which accept a String object instead of char *.
     */
    int parse_one_key (const PinyinValidator &validator, PinyinKey &key, const char * &str) const
    {
        return parse_one_key (validator, key, str, strlen (str));
    }

    /**
     * @brief Translate the source string into a set of PinyinKeys.
     *
     * @param validator PinyinValidator object to valid result.
     * @param keys Stores result PinyinKeys.
     * @param str Input string in UTF-8 encoding, in most case this string is just a plain ASCII string,
     *            but for ZhuYin Parser works in ZHUYIN_ZHUYIN scheme,
     *            it's an UTF-8 string which contains ZhuYin chars.
     * @param len The length of str, in number of chars rather than bytes.
     *
     * @return the number of chars were actually used.
     */
    virtual int parse (const PinyinValidator &validator, PinyinKeyVector & keys,PinyinKeyPosVector & poses, const char *str, int len = -1) const = 0;

public:
    static void normalize (PinyinKey &key);
};

/**
 * The default Pinyin Parser which parses full pinyin string into PinyinKeys.
 */
class PinyinDefaultParser : public PinyinParser
{
public: 
    virtual ~PinyinDefaultParser ();

    virtual int parse_one_key (const PinyinValidator &validator, PinyinKey &key, const char *str, int len) const;
    virtual int parse (const PinyinValidator &validator, PinyinKeyVector & keys, PinyinKeyPosVector & poses, const char *str, int len = -1) const;

public:
    using PinyinParser::parse_one_key;
    using PinyinParser::parse;
};

/* The valid input chars of ShuangPin is a-z and ';'
 */
class PinyinShuangPinParser : public PinyinParser
{
    PinyinInitial m_initial_map [27];
    PinyinFinal   m_final_map [27][2];

public:
    /**
     * Constructor 
     *
     * @param scheme the predefined ShuangPin scheme to be used.
     */
    PinyinShuangPinParser (PinyinShuangPinScheme scheme = SHUANG_PIN_DEFAULT);
    PinyinShuangPinParser (const PinyinInitial initial_map[27], const PinyinFinal final_map[27][2]);

    virtual ~PinyinShuangPinParser ();

    virtual int parse_one_key (const PinyinValidator &validator, PinyinKey &key, const char *str, int len) const;
    virtual int parse (const PinyinValidator &validator, PinyinKeyVector &keys, PinyinKeyPosVector & poses, const char *str, int len = -1) const;

public:
    void set_scheme (PinyinShuangPinScheme scheme);
    void set_scheme (const PinyinInitial initial_map[27], const PinyinFinal final_map[27][2]);

    void get_scheme (PinyinInitial initial_map[27], PinyinFinal final_map[27][2]);

public:
    using PinyinParser::parse_one_key;
    using PinyinParser::parse;
};

/**
 * @brief Class to parse ZhuYin input string
 *
 * Several keyboard scheme are supported:
 * * ZHUYIN_ZHUYIN    Parse original ZhuYin string, such as ㄅㄧㄢ
 * * ZHUYIN_STANDARD  Standard ZhuYin keyboard, which maps 1 to Bo(ㄅ), q to Po(ㄆ) etc.
 * * ZHUYIN_HSU       Hsu ZhuYin keyboard, which uses a-z (except q) chars.
 * * ZHUYIN_IBM       IBM ZhuYin keyboard, which maps 1 to Bo(ㄅ), 2 to Po(ㄆ) etc.
 * * ZHUYIN_GIN_YIEH  Gin-Yieh ZhuYin keyboard.
 * * ZHUYIN_ET        Eten (倚天) ZhuYin keyboard.
 * * ZHUYIN_ET26      Eten (倚天) ZhuYin keyboard, which only uses a-z chars.
 *
 * In order to enable upper-level input method to display intermediate inputted string in ZhuYin chars,
 * ZhuYin parser may return invalid keys, so that PinyinKey::get_key_zhuyin_string() can be called for
 * each of these keys to get the intermediate inputted ZhuYin string.
 *
 * UTF-8 string is used in ZhuYin Parser, because the requirement of supporting original ZhuYin strings.
 * So that the length of inputted string is calculated in number of utf8 chars instead of bytes.
 */
class PinyinZhuYinParser : public PinyinParser
{
    PinyinZhuYinScheme m_scheme;

public:
    /**
     * Constructor 
     *
     * @param scheme the predefined ZhuYIn scheme to be used.
     */
    PinyinZhuYinParser (PinyinZhuYinScheme scheme = ZHUYIN_DEFAULT);

    virtual ~PinyinZhuYinParser ();

    virtual int parse_one_key (const PinyinValidator &validator, PinyinKey &key, const char *str, int len = -1) const;
    virtual int parse (const PinyinValidator &validator, PinyinKeyVector &keys, PinyinKeyPosVector & poses, const char *str, int len = -1) const;

public:
    void set_scheme (PinyinZhuYinScheme scheme);
    PinyinZhuYinScheme get_scheme () const;

private:
    bool get_keys (PinyinKey keys[], gunichar ch) const;

    int pack_keys (PinyinKey &key, const PinyinValidator &validator, const PinyinKey keys[][3]) const;

public:
    using PinyinParser::parse_one_key;
    using PinyinParser::parse;
};


int pinyin_compare_initial (const PinyinCustomSettings &custom,
			    PinyinInitial lhs,
			    PinyinInitial rhs);

int pinyin_compare_final (const PinyinCustomSettings &custom,
			  PinyinFinal lhs,
			  PinyinFinal rhs);

int pinyin_compare_tone (const PinyinCustomSettings &custom,
			 PinyinTone lhs,
			 PinyinTone rhs);

};

#endif
