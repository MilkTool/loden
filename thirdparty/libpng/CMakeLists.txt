# OUR SOURCES

set(libpng_public_hdrs
  png.h
  pngconf.h
  pnglibconf.h
)
set(libpng_sources
  ${libpng_public_hdrs}
  pngdebug.h
  pnginfo.h
  pngpriv.h
  pngstruct.h
  png.c
  pngerror.c
  pngget.c
  pngmem.c
  pngpread.c
  pngread.c
  pngrio.c
  pngrtran.c
  pngrutil.c
  pngset.c
  pngtrans.c
  pngwio.c
  pngwrite.c
  pngwtran.c
  pngwutil.c
)

add_library(libpng SHARED ${libpng_sources})
target_link_libraries(libpng zlib)

set_target_properties(libpng PROPERTIES FOLDER "thirdparty")
