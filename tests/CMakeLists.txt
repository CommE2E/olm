enable_testing()

add_executable(test_base64 test_base64.cpp)
target_include_directories(test_base64 PRIVATE include)
target_link_libraries(test_base64 Olm::Olm)
add_test(Base64 test_base64)

add_executable(test_crypto test_crypto.cpp)
target_include_directories(test_crypto PRIVATE include)
target_link_libraries(test_crypto Olm::Olm)
add_test(Crypto test_crypto)

add_executable(test_group_session test_group_session.cpp)
target_include_directories(test_group_session PRIVATE include)
target_link_libraries(test_group_session Olm::Olm)
add_test(GroupSession test_group_session)

add_executable(test_list test_list.cpp)
target_include_directories(test_list PRIVATE include)
target_link_libraries(test_list Olm::Olm)
add_test(List test_list)

add_executable(test_megolm test_megolm.cpp)
target_include_directories(test_megolm PRIVATE include)
target_link_libraries(test_megolm Olm::Olm)
add_test(Megolm test_megolm)

add_executable(test_message test_message.cpp)
target_include_directories(test_message PRIVATE include)
target_link_libraries(test_message Olm::Olm)
add_test(Message test_message)

add_executable(test_olm test_olm.cpp)
target_include_directories(test_olm PRIVATE include)
target_link_libraries(test_olm Olm::Olm)
add_test(Olm test_olm)

add_executable(test_olm_decrypt test_olm_decrypt.cpp)
target_include_directories(test_olm_decrypt PRIVATE include)
target_link_libraries(test_olm_decrypt Olm::Olm)
add_test(OlmDecrypt test_olm_decrypt)

add_executable(test_olm_sha256 test_olm_sha256.cpp)
target_include_directories(test_olm_sha256 PRIVATE include)
target_link_libraries(test_olm_sha256 Olm::Olm)
add_test(OlmSha256 test_olm_sha256)

add_executable(test_olm_signature test_olm_signature.cpp)
target_include_directories(test_olm_signature PRIVATE include)
target_link_libraries(test_olm_signature Olm::Olm)
add_test(OlmSignature test_olm_signature)

add_executable(test_olm_using_malloc test_olm_using_malloc.cpp)
target_include_directories(test_olm_using_malloc PRIVATE include)
target_link_libraries(test_olm_using_malloc Olm::Olm)
add_test(OlmUsingMalloc test_olm_using_malloc)

add_executable(test_ratchet test_ratchet.cpp)
target_include_directories(test_ratchet PRIVATE include)
target_link_libraries(test_ratchet Olm::Olm)
add_test(Ratchet test_ratchet)

add_executable(test_session test_session.cpp)
target_include_directories(test_session PRIVATE include)
target_link_libraries(test_session Olm::Olm)
add_test(Session test_session)
