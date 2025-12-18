set(
    CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC"
)

set(
    LIBSTORAGE_HEADERS
    pinyin_custom2.h
)

set(
    LIBSTORAGE_SOURCES
    phrase_index.cpp
    phrase_large_table2.cpp
    phrase_large_table3.cpp
    ngram.cpp
    tag_utility.cpp
    chewing_key.cpp
    pinyin_parser2.cpp
    zhuyin_parser2.cpp
    phonetic_key_matrix.cpp
    chewing_large_table.cpp
    chewing_large_table2.cpp
    table_info.cpp
    punct_table.cpp
)

if (HAVE_BERKELEY_DB)
  set(
    LIBSTORAGE_SOURCES
    ${LIBSTORAGE_SOURCES}
    chewing_large_table2_bdb.cpp
    ngram_bdb.cpp
    phrase_large_table3_bdb.cpp
    punct_table_bdb.cpp
    )
endif (HAVE_BERKELEY_DB)

if (HAVE_KYOTO_CABINET)
  set(
    LIBSTORAGE_SOURCES
    ${LIBSTORAGE_SOURCES}
    chewing_large_table2_kyotodb.cpp
    ngram_kyotodb.cpp
    phrase_large_table3_kyotodb.cpp
    punct_table_kyotodb.cpp
    )
endif (HAVE_KYOTO_CABINET)

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
