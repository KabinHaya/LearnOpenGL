# 项目特定的逻辑。
#

# 将源代码添加到此项目的可执行文件。
add_executable (03_HelloTriangleExercise
    main_Exercise3.cpp

    ${GLAD_SRC_PATH}
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET 03_HelloTriangleExercise  PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。
target_include_directories(03_HelloTriangleExercise
    PRIVATE ${MYLIB_INCLUDE_DIR}
)
target_link_libraries(03_HelloTriangleExercise
    ${MYLIB_LIBRARY}
)