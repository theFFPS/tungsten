from conan import ConanFile
from conan.tools.cmake import cmake_layout

class fengine(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        # Security
        self.requires("openssl/4.1.0")
        self.requires("libsodium/1.0.22")

        # UI
        self.requires("libqrencode/4.1.1")

        # Compression
        self.requires("zstd/1.5.7")
        self.requires("zlib-ng/2.3.2")

        # Utils
        self.requires("stduuid/1.2.3")
        self.requires("spdlog/1.16.0")
        self.requires("glaze/8.4.0")

        # I/O
        self.requires("asio/1.38.0")
        if (self.settings.os == "Linux"):
            self.requires("liburing/2.13")

    def layout(self):
        cmake_layout(self)
