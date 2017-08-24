cmake_minimum_required(VERSION 3.4.3)

# list of felix software dependencies
set(felix_deps FlxCard drivers_rcc regmap)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")

# Find all the packages, there should be a better way -- right now there is no flexibility at all
## find RCDAQ
if(EXISTS $ENV{RCDAQ_ROOT})
    message("-- Detecting RCDAQ installation at $ENV{RCDAQ_ROOT}.")
    include_directories($ENV{RCDAQ_ROOT}/install/include)
    link_directories($ENV{RCDAQ_ROOT}/install/lib)
else()
    message(FATAL_ERROR "-- Cannot find RCDAQ.")
endif()

## find felix related software
if(EXISTS $ENV{FELIX_ROOT})
    message("-- Detecting FELIX installation at $ENV{FELIX_ROOT}.")
    foreach(felix_dep ${felix_deps})
        string(TOLOWER ${felix_dep} dep_loc)
        include_directories($ENV{FELIX_ROOT}/${dep_loc})
        link_directories($ENV{FELIX_ROOT}/$ENV{FELIX_ARCH}/${dep_loc})
    endforeach(felix_dep)
else()
    message(FATAL_ERROR "-- Cannot find FELIX.")
endif()

## find BOOST
find_package(Boost)
if(Boost_FOUND)
    message("-- Detecting BOOST headers at ${Boost_INCLUDE_DIRS}")
    include_directories(${Boost_INCLUDE_DIRS})
elseif(EXISTS $ENV{BOOST_ROOT})
    message("-- Detecting BOOST headers at $ENV{BOOST_ROOT}/include")
    include_directories(${BOOST}/include)
else()
    message(FATAL_ERROR "-- Cannot find Boost.")
endif()

include_directories(${PROJECT_SOURCE_DIR})
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cc)

add_library(rcdaqplugin_felix SHARED ${sources})
add_executable(test_felix ${PROJECT_SOURCE_DIR}/test/test_felix.cc ${sources})

# link RCDAQ libraries
target_link_libraries(rcdaqplugin_felix rcdaq rcdaqutils)
target_link_libraries(test_felix        rcdaq rcdaqutils)

# link FELIX libraries
foreach(felix_dep ${felix_deps})
    target_link_libraries(rcdaqplugin_felix ${felix_dep})
    target_link_libraries(test_felix        ${felix_dep})
endforeach(felix_dep)
install(TARGETS rcdaqplugin_felix DESTINATION "$ENV{RCDAQ_ROOT}/install/lib")
