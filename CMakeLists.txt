
cmake_minimum_required(VERSION 3.13)

project("netdisk-cpp" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_C_STANDARD 23)

add_subdirectory(lib)
add_subdirectory(tools)

# find_package(Boost REQUIRED)
# find_package(OpenSSL COMPONENTS Crypto SSL REQUIRED)

# # find_package(LLVM REQUIRED)
# list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# string(REGEX REPLACE "/include.*$" "" Boost_ROOT ${Boost_INCLUDE_DIRS})
# set(Boost_LIBRARY_DIR "${Boost_ROOT}/lib" CACHE PATH "${Boost_ROOT}/lib")

# add_executable(router router.cpp impl/matches.cpp detail/impl/router.cpp)
# target_include_directories(router PRIVATE ${Boost_INCLUDE_DIRS})
# target_link_directories(router PRIVATE ${Boost_LIBRARY_DIR})
# target_link_libraries(router PRIVATE OpenSSL::SSL OpenSSL::Crypto)

# install(TARGETS router DESTINATION bin)
# install(FILES $<TARGET_PDB_FILE:router> DESTINATION bin OPTIONAL)
