#include "runir/kr/dl/semantics/builder.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"

#include <cassert>
#include <memory>
#include <runir/kr/database/syntax/query_repository.hpp>
#include <utility>
#include <yggdrasil/formalism/builder_instantiations.hpp>
#include <yggdrasil/formalism/keyed_impl.hpp>
#include <yggdrasil/formalism/part_instantiations.hpp>
#include <yggdrasil/formalism/symbol_repository.hpp>

namespace runir::kr::dl::semantics
{

/// Its stores: its symbols, and the pools of its concept and role denotations.
struct DenotationRepository::Impl
{
    Symbols symbols;
    DenotationPool<ConceptTag> concepts;
    DenotationPool<RoleTag> roles;
};

DenotationRepository::DenotationRepository(std::shared_ptr<const tyr::formalism::Repository> formalism_repository) :
    NestsBase(std::move(formalism_repository)),
    m_impl(std::make_unique<Impl>()),
    m_relation_repository(),
    m_index(ygg::next_layer())
{ this->reset(); }

DenotationRepository::~DenotationRepository() = default;

void DenotationRepository::reset() noexcept
{
    m_impl->symbols.clear();
    m_impl->concepts.clear();
    m_impl->roles.clear();
    m_relation_repository.clear();
}

auto DenotationRepository::own_parts() const -> OwnParts<false>
{ return { { *this, m_impl->symbols }, { *this, m_impl->concepts }, { *this, m_impl->roles } }; }

auto DenotationRepository::own_parts() -> OwnParts<true> { return { { *this, m_impl->symbols }, { *this, m_impl->concepts }, { *this, m_impl->roles } }; }

}  // namespace runir::kr::dl::semantics

// The parts and builders of the library's repositories, which its other sources only declare.
YGG_FORMALISM_PART(, runir::kr::dl::semantics::DenotationRepository, 0, 4)
YGG_FORMALISM_PART(, runir::kr::dl::semantics::DenotationRepository, 1, 1)
YGG_FORMALISM_PART(, runir::kr::dl::semantics::DenotationRepository, 2, 1)
YGG_FORMALISM_BUILDER(, runir::kr::dl::semantics::RecordStorage, 6)
YGG_FORMALISM_BUILDER(, runir::kr::dl::semantics::RelationStorage, 4)
