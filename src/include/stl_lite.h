#ifndef STL_LITE_H
#define STL_LITE_H

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

namespace std_lite{

    /**
     * To restrict the usage of STL functions in libpinyin,
     * all needed functions should be imported here.
     */


    using std::min;


    using std::max;


    using std::pair;


    using std::make_pair;


    using std::lower_bound;


    using std::upper_bound;


    using std::equal_range;


    using std::make_heap;


    using std::pop_heap;


    using std::push_heap;


}
#endif
