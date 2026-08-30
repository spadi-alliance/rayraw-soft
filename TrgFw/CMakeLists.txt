set(FW_NAME TrgFw)
set(MY_TARGET ${FW_NAME}_daq)

set(MY_PUB_HEADER
    "RegisterMap.hh"
    "MhTdcFuncs.hh"
    "AdcFuncs.hh"
    "SelfTriggerConfig.hh"
)

set(TRGFW_CONFIG_DIR
    "${CMAKE_INSTALL_PREFIX}/${FW_NAME}/config"
)

install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/self_trig.txt"
    DESTINATION "${FW_NAME}/config"
)

add_executable(${MY_TARGET}
    daq_main.cc
    DaqOptions.cc
    DaqSetup.cc
    SelfTriggerConfig.cc
    MhTdcFuncs.cc
    AdcFuncs.cc
)

target_compile_definitions(${MY_TARGET} PRIVATE
    RAYRAW_CONFIG_DIR="${TRGFW_CONFIG_DIR}"
)

target_compile_options(${MY_TARGET} PRIVATE
    ${my_cxx_options}
)

set_target_properties(${MY_TARGET} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(${MY_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${MY_TARGET} PRIVATE
    HulCore
)

set_target_properties(${MY_TARGET} PROPERTIES
    OUTPUT_NAME daq
    PUBLIC_HEADER "${MY_PUB_HEADER}"
)

install(TARGETS ${MY_TARGET}
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${FW_NAME}/bin
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_PREFIX}/${FW_NAME}/include
)
