add_executable(
    gen_binary_files
    gen_binary_files.cpp
)

target_link_libraries(
    gen_binary_files
    pinyin
)
if(INSTALL_DATA_UTILS)
    install(
        TARGETS
            gen_binary_files
        RUNTIME DESTINATION
            ${DIR_BIN}
    )
endif()

add_executable(
    import_interpolation
    import_interpolation.cpp
)

target_link_libraries(
    import_interpolation
    pinyin
)
if(INSTALL_DATA_UTILS)
    install(
        TARGETS
            import_interpolation
        RUNTIME DESTINATION
            ${DIR_BIN}
    )
endif()

add_executable(
    export_interpolation
    export_interpolation.cpp
)

target_link_libraries(
    export_interpolation
    pinyin
)
if(INSTALL_DATA_UTILS)
    install(
        TARGETS
            export_interpolation
        RUNTIME DESTINATION
            ${DIR_BIN}
    )
endif()
