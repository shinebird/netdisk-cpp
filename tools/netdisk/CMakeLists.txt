find_package(Boost REQUIRED)

# find_package(LLVM REQUIRED)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

string(REGEX REPLACE "/include.*$" "" Boost_ROOT ${Boost_INCLUDE_DIRS})
set(Boost_LIBRARY_DIR "${Boost_ROOT}/lib" CACHE PATH "${Boost_ROOT}/lib")

set(netdisk-main_sources Main.cpp CommandLine.cpp)

if(MSVC)
    set(netdisk-main_sources ${netdisk-main_sources} ${CMAKE_SOURCE_DIR}/misc/CommonWindows.manifest)
endif()

add_executable(netdisk ${netdisk-main_sources})
target_include_directories(netdisk PRIVATE ${Boost_INCLUDE_DIRS})
target_link_directories(netdisk PRIVATE ${Boost_LIBRARY_DIR})
target_include_directories(netdisk PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(netdisk PRIVATE Netdisk::Utils)
target_link_libraries(netdisk PRIVATE Netdisk::Core)
target_link_libraries(netdisk PRIVATE Netdisk::Controller)
target_link_libraries(netdisk PRIVATE Netdisk::Service)
target_link_libraries(netdisk PRIVATE Netdisk::Repository)
target_compile_definitions(netdisk PRIVATE NETDISK_REPOSITORY_DATABASE_${NETDISK_REPOSITORY_DATABASE})

install(TARGETS netdisk DESTINATION bin)
install(FILES $<TARGET_PDB_FILE:netdisk> DESTINATION bin OPTIONAL)