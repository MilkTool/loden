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
add_library(LodenCore SHARED ${LodenCore_SRCS} ${LodenCoreGUI_SRCS})
target_link_libraries(LodenCore ${Loden_DEP_LIBS})
