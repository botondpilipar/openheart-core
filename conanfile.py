from conan import ConanFile
from conan.tools.cmake import cmake_layout

class OpenHeartDependencies(ConanFile):
    settings = "os", "compiler", "arch"
    build_type = "Debug"
    generators = "CMakeDeps", "CMakeToolchain"
    options = {
        "glib/2.78.3:shared": True,
        "libcurl/8.6.0:shared": True
    }
    
    def requirements(self):
        self.requires("log.c/cci.20200620")
        self.requires("glib/2.78.3")
        self.requires("libcurl/8.6.0")
        

    def layout(self):
        cmake_layout(self)
