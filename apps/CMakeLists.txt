#=============
# CMake setup
#=============

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

add_executable(EpidemicSim EpidemicSim.cpp)
target_link_libraries(EpidemicSim
    PUBLIC
        Epidemic
        ${CONAN_LIBS}
)

target_compile_definitions(EpidemicSim PRIVATE
        BASE_DIR="${CMAKE_SOURCE_DIR}"
)