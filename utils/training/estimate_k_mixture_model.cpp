#include "pinyin.h"
#include "k_mixture_model.h"

parameter_t compute_interpolation(KMixtureModelSingleGram * deleted_bigram,
                                  KMixtureModelBigram * unigram,
                                  KMixtureModelSingleGram * bigram){
    bool success;
    parameter_t lambda = 0, next_lambda = 0.6;
    parameter_t epsilon = 0.001;

    while (fabs(lambda - next_lambda) > epsilon){
        lambda = next_lambda;
        next_lambda = 0;
        parameter_t numerator = 0;
        parameter_t part_of_denominator = 0;

        FlexibleBigramPhraseArray array = g_array_new(FALSE, FALSE, sizeof(KMixtureModelArrayItemWithToken));
        deleted_bigram->retrieve_all(array);

        for ( int i = 0; i < array->len; ++i){
            KMixtureModelArrayItemWithToken * item = &g_array_index(array, KMixtureModelArrayItemWithToken, i);
            //get the phrase token
            phrase_token_t token = item->m_token;
            guint32 deleted_count = item->m_item.m_WC;

            {
                parameter_t elem_poss = 0;
                KMixtureModelArrayItem item;
                if ( bigram && bigram->get_array_item(token, item) ){
                    KMixtureModelArrayHeader header;
                    assert(bigram->get_array_header(header));
                    assert(0 != header.m_WC);
                    elem_poss = item.m_WC / (parameter_t) header.m_WC;
                }
                numerator = lambda * elem_poss;
            }

            {
                parameter_t elem_poss = 0;
                KMixtureModelMagicHeader magic_header;
                KMixtureModelArrayHeader array_header;
                if (unigram->get_array_header(token, array_header)){
                    /* Note: optimize here? */
                    assert(unigram->get_magic_header(magic_header));
                    assert(0 != magic_header.m_WC);
                    elem_poss = array_header.m_WC / (parameter_t) magic_header.m_WC;
                }
                part_of_denominator = (1 - lambda) * elem_poss;
            }
            if (0 == (numerator + part_of_denominator))
                continue;

            next_lambda += deleted_count * (numerator / (numerator + part_of_denominator));
        }
        KMixtureModelArrayHeader header;
        assert(deleted_bigram->get_array_header(header));
        assert(0 != header.m_WC);
        next_lambda /= header.m_WC;

        g_array_free(array, TRUE);
    }
    lambda = next_lambda;
    return lambda;
}

int main(int argc, char * argv[]){
    return 0;
}
