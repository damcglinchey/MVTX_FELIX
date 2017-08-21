cmake_minimum_required(VERSION 3.4.3)

# Find all the packages, there should be a better way -- right now there is no flexibility at all
## find RCDAQ
if(EXISTS $ENV{RCDAQ_ROOT})
    message("-- Detecting RCDAQ installation at $ENV{RCDAQ_ROOT}.")
    include_directories($ENV{RCDAQ_ROOT}/install/include)
else()
    message(FATAL_ERROR "-- Cannot find RCDAQ.")
endif()

## find felix related software
if(EXISTS $ENV{FELIX_ROOT})
    message("-- Detecting FELIX installation at $ENV{FELIX_ROOT}.")
    include_directories($ENV{FELIX_ROOT}/flxcard)
    include_directories($ENV{FELIX_ROOT}/drivers_rcc)
    include_directories($ENV{FELIX_ROOT}/regmap)

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

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")

include_directories(${PROJECT_SOURCE_DIR})
file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cc)

add_library(rcdaqplugin_felix SHARED ${sources})
install(TARGETS rcdaqplugin_felix DESTINATION "$ENV{RCDAQ_ROOT}/install/lib")
