# Build the freetype library
if(WIN32)
    set(BUILD_SHARED_LIBS False)
else()
    set(BUILD_SHARED_LIBS True)
endif()
add_subdirectory(freetype)
