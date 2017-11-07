# Use static anttweakbar.
add_definitions(-DTW_NO_LIB_PRAGMA -DTW_STATIC)

# Use static GLEW.
add_definitions(-DGLEW_STATIC)

# Use static freeglut.
add_definitions(-DFREEGLUT_LIB_PRAGMAS=0 -DFREEGLUT_STATIC)

file(TO_NATIVE_PATH "${PROJECT_SOURCE_DIR}/resources/" GLVIZ_RESOURCES_DIR)
configure_file(config.hpp.in "${CMAKE_CURRENT_BINARY_DIR}/config.hpp")

add_executable(example
    main.cpp
    CompactNSearch.cpp
    CompactNSearch.h   
	Field.h
	Field.inl
	NeighborhoodSearch.cpp
	NeighborhoodSearch.h
	ParticleManager.cpp
	ParticleManager.h
	SimProgramm.cpp
	SimProgramm.h
	SimSystem.h
	SimSystem.cpp
	SimViz.cpp
	SimViz.h
	Slot.h
	Slot.inl
	z_sort_table.h
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${glviz_INCLUDE_DIRS}
)

add_dependencies(example
    glviz
)

target_link_libraries(example
    ${glviz_LIBRARIES}
)
