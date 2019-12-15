cmake_minimum_required(VERSION 3.0)
project(MetisGeomExp)
set(CMAKE_CXX_STANDARD 11)
message(${CMAKE_VERSION})
include_directories(.)
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})
include_directories(/home/mas/17/cdsbhat/parmetis-4.0.3/include)
include_directories(/home/mas/17/cdsbhat/metis-5.1.0/include)
link_directories(/home/mas/17/cdsbhat/metis-5.1.0/lib)
link_directories(/home/mas/17/cdsbhat/parmetis-4.0.3/lib)
add_executable(MetisGeomExp
        adjtocsr.cpp
        CMakeLists.txt
        main.cpp)
target_link_libraries(MetisGeomExp ${MPI_LIBRARIES} libparmetis.a libmetis.a)
if(MPI_COMPILE_FLAGS)
    set_target_properties(MetisGeomExp PROPERTIES
            COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
    set_target_properties(MetisGeomExp PROPERTIES
            LINK_FLAGS "${MPI_LINK_FLAGS}")
endif()


