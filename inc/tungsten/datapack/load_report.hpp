/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#pragma once

#include <mutex>
#include <string>
#include <vector>

namespace tungsten {

struct load_issue_t {
    enum class severity_t { warning, error };

    severity_t m_severity = severity_t::error;
    std::string m_registry; // may be empty
    std::string m_id;       // may be empty
    std::string m_pack;     // may be empty
    std::string m_message;

    std::string to_string() const;
};

// collects problems found while loading data packs instead of failing on the first one
struct load_report_t {
private:
    mutable std::mutex m_mutex;
    std::vector<load_issue_t> m_issues;
public:
    load_report_t() = default;
    load_report_t(const load_report_t& other);
    load_report_t& operator=(const load_report_t& other);

    void add(load_issue_t issue);
    void warning(std::string registry, std::string id, std::string pack, std::string message);
    void error(std::string registry, std::string id, std::string pack, std::string message);

    std::vector<load_issue_t> issues() const;
    std::size_t error_count() const;
    std::size_t warning_count() const;
    bool ok() const { return error_count() == 0; }
    void clear();
};

}
