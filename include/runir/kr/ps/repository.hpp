#ifndef RUNIR_KR_PS_REPOSITORY_HPP_
#define RUNIR_KR_PS_REPOSITORY_HPP_

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/ps/declarations.hpp"
#include "runir/kr/ps/family_traits.hpp"

#include <boost/mp11.hpp>
#include <cassert>
#include <memory>
#include <urd/config.hpp>
#include <urd/elements/flat.hpp>
#include <utility>
#include <yggdrasil/formalism/nested.hpp>
#include <yggdrasil/formalism/symbol_context.hpp>

namespace runir::kr::ps
{

/// A program family's records, over the DL repository whose records its rules refer to (nested, interned into). Derived
/// is the family's own repository class (its Repository): the context of its records' views and builders, so that their
/// names spell its name instead of every one of its types.
template<typename Derived, FamilyTag F, typename Types>
class BasicRepository :
    public ygg::formalism::SymbolContext<Derived, Types>,
    public ygg::formalism::Nests<Derived, ygg::formalism::Nested<runir::kr::dl::ConstructorRepository>>
{
    using Base = ygg::formalism::SymbolContext<Derived, Types>;
    using NestsBase = ygg::formalism::Nests<Derived, ygg::formalism::Nested<runir::kr::dl::ConstructorRepository>>;

public:
    /// The family whose records it holds, and their types (FamilyRepository).
    using Family = F;
    using RepositoryTypes = Types;
    using DlRepositoryPtr = runir::kr::dl::ConstructorRepositoryPtr;

private:
    size_t m_index;

    /// Constructed only as its Derived, which forwards to it (a CRTP base, not a class of its own).
    friend Derived;

    /// Over the DL repository whose records its rules refer to. Numbered by ygg::next_layer(), the process-wide counter,
    /// so that no two contexts share a number.
    explicit BasicRepository(DlRepositoryPtr dl_repository) : Base(), NestsBase(std::move(dl_repository)), m_index(ygg::next_layer()) { this->clear(); }

public:
    BasicRepository(const BasicRepository&) = delete;
    BasicRepository& operator=(const BasicRepository&) = delete;
    BasicRepository(BasicRepository&&) = delete;
    BasicRepository& operator=(BasicRepository&&) = delete;
    ~BasicRepository() = default;

    /// The repository's number, as urd's views ask of a context.
    [[nodiscard]] std::size_t layer() const noexcept { return m_index; }

    using Base::find;
    using Base::insert;
    using Base::view;
    using NestsBase::get;
    using NestsBase::get_ptr;
};

}  // namespace runir::kr::ps

#endif
