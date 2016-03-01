set(LevelEditor_SOURCES
    LevelEditor.cpp
    LevelEditor.hpp
    LevelView.cpp
    LevelView.hpp
    MainFrame.cpp
    MainFrame.hpp
)

add_executable(LevelEditor WIN32 ${LevelEditor_SOURCES})
target_link_libraries(LevelEditor LodenCore ${LodenProgram_DEP_LIBS})
set_target_properties(LevelEditor PROPERTIES FOLDER "editor")
