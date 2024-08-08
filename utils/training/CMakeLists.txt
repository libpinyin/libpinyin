add_executable(
    gen_ngram
    gen_ngram.cpp
)

target_link_libraries(
    gen_ngram
    pinyin
)

add_executable(
    gen_deleted_ngram
    gen_deleted_ngram.cpp
)

target_link_libraries(
    gen_deleted_ngram
    pinyin
)

add_executable(
    gen_unigram
    gen_unigram.cpp
)
if(INSTALL_DATA_UTILS)
    install(
        TARGETS
            gen_unigram
        RUNTIME DESTINATION
            ${DIR_BIN}
    )
endif()

target_link_libraries(
    gen_unigram
    pinyin
)

add_executable(
    gen_k_mixture_model
    gen_k_mixture_model.cpp
)

target_link_libraries(
    gen_k_mixture_model
    pinyin
)

add_executable(
    estimate_interpolation
    estimate_interpolation.cpp
)

target_link_libraries(
    estimate_interpolation
    pinyin
)

add_executable(
    estimate_k_mixture_model
    estimate_k_mixture_model.cpp
)

target_link_libraries(
    estimate_k_mixture_model
    pinyin
)

add_executable(
    merge_k_mixture_model
    merge_k_mixture_model.cpp
)

target_link_libraries(
    merge_k_mixture_model
    pinyin
)

add_executable(
    prune_k_mixture_model
    prune_k_mixture_model.cpp
)

target_link_libraries(
    prune_k_mixture_model
    pinyin
)

add_executable(
    import_k_mixture_model
    import_k_mixture_model.cpp
)

target_link_libraries(
    import_k_mixture_model
    pinyin
)

add_executable(
    export_k_mixture_model
    export_k_mixture_model.cpp
)

target_link_libraries(
    export_k_mixture_model
    pinyin
)

add_executable(
    k_mixture_model_to_interpolation
    k_mixture_model_to_interpolation.cpp
)

target_link_libraries(
    k_mixture_model_to_interpolation
    pinyin
)

add_executable(
    validate_k_mixture_model
    validate_k_mixture_model.cpp
)

target_link_libraries(
    validate_k_mixture_model
    pinyin
)

add_executable(
    eval_correction_rate
    eval_correction_rate.cpp
)

target_link_libraries(
    eval_correction_rate
    pinyin
)
