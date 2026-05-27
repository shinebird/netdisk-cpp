find_package(Boost REQUIRED)
find_package(msft_proxy4 REQUIRED)

# find_package(LLVM REQUIRED)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

string(REGEX REPLACE "/include.*$" "" Boost_ROOT ${Boost_INCLUDE_DIRS})
set(Boost_LIBRARY_DIR "${Boost_ROOT}/lib" CACHE PATH "${Boost_ROOT}/lib")

add_library(NetdiskRepoistoryGeneric STATIC UserRepository.cpp
    User.cpp)
add_library(Netdisk::Repoistory::Generic ALIAS NetdiskRepoistoryGeneric)
target_include_directories(NetdiskRepoistoryGeneric PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(NetdiskRepoistoryGeneric PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(NetdiskRepoistoryGeneric PUBLIC ${CMAKE_BINARY_DIR})
target_link_directories(NetdiskRepoistoryGeneric PUBLIC ${Boost_LIBRARY_DIR})
target_link_libraries(NetdiskRepoistoryGeneric PUBLIC msft_proxy4::proxy)
target_link_libraries(NetdiskRepoistoryGeneric PUBLIC Netdisk::Utils)