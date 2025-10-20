# 项目特定的逻辑。
#

# 将源代码添加到此项目的可执行文件。
add_executable (04.ShaderGLSL
    main2.cpp

    ${GLAD_SRC_PATH}
 )

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET 04.ShaderGLSL  PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。
target_include_directories(04.ShaderGLSL
    PRIVATE ${MYLIB_INCLUDE_DIR}
)
target_link_libraries(04.ShaderGLSL
    ${MYLIB_LIBRARY}
)