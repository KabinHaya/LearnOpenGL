from conan import ConanFile
from conan.tools.cmake import cmake_layout


class ExampleRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("opengl/system")
        self.requires("glfw/3.4")
        self.requires("assimp/6.0.2")
        self.requires("glm/0.9.9.8")
        self.requires("glad/0.1.36")
        self.requires("imgui/1.92.6")

    def layout(self):
        cmake_layout(self)