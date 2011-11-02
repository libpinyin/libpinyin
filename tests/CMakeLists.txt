add_subdirectory(include)
add_subdirectory(storage)
add_subdirectory(lookup)

add_executable(
    test_pinyin
    test_pinyin.cpp
)

target_link_libraries(
    test_pinyin
    libpinyin
)

add_executable(
    test_phrase
    test_phrase.cpp
)

target_link_libraries(
    test_phrase
    libpinyin
)

add_executable(
    test_chewing
    test_chewing.cpp
)

target_link_libraries(
    test_chewing
    libpinyin
)
