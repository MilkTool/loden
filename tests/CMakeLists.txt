set(Test_Sources
    Color.cpp
    Math.cpp

    TestMain.cpp
)

add_executable(Tests ${Test_Sources})
target_link_libraries(Tests LodenCore ${Loden_DEP_LIBS} UnitTest++)
