/*

     tungsten
     Copyright (C) 2026  40fpsss

     This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

     This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.

     You should have received a copy of the GNU Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

// Checks of the data pack loader. Usage: tungsten-datapack-tests [vanilla data directory (default: test/data)]

#include <cstdlib>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <miniz.h>
#include <tungsten/datapack/datapack_manager.hpp>
#include <tungsten/nbt/nbt.hpp>
#include <tungsten/type/int_provider.hpp>
#include <tungsten/util/json_reflect.hpp>

import tungsten.compression;

using namespace tungsten;
namespace fs = std::filesystem;
using json = glz::generic;

namespace {

int g_failures = 0;
int g_checks = 0;

#define CHECK(expr) do { ++g_checks; if (!(expr)) { ++g_failures; std::cerr << "FAILED " << __FILE__ << ":" << __LINE__ << ": " #expr "\n"; } } while (0)
#define CHECK_THROWS(expr) do { ++g_checks; bool thrown = false; try { (void) (expr); } catch (const std::exception&) { thrown = true; } \
    if (!thrown) { ++g_failures; std::cerr << "FAILED " << __FILE__ << ":" << __LINE__ << ": expected an exception from " #expr "\n"; } } while (0)

struct temp_dir_t {
    fs::path m_path;
    temp_dir_t() {
        m_path = fs::temp_directory_path() / ("tungsten-datapack-tests-" + std::to_string(std::rand()) + "-" + std::to_string(::getpid()));
        fs::create_directories(m_path);
    }
    ~temp_dir_t() { std::error_code ec; fs::remove_all(m_path, ec); }
    void write(const std::string& relative, const std::string& content) const {
        auto full = m_path / relative;
        fs::create_directories(full.parent_path());
        std::ofstream(full, std::ios::binary) << content;
    }
};

std::string mcmeta(int min_format = 121, const std::string& extra = "") {
    return R"({"pack":{"description":"test","min_format":)" + std::to_string(min_format) + R"(,"max_format":)" + std::to_string(min_format) + "}" + extra + "}";
}

void test_resource_location() {
    resource_location_t a ("minecraft:stone");
    CHECK(a.m_namespace == "minecraft" && a.m_path == "stone");
    CHECK(resource_location_t("stone").m_namespace == "minecraft");
    CHECK(resource_location_t(":stone").to_string() == "minecraft:stone");
    CHECK(resource_location_t("my_pack:a/b/c").m_path == "a/b/c");
    CHECK_THROWS(resource_location_t("a:b:c"));
    CHECK_THROWS(resource_location_t("Upper:case"));
    CHECK_THROWS(resource_location_t("ns:with space"));
    CHECK(!resource_location_t::is_valid("bad!"));
    CHECK(resource_location_t("a:b") < resource_location_t("a:c"));
    CHECK(resource_location_t("a:z") < resource_location_t("b:a")); // namespace first
    CHECK(jr::read<resource_location_t>(R"("x:y")") == resource_location_t("x", "y"));
    CHECK_THROWS(jr::read<resource_location_t>("5"));
}

void test_pack_meta() {
    auto m = pack_meta_t::parse(R"({"pack":{"description":"d","min_format":88,"max_format":[94,1]}})");
    CHECK(m.m_range.m_min == (pack_version_t{ 88, 0 }));
    CHECK(m.m_range.m_max == (pack_version_t{ 94, 1 }));
    CHECK(m.m_range.contains({ 90, 5 }));
    CHECK(!m.m_range.contains({ 94, 2 }));

    // a single integer max_format accepts every minor version
    auto single = pack_meta_t::parse(R"({"pack":{"description":"d","min_format":121,"max_format":121}})");
    CHECK(single.m_range.contains({ 121, 0 }) && single.m_range.contains({ 121, 99 }) && !single.m_range.contains({ 122, 0 }));

    // legacy formats
    auto legacy = pack_meta_t::parse(R"({"pack":{"description":"d","pack_format":48,"supported_formats":{"min_inclusive":45,"max_inclusive":50}}})");
    CHECK(legacy.m_range.m_min.m_major == 45 && legacy.m_range.m_max.m_major == 50);
    auto legacy_only = pack_meta_t::parse(R"({"pack":{"description":"d","pack_format":15}})");
    CHECK(legacy_only.m_range.contains({ 15, 0 }) && !legacy_only.m_range.contains({ 16, 0 }));
    auto legacy_list = pack_meta_t::parse(R"({"pack":{"description":"d","pack_format":15,"supported_formats":[15,18]}})");
    CHECK(legacy_list.m_range.m_max.m_major == 18);

    CHECK_THROWS(pack_meta_t::parse("{"));
    CHECK_THROWS(pack_meta_t::parse("[]"));
    CHECK_THROWS(pack_meta_t::parse(R"({"pack":{"min_format":1,"max_format":1}})")); // no description
    CHECK_THROWS(pack_meta_t::parse(R"({"pack":{"description":"d"}})")); // no format at all
    CHECK_THROWS(pack_meta_t::parse(R"({"pack":{"description":"d","min_format":5,"max_format":4}})"));
    CHECK_THROWS(pack_meta_t::parse(R"({"pack":{"description":"d","min_format":5}})"));

    auto full = pack_meta_t::parse(R"({
        "pack":{"description":{"translate":"x.y"},"min_format":121,"max_format":121},
        "features":{"enabled":["minecraft:trade_rebalance"]},
        "filter":{"block":[{"namespace":"minecraft","path":"loot_table/.*"},{"path":"^recipe/secret"}]},
        "overlays":{"entries":[{"directory":"ov-1","min_format":[121,0],"max_format":125},{"directory":"old","min_format":1,"max_format":2}]}})");
    CHECK(full.m_enabled_features.size() == 1 && full.m_enabled_features[0] == resource_location_t("trade_rebalance"));
    CHECK(full.is_filtered("minecraft", "loot_table/blocks/stone.json"));
    CHECK(!full.is_filtered("other", "loot_table/blocks/stone.json"));
    CHECK(full.is_filtered("any", "recipe/secret_x.json"));
    CHECK(!full.is_filtered("any", "recipe/open.json"));
    CHECK(full.active_overlays({ 121, 0 }) == std::vector<std::string>{ "ov-1" });
    CHECK(full.active_overlays({ 2, 0 }) == std::vector<std::string>{ "old" });
    CHECK(full.description_text().m_value.index() == 1); // translatable
    CHECK_THROWS(pack_meta_t::parse(R"({"pack":{"description":"d","min_format":1,"max_format":1},"filter":{"block":[{"path":"("}]}})"));
    CHECK_THROWS(pack_meta_t::parse(R"({"pack":{"description":"d","min_format":1,"max_format":1},"overlays":{"entries":[{"directory":"../x","min_format":1,"max_format":1}]}})"));
    // a UTF-8 BOM is tolerated
    (void) pack_meta_t::parse(std::string("\xEF\xBB\xBF") + mcmeta());
}

void test_text_component() {
    auto parse = [](const char* s) { return jr::read<text_t>(s); };
    CHECK(parse(R"("hello")").to_plain_string() == "hello");
    CHECK(parse("12").to_plain_string() == "12");
    CHECK(parse(R"({"text":"a","extra":[{"text":"b"},"c"]})").to_plain_string() == "abc");
    CHECK(parse(R"(["a",{"text":"b"}])").to_plain_string() == "ab");
    CHECK(parse(R"({"translate":"k","fallback":"fb","with":["x",{"text":"y"}]})").to_plain_string() == "fb");
    auto styled = parse(R"({"text":"a","color":"red","bold":true,"font":"minecraft:uniform"})");
    CHECK(styled.m_color == "red" && styled.m_bold == true && styled.m_font.has_value());
    CHECK(std::holds_alternative<text_t::score_t>(parse(R"({"score":{"name":"@p","objective":"o"}})").m_value));
    CHECK(std::holds_alternative<text_t::keybind_t>(parse(R"({"keybind":"key.jump"})").m_value));
    CHECK(std::holds_alternative<text_t::nbt_t>(parse(R"({"nbt":"a.b","storage":"x:y"})").m_value));
    CHECK(std::holds_alternative<text_t::object_t>(parse(R"({"type":"object","atlas":"blocks","sprite":"x:y"})").m_value));
    CHECK_THROWS(parse(R"({"type":"bogus"})"));
    CHECK_THROWS(parse("[]"));
    CHECK_THROWS(parse("null"));
    // round trip
    auto j = parse(R"({"text":"a","italic":false,"extra":["b"]})").to_generic();
    CHECK(j["extra"][0]["text"].get_string() == "b" && j["italic"].get_boolean() == false);
}

}  // namespace

// test-only type; its glaze specialization must sit at global scope
struct sample_t {
    int required;
    std::optional<std::string> opt;
    int defaulted = 7;
    int renamed = 0;
    std::vector<resource_location_t> list = {};
};

// members are named like their key; keys which are no identifier are mapped explicitly
template <> struct glz::meta<sample_t> {
    using T = sample_t;
    static constexpr auto value = glz::object("required", &T::required, "opt", &T::opt, "defaulted", &T::defaulted, "odd-name", &T::renamed, "list", &T::list);
};

namespace {

void test_json_reflect() {
    auto s = jr::read<sample_t>(R"({"required":1,"odd-name":3,"list":["a:b"]})");
    CHECK(s.required == 1 && s.defaulted == 7 && s.renamed == 3 && !s.opt && s.list.size() == 1);
    std::string text;
    (void) glz::write_json(s, text);
    json out;
    (void) glz::read_json(out, text);
    CHECK(out["required"].get_number() == 1 && out["odd-name"].get_number() == 3 && !out.contains("opt") && out["defaulted"].get_number() == 7);
    CHECK_THROWS(jr::read<sample_t>(R"({"required":"str"})"));
    CHECK_THROWS(jr::read<sample_t>("[]"));
    // the error names the failing location
    try {
        (void) jr::read<sample_t>(R"({"required":1,"list":["ok:fine","BAD"]})");
        CHECK(false);
    } catch (const std::exception& e) {
        CHECK(std::string(e.what()).find("BAD") != std::string::npos);
    }
    std::vector<std::string> unknown;
    {
        jr::unknown_key_scope_t scope (unknown);
        (void) jr::read<sample_t>(R"({"required":1,"extra_key":1})");
    }
    CHECK(unknown.size() == 1 && unknown[0].ends_with("::extra_key"));
    // namespaced and bare discriminators select the same alternative
    auto bare = jr::read<int_provider_t>(R"({"type":"uniform","min_inclusive":1,"max_inclusive":2})");
    auto namespaced = jr::read<int_provider_t>(R"({"type":"minecraft:uniform","min_inclusive":1,"max_inclusive":2})");
    CHECK(bare.index() == 1 && namespaced.index() == 1);
    CHECK(jr::read<int_provider_t>("7").index() == 0);
}

void test_nbt() {
    // hand-built: root compound "" { "a": int 5, "l": list<byte>[1,2], "s": "str", "c": { "d": double 1.5 } }
    std::vector<std::uint8_t> raw = { 10, 0, 0,
        3, 0, 1, 'a', 0, 0, 0, 5,
        9, 0, 1, 'l', 1, 0, 0, 0, 2, 1, 2,
        8, 0, 1, 's', 0, 3, 's', 't', 'r',
        10, 0, 1, 'c', 6, 0, 1, 'd', 0x3f, 0xf8, 0, 0, 0, 0, 0, 0, 0,
        0 };
    auto n = nbt_t::parse(raw);
    CHECK(n.find("a")->as_integer() == 5);
    CHECK(n.find("l")->list().m_items.size() == 2);
    CHECK(n.find("s")->string() == "str");
    CHECK(n.find("c")->find("d")->as_floating() == 1.5);
    auto gz = compression_t::compress(compression_type_t::gzip, raw.data(), raw.size(), 6);
    CHECK(gz.size() > 2 && gz[0] == 0x1f && gz[1] == 0x8b);
    CHECK(nbt_t::parse(gz).find("a")->as_integer() == 5); // gzip detected automatically
    CHECK(compression_t::decompress(compression_type_t::gzip, gz.data(), gz.size(), 0) == raw);
    CHECK_THROWS(nbt_t::parse(std::span<const std::uint8_t>(raw.data(), raw.size() - 3))); // truncated
    CHECK_THROWS(nbt_t::parse(std::vector<std::uint8_t>{ 3, 0, 0, 0, 0, 0, 1 })); // root is not a compound
    CHECK_THROWS(nbt_t::parse(std::vector<std::uint8_t>{ 10, 0, 0, 7, 0, 1, 'x', 0x7f, 0xff, 0xff, 0xff })); // absurd array length
    std::vector<std::uint8_t> corrupt = gz; corrupt[corrupt.size() / 2] ^= 0xff;
    CHECK_THROWS(nbt_t::parse(corrupt));
    // deep nesting must not overflow the stack
    std::vector<std::uint8_t> deep = { 10, 0, 0 };
    for (int i = 0; i < 2000; ++i) { deep.insert(deep.end(), { 10, 0, 1, 'x' }); }
    CHECK_THROWS(nbt_t::parse(deep));
}

void test_function() {
    auto f = function_t::parse(std::span<const std::uint8_t>(reinterpret_cast<const std::uint8_t*>("# comment\n\nsay hi\n/say slash\n  tp @s ~ ~ ~ \\\n  ~1\r\n$say $(x)\n"), 61));
    CHECK(f.m_commands.size() == 4);
    CHECK(f.m_commands[0].m_text == "say hi" && f.m_commands[0].m_line_number == 3);
    CHECK(f.m_commands[1].m_text == "say slash");
    CHECK(f.m_commands[2].m_text == "tp @s ~ ~ ~ ~1");
    CHECK(f.m_commands[3].m_macro);
    CHECK_THROWS(function_t::parse(std::span<const std::uint8_t>(reinterpret_cast<const std::uint8_t*>("say a \\\n"), 8)));
    const std::uint8_t bad_utf8[] = { 's', 'a', 'y', ' ', 0xff };
    CHECK_THROWS(function_t::parse(bad_utf8));
}

void test_layering() {
    temp_dir_t root;
    root.write("base/pack.mcmeta", mcmeta());
    root.write("base/data/minecraft/damage_type/a.json", R"({"message_id":"base_a","scaling":"never","exhaustion":0.1})");
    root.write("base/data/minecraft/damage_type/b.json", R"({"message_id":"base_b","scaling":"never","exhaustion":0.1})");
    root.write("base/data/minecraft/damage_type/sub/c.json", R"({"message_id":"base_c","scaling":"never","exhaustion":0.1})");
    root.write("base/data/minecraft/tags/damage_type/t.json", R"({"values":["minecraft:a","minecraft:b"]})");
    root.write("base/data/minecraft/tags/damage_type/replaced.json", R"({"values":["minecraft:a"]})");
    root.write("base/data/minecraft/tags/damage_type/nested.json", R"({"values":["#minecraft:t","minecraft:sub/c"]})");
    root.write("base/data/minecraft/tags/damage_type/cycle_a.json", R"({"values":["#minecraft:cycle_b"]})");
    root.write("base/data/minecraft/tags/damage_type/cycle_b.json", R"({"values":["#minecraft:cycle_a"]})");
    root.write("base/data/minecraft/tags/damage_type/dangling.json", R"({"values":["minecraft:missing",{"id":"minecraft:also_missing","required":false},"#minecraft:nope"]})");

    // overrides a, extends the tag, replaces another, hides base's b through its filter... (filter only blocks packs below it)
    root.write("top/pack.mcmeta", mcmeta(121, R"(,"filter":{"block":[{"path":"damage_type/b\\.json"}]})"));
    root.write("top/data/minecraft/damage_type/a.json", R"({"message_id":"top_a","scaling":"always","exhaustion":0.2})");
    root.write("top/data/minecraft/damage_type/b.json", R"({"message_id":"top_b","scaling":"always","exhaustion":0.2})");
    root.write("top/data/minecraft/damage_type/own.json", R"({"message_id":"own","scaling":"always","exhaustion":0.2})");
    root.write("top/data/minecraft/tags/damage_type/t.json", R"({"values":["minecraft:own"]})");
    root.write("top/data/minecraft/tags/damage_type/replaced.json", R"({"replace":true,"values":["minecraft:own"]})");
    root.write("top/data/custom/damage_type/x.json", R"({"message_id":"x","scaling":"always","exhaustion":1})");
    root.write("top/data/custom/damage_type/broken.json", R"({"message_id":"x"})");
    root.write("top/data/custom/damage_type/notjson.json", "{ nope");

    // an overlay of a third pack
    root.write("ov/pack.mcmeta", mcmeta(121, R"(,"overlays":{"entries":[{"directory":"newer","min_format":121,"max_format":121},{"directory":"older","min_format":1,"max_format":5}]})"));
    root.write("ov/data/minecraft/damage_type/o.json", R"({"message_id":"plain","scaling":"never","exhaustion":0})");
    root.write("ov/newer/data/minecraft/damage_type/o.json", R"({"message_id":"overlay","scaling":"never","exhaustion":0})");
    root.write("ov/older/data/minecraft/damage_type/o.json", R"({"message_id":"wrong","scaling":"never","exhaustion":0})");

    datapack_manager_t manager;
    manager.add_pack(root.m_path / "base");
    manager.add_pack(root.m_path / "top");
    manager.add_pack(root.m_path / "ov");
    manager.load();

    auto& dt = registry::damage_type;
    CHECK(dt.by_identifier(resource_location_t("a"))->message_id == "top_a"); // override
    CHECK(dt.source_of(resource_location_t("a")) == "top");
    CHECK(dt.by_identifier(resource_location_t("b"))->message_id == "top_b");
    CHECK(dt.by_identifier(resource_location_t("sub/c"))->message_id == "base_c"); // nested directory, id has no directory prefix
    CHECK(dt.contains(resource_location_t("custom:x")));
    CHECK(dt.contains(resource_location_t("custom:broken"))); // missing fields are lenient on the read path
    CHECK(!dt.contains(resource_location_t("custom:notjson")));
    CHECK(dt.by_identifier(resource_location_t("o"))->message_id == "overlay"); // matching overlay wins, non matching ignored
    CHECK(dt.by_idx(dt.object_idx(resource_location_t("a")))->message_id == "top_a");

    CHECK(manager.report().error_count() >= 3); // notjson, cycle x2 (+ dangling)
    bool saw_notjson = false, saw_cycle = false, saw_missing = false, saw_nope = false, saw_optional = false;
    for (const auto& issue : manager.report().issues()) {
        saw_notjson |= issue.m_id == "custom:notjson";
        saw_cycle |= issue.m_message.find("cycle") != std::string::npos;
        saw_missing |= issue.m_message.find("minecraft:missing") != std::string::npos;
        saw_nope |= issue.m_message.find("#minecraft:nope") != std::string::npos;
        saw_optional |= issue.m_message.find("also_missing") != std::string::npos;
    }
    CHECK(saw_notjson && saw_cycle && saw_missing && saw_nope);
    CHECK(!saw_optional); // "required": false entries are never reported

    const auto& tags = manager.tags().require_registry("damage_type");
    auto has = [&](const char* tag, const char* element) { return tags.contains(resource_location_t(tag), resource_location_t(element)); };
    CHECK(has("t", "a") && has("t", "b") && has("t", "own")); // merged
    CHECK(tags.elements(resource_location_t("t")).size() == 3);
    CHECK(has("replaced", "own") && !has("replaced", "a")); // "replace": true
    CHECK(has("nested", "a") && has("nested", "own") && has("nested", "sub/c")); // nested tag + plain entry
    CHECK_THROWS(tags.elements(resource_location_t("does_not_exist")));

    // the filter of "top" must hide b of base only: top's own b stays, and a pack's filter never affects itself
    datapack_manager_t second;
    second.add_pack(root.m_path / "base");
    second.add_pack(root.m_path / "top");
    auto layers = second.stack().layers("minecraft", "damage_type/b.json");
    CHECK(layers.size() == 1 && layers[0].m_pack->name() == "top");
    CHECK(second.stack().layers("minecraft", "damage_type/a.json").size() == 2);

    // removing / reloading is repeatable
    manager.remove_pack("top");
    manager.load();
    CHECK(dt.by_identifier(resource_location_t("a"))->message_id == "base_a");
    CHECK(!dt.contains(resource_location_t("custom:x")));
}

void test_zip_pack() {
    mz_zip_archive zip {};
    CHECK(mz_zip_writer_init_heap(&zip, 0, 0));
    auto add = [&zip](const char* name, const std::string& content) {
        CHECK(mz_zip_writer_add_mem(&zip, name, content.data(), content.size(), MZ_DEFAULT_COMPRESSION));
    };
    add("pack.mcmeta", mcmeta());
    add("data/minecraft/damage_type/zipped.json", R"({"message_id":"zipped","scaling":"never","exhaustion":0.5})");
    add("data/minecraft/tags/damage_type/z.json", R"({"values":["minecraft:zipped"]})");
    void* data = nullptr; std::size_t size = 0;
    CHECK(mz_zip_writer_finalize_heap_archive(&zip, &data, &size));
    std::vector<std::uint8_t> bytes (static_cast<std::uint8_t*>(data), static_cast<std::uint8_t*>(data) + size);
    mz_zip_writer_end(&zip);
    free(data);

    temp_dir_t root;
    {
        std::ofstream out (root.m_path / "zipped.zip", std::ios::binary);
        out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }
    root.write("notapack.zip", "this is not a zip");
    root.write("emptydir/readme.txt", "no pack.mcmeta");

    datapack_manager_t manager;
    manager.add_packs_from_directory(root.m_path);
    manager.load();
    CHECK(registry::damage_type.contains(resource_location_t("zipped")));
    CHECK(manager.tags().require_registry("damage_type").contains(resource_location_t("z"), resource_location_t("zipped")));
    CHECK(manager.report().error_count() == 2); // the invalid zip and the directory without pack.mcmeta are reported, not fatal
    CHECK_THROWS(datapack_t::open(root.m_path / "notapack.zip"));
    CHECK_THROWS(datapack_t::open(root.m_path / "emptydir"));
    CHECK_THROWS(open_pack_source(root.m_path / "does_not_exist"));

    zip_pack_source_t from_memory (bytes, "memory");
    CHECK(from_memory.exists("pack.mcmeta") && !from_memory.exists("nope"));
    CHECK_THROWS(from_memory.read("nope"));
}

void test_pack_compatibility() {
    temp_dir_t root;
    root.write("old/pack.mcmeta", mcmeta(48));
    root.write("future/pack.mcmeta", mcmeta(999));
    datapack_manager_t manager;
    manager.add_pack(root.m_path / "old");
    manager.add_pack(root.m_path / "future");
    CHECK(manager.report().warning_count() == 2);
    CHECK(datapack_t::open(root.m_path / "old")->compatibility() == datapack_t::compatibility_t::too_old);
    CHECK(datapack_t::open(root.m_path / "future")->compatibility() == datapack_t::compatibility_t::too_new);
}

// write(read(x)) must be a fixed point for every typed registry entry
template<class T> bool stable_round_trip(const T& object) {
    std::string first;
    if (glz::write_json(object, first)) return false;
    T parsed{};
    if (glz::read<jr::read_opts>(parsed, first)) return false;
    std::string second;
    if (glz::write_json(parsed, second)) return false;
    if (first != second) std::cerr << "--- original ---\n" << first << "\n" << "--- roundtripped ---\n" << second << "\n";
    return first == second;
}

void test_vanilla(const fs::path& data) {
    if (!fs::exists(data / "minecraft")) {
        std::cout << "vanilla data not found at " << data << ": skipping\n";
        return;
    }
    datapack_manager_t manager;
    manager.add_base_directory(data);
    manager.load();

    for (const auto& issue : manager.report().issues()) std::cerr << issue.to_string() << "\n";
    CHECK(manager.report().error_count() == 0);
    CHECK(manager.report().warning_count() == 0);

    // every file has to be loaded: compare registry sizes with file counts
    std::size_t checked = 0;
    for (auto* registry : registry::all()) {
        auto directory = data / "minecraft" / registry->identifier().m_path;
        if (!fs::exists(directory)) continue;
        std::string extension = registry->identifier().m_path == "structure" ? ".nbt" : registry->identifier().m_path == "function" ? ".mcfunction" : ".json";
        std::size_t files = 0;
        for (const auto& e : fs::recursive_directory_iterator(directory)) files += e.is_regular_file() && e.path().extension() == extension;
        if (registry->size() != files) {
            std::cerr << registry->identifier().m_path << ": " << registry->size() << " loaded, " << files << " files\n";
        }
        CHECK(registry->size() == files);
        ++checked;
    }
    CHECK(checked >= 45);

    std::size_t tag_files = 0;
    for (const auto& e : fs::recursive_directory_iterator(data / "minecraft" / "tags")) tag_files += e.is_regular_file();
    CHECK(manager.tags().tag_count() == tag_files);

    // spot checks of decoded content
    auto sharpness = registry::enchantment.by_identifier(resource_location_t("sharpness"));
    CHECK(sharpness->max_level == 5 && sharpness->weight == 10 && sharpness->min_cost.per_level_above_first == 11);
    CHECK(sharpness->effects.contains(resource_location_t("damage")));
    auto wolf = registry::wolf_variant.by_identifier(resource_location_t("ashen"));
    CHECK(wolf->assets.angry == resource_location_t("entity/wolf/wolf_ashen_angry"));
    CHECK(wolf->spawn_conditions.size() == 1 && wolf->spawn_conditions[0].condition.has_value());
    auto camp = registry::structure.by_identifier(resource_location_t("abandoned_camp/camp/bamboo_jungle/campsite_bamboo_jungle_1"));
    CHECK(camp->m_size == (std::array<std::int32_t, 3>{ 10, 10, 10 }) && !camp->m_blocks.empty() && camp->m_palette[0].m_name == resource_location_t("grass_block"));
    CHECK(camp->state_of(camp->m_blocks[0]).m_name.m_path.size() > 0);
    auto stick = registry::recipe.by_identifier(resource_location_t("stick"));
    CHECK(std::holds_alternative<crafting_shaped_t>(*stick) && !std::get<crafting_shaped_t>(*stick).key.empty());
    CHECK(registry::advancement.by_identifier(resource_location_t("story/mine_stone"))->display.has_value());
    CHECK(registry::loot_table.by_identifier(resource_location_t("blocks/stone"))->pools.size() == 1);
    CHECK(registry::worldgen::biome.by_identifier(resource_location_t("plains"))->has_precipitation);
    CHECK(registry::dialog.by_identifier(resource_location_t("quick_actions"))->type == resource_location_t("dialog_list"));
    CHECK(registry::test_instance.by_identifier(resource_location_t("always_pass"))->function.has_value());
    CHECK(manager.tags().require_registry("block").contains(resource_location_t("climbable"), resource_location_t("ladder")));
    CHECK(manager.tags().require_registry("worldgen/biome").has_tag(resource_location_t("is_forest")));
    CHECK(!manager.tags().require_registry("block").elements(resource_location_t("mineable/pickaxe")).empty());


    // bundled experimental packs
    CHECK(manager.bundled_packs().size() == 3);
    std::size_t base_loot = registry::loot_table.size();
    std::size_t base_tags = manager.tags().tag_count();
    manager.enable_feature(resource_location_t("trade_rebalance"));
    manager.load();
    CHECK(manager.report().error_count() == 0);
    CHECK(registry::loot_table.size() == base_loot); // only overrides
    CHECK(registry::loot_table.source_of(resource_location_t("chests/ancient_city")) == "trade_rebalance");
    CHECK(manager.tags().tag_count() >= base_tags);
    CHECK(!manager.tags().require_registry("enchantment").elements(resource_location_t("trades/desert_common")).empty());
    manager.disable_feature(resource_location_t("trade_rebalance"));
    manager.load();
    CHECK(registry::loot_table.source_of(resource_location_t("chests/ancient_city")) == "vanilla");
}

}

int main(int argc, char** argv) {
    test_resource_location();
    test_pack_meta();
    test_text_component();
    test_json_reflect();
    test_nbt();
    test_function();
    test_layering();
    test_zip_pack();
    test_pack_compatibility();
    test_vanilla(argc > 1 ? argv[1] : "test/data");
    std::cout << g_checks << " checks, " << g_failures << " failures\n";
    return g_failures == 0 ? 0 : 1;
}
