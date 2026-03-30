### 更新后的日志

#### 更新内容

- 使用`conan`包管理器重置了该项目，使得项目更易上手，`conan`使用方式请去B站上找
- 由于项目中使用的是`glfw`和`opengl`，而`conan`没有提供`glfw+opengl`的后端选项，因此在第三方库中添加了该`imgui`版本的后端文件
- 我没测试过是否所有文件都能运行，目前在我的测试中，都能通过编译，且测试的几个都能运行，如果有问题，在**issue**中反馈

#### 使用方式

这里我只用粗略的方式进行参考，详细的`conan`使用步骤还是去搜索或问AI

1. 安装`conan`，添加配置文件，以我的为例，在Windows平台，使用gcc15，C++版本为20，Debug模式

   ```plaintext
   [settings]
   os=Windows
   arch=x86_64
   compiler=gcc
   compiler.cppstd=20
   compiler.version=15
   compiler.libcxx=libstdc++11
   build_type=Debug
   ```

   如果要使用MSVC，俺么请注意一下这的配置文件和这里差距很大

2. 在项目的顶级目录下使用`conan`安装外部库，并运行下面指令

   ```bash
   conan install . --build=missing
   ```

   如果运行较慢则需要挂梯子（这没办法）

3. 如果使用的是VSCode，那么重启VSCode，在CMake Tools的配置中选择**conan-debug**配置

4. 点击底部栏的生成按钮，或者在CMake Tools的调试一栏中选择需要生成的那一节，然后点击底部栏的启动按钮





### 环境

- ==GCC 15.2.0 + CMake==

| 编译环境 | 版本   |
| -------- | ------ |
| GCC      | 15.2.0 |
| CMake    | 4.2.0  |

| 外部库 | 版本    |
| ------ | ------- |
| glfw   | 3.4     |
| assimp | 6.0.2   |
| glm    | 0.9.9.8 |
| glad   | 0.1.36  |
| imgui  | 1.92.6  |

### 目录结构

- `assets`：存放资源的目录，与B站上的教程一致
- `src`：根据章节和小节写下的代码，问题可能是前半程未确定好自己的目录结构，会有很多有问题，如果遇到了麻烦，可以及时反馈
- `third_party`：第三方库的头文件、静态库、动态库、源文件
  - `include`：头文件
  - `imgui_backend`：`imgui`的后端文件

### 注意事项

1. 资源路径和着色器路径是让CMake传递对应宏，来进行字符串拼接的。但前面一开始没有这么做，一开始是直接将资源复制到exe所在目录来实现的
2. 有些代码可能会有问题，因为文件结构修改过很多次，有些代码后面修正了，但是前面没有
   - 例如：按`L`解锁鼠标，后面是使用回调函数，而一开始则直接是写入`processInput()`函数中，导致按`L`并不能保证只执行一次
3. 有些文件进行了修改，比如摄像机类，`WSAD`只能在XOZ平面上移动，按`Space`和`LCtrl`才能上升或下降
4. `ReplaceText.py`是因为摄像机类把函数进行了修改，因此直接使用Python脚本进行了文本替换
5. 编码格式所有文件都为`utf-8`，在CMake中进行了设置，让MSVC能编译`utf-8`的文件，而打印消息时使用`GBK`编码，让打印消息不会乱码

### 参考

[BV11Z4y1c7so](https://www.bilibili.com/video/BV11Z4y1c7so/)

[LearnOpenGL CN](https://learnopengl-cn.github.io/)

[Conan 教学视频](https://www.bilibili.com/video/BV11Z4y1c7so/)

