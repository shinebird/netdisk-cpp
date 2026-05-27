find_package(Boost REQUIRED)
find_package(msft_proxy4 REQUIRED)
find_package(TBB REQUIRED)

# find_package(LLVM REQUIRED)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

string(REGEX REPLACE "/include.*$" "" Boost_ROOT ${Boost_INCLUDE_DIRS})
set(Boost_LIBRARY_DIR "${Boost_ROOT}/lib" CACHE PATH "${Boost_ROOT}/lib")

add_library(NetdiskControllerGeneric STATIC security/UserAuthenticator.cpp)
add_library(Netdisk::Controller::Generic ALIAS NetdiskControllerGeneric)
target_include_directories(NetdiskControllerGeneric PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(NetdiskControllerGeneric PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(NetdiskControllerGeneric PUBLIC ${CMAKE_BINARY_DIR})
target_link_directories(NetdiskControllerGeneric PUBLIC ${Boost_LIBRARY_DIR})
target_link_libraries(NetdiskControllerGeneric PUBLIC msft_proxy4::proxy)
target_link_libraries(NetdiskControllerGeneric PUBLIC TBB::tbb)
target_link_libraries(NetdiskControllerGeneric PUBLIC Netdisk::Utils)