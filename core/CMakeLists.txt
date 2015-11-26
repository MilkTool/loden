FILE(GLOB LodenCore_HEADERS
	"${PROJECT_SOURCE_DIR}/include/Loden/*.hpp"
)

FILE(GLOB LodenCoreGUI_HEADERS
	"${PROJECT_SOURCE_DIR}/include/Loden/GUI/*.hpp"
)

set(LodenCoreGUI_SRCS
	GUI/AgpuCanvas.cpp
	GUI/Button.cpp
	GUI/Canvas.cpp
	GUI/ContainerWidget.cpp
	GUI/SystemWindow.cpp
	GUI/Widget.cpp
	GUI/Window.cpp
)

set(LodenCore_SRCS
	Application.cpp
	FileSystem.cpp
	JSON.cpp
	Matrices.cpp
	Printing.cpp
	PipelineStateManager.cpp
)

source_group("Public Headers" FILES ${LodenCore_HEADERS})
source_group("Public Headers\\GUI" FILES ${LodenCoreGUI_HEADERS})


add_definitions(-DBUILDING_LODEN_CORE)
add_library(LodenCore SHARED ${LodenCore_SRCS} ${LodenCoreGUI_SRCS} ${LodenCore_HEADERS} ${LodenCoreGUI_HEADERS})
target_link_libraries(LodenCore ${Loden_DEP_LIBS})
