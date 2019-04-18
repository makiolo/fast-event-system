import os
from conans import ConanFile, CMake

class FastEventSystem(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"

    def requirements(self):
        self.requires('boost/1.69.0@conan/stable')
        self.requires('gtest/1.8.1@bincrafters/stable')

    def build(self):
        # self.run('cmaki setup -e BUILD_DIR={}'.format(os.getcwd()))
        # self.run('cmaki compile -e BUILD_DIR={}'.format(os.getcwd()))
        self.run('npm install')

    def test(self):
        # self.run('cmaki test -e BUILD_DIR={}'.format(os.getcwd()))
        self.run('npm test')

