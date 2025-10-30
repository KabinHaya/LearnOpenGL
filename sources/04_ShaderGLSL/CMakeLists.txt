# 将源代码添加到此项目的可执行文件。
set(PROJECT_NAME 04_ShaderGLSL)

add_executable (${PROJECT_NAME}
    main1.cpp
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${MYLIB_INCLUDE_DIR}
)
target_link_libraries(${PROJECT_NAME}
    ${MYLIB_LIBRARY}
    glad
)