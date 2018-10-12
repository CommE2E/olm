foreach(fuzz IN ITEMS
    fuzz_decode_message
    fuzz_decrypt
    fuzz_group_decrypt
    fuzz_unpickle_account
    fuzz_unpickle_session
    )
add_executable(${fuzz} ${fuzz}.cpp)
target_include_directories(${fuzz} PRIVATE include)
target_link_libraries(${fuzz} Olm::Olm)
endforeach(fuzz)
