# 项目特定的逻辑。
#

# 将源代码添加到此项目的可执行文件。
add_executable (04_ShaderGLSL
    main2.cpp

    ${GLAD_SRC_PATH}
 )

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET 04_ShaderGLSL  PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。
target_include_directories(04_ShaderGLSL
    PRIVATE ${MYLIB_INCLUDE_DIR}
)
target_link_libraries(04_ShaderGLSL
    ${MYLIB_LIBRARY}
)