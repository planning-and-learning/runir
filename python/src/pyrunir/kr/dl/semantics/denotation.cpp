#include "bindings.hpp"
#include "pyrunir/module.hpp"

#include <concepts>
#include <cstddef>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/tuple.h>
#include <tuple>
#include <optional>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/denotation_view.hpp>
#include <runir/kr/dl/semantics/formatter.hpp>
#include <tyr/formalism/object_view.hpp>
#include <urd/python/bindings.hpp>
#include <urd/python/containers/bitset_span.hpp>
#include <urd/python/owner.hpp>
#include <vector>

namespace runir::kr::dl
{
namespace
{

/// The view of the denotation of Category (its record is bound first, bind_data); a concept's or role's denotation also gives its objects (or
/// object pairs) as the views it holds, and a boolean's or numerical's its value.
template<CategoryTag Category>
void bind_denotation_view(nb::module_& m)
{
    using View = urd::index_view<runir::kr::dl::semantics::Denotation<Category>, runir::kr::dl::semantics::DenotationRepository>;
    urd::python::bind_view<View>(m);
}

}  // namespace

void bind_semantics_denotation(nb::module_& m)
{
    urd::python::bind_data<semantics::Denotation<BooleanTag>>(m);
    urd::python::bind_data<semantics::Denotation<NumericalTag>>(m);
    urd::python::bind_data<semantics::Denotation<ConceptTag>>(m);
    urd::python::bind_data<semantics::Denotation<RoleTag>>(m);
    bind_denotation_view<BooleanTag>(m);
    bind_denotation_view<NumericalTag>(m);
    bind_denotation_view<ConceptTag>(m);
    bind_denotation_view<RoleTag>(m);

    // Diagnostic only.
    m.def("diag_pairs",
          [](unsigned n)
          {
              using Bits = urd::bitset_span<const ygg::uint_t>;
              const auto row_blocks = Bits::num_blocks(n);
              std::vector<ygg::uint_t> blocks(row_blocks * n, ygg::uint_t((1u << n) - 1u));
              const auto bits = Bits(blocks.data(), blocks.size() * Bits::Digits);
              const auto computed = static_cast<std::size_t>(Bits::num_blocks(n) * Bits::Digits);
              const auto range = semantics::detail::object_pair_indices(bits, n);
              const std::size_t stored = range.function().context;
              std::vector<std::pair<unsigned, unsigned>> out;
              for (const auto& [a, b] : range)
                  out.emplace_back(unsigned(a.value), unsigned(b.value));
              return std::make_tuple(computed, stored, out);
          });
    m.def("diag_view",
          [](const urd::index_view<semantics::Denotation<RoleTag>, semantics::DenotationRepository>& view)
          {
              const auto range = view.indices();
              const std::size_t n = view.get_num_objects();
              const std::size_t stored = range.function().context;
              std::vector<std::pair<unsigned, unsigned>> out;
              for (const auto& [a, b] : range)
                  out.emplace_back(unsigned(a.value), unsigned(b.value));
              return std::make_tuple(n, stored, out);
          });
}

}  // namespace runir::kr::dl
