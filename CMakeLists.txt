project(comath)
set(CMAKE_CXX_FLAGS "-std=c++11")
cmake_minimum_required(VERSION 2.8)
aux_source_directory(./ SRC_LIST)
set(INCLUDES ./comath.h ./vmath.h)
find_package(OpenMP REQUIRED)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS} -fopenmp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -mfpmath=sse")
add_executable(${PROJECT_NAME} ${SRC_LIST})

install(FILES ${INCLUDES}  DESTINATION "/usr/local/include/vecmath")
install(TARGETS ${PROJECT_NAME}  DESTINATION "/usr/local/bin/")


# Setup DEBIAN control files
set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE 1)
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "3")
set(CPACK_PACKAGE_VERSION_PATCH "4")
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_DESCRIPTION "Vector math")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Red Dec <net.dev@mail.ru>")

include(CPack)

# Build .DEB package
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                  COMMAND make package)
