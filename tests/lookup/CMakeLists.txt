include_directories(..)

add_executable(
    test_pinyin_lookup
    test_pinyin_lookup.cpp
)

target_link_libraries(
    test_pinyin_lookup
    pinyin
)

add_executable(
    test_phrase_lookup
    test_phrase_lookup.cpp
)

target_link_libraries(
    test_phrase_lookup
    pinyin
)
