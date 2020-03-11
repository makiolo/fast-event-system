import os
from conans import ConanFile, tools

class NpmMasMas(ConanFile):
    name = "fast-event-system"
    version = "1.0.23"
    license = "Attribution 4.0 International"
    url = "https://github.com/makiolo/fast-event-system"
    description = "This fast event system allows calls between two interfaces decoupled (sync or async)"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {"shared": True}
    generators = "cmake"

    def configure(self):
        self.options["boost"].shared = True

    def requirements(self):
        self.requires('boost/1.70.0@conan/stable')
        self.requires('gtest/1.8.1@bincrafters/stable')

    def source(self):
        self.run("git clone {}".format(self.url))

    def build(self):
        self.run("cd {} && CMAKI_INSTALL={} npm install && npm test".format(self.name, self.package_folder))

    def package(self):
        self.copy(pattern="*.h", src="{}/include".format(self.name), dst=os.path.join('include', self.name), keep_path=False)
        self.copy(pattern="*.h", src="{}/concurrentqueue".format(self.name), dst=os.path.join('include', 'concurrentqueue'), keep_path=False)
        self.copy(pattern="*.lib", src="{}/bin/{}".format(self.name, self.settings.build_type), dst="lib", keep_path=False)
        self.copy(pattern="*.dll", src="{}/bin/{}".format(self.name, self.settings.build_type), dst="bin", keep_path=False)
        self.copy(pattern="*.so", src="{}/bin/{}".format(self.name, self.settings.build_type), dst="lib", keep_path=False)
        self.copy(pattern="*.dylib", src="{}/bin/{}".format(self.name, self.settings.build_type), dst="lib", keep_path=False)
        self.copy(pattern="*.a", src="{}/bin/{}".format(self.name, self.settings.build_type), dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.includedirs = ['include', ]
        self.cpp_info.libdirs = ['Debug', 'Release']
        self.cpp_info.libs = tools.collect_libs(self)

