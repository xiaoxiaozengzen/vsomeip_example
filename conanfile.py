from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout, CMakeDeps
from conan.tools.env import VirtualBuildEnv , VirtualRunEnv
from conan.tools.files import (
    apply_conandata_patches,
    copy,
    export_conandata_patches,
    get,
    replace_in_file,
    rmdir,
    collect_libs,
    rm,
)
from conan.tools.scm import Version
import os
from conan.tools.scm import Git

required_conan_version = ">=1.54.0"


class VsomeipExampleConan(ConanFile):
    name = "vsomeip_example"
    version = "dev"
    url = "https://github.com/xiaoxiaozengzen/vsomeip_example"
    description = "example for vsomeip"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_commonapi": [True, False],
    }
    default_options = {"shared": False, "fPIC": True, "with_commonapi": False}

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")
            
    def requirements(self):
      self.requires("vsomeip/3.5.4@transformer/stable")
      if self.options.with_commonapi:
        self.requires("commonapicore/3.2.4@transformer/stable")
        self.requires("commonapisomeip/3.2.4@transformer/stable")

    def generate(self):
        tc = CMakeToolchain(self)
        if self.options.with_commonapi:
            tc.variables["COMMONAPI_USING"] = True
        tc.generate()
        tc = CMakeDeps(self)
        tc.generate()
        tc = VirtualRunEnv(self)
        tc.generate()
        tc = VirtualBuildEnv(self)
        tc.generate(scope="build")

