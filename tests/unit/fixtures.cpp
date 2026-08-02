#include "fixtures.hpp"

#include <boost/json/value.hpp>
#include <yggdrasil/core/path.hpp>
#include <yggdrasil/serialization/json.hpp>

namespace runir::tests
{

namespace
{

auto fixture_path(std::string_view relative) -> std::filesystem::path { return std::filesystem::path(ROOT_DIR) / "tests/fixtures" / relative; }

}  // namespace

auto benchmark_path(std::string_view relative) -> std::filesystem::path { return std::filesystem::path(BENCHMARKS_DIR) / relative; }

auto load_fixture_json(std::string_view relative) -> boost::json::value { return ygg::common::load_json_file(fixture_path(relative)); }

auto read_fixture(std::string_view relative) -> std::string { return ygg::common::read_file(fixture_path(relative)); }

}  // namespace runir::tests
