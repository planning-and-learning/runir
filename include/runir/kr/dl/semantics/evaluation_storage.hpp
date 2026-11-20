#ifndef RUNIR_KR_DL_SEMANTICS_EVALUATION_STORAGE_HPP_
#define RUNIR_KR_DL_SEMANTICS_EVALUATION_STORAGE_HPP_

#include "runir/kr/dl/semantics/denotation_caches.hpp"
#include "runir/kr/dl/semantics/denotation_repository.hpp"
#include "runir/kr/dl/semantics/register_values_builder.hpp"

namespace runir::kr::dl::semantics
{

/// Owns reusable static/dynamic result repositories and matching memoization.
class EvaluationStorage
{
    DenotationRepositoryPtr m_static_denotations;
    DenotationRepositoryPtr m_dynamic_denotations;
    DenotationCaches m_caches;
    /// The builder a context evaluated with this storage reads registers given in another form (interned) from: copied
    /// in, its buffers retained.
    urd::builder<RegisterValues, runir::kr::dl::semantics::DenotationRepository> m_registers;

public:
    explicit EvaluationStorage(const DenotationRepository& prototype) :
        m_static_denotations(std::make_shared<DenotationRepository>(prototype.template get_ptr<tyr::formalism::planning::Repository>())),
        m_dynamic_denotations(std::make_shared<DenotationRepository>(prototype.template get_ptr<tyr::formalism::planning::Repository>()))
    {
    }

    EvaluationStorage(const EvaluationStorage&) = delete;
    EvaluationStorage& operator=(const EvaluationStorage&) = delete;
    EvaluationStorage(EvaluationStorage&&) = delete;
    EvaluationStorage& operator=(EvaluationStorage&&) = delete;
    ~EvaluationStorage() = default;

    auto& get_caches() noexcept { return m_caches; }

    auto& get_registers() noexcept { return m_registers; }

    [[nodiscard]] const auto& get_caches() const noexcept { return m_caches; }

    auto& get_denotation_repository(bool is_static) noexcept { return *(is_static ? m_static_denotations : m_dynamic_denotations); }

    [[nodiscard]] const auto& get_denotation_repository(bool is_static) const noexcept { return *(is_static ? m_static_denotations : m_dynamic_denotations); }

    void reset_dynamic() noexcept
    {
        m_caches.reset_dynamic();
        m_dynamic_denotations->reset();
    }

    void reset_all() noexcept
    {
        m_caches.reset_all();
        m_dynamic_denotations->reset();
        m_static_denotations->reset();
    }
};

}  // namespace runir::kr::dl::semantics

#endif
