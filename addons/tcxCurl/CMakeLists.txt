# =============================================================================
# tcxCurl - HTTPS Client Addon (libcurl)
# =============================================================================

cmake_minimum_required(VERSION 3.20)
message(STATUS "[tcxCurl] Loading addon...")

add_library(tcxCurl INTERFACE)
target_include_directories(tcxCurl INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(EMSCRIPTEN)
    # WASM: use Emscripten Fetch API
    target_compile_definitions(tcxCurl INTERFACE TCX_HTTP_EMSCRIPTEN=1)
    target_link_options(tcxCurl INTERFACE -sFETCH=1)
else()
    # Native: use libcurl (find system install, or fetch from source)
    find_package(CURL QUIET)
    if(NOT CURL_FOUND)
        message(STATUS "[tcxCurl] System libcurl not found, fetching from source...")
        include(FetchContent)
        set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
        set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
        set(CURL_USE_OPENSSL OFF CACHE BOOL "" FORCE)
        if(WIN32)
            # Windows: use Schannel (native TLS)
            # If Schannel causes renegotiation issues, try CURL_USE_OPENSSL instead
            set(CURL_USE_SCHANNEL ON CACHE BOOL "" FORCE)
        endif()
        set(CURL_ZLIB OFF CACHE BOOL "" FORCE)
        set(CURL_USE_LIBPSL OFF CACHE BOOL "" FORCE)
        set(HTTP_ONLY ON CACHE BOOL "" FORCE)
        FetchContent_Declare(
            curl
            URL https://github.com/curl/curl/releases/download/curl-8_12_1/curl-8.12.1.tar.xz
            URL_HASH SHA256=0341f1ed97a26c811abaebd37d62b833956792b7607ea3f15d001613c76de202
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        FetchContent_MakeAvailable(curl)
        # Create CURL::libcurl alias if not defined
        if(NOT TARGET CURL::libcurl)
            add_library(CURL::libcurl ALIAS libcurl_static)
        endif()
    endif()
    target_compile_definitions(tcxCurl INTERFACE TCX_HTTP_CURL=1)
    target_link_libraries(tcxCurl INTERFACE CURL::libcurl)
endif()

message(STATUS "[tcxCurl] Addon loaded")
