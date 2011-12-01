set(
    LIBPINYIN_INCLUDE_HEADERS
    novel_types.h
)

install(
    FILES
        ${LIBPINYIN_INCLUDE_HEADERS}
    DESTINATION
        ${DIR_INCLUDE_LIBPINYIN}
)
