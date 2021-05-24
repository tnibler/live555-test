from conans import ConanFile, CMake, tools


class Live555TestConan(ConanFile):
    name = "live555-test"
    version = "0.1.0"
    license = "<Put the package license here>"
    author = "<Put your name here> <And your email here>"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "<Description of Live555Test here>"
    topics = ("<Put some tag here>", "<here>", "<and here>")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    generators = "cmake"

    requires = (
        "live555/1.24.0@camposs/stable"
    )

    def __configure_cmake(self):
        cmake = CMake(self)
        cmake.configure()
        return cmake

    def build(self):
        cmake = self.__configure_cmake()
        cmake.build()

    def package(self):
        cmake = self.__configure_cmake()
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
