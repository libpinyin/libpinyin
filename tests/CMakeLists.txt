add_subdirectory(include)
add_subdirectory(storage)
add_subdirectory(lookup)

add_executable(
    test_pinyin
    test_pinyin.cpp
)

target_link_libraries(
    test_pinyin
    pinyin
)

add_test(NAME pinyin COMMAND test_pinyin)

add_executable(
    test_phrase
    test_phrase.cpp
)

target_link_libraries(
    test_phrase
    pinyin
)

add_test(NAME phrase COMMAND test_phrase)

add_executable(
    test_chewing
    test_chewing.cpp
)

target_link_libraries(
    test_chewing
    pinyin
)

add_test(NAME chewing COMMAND test_chewing)
