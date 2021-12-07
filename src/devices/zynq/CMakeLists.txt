list(APPEND SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/zynq.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/zynq_fine_grained.cpp    
)
if (${PARTITIONING_MODE})
    list(APPEND SRC_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/milp_model_zynq_with_partition.cpp
    )
else()
    list(APPEND SRC_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/milp_model_zynq.cpp
    )
endif()

# send the definitions for the parent scope
set(SRC_FILES ${SRC_FILES} PARENT_SCOPE)
set(DEVICE_DEFINE -DFPGA_ZYNQ PARENT_SCOPE)
