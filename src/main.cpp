/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <iostream>
#include <memory>
#include <tungsten/vfs/vfs.hpp>
#include <tungsten/crypto/shared.hpp>
#include <sys/resource.h>

#include <tungsten/util/file_io.hpp>

using namespace tungsten;

void stop() {
    g_file_io_ctx->stop();
    g_file_io_ctx = nullptr;
    CRYPTO_secure_malloc_done();
    OPENSSL_cleanup();
}

void disable_core_dumps() {
    struct rlimit rl;
    rl.rlim_cur = 0;
    rl.rlim_max = 0;
    setrlimit(RLIMIT_CORE, &rl);
}
int main() {
    disable_core_dumps();
    if (CRYPTO_secure_malloc_init(1024*1024, 16) != 1) {
        throw std::runtime_error("tungsten::main error: failed to initialize OpenSSL secure heap.");
    }

    g_file_io_ctx = std::make_shared<asio::io_context>();
    auto work_guard = asio::make_work_guard(*g_file_io_ctx);
    std::thread worker_thread ([]() {
        if (g_file_io_ctx) g_file_io_ctx->run();
    });
    // worker_thread.detach();

    g_vfs = std::make_shared<vfs_t>();
    g_vfs->initialize();

    std::string test = "Hello World!";
    auto handle = vfs_file_handle_t("/test.file", std::ios::out);
    handle.write((const uint8_t*) test.data(), test.size());
    handle.close();
    handle.open("/test.file", std::ios::in);
    std::vector<uint8_t> data (handle.size());
    handle.read(data.data(), data.size());
    handle.close();

    std::cout << std::string_view((const char*) data.data(), data.size()) << "\n";

//     try {
//         std::string test = "Hello World!";
//
//         auto file_handle = asio_file_handle_t("test.file", std::ios::out);
//         file_handle.write((const uint8_t*) test.data(), test.size());
//         file_handle.close();
//         file_handle.open("test.file", std::ios::in);
//         std::vector<uint8_t> buffer (file_handle.size());
//         file_handle.read(buffer.data(), buffer.size());
//         file_handle.close();
//
//         std::cout << std::string_view((const char*) buffer.data(), buffer.size()) << "\n";
//
//         while (1) {
//             std::string s;
//             std::cin >> s;
//             break;
//         }
    // } catch (std::exception& e) {
    //     std::cout << e.what() << "\n";
    // }

    work_guard.reset();
    if (worker_thread.joinable()) worker_thread.join();
    stop();
}
