#ifndef RUNIR_KR_DL_SEMANTICS_EVALUATION_STORAGE_HPP_
#define RUNIR_KR_DL_SEMANTICS_EVALUATION_STORAGE_HPP_

#include "runir/kr/dl/semantics/denotation_caches.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"

namespace runir::kr::dl::semantics
{

/// Owns reusable static/dynamic result repositories and matching memoization.
template<FamilyTag Family>
class EvaluationStorage
{
    DenotationRepositoryPtr m_static_denotations;
    DenotationRepositoryPtr m_dynamic_denotations;
    DenotationCaches<Family> m_caches;

public:
    explicit EvaluationStorage(const DenotationRepository& prototype) :
        m_static_denotations(prototype.get_factory().create_shared(prototype.get_formalism_repository_ptr())),
        m_dynamic_denotations(prototype.get_factory().create_shared(prototype.get_formalism_repository_ptr()))
    {
    }

    EvaluationStorage(const EvaluationStorage&) = delete;
    EvaluationStorage& operator=(const EvaluationStorage&) = delete;

    auto& get_caches() noexcept { return m_caches; }
    const auto& get_caches() const noexcept { return m_caches; }
    auto& get_denotation_repository(bool is_static) noexcept { return *(is_static ? m_static_denotations : m_dynamic_denotations); }
    const auto& get_denotation_repository(bool is_static) const noexcept { return *(is_static ? m_static_denotations : m_dynamic_denotations); }

    void reset_dynamic() noexcept
    {
        m_caches.reset_dynamic();
        m_dynamic_denotations->clear();
    }

    void reset_all() noexcept
    {
        m_caches.reset_all();
        m_dynamic_denotations->clear();
        m_static_denotations->clear();
    }
};

}  // namespace runir::kr::dl::semantics

#endif
