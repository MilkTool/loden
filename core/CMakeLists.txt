FILE(GLOB LodenCore_HEADERS
	"${PROJECT_SOURCE_DIR}/include/Loden/*.hpp"
)

FILE(GLOB LodenCoreGUI_HEADERS
	"${PROJECT_SOURCE_DIR}/include/Loden/GUI/*.hpp"
)

FILE(GLOB LodenCoreImage_HEADERS
	"${PROJECT_SOURCE_DIR}/include/Loden/Image/*.hpp"
)

set(LodenCoreGUI_SRCS
	GUI/AgpuCanvas.cpp
	GUI/Button.cpp
	GUI/Canvas.cpp
	GUI/CanvasWidget.cpp
	GUI/ContainerWidget.cpp
	GUI/DockingLayout.cpp
	GUI/Font.cpp
	GUI/FontManager.cpp
	GUI/FreeTypeFont.cpp
	GUI/FreeTypeFont.hpp
	GUI/Label.cpp
	GUI/Layout.cpp
	GUI/LodenFont.cpp
	GUI/LodenFont.hpp
	GUI/Menu.cpp
	GUI/MenuBar.cpp
	GUI/MenuItem.cpp
	GUI/StatusBar.cpp
	GUI/SystemWindow.cpp
	GUI/TextInput.cpp
	GUI/Widget.cpp
	GUI/Window.cpp
)

set(LodenCoreImage_SRCS
	Image/PngImage.cpp
)

set(LodenCore_SRCS
	Application.cpp
	BinarySerializator.cpp
	Engine.cpp
	FileSystem.cpp
	JSON.cpp
	JSONSerializator.cpp
	Matrices.cpp
	Object.cpp
	Printing.cpp
	PipelineStateManager.cpp
	Structure.cpp
	Settings.cpp
	Texture.cpp
	TextureFormats.cpp
	TextureManager.cpp
)

source_group("Public Headers" FILES ${LodenCore_HEADERS})
source_group("Public Headers\\GUI" FILES ${LodenCoreGUI_HEADERS})
source_group("Public Headers\\Image" FILES ${LodenCoreImage_HEADERS})
source_group("Source Files\\GUI" FILES ${LodenCoreGUI_SRCS})
source_group("Source Files\\Image" FILES ${LodenCoreImage_SRCS})
source_group("Source Files" FILES ${LodenCore_SRCS})

add_definitions(-DBUILDING_LODEN_CORE)
add_library(LodenCore SHARED ${LodenCore_SRCS} ${LodenCoreGUI_SRCS} ${LodenCoreImage_SRCS} ${LodenCore_HEADERS} ${LodenCoreGUI_HEADERS} ${LodenCoreImage_HEADERS})
target_link_libraries(LodenCore ${Loden_DEP_LIBS} freetype libpng zlib)
set_target_properties(LodenCore PROPERTIES FOLDER "core")
