set(FW_NAME TrgFw)
#set(LIB_NAME FwFuncs)

set(MY_PUB_HEADER
  "RegisterMap.hh"
  "MhTdcFuncs.hh"
  "AdcFuncs.hh"
  )

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/config)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/self_trig.txt ${CMAKE_BINARY_DIR}/config/self_trig.txt COPYONLY)

foreach(val IN ITEMS
    daq
    )

  set(MY_OUTPUT_NAME ${val})
  set(MY_TARGET ${FW_NAME}_${val})
  add_executable(${MY_TARGET}
    ${MY_OUTPUT_NAME}_main.cc
    MhTdcFuncs.cc
    )

  if(${MYDEBUG})
    message(STATUS "MY_TARGET @ Common: " ${MY_TARGET})
  endif()

  target_compile_options(${MY_TARGET} PUBLIC
    ${my_cxx_options}
    )
  target_compile_features(${MY_TARGET} PUBLIC
    ${my_cxx_features}
    )
  target_include_directories(${MY_TARGET} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    )
  target_link_libraries(${MY_TARGET} PUBLIC
    HulCore;
    )

  set_target_properties(${MY_TARGET} PROPERTIES
    OUTPUT_NAME ${MY_OUTPUT_NAME}
    PUBLIC_HEADER "${MY_PUB_HEADER}"
    )

  install(TARGETS ${MY_TARGET}
    RUNTIME       DESTINATION ${CMAKE_INSTALL_PREFIX}/${FW_NAME}/bin
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_PREFIX}/${FW_NAME}/include
    )
endforeach()

