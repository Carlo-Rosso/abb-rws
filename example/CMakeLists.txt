add_executable(rws_test rws_test.cpp)

target_link_libraries(rws_test PRIVATE abb_rws::abb_rws ${Poco_LIBRARIES})

# Windows does not search the POCO import-library directory for runtime DLLs.
# Copy the dependencies beside rws_test so the executable can be launched
# directly after a build, without requiring a manually modified PATH.
if(WIN32)
  set(RWS_TEST_RUNTIME_DLLS)

  # FindPoco always provides the absolute Foundation import-library path.
  # Derive the matching runtime bin directory from it.
  get_filename_component(POCO_PREFIX_DIR "${Poco_INCLUDE_DIR}/.." ABSOLUTE)

  # Include transitive POCO DLLs as well (for example PocoUtil depends on
  # PocoJSON), not only the libraries directly linked by abb_rws.
  if(POCO_PREFIX_DIR)
    file(GLOB POCO_RUNTIME_DLLS "${POCO_PREFIX_DIR}/bin/Poco*.dll")
    list(APPEND RWS_TEST_RUNTIME_DLLS ${POCO_RUNTIME_DLLS})
  endif()

  # PocoNetSSL uses OpenSSL; these names vary by OpenSSL version.
  if(POCO_PREFIX_DIR)
    file(GLOB OPENSSL_RUNTIME_DLLS
      "${POCO_PREFIX_DIR}/bin/libssl*.dll"
      "${POCO_PREFIX_DIR}/bin/libcrypto*.dll"
      "${POCO_PREFIX_DIR}/bin/zlib*.dll")
    list(APPEND RWS_TEST_RUNTIME_DLLS ${OPENSSL_RUNTIME_DLLS})
  endif()

  list(REMOVE_DUPLICATES RWS_TEST_RUNTIME_DLLS)
  foreach(RUNTIME_DLL IN LISTS RWS_TEST_RUNTIME_DLLS)
    add_custom_command(
      TARGET rws_test POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "${RUNTIME_DLL}"
              "$<TARGET_FILE_DIR:rws_test>"
      VERBATIM)
  endforeach()
endif()

install(
  TARGETS rws_test
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(WIN32)
  foreach(RUNTIME_DLL IN LISTS RWS_TEST_RUNTIME_DLLS)
    install(FILES "${RUNTIME_DLL}" DESTINATION ${CMAKE_INSTALL_BINDIR})
  endforeach()
endif()