add_executable(
    test_memory_chunk
    test_memory_chunk.cpp
)

target_link_libraries(
    test_memory_chunk
    libpinyin
)