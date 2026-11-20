/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <tungsten/datapack/load_report.hpp>

namespace tungsten {

std::string load_issue_t::to_string() const {
    std::string result = m_severity == severity_t::error ? "error" : "warning";
    if (!m_pack.empty()) result += " [" + m_pack + "]";
    if (!m_registry.empty()) result += " " + m_registry;
    if (!m_id.empty()) result += (m_registry.empty() ? " " : ":") + m_id;
    return result + ": " + m_message;
}

load_report_t::load_report_t(const load_report_t& other) : m_issues(other.issues()) {}
load_report_t& load_report_t::operator=(const load_report_t& other) {
    if (this != &other) {
        auto copy = other.issues();
        std::lock_guard lock (m_mutex);
        m_issues = std::move(copy);
    }
    return *this;
}
void load_report_t::add(load_issue_t issue) {
    std::lock_guard lock (m_mutex);
    m_issues.push_back(std::move(issue));
}
void load_report_t::warning(std::string registry, std::string id, std::string pack, std::string message) {
    add({ load_issue_t::severity_t::warning, std::move(registry), std::move(id), std::move(pack), std::move(message) });
}
void load_report_t::error(std::string registry, std::string id, std::string pack, std::string message) {
    add({ load_issue_t::severity_t::error, std::move(registry), std::move(id), std::move(pack), std::move(message) });
}
std::vector<load_issue_t> load_report_t::issues() const {
    std::lock_guard lock (m_mutex);
    return m_issues;
}
std::size_t load_report_t::error_count() const {
    std::lock_guard lock (m_mutex);
    std::size_t count = 0;
    for (const auto& issue : m_issues) if (issue.m_severity == load_issue_t::severity_t::error) ++count;
    return count;
}
std::size_t load_report_t::warning_count() const {
    std::lock_guard lock (m_mutex);
    return m_issues.size() - [this] {
        std::size_t count = 0;
        for (const auto& issue : m_issues) if (issue.m_severity == load_issue_t::severity_t::error) ++count;
        return count;
    }();
}
void load_report_t::clear() {
    std::lock_guard lock (m_mutex);
    m_issues.clear();
}

}
