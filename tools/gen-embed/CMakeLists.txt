find_package(Boost REQUIRED)

# find_package(LLVM REQUIRED)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

string(REGEX REPLACE "/include.*$" "" Boost_ROOT ${Boost_INCLUDE_DIRS})
set(Boost_LIBRARY_DIR "${Boost_ROOT}/lib" CACHE PATH "${Boost_ROOT}/lib")

set(gen-embed_sources "gen-embed.cpp")
if(MSVC)
    set(gen-embed_sources ${gen-embed_sources} ${CMAKE_SOURCE_DIR}/misc/CommonWindows.manifest)
endif()

add_executable(gen-embed ${gen-embed_sources})
target_include_directories(gen-embed PRIVATE ${Boost_INCLUDE_DIRS})
target_link_directories(gen-embed PRIVATE ${Boost_LIBRARY_DIR})

install(TARGETS gen-embed DESTINATION bin)
install(FILES $<TARGET_PDB_FILE:gen-embed> DESTINATION bin OPTIONAL)