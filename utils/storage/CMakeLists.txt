add_executable(
    gen_binary_files
    gen_binary_files.cpp
)

target_link_libraries(
    gen_binary_files
    libpinyin
)

add_executable(
    import_interpolation
    import_interpolation.cpp
)

target_link_libraries(
    import_interpolation
    libpinyin
)

add_executable(
    export_interpolation
    export_interpolation.cpp
)

target_link_libraries(
    export_interpolation
    libpinyin
)
