find_package(Boost REQUIRED)

# find_package(LLVM REQUIRED)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

string(REGEX REPLACE "/include.*$" "" Boost_ROOT ${Boost_INCLUDE_DIRS})
set(Boost_LIBRARY_DIR "${Boost_ROOT}/lib" CACHE PATH "${Boost_ROOT}/lib")

set(gen-mime-types_sources "gen-mime-types.cpp")

if(MSVC)
    set(gen-mime-types_sources ${gen-mime-types_sources} ${CMAKE_SOURCE_DIR}/misc/CommonWindows.manifest)
endif()

add_executable(gen-mime-types ${gen-mime-types_sources})
target_include_directories(gen-mime-types PRIVATE ${Boost_INCLUDE_DIRS})
target_link_directories(gen-mime-types PRIVATE ${Boost_LIBRARY_DIR})
target_include_directories(gen-mime-types PUBLIC ${CMAKE_SOURCE_DIR}/include)

install(TARGETS gen-mime-types DESTINATION bin)
install(FILES $<TARGET_PDB_FILE:gen-mime-types> DESTINATION bin OPTIONAL)