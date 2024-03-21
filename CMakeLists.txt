cmake_minimum_required(VERSION 3.5) # setting this is required

project(vroads)            # this sets the project name
#set debug flags if debug build
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -ggdb -g3 -O0 -Wall -Wextra -Wfloat-equal -fverbose-asm  --coverage -fsanitize=address -Wno-missing-braces")
    add_definitions(-DDEBUG)
    #elseif (CMAKE_BUILD_TYPE MATCHES Release)
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -O3 -fsanitize=address")
    add_definitions(-DRelease)
endif()


include(CTest)
###############################################################################
## file globbing ##############################################################
###############################################################################

# These instructions search the directory tree when cmake is
# invoked and put all files that match the pattern in the variables 
# `sources` and `data`.
file(GLOB_RECURSE data resources/*)

###############################################################################
## target definitions #########################################################
###############################################################################

add_executable(${PROJECT_NAME} 
    src/common/0_GlobalIncludes.c
    src/main/overpassQueries.c
    src/main/fileOperations.c
    src/glad/glad.c
    src/glad/khrplatform.h 
    src/minimap/minimap.c
    src/main/main.c
    )

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 23)
add_definitions(-D_GNU_SOURCE)

# This allows to include files relative to the root of the src directory with a <> pair
target_include_directories(${PROJECT_NAME} PUBLIC
    src/main 
    src/glad
    src/minimap
    src/common)

# This copies all resource files in the build directory.
# We need this, because we want to work with paths relative to the executable.
file(COPY ${data} DESTINATION resources)
###############################################################################
## git stuff    ###############################################################
###############################################################################
#if USESHAREDLIBS is set, use shared libraries
find_package(Git QUIET)
if (GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git") 
    option(GIT_SUBMODULE "Check submodules during build" ON)
    find_library( EASYCRINGELIB easycringelib )
    if (NOT EASYCRINGELIB)
        message(WARNING "easycringelib not found hence the submodule will be used")
        target_include_directories(${PROJECT_NAME}
            PUBLIC extern/easycringelib/src
            )
    endif()
    execute_process(COMMAND git submodule update --init
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if (NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()


else()
    message("Git not found")
endif()



###############################################################################
## dependencies ###############################################################
###############################################################################


add_subdirectory(extern/logc)
add_subdirectory(extern/easycringelib)

# This defines the variables Boost_LIBRARIES that containts all library names
# that we need to link into the program.


target_link_directories(${PROJECT_NAME} 
    PUBLIC extern/logc
    PUBLIC src/common
    )
target_link_libraries(${PROJECT_NAME} 
    PUBLIC glfw
    PUBLIC logc
    PUBLIC curl
    PUBLIC json-c
    PUBLIC raylib
    PUBLIC m
    )

###############################################################################
## testing ####################################################################
###############################################################################
#Who needs testing?
enable_testing()
add_executable(testMiniDataStructs
    src/test/testMiniDataStructs.c
    src/common/0_GlobalIncludes.c
    src/common/1_CringeMiniDataStructs.c
    )
set_property(TARGET testMiniDataStructs PROPERTY C_STANDARD 23)
target_include_directories(
    testMiniDataStructs
    PUBLIC src/glad
    PUBLIC extern/logc/src
    PUBLIC extern/easycringelib/src
    PUBLIC src/common
    )
target_link_libraries(testMiniDataStructs 
    PUBLIC glfw
    PUBLIC logc
    PUBLIC curl
    PUBLIC json-c
    PUBLIC raylib
    PUBLIC m
    )

add_test(NAME testMiniDataStructs COMMAND testMiniDataStructs)


###############################################################################
## packaging ##################################################################
###############################################################################
#Not thanks
install( TARGETS ${PROJECT_NAME} DESTINATION bin)
