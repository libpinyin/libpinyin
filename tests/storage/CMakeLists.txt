add_executable(
    test_parser
    test_parser.cpp
)

target_link_libraries(
    test_parser
    libpinyin
)

add_executable(
    test_pinyin_index
    test_pinyin_index.cpp
)

target_link_libraries(
    test_pinyin_index
    libpinyin
)

add_executable(
    test_phrase_index
    test_phrase_index.cpp
)

target_link_libraries(
    test_phrase_index
    libpinyin
)

add_executable(
    test_phrase_table
    test_phrase_table.cpp
)

target_link_libraries(
    test_phrase_table
    libpinyin
)

add_executable(
    test_ngram
    test_ngram.cpp
)

target_link_libraries(
    test_ngram
    libpinyin
)

add_executable(
    test_flexible_ngram
    test_flexible_ngram.cpp
)

target_link_libraries(
    test_flexible_ngram
    libpinyin
)