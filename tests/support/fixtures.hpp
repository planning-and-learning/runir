#ifndef RUNIR_TESTS_UNIT_FIXTURES_HPP_
#define RUNIR_TESTS_UNIT_FIXTURES_HPP_

#include <boost/json/fwd.hpp>
#include <filesystem>
#include <string>
#include <string_view>

namespace runir::tests
{

auto benchmark_path(std::string_view relative) -> std::filesystem::path;

auto load_fixture_json(std::string_view relative) -> boost::json::value;

auto read_fixture(std::string_view relative) -> std::string;

}  // namespace runir::tests

#endif
