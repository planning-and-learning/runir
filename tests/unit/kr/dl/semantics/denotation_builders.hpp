#ifndef RUNIR_TESTS_KR_DL_SEMANTICS_DENOTATION_BUILDERS_HPP_
#define RUNIR_TESTS_KR_DL_SEMANTICS_DENOTATION_BUILDERS_HPP_

#include <memory>
#include <runir/kr/dl/semantics/builder.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/interning.hpp>
#include <tyr/formalism/repository.hpp>
#include <urd/elements/builder.hpp>
#include <utility>
#include <variant>
#include <yggdrasil/core/config.hpp>

namespace runir::tests
{

/// An empty builder of a concept's or role's denotation over num_objects objects, its words drawn from `storage`.
template<typename T>
urd::builder<T, runir::kr::dl::semantics::DenotationRepository> make_denotation(ygg::uint_t num_objects, runir::kr::dl::semantics::Builder& storage)
{
    auto builder = urd::builder<T, runir::kr::dl::semantics::DenotationRepository> {};
    runir::kr::dl::semantics::initialize(builder, num_objects, storage.records());
    return builder;
}

/// A copy of a denotation builder (builders own their words, so are not copyable): its values, words of its own.
template<typename T>
urd::builder<T, runir::kr::dl::semantics::DenotationRepository> copy_denotation(const urd::builder<T, runir::kr::dl::semantics::DenotationRepository>& source,
                                                                                runir::kr::dl::semantics::Builder& storage,
                                                                                const runir::kr::dl::semantics::DenotationRepository& repository)
{
    auto copy = urd::builder<T, runir::kr::dl::semantics::DenotationRepository> {};
    runir::kr::dl::semantics::assign(copy, urd::resolve(source, repository), storage.records());
    copy.index = source.index;
    return copy;
}

/// The denotation repository a test's builders are read through, over a planning repository, and the storage their
/// words are drawn from (declared first: the builders give their words back to it).
struct Words
{
    runir::kr::dl::semantics::Builder storage;
    std::shared_ptr<tyr::formalism::Repository> planning;
    runir::kr::dl::semantics::DenotationRepository repository;

    explicit Words(std::shared_ptr<tyr::formalism::Repository> planning_ = std::make_shared<tyr::formalism::Repository>()) :
        storage(),
        planning(planning_),
        repository(std::move(planning_))
    {
    }

    /// An empty builder of a concept's or role's denotation over num_objects objects.
    template<typename T>
    urd::builder<T, runir::kr::dl::semantics::DenotationRepository> make(ygg::uint_t num_objects)
    { return make_denotation<T>(num_objects, storage); }

    /// The builder read: its view through the repository (a builder's reads are its view's).
    template<typename T>
    [[nodiscard]] auto view(const urd::builder<T, runir::kr::dl::semantics::DenotationRepository>& builder) const
    { return urd::resolve(builder, repository); }

    /// The builder interned.
    template<typename T>
    auto insert(const urd::builder<T, runir::kr::dl::semantics::DenotationRepository>& builder)
    { return repository.insert(builder, storage.records()).first; }
};

/// A builder's own words (initialize gave them).
template<typename B>
const auto& own_words(const B& builder)
{ return builder.values.words; }

}  // namespace runir::tests

#endif
