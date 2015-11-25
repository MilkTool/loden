FILE(GLOB Common_HEADERS
	"${PROJECT_SOURCE_DIR}/include/Loden/*.hpp"
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
	Printing.cpp
	PipelineBuilder.cpp
	PipelineStateManager.cpp
)

add_definitions(-DBUILDING_LODEN_CORE)
add_library(LodenCore SHARED ${LodenCore_SRCS} ${LodenCoreGUI_SRCS} ${Common_HEADERS})
target_link_libraries(LodenCore ${Loden_DEP_LIBS})
