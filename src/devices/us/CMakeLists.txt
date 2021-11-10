list(APPEND SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/us.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/us_fine_grained.cpp
)
if (${PARTITIONING_MODE})
    message(FATAL_ERROR "FPGA '${FPGA}' not implemented in partitioning mode!")
else()
    list(APPEND SRC_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/milp_model_us.cpp
    )
endif()

# send the definitions for the parent scope
set(SRC_FILES ${SRC_FILES} PARENT_SCOPE)
set(DEVICE_DEFINE -DFPGA_US PARENT_SCOPE)
