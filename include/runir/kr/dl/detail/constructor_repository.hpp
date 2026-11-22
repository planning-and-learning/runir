#ifndef RUNIR_KR_DL_DETAIL_CONSTRUCTOR_REPOSITORY_HPP_
#define RUNIR_KR_DL_DETAIL_CONSTRUCTOR_REPOSITORY_HPP_

#include "tyr/formalism/repository.hpp"
#include "runir/kr/dl/declarations.hpp"

#include <boost/mp11.hpp>
#include <cassert>
#include <memory>
#include <runir/kr/database/datalog/repository.hpp>
#include <tyr/formalism/declarations.hpp>
#include <urd/config.hpp>
#include <urd/elements/flat.hpp>
#include <utility>
#include <yggdrasil/formalism/nested.hpp>
#include <yggdrasil/formalism/symbol_context.hpp>

namespace runir::kr::dl::detail
{

/// Constructors, over the planning repository whose symbols their records refer to (nested, read only).
template<typename RepositoryTypes>
class BasicConstructorRepository :
    public ygg::formalism::SymbolContext<BasicConstructorRepository<RepositoryTypes>, RepositoryTypes>,
    public ygg::formalism::Nests<BasicConstructorRepository<RepositoryTypes>, ygg::formalism::Nested<const tyr::formalism::Repository>>
{
    using Base = ygg::formalism::SymbolContext<BasicConstructorRepository<RepositoryTypes>, RepositoryTypes>;
    using NestsBase = ygg::formalism::Nests<BasicConstructorRepository<RepositoryTypes>, ygg::formalism::Nested<const tyr::formalism::Repository>>;

private:
    /// The rules of the repository's DatalogPrograms.
    std::unique_ptr<runir::kr::database::datalog::DatalogRepository> m_datalog_repository;
    size_t m_index;

public:
    /// Over the planning repository whose symbols its records refer to. Numbered by ygg::next_layer(), the process-wide
    /// counter, so that no two contexts share a number.
    explicit BasicConstructorRepository(std::shared_ptr<const tyr::formalism::Repository> planning_repository) :
        Base(),
        NestsBase(std::move(planning_repository)),
        m_datalog_repository(std::make_unique<runir::kr::database::datalog::DatalogRepository>()),
        m_index(ygg::next_layer())
    { this->clear(); }

    BasicConstructorRepository(const BasicConstructorRepository&) = delete;
    BasicConstructorRepository& operator=(const BasicConstructorRepository&) = delete;
    BasicConstructorRepository(BasicConstructorRepository&&) = delete;
    BasicConstructorRepository& operator=(BasicConstructorRepository&&) = delete;
    ~BasicConstructorRepository() = default;

    /// The repository's number, as urd's views ask of a context.
    [[nodiscard]] std::size_t layer() const noexcept { return m_index; }

    [[nodiscard]] const runir::kr::database::datalog::DatalogRepository& get_datalog_repository() const noexcept { return *m_datalog_repository; }

    [[nodiscard]] runir::kr::database::datalog::DatalogRepository& get_datalog_repository() noexcept { return *m_datalog_repository; }

    using Base::find;
    using Base::insert;
    using Base::view;
    using NestsBase::get;
    using NestsBase::get_ptr;
};

template<typename RepositoryTypes>
inline const BasicConstructorRepository<RepositoryTypes>&
get_repository(const BasicConstructorRepository<RepositoryTypes>& repository URD_LIFETIMEBOUND) noexcept
{ return repository; }

template<typename RepositoryTypes>
inline BasicConstructorRepository<RepositoryTypes>& get_repository(BasicConstructorRepository<RepositoryTypes>& repository) noexcept
{ return repository; }

}  // namespace runir::kr::dl::detail

#endif
