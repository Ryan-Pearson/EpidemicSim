#=============
# CMake setup
#=============

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

if (NOT USE_CONAN)
    find_package(Boost ${MODULES_BOOST_VERSION} COMPONENTS thread REQUIRED QUIET)
    find_package(fmt REQUIRED)
endif()

add_executable(EpidemicSim EpidemicSim.cpp)

target_link_libraries(EpidemicSim
    PUBLIC
        Epidemic
)

if (USE_CONAN)
    target_link_libraries(EpidemicSim
        PUBLIC
            ${CONAN_LIBS}
    )
else()
    target_link_libraries(EpidemicSim
        PUBLIC
	    fmt
            Boost::boost
            Boost::thread
    )
endif()

target_compile_definitions(EpidemicSim PRIVATE
        BASE_DIR="${CMAKE_SOURCE_DIR}"
)