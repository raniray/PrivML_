cmake_minimum_required(VERSION 3.10)
project(DAPPLE)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake-build-debug)

find_package(OpenMP)

if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()



# Must be added to include all the sources
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/*/*.c PROPERTIES LANGUAGE C )

file(GLOB Common_sources "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cc" "${CMAKE_CURRENT_SOURCE_DIR}/*/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/ML/VFDT/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/ML/VFDT/*.cc" "${CMAKE_CURRENT_SOURCE_DIR}/ML/VFDT/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/ML/NB/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/ML/NB/*.cc" "${CMAKE_CURRENT_SOURCE_DIR}/ML/NB/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/PPML/*/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/PPML/*/*.cc" "${CMAKE_CURRENT_SOURCE_DIR}/PPML/*/*.h"
        )


ADD_EXECUTABLE(DAPPLE ${Common_sources} ML/LR/LR.cpp ML/LR/LR.h CRYPTO/DTPKC.cpp CRYPTO/DTPKC.h ML/NN/neuron.cpp ML/NN/neuron.h ML/NN/NN.cpp ML/NN/NN.h)



target_link_libraries(DAPPLE gmp gmpxx ntl m config4cpp OpenMP::OpenMP_CXX mpfr ssl crypto)