cmake_minimum_required(VERSION ${MODULES_MIN_CMAKE_VERSION} FATAL_ERROR)

# Enable policies if applicable
foreach (pol IN LISTS MODULES_CMP_ENABLES)
   if (POLICY ${pol})
      cmake_policy(SET ${pol} NEW)
   endif ()
endforeach ()

# Create the library target
add_library(Epidemic ${ModuleLibType}

        # Listing of all headers
        include/Epidemic/Agent.h
        include/Epidemic/Building.h
        include/Epidemic/Community.h
        include/Epidemic/Distributions.h
        include/Epidemic/World.h
        include/Epidemic/XmlReader.h

        # Listing of all sources
        src/Agent.cpp
        src/Building.cpp
        src/Community.cpp
        src/Distributions.cpp
        src/World.cpp
        src/XmlReader.cpp
        )

# Defining headers that this library uses
target_include_directories(Epidemic
      PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>
      )

# Create an export header so that we can export our interface when creating a shared library
generate_export_header(Epidemic EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/exports/epidemic_export.h)

#find_package(Boost ${MODULES_BOOST_VERSION} REQUIRED QUIET)

# Stating dependancies
target_link_libraries(Epidemic
      PUBLIC
      modules_public_config
      ${CONAN_LIBS}

      PRIVATE
      modules_private_config
      )

# Need to include the export directory for "epidemic_export.h"
target_include_directories(Epidemic
      PUBLIC
      ${CMAKE_BINARY_DIR}/exports
      )

# Add unit tests
if (BUILD_UNIT_TESTS)

   find_package(GTest ${MODULES_GTEST_VERSION} REQUIRED QUIET)

endif ()
