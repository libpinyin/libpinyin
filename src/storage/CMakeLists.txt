set(
    CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC"
)

set(
    LIBSTORAGE_HEADERS
    pinyin_large_table.h
    pinyin_base.h
    pinyin_phrase.h
    phrase_index.h
    phrase_index_logger.h
    pinyin_zhuyin_map_data.h
    phrase_large_table.h
    ngram.h
    flexible_ngram.h
    tag_utility.h
)

set(
    LIBSTORAGE_SOURCES
    pinyin_base.cpp
    pinyin_large_table.cpp
    phrase_index.cpp
    phrase_large_table.cpp
    ngram.cpp
    tag_utility.cpp
)

add_library(
    storage
    STATIC
    ${LIBSTORAGE_SOURCES}
)

target_link_libraries(
    storage
    ${GLIB2_LIBRARIES}
    ${BERKELEY_DB_LIBRARIES}
)

install(
    FILES
        ${LIBSTORAGE_HEADERS}
    DESTINATION
        ${DIR_INCLUDE_LIBPINYIN}
)
