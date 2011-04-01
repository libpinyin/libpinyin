

#ifndef FLEXIBLE_NGRAM_H
#define FLEXIBLE_NGRAM_H


/* Note: the signature of the template parameters.
 * struct MagicHeader, ArrayHeader, ArrayItem.
 */

typedef GArray * FlexibleBigramPhraseArray;

template<typename ArrayHeader, typename ArrayItem>
class FlexibleSingleGram{
    template<typename MagicHeader, typename ArrayHeader,
             typename ArrayItem>
    friend class FlexibleBigram;
private:
    MemoryChunk m_chunk;
    FlexibleSingleGram(void * buffer, size_t length);
public:
    /* item typedefs */
    typedef struct{
        phrase_token_t m_token;
        ArrayItem m_item;
    } ArrayItemWithToken;

    /* Null Constructor */
    FlexibleSingleGram();
    /* retrieve all items */
    bool retrieve_all(/* out */ FlexibleBigramPhraseArray array);

    /* search method */
    /* the array result contains many items */
    bool search(/* in */ PhraseIndexRange * range,
                /* out */ FlexibleBigramPhraseArray array);

    /* get array item */
    bool get_array_item(/* in */ phrase_token_t token,
                        /* out */ ArrayItem & item);
    /* set array item */
    bool set_array_item(/* in */ phrase_token_t token,
                        /* in */ const ArrayItem & item);

    /* get array header */
    bool get_array_header(/* out */ ArrayHeader & header);

    /* set array header */
    bool set_array_header(/* in */ const ArrayHeader & header);
};

template<typename MagicHeader, typename ArrayHeader,
         typename ArrayItem>
class FlexibleBigram{
private:
    DB * m_db;

    void reset(){
        if ( m_db ){
            m_db->close(m_db, 0);
            m_db = NULL;
        }
    }

public:
    FlexibleBigram(){
        m_db = NULL;
    }

    ~FlexibleBigram(){
        reset();
    }

    /* attach berkeley db on filesystem for training purpose. */
    bool attach(const char * dbfile);
    /* load/store one array. */
    bool load(phrase_token_t index,
              FlexibleSingleGram<ArrayHeader, ArrayItem> * & single_gram);
    bool store(phrase_token_t index, FlexibleSingleGram * & single_gram);
    /* array of phrase_token_t items, for parameter estimation. */
    bool get_all_items(GArray * items);

    /* get/set magic header. */
    bool get_magic_header(MagicHeader & header);
    bool set_magic_header(const MagicHeader & header);
};

#endif
