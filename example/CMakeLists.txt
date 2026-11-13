add_executable(rws_test rws_test.cpp)

# abb_rws::abb_rws already propagates the POCO libraries and include paths.
target_link_libraries(rws_test PRIVATE abb_rws::abb_rws)

# Windows does not search the POCO import-library directory for runtime DLLs.
# Copy the dependencies beside rws_test so the executable can be launched
# directly after a build, without requiring a manually modified PATH.
if(WIN32)
  set(RWS_TEST_RUNTIME_DLLS)
  set(POCO_RUNTIME_DIR)

  if(TARGET Poco::Foundation)
    # POCO CMake config: the imported location of PocoFoundation is the DLL.
    foreach(config_suffix _RELEASE _RELWITHDEBINFO _MINSIZEREL _DEBUG "")
      get_target_property(POCO_FOUNDATION_DLL Poco::Foundation IMPORTED_LOCATION${config_suffix})
      if(POCO_FOUNDATION_DLL)
        get_filename_component(POCO_RUNTIME_DIR "${POCO_FOUNDATION_DLL}" DIRECTORY)
        break()
      endif()
    endforeach()
  elseif(Poco_INCLUDE_DIR)
    # Bundled FindPoco module: derive <prefix>/bin from <prefix>/include.
    get_filename_component(POCO_RUNTIME_DIR "${Poco_INCLUDE_DIR}/../bin" ABSOLUTE)
  endif()

  if(POCO_RUNTIME_DIR)
    # Include transitive POCO DLLs as well (for example PocoUtil depends on
    # PocoJSON), not only the libraries directly linked by abb_rws.
    # PocoNetSSL uses OpenSSL; these names vary by OpenSSL version.
    file(GLOB RWS_TEST_RUNTIME_DLLS
      "${POCO_RUNTIME_DIR}/Poco*.dll"
      "${POCO_RUNTIME_DIR}/libssl*.dll"
      "${POCO_RUNTIME_DIR}/libcrypto*.dll"
      "${POCO_RUNTIME_DIR}/zlib*.dll")
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
