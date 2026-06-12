add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/netdisk-cpp/embed_data/EmbedData.cpp" "${CMAKE_BINARY_DIR}/netdisk-cpp/embed_data/EmbedData.hpp"
    COMMAND $<TARGET_FILE:gen-embed> "--base-path" "${CMAKE_SOURCE_DIR}/data" "-o" "${CMAKE_BINARY_DIR}/netdisk-cpp/embed_data/EmbedData.cpp"
    DEPENDS gen-embed
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating cpp embed file")
add_library(NetdiskEmbedData STATIC "${CMAKE_BINARY_DIR}/netdisk-cpp/embed_data/EmbedData.cpp")
add_library(Netdisk::EmbedData ALIAS NetdiskEmbedData)
add_custom_target(NetdiskEmbedDataSources DEPENDS "${CMAKE_BINARY_DIR}/netdisk-cpp/embed_data/EmbedData.cpp" "${CMAKE_BINARY_DIR}/netdisk-cpp/embed_data/EmbedData.hpp"
)

add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/netdisk-cpp/mime_types/MimeTypes.cpp" "${CMAKE_BINARY_DIR}/netdisk-cpp/mime_types/MimeTypes.hpp"
    COMMAND $<TARGET_FILE:gen-mime-types> "${CMAKE_SOURCE_DIR}/lib/utils/MimeTypes.csv" "-o" "${CMAKE_BINARY_DIR}/netdisk-cpp/mime_types/MimeTypes.cpp"
    DEPENDS gen-mime-types
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating cpp mime-types file")
add_library(NetdiskMimeTypes STATIC "${CMAKE_BINARY_DIR}/netdisk-cpp/mime_types/MimeTypes.cpp")
add_library(Netdisk::MimeTypes ALIAS NetdiskMimeTypes)
target_include_directories(NetdiskMimeTypes PRIVATE "${CMAKE_BINARY_DIR}/mime_types")
add_custom_target(NetdiskMimeTypesSources DEPENDS "${CMAKE_BINARY_DIR}/netdisk-cpp/mime_types/MimeTypes.cpp" "${CMAKE_BINARY_DIR}/netdisk-cpp/mime_types/MimeTypes.hpp"
)

find_package(Boost REQUIRED)
find_package(OpenSSL COMPONENTS Crypto SSL REQUIRED)
find_package(spdlog REQUIRED)
find_package(jwt-cpp REQUIRED)

# find_package(LLVM REQUIRED)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

string(REGEX REPLACE "/include.*$" "" Boost_ROOT ${Boost_INCLUDE_DIRS})
set(Boost_LIBRARY_DIR "${Boost_ROOT}/lib" CACHE PATH "${Boost_ROOT}/lib")

add_library(NetdiskUtils STATIC ssl/SSL.cpp url/MatchesImpl.cpp url/RouterImpl.cpp log/Logger.cpp
    exception/SignalHandler.cpp
    log/RelativePathFormatter.cpp
    jwt/JWT.cpp
    filesystem/ListRoots.cpp
    filesystem/FileQuery.cpp)
add_library(Netdisk::Utils ALIAS NetdiskUtils)
target_include_directories(NetdiskUtils PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(NetdiskUtils PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_directories(NetdiskUtils PUBLIC ${Boost_LIBRARY_DIR})
target_link_libraries(NetdiskUtils PUBLIC OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(NetdiskUtils PUBLIC spdlog::spdlog)
target_link_libraries(NetdiskUtils PUBLIC Netdisk::EmbedData)
target_link_libraries(NetdiskUtils PUBLIC Netdisk::MimeTypes)
target_link_libraries(NetdiskUtils PUBLIC jwt-cpp::jwt-cpp)
target_compile_definitions(NetdiskUtils PUBLIC SPDLOG_USE_STD_FORMAT)
target_compile_definitions(NetdiskUtils PUBLIC SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG)
target_compile_definitions(NetdiskUtils PRIVATE NETDISK_BASE_DIR="${CMAKE_SOURCE_DIR}")

if(MSVC)
    target_compile_definitions(NetdiskUtils PUBLIC BOOST_STACKTRACE_USE_WINDBG_CACHED)
    target_link_libraries(NetdiskUtils PUBLIC dbgeng)
endif()