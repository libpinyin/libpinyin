#include <glib.h>
#include "tag_utility.h"


/* internal taglib structure */
struct tag_entry{
    int m_line_type;
    const char * m_line_tag;
    int m_num_of_values;
    const char ** m_required_tags;
    /* const char ** m_optional_tags; */
    /* int m_optional_count = 0; */
    const char ** m_ignored_tags;

public:
    tag_entry(int line_type, const char * line_tag, int num_of_values,
              const char * required_tags[], const char * ignored_tags[]){
        m_line_type = line_type; m_line_tag = line_tag;
        m_num_of_values = num_of_values;
        m_required_tags = required_tags;
        m_ignored_tags = ignored_tags;
    }
};

void test(){
    TAGLIB_BEGIN_ADD_TAG(1, "\\data", 0);
    TAGLIB_REQUIRED_TAGS = {"model", NULL};
    TAGLIB_IGNORED_TAGS = {"data", NULL};
    TAGLIB_END_ADD_TAG;
}
