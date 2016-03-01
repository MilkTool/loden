# Build UnitTest++
add_subdirectory(unittest-cpp)
set_target_properties(TestUnitTest++ PROPERTIES FOLDER "thirdparty")
set_target_properties(UnitTest++ PROPERTIES FOLDER "thirdparty")

# Build zlib
add_subdirectory(zlib)

# Build libpng
add_subdirectory(libpng)

# Build the freetype library
if(WIN32)
    set(BUILD_SHARED_LIBS False)
else()
    set(BUILD_SHARED_LIBS True)
endif()
add_subdirectory(freetype)
set_target_properties(freetype PROPERTIES FOLDER "thirdparty")
