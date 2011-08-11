add_executable(
    test_simple_lookup
    test_simple_lookup.cpp
)

target_link_libraries(
    test_simple_lookup
    libpinyin
)

add_executable(
    test_phrase_lookup
    test_phrase_lookup.cpp
)

target_link_libraries(
    test_phrase_lookup
    libpinyin
)