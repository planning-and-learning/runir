#include "module.hpp"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <filesystem>
#include <fstream>
#include <memory>
#include <nanobind/stl/filesystem.h>
#include <nanobind/stl/shared_ptr.h>
#include <runir/datasets/serialization.hpp>
#include <stdexcept>

namespace runir::datasets
{

using namespace nanobind::literals;

namespace
{

template<typename Archive>
void write_archive(const std::filesystem::path& archive_filepath, Archive archive)
{
    auto file = std::ofstream(archive_filepath);
    if (!file)
        throw std::runtime_error("Could not open archive file for writing: " + archive_filepath.string() + ".");

    auto out = boost::archive::text_oarchive(file);
    out << archive;
}

template<typename Archive>
auto read_archive(const std::filesystem::path& archive_filepath) -> Archive
{
    auto file = std::ifstream(archive_filepath);
    if (!file)
        throw std::runtime_error("Could not open archive file for reading: " + archive_filepath.string() + ".");

    auto archive = Archive {};
    auto in = boost::archive::text_iarchive(file);
    in >> archive;
    return archive;
}

template<tyr::planning::TaskKind Kind>
void bind_serialization_for_kind(nb::module_& m, const char* function_prefix)
{
    using Graph = StateGraph<Kind>;
    using AnnotatedGraph = AnnotatedStateGraph<Kind>;
    using Context = TaskSearchContext<Kind>;

    m.def((std::string("save_") + function_prefix + "_state_graph").c_str(),
          [](const std::filesystem::path& archive_filepath, const std::filesystem::path& task_filepath, const Graph& graph)
          { write_archive(archive_filepath, serialization::save(task_filepath, graph)); },
          "archive_filepath"_a,
          "task_filepath"_a,
          "graph"_a);

    m.def((std::string("load_") + function_prefix + "_state_graph").c_str(),
          [](const std::filesystem::path& archive_filepath, Context& context)
          {
              auto archive = read_archive<serialization::StateGraphArchive>(archive_filepath);
              return std::shared_ptr<Graph>(serialization::deserialize(context, archive));
          },
          "archive_filepath"_a,
          "context"_a);

    m.def((std::string("save_") + function_prefix + "_annotated_state_graph").c_str(),
          [](const std::filesystem::path& archive_filepath, const std::filesystem::path& task_filepath, const AnnotatedGraph& graph)
          { write_archive(archive_filepath, serialization::save(task_filepath, graph)); },
          "archive_filepath"_a,
          "task_filepath"_a,
          "graph"_a);

    m.def((std::string("load_") + function_prefix + "_annotated_state_graph").c_str(),
          [](const std::filesystem::path& archive_filepath, Context& context)
          {
              auto archive = read_archive<serialization::AnnotatedStateGraphArchive>(archive_filepath);
              return std::shared_ptr<AnnotatedGraph>(serialization::deserialize(context, archive));
          },
          "archive_filepath"_a,
          "context"_a);
}

}  // namespace

void bind_serialization(nb::module_& m)
{
    bind_serialization_for_kind<tyr::planning::GroundTag>(m, "ground");
    bind_serialization_for_kind<tyr::planning::LiftedTag>(m, "lifted");
}

}  // namespace runir::datasets
