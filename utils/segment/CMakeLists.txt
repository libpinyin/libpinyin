add_executable(
    spseg
    spseg.cpp
)

target_link_libraries(
    spseg
    libpinyin
)

add_executable(
    ngseg
    ngseg.cpp
)

target_link_libraries(
    ngseg
    libpinyin
)