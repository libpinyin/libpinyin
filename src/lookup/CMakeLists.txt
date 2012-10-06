set(
    CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC"
)

set(
    LIBLOOKUP_SOURCES
    pinyin_lookup2.cpp
    phrase_lookup.cpp
    lookup.cpp
)

add_library(
    lookup
    STATIC
    ${LIBLOOKUP_SOURCES}
)

install(
    FILES
        ${LIBLOOKUP_HEADERS}
    DESTINATION
        ${DIR_INCLUDE_LIBPINYIN}
)
