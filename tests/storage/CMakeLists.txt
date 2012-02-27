include_directories(..)

add_executable(
    test_parser2
    test_parser2.cpp
)

target_link_libraries(
    test_parser2
    libpinyin
)

add_executable(
    test_chewing_table
    test_chewing_table.cpp
)

target_link_libraries(
    test_chewing_table
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
    test_phrase_index_logger
    test_phrase_index_logger.cpp
)

target_link_libraries(
    test_phrase_index_logger
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
