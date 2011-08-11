set(
    LIBPINYIN_INCLUDE_HEADERS
    memory_chunk.h
    novel_types.h
    stl_lite.h
)

install(
    FILES
        ${LIBPINYIN_INCLUDE_HEADERS}
    DESTINATION
        ${DIR_INCLUDE_LIBPINYIN}
)
