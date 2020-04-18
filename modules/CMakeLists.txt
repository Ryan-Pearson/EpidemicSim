# Determines the minimum version of CMake modules require for use
set(MODULES_MIN_CMAKE_VERSION 3.5.0)

# Each module will set these CMP Variables to use the "NEW" behaviour
set (MODULES_CMP_ENABLES
   CMP0025 # Using AppleClang instead of clang on mac
   CMP0028 # Ensure that double colon in target names mean alias or imported target only
   CMP0042 # Enable @rpaths in target's install name
   CMP0054 # Only interpret if arguments as variables or keywords when unquoted
   CMP0063 # Honor visibility properties for all target types
)

# Setting required 3rd party dependency versions for use in modules
set(MODULES_BOOST_VERSION 1.66)

# Sets -fvisibility=hidden on Mac/Linux, matches Windows behaviour for exporting symbols in shared libs.
# This also dramatically improves .dll/.so load times & file size.
include(GenerateExportHeader)

option(BUILD_UNIT_TESTS "Whether to build units tests." ON)
option(BUILD_BENCHMARKS "Determines if modules should build any googlebenchmark targets they have" OFF)
option(BUILD_SHARED_LIBS "Determines if cmake targets should build dll/dylib/so or static libs" OFF)
option(OPTIMIZE_FOR_NATIVE "Build with -march=native or equivalent" OFF)
option(INCLUDE_NIGHTLY_TESTS "Whether to include longer tests in the unit test." OFF)
option(BUILD_WITH_CODE_COVERAGE "Whether to build with code coverage flags." OFF)

# Enable testing if needed
if (BUILD_UNIT_TESTS)
    enable_testing()
endif()

# Interface target the modules can depend on for private dependencies
# For example: Warning set being compiled against, compiler features required to compile the code, etc
add_library(modules_private_config INTERFACE)

# Interface target the modules can depend on for public dependencies
# For example: Compile flags being set, etc
add_library(modules_public_config INTERFACE)

target_compile_features(modules_private_config
        INTERFACE
        cxx_alias_templates
        cxx_auto_type
        cxx_constexpr
        cxx_decltype
        cxx_default_function_template_args
        cxx_defaulted_move_initializers
        cxx_delegating_constructors
        cxx_explicit_conversions
        cxx_final
        cxx_generic_lambdas
        cxx_lambdas
        cxx_lambda_init_captures
        cxx_noexcept
        cxx_nonstatic_member_init
        cxx_nullptr
        cxx_override
        cxx_range_for
        cxx_reference_qualified_functions
        cxx_return_type_deduction
        cxx_static_assert
        cxx_strong_enums
        cxx_trailing_return_types
        cxx_uniform_initialization
        cxx_user_literals
        cxx_variable_templates
        cxx_variadic_templates
        cxx_template_template_parameters
        )

target_compile_definitions(modules_public_config INTERFACE
        _USE_MATH_DEFINES # Used in various places in boost
        BOOST_SYSTEM_NO_DEPRECATED
        BOOST_PP_VARIADICS=1
        $<$<CONFIG:Release>:BOOST_UBLAS_NDEBUG>
        )

# BOOST_ALL_NO_LIB prevents VS from trying to automatically link Boost libraries
if(MSVC)
    target_compile_definitions(modules_public_config INTERFACE
            $<$<CXX_COMPILER_ID:MSVC>:BOOST_ALL_DYN_LINK>
            $<$<CXX_COMPILER_ID:MSVC>:BOOST_ALL_NO_LIB>
            )
endif()

# Define the appropriate things if using nightly tests
if (INCLUDE_NIGHTLY_TESTS)
    target_compile_definitions(modules_private_config INTERFACE
            NIGHTLY_TESTS
            )
endif(INCLUDE_NIGHTLY_TESTS)

