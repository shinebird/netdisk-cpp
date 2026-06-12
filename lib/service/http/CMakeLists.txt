find_package(Boost REQUIRED)
find_package(msft_proxy4 REQUIRED)

# find_package(LLVM REQUIRED)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

string(REGEX REPLACE "/include.*$" "" Boost_ROOT ${Boost_INCLUDE_DIRS})
set(Boost_LIBRARY_DIR "${Boost_ROOT}/lib" CACHE PATH "${Boost_ROOT}/lib")

add_library(NetdiskServiceHTTP STATIC FileService.cpp ${generated_sources})
add_library(Netdisk::Service::HTTP ALIAS NetdiskServiceHTTP)
target_include_directories(NetdiskServiceHTTP PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(NetdiskServiceHTTP PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(NetdiskServiceHTTP PUBLIC ${CMAKE_BINARY_DIR})
target_link_directories(NetdiskServiceHTTP PUBLIC ${Boost_LIBRARY_DIR})
target_link_libraries(NetdiskServiceHTTP PUBLIC msft_proxy4::proxy)
target_link_libraries(NetdiskServiceHTTP PUBLIC Netdisk::Utils)
target_link_libraries(NetdiskServiceHTTP PUBLIC Netdisk::Data)