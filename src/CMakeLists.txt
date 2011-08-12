set(
    LIBPINYIN_HEADERS
    pinyin.h
)

set(
    LIBPINYIN_SOURCES
    pinyin.cpp
)

add_library(
    libpinyin
    SHARED
    ${LIBPINYIN_SOURCES}
)

target_link_libraries(
    libpinyin
    storage
    lookup
)

set_target_properties(
    libpinyin
    PROPERTIES
        OUTPUT_NAME
            pinyin
        VERSION
            0.0.0
        SOVERSION
            0
)

install(
    TARGETS
        libpinyin
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