if(MSVC)
    target_compile_options(modules_private_config INTERFACE
            # /Wall # Enable all warnings
            /WX # Treat warnings as errors
            # "/permissive-"
            /MP # Compile with multiple processes
            /Zm200 # Use 2x the normally alloted memory to hold precompiled headers
            /EHsc # Don't "catch" SEH exceptions and assume "extern C" functions won't throw
            /bigobj # Increase the number of sections that an object file can contain
			/wd4251 # We are exporting classes which internally hold classes that are not exported. The dangers of making a C++ DLL
			/wd4275 # Non-exported class used as base class of exported class... should investigate how this works
			/wd4503 # Full symbol name is longer than 4096 bytes and will get truncated. Can happen with boost debug, doesn't affect code correctness
            )

    target_compile_definitions(modules_public_config INTERFACE
            NOMINMAX # Prevent Microsoft headers from defining min/max
            _SCL_SECURE_NO_WARNINGS  # Disable warnings related to not using MS specific secure C Functions
            _CRT_SECURE_NO_WARNINGS  # Disable warnings related to not using MS specific secure C Functions
            WIN32_LEAN_AND_MEAN=1 # Tells windows.h not to include some stuff we don't need
            )

    # MSVC2017-specific. Different versions of MSVC2017 handle alignment requirements for large objects differently,
    # and this tells it whether you want the new or deprecated behaviour. Also, Boost 1.66 is too old for MSVC2017
    # and throws tons of warnings without this suppression
    if(MSVC_VERSION GREATER 1900)
        target_compile_definitions(modules_public_config INTERFACE
                _ENABLE_EXTENDED_ALIGNED_STORAGE
                BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE
                )
    endif()

else()
    target_compile_options(modules_private_config INTERFACE
            -Wall # All warnings
            -Werror # treat all warnings as errors
            -Wextra # reasonable and standard
            # -Wshadow # warn the user if a variable declaration shadows one from a parent context
            -Wnon-virtual-dtor # warn the user if a class with virtual functions has a non-virtual destructor. This helps catch hard to track down memory errors
            # -Wold-style-cast # warn for c-style casts
            # -Wcast-align # warn for potential performance problem casts
            # -Wunused # warn on anything being unused
            # -Woverloaded-virtual # warn if you overload (not override) a virtual function
            -Wpedantic # warn if non-standard C++ is used
            # -Wconversion # warn on type conversions that may lose data
            # -Wsign-conversion # warn on sign conversions
            # -Wnull-dereference # warn if a null dereference is detected
            -Wdouble-promotion # warn if float is implicit promoted to double
            # -Wformat=2 # warn on security issues around functions that format output (ie printf)
            -Wfatal-errors # Stop on first error occurred as opposed to trying to continue on to get more error messages
            -Wno-unused-local-typedefs # Allow unused local typedefs
            )

    target_compile_options(modules_public_config INTERFACE
            # -fno-omit-frame-pointer
            -ftemplate-backtrace-limit=0 # Disable the maximum limit to how many times a template function can recurse
            -ftemplate-depth=512 # Limit template recursion depth to 512 (conflicts with previous?)
            -pipe # Compiler and linker use pipes as opposed to temporary files to communicate
            )

    # Enable architecture-specific optimizations
    if(${OPTIMIZE_FOR_NATIVE})
        target_compile_options(modules_public_config INTERFACE
                -march=native
                )
    endif()

endif()

# Output diagnostics format for ninja builds
if ("${CMAKE_GENERATOR}" STREQUAL "Ninja")
    # Flag for clang/appleclang-based
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(modules_private_config INTERFACE
                -fcolor-diagnostics
                )
    # Flag for gcc
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_compile_options(modules_private_config INTERFACE
                -fdiagnostics-color=always
                )
    endif()
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(modules_private_config INTERFACE
            -Wno-error=maybe-uninitialized # Needed for boost optional // TODO: make this not so somehow
            # -Wmisleading-indentation # warn if identation implies blocks where blocks do not exist
            # -Wduplicated-cond # warn if if / else chain has duplicated conditions
            # -Wduplicated-branches # warn if if / else branches have duplicated code
            # -Wlogical-op # warn about logical operations being used where bitwise were probably wanted
            # -Wuseless-cast # warn if you perform a cast to the same type
            )

    # Used for building against static libstdcpp
    # Provided by Dan Bryant. Purpose is to build on older linux machine with updated toolset
    # CentOS 6 w/DevToolset7+ (through docker in this case) to have shared objects which only need
    # to dynamically load the C Runtime (not C++ runtime) as newer versions of linux don't break the CRT ABI.
    if(${BUILD_AGAINST_STATIC_LIBSTDCPP})
        target_compile_options(modules_public_config INTERFACE
                -Bsymbolic-functions
                -Wl
                --allow-multiple-definition
                -static-libstdc++
                )
    endif()

    # Used for building with code coverage, only supporting that on gcc at the moment
    if(${BUILD_WITH_CODE_COVERAGE})
        target_compile_options(modules_public_config INTERFACE
                --coverage
                )
        target_link_options(modules_public_config INTERFACE
                --coverage
                )
    endif()

endif()

add_subdirectory(Epidemic)
