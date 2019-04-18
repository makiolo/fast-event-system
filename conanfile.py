import os
from conans import ConanFile, tools

class FastEventSystem(ConanFile):
    name = "fast-event-system"
    version = "1.0.15"
    license = "Attribution 4.0 International"
    url = "https://github.com/makiolo/fast-event-system"
    description = "This fast event system allows calls between two interfaces decoupled (sync or async)"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {"shared": True}
    generators = "cmake"
    build_policy = "missing"

    def configure(self):
        self.options["boost"].shared = True
        self.options["boost"].without_test = True
    
    # def requirements(self):
    #     self.requires('boost/1.69.0@conan/stable')
    #     self.requires('gtest/1.8.1@bincrafters/stable')

    def source(self):
        self.run("git clone https://github.com/makiolo/{}".format(self.name))

    def build(self):
        self.run("cd {} && npm install".format(self.name))

    def package(self):
        self.copy("{}/*.h".format(self.name), dst="include", excludes=["{}/node_modules".format(self.name), "{}/readerwriterqueue".format(self.name)])
        self.copy("{}/bin/{}/*.lib".format(self.name, self.settings.build_type), dst="lib", excludes=["{}/bin/{}/libboost_*".format(self.name, self.settings.build_type)], keep_path=False)
        self.copy("{}/bin/{}/*.dll".format(self.name, self.settings.build_type), dst="bin", excludes=["{}/bin/{}/libboost_*".format(self.name, self.settings.build_type)], keep_path=False)
        self.copy("{}/bin/{}/*_unittest".format(self.name, self.settings.build_type), dst="unittest", excludes=["{}/bin/{}/libboost_*".format(self.name, self.settings.build_type)], keep_path=False)
        self.copy("{}/bin/{}/*.so".format(self.name, self.settings.build_type), dst="lib", excludes=["{}/bin/{}/libboost_*".format(self.name, self.settings.build_type)], keep_path=False)
        self.copy("{}/bin/{}/*.dylib".format(self.name, self.settings.build_type), dst="lib", excludes=["{}/bin/{}/libboost_*".format(self.name, self.settings.build_type)], keep_path=False)
        self.copy("{}/bin/{}/*.a".format(self.name, self.settings.build_type), dst="lib", excludes=["{}/bin/{}/libboost_*".format(self.name, self.settings.build_type)], keep_path=False)

    def package_info(self):
        self.cpp_info.libs = [lib for lib in tools.collect_libs(self)]

