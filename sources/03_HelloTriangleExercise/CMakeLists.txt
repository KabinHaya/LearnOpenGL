# 将源代码添加到此项目的可执行文件。
set(PROJECT_NAME 03_HelloTriangleExercise)

add_executable (${PROJECT_NAME}
    # main_Exercise2.cpp
    main_Exercise3.cpp
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${MYLIB_INCLUDE_DIR}

)
target_link_libraries(${PROJECT_NAME}
    ${MYLIB_LIBRARY}
    glad
)