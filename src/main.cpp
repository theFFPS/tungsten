/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/datapack/datapack_manager.hpp>
#include <iostream>
#ifdef _WIN32
    #include <windows.h>
    #include <crtdbg.h>
    #pragma comment(linker, "/STACK:67108864")
#else
    #include <sys/resource.h>
    #include <algorithm>
#endif

import tungsten.io;
import tungsten.vfs;

#include <tungsten/crypto.hpp>
import tungsten.crypto;

using namespace tungsten;

void stop() {
    stop_asio();
    CRYPTO_secure_malloc_done();
    OPENSSL_cleanup();
}

void manage_resources() {
#ifdef _WIN32
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);

    #ifdef _DEBUG
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
    #endif

#else
    struct rlimit rl;
    rl.rlim_cur = 0;
    rl.rlim_max = 0;
    setrlimit(RLIMIT_CORE, &rl);

    struct rlimit rls;
    if (getrlimit(RLIMIT_STACK, &rls) == 0) {
        const rlim_t target_stack = 64ULL * 1024 * 1024;
        if (rls.rlim_cur < target_stack) {
            rls.rlim_cur = (rls.rlim_max == RLIM_INFINITY)
                            ? target_stack
                            : std::min(target_stack, rls.rlim_max);
            setrlimit(RLIMIT_STACK, &rls);
        }
    }
#endif
}
int main(int argc, char** argv) {
    manage_resources();
    if (CRYPTO_secure_malloc_init(1024*1024, 16) != 1) {
        throw std::runtime_error("tungsten::main error: failed to initialize OpenSSL secure heap.");
    }

    init_asio();
    // worker_thread.detach();

    g_vfs = std::make_shared<vfs_t>();
    g_vfs->initialize();

    // usage: tungsten [vanilla data directory] [extra packs directory]
    {
        datapack_manager_t manager;
        manager.add_base_directory(std::filesystem::path(argc > 1 ? argv[1] : "test/data"));
        if (argc > 2) manager.add_packs_from_directory(argv[2]);
        manager.load();
        std::cout << manager.summary();
        for (const auto& issue : manager.report().issues()) std::cout << issue.to_string() << "\n";
        std::cout << manager.report().error_count() << " errors, " << manager.report().warning_count() << " warnings\n";
    }

    stop();
}
