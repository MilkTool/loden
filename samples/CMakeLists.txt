add_executable(Sample1 WIN32 Sample1.cpp)
target_link_libraries(Sample1 LodenCore ${LodenProgram_DEP_LIBS})

add_executable(Sample2 WIN32 Sample2.cpp)
target_link_libraries(Sample2 LodenCore ${LodenProgram_DEP_LIBS})

add_executable(Sample3 WIN32 Sample3.cpp)
target_link_libraries(Sample3 LodenCore ${LodenProgram_DEP_LIBS})
