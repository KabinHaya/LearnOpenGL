# CMakeList.txt: 01.HelloWindow 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#

# 将源代码添加到此项目的可执行文件。
set(PROJECT_NAME 02_HelloTriangle)

add_executable (${PROJECT_NAME}
    main.cpp

    ${GLAD_SRC}
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。
target_include_directories(${PROJECT_NAME}
    PRIVATE ${MYLIB_INCLUDE_DIR}
)
target_link_libraries(${PROJECT_NAME}
    ${MYLIB_LIBRARY}
)