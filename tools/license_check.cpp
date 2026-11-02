/*

     tungsten-server
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <fstream>
#include <filesystem>
#include <iostream>

int main(int argc, char **argv) {
    if (argc < 2) return 1;

    std::ifstream in_f (argv[1]);
    std::string license_data (std::filesystem::file_size(argv[1]), ' ');
    in_f.read(license_data.data(), license_data.size());
    in_f.close();

    for (int i = 2; i < argc; ++i) {
        std::string path = argv[i]; 
        std::ifstream in_f (path);
        std::string data (std::filesystem::file_size(path), ' ');
        in_f.read(data.data(), data.size());
        in_f.close();

        if (!data.starts_with(license_data)) {
            std::cout << "License header not found for " << path << ": applying license header\n";

            std::ofstream out_f (path);
            out_f << license_data;
            if (!license_data.ends_with('\n')) out_f << "\n";
            out_f << "\n";
            out_f << data;
            out_f.close();
        }
    }
}