set(
    CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC"
)

set(
    LIBSTORAGE_HEADERS
    chewing_key.h
    pinyin_custom2.h
)

set(
    LIBSTORAGE_SOURCES
    phrase_index.cpp
    phrase_large_table2.cpp
    ngram.cpp
    tag_utility.cpp
    pinyin_parser2.cpp
    chewing_large_table.cpp
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
