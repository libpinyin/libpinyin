set(
    LIBLOOKUP_HEADERS
    lookup.h
    pinyin_lookup.h
    phrase_lookup.h
    winner_tree.h
)

set(
    LIBLOOKUP_SOURCES
    pinyin_lookup.cpp
    winner_tree.cpp
    phrase_lookup.cpp
)

add_library(
    liblookup
    STATIC
    ${LIBLOOKUP_SOURCES}
)

install(
    FILES
        ${LIBLOOKUP_HEADERS}
    DESTINATION
        ${DIR_INCLUDE_LIBPINYIN}
)
