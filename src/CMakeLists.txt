set(
    LIBPINYIN_HEADERS
    pinyin.h
)

set(
    LIBPINYIN_SOURCES
    pinyin.cpp
)

add_library(
    pinyin
    SHARED
    ${LIBPINYIN_SOURCES}
)

target_link_libraries(
    pinyin
    storage
    lookup
)

set_target_properties(
    pinyin
    PROPERTIES
        OUTPUT_NAME
            pinyin
        VERSION
            13.0.0
        SOVERSION
            13
)

install(
    TARGETS
        pinyin
    LIBRARY DESTINATION
        ${DIR_LIBRARY}
)

install(
    FILES
        ${LIBPINYIN_HEADERS}
    DESTINATION
        ${DIR_INCLUDE_LIBPINYIN}
)

add_subdirectory(include)
add_subdirectory(storage)
add_subdirectory(lookup)
