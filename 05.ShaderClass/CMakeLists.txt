# 项目特定的逻辑。
#

# 将源代码添加到此项目的可执行文件。
add_executable (05.ShaderClass
    main.cpp

    ${GLAD_SRC_PATH}
 )

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET 05.ShaderClass  PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。
target_include_directories(05.ShaderClass
    PRIVATE ${MYLIB_INCLUDE_DIR}
)
target_link_libraries(05.ShaderClass
    ${MYLIB_LIBRARY}
)

set(RESOURCES_SRC ${CMAKE_CURRENT_SOURCE_DIR}/shader)
set(RESOURCES_DST ${CMAKE_CURRENT_BINARY_DIR}/shader)

add_custom_command(
    TARGET 05.ShaderClass POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${RESOURCES_SRC}"
            "${RESOURCES_DST}"
    COMMENT "Deploying resources to output directory..."
    VERBATIM
)