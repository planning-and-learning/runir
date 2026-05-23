#ifndef RUNIR_KR_MPG_REPOSITORY_HPP_
#define RUNIR_KR_MPG_REPOSITORY_HPP_

#include "runir/kr/dl/repository.hpp"
#include "runir/kr/gp/repository.hpp"
#include "runir/kr/mpg/memory_state_data.hpp"
#include "runir/kr/mpg/memory_state_view.hpp"
#include "runir/kr/mpg/condition_data.hpp"
#include "runir/kr/mpg/condition_view.hpp"
#include "runir/kr/mpg/dl/concept_data.hpp"
#include "runir/kr/mpg/dl/concept_view.hpp"
#include "runir/kr/mpg/dl/constructor_data.hpp"
#include "runir/kr/mpg/dl/constructor_view.hpp"
#include "runir/kr/mpg/dl/feature_data.hpp"
#include "runir/kr/mpg/dl/feature_view.hpp"
#include "runir/kr/mpg/effect_data.hpp"
#include "runir/kr/mpg/effect_view.hpp"
#include "runir/kr/mpg/feature_data.hpp"
#include "runir/kr/mpg/feature_view.hpp"
#include "runir/kr/mpg/module_data.hpp"
#include "runir/kr/mpg/module_view.hpp"
#include "runir/kr/mpg/register_data.hpp"
#include "runir/kr/mpg/register_view.hpp"
#include "runir/kr/mpg/rule_data.hpp"
#include "runir/kr/mpg/rule_view.hpp"
#include "runir/kr/mpg/rule_variant_data.hpp"
#include "runir/kr/mpg/rule_variant_view.hpp"

#include <cassert>
#include <memory>
#include <optional>
#include <tyr/common/type_list.hpp>
#include <tyr/common/types.hpp>
#include <tyr/formalism/symbol_repository.hpp>
#include <utility>

namespace runir::kr::mpg
{

using ConcreteRuleTypes = tyr::MapTypeListT<Rule, tyr::TypeList<SketchTag, LoadTag, DoTag, CallTag>>;
using RuleTypes = tyr::ConcatTypeListsT<tyr::TypeList<RuleVariant>, ConcreteRuleTypes>;
using ConstructorTypes = tyr::TypeList<dl::Concept<dl::RegisterTag>,
                                       dl::Constructor<runir::kr::dl::ConceptTag>,
                                       dl::Constructor<runir::kr::dl::RoleTag>,
                                       dl::Constructor<runir::kr::dl::BooleanTag>,
                                       dl::Constructor<runir::kr::dl::NumericalTag>>;
using FeatureTypes = tyr::TypeList<Feature<dl::ConceptFeature>,
                                   Feature<dl::BooleanFeature>,
                                   Feature<dl::NumericalFeature>,
                                   ConcreteFeature<runir::kr::DlTag, dl::ConceptFeature>,
                                   ConcreteFeature<runir::kr::DlTag, dl::BooleanFeature>,
                                   ConcreteFeature<runir::kr::DlTag, dl::NumericalFeature>>;
using ConditionTypes = tyr::TypeList<ConditionVariant>;
using EffectTypes = tyr::TypeList<EffectVariant>;
using ProgramTypes = tyr::TypeList<Register, MemoryState, Module>;
using RepositoryTypes =
    tyr::ConcatTypeListsT<runir::kr::gp::RepositoryTypes, ConstructorTypes, FeatureTypes, ConditionTypes, EffectTypes, RuleTypes, ProgramTypes>;
using SymbolRepository = tyr::ApplyTypeListT<tyr::formalism::SymbolRepository, RepositoryTypes>;

class Repository
{
    friend class RepositoryFactory;

private:
    SymbolRepository m_symbol_repository;
    runir::kr::dl::ConstructorRepositoryPtr m_dl_repository;
    size_t m_index;

    Repository(size_t index, runir::kr::dl::ConstructorRepositoryPtr dl_repository) :
        m_symbol_repository(nullptr),
        m_dl_repository(std::move(dl_repository)),
        m_index(index)
    {
        assert(m_dl_repository);
        clear();
    }

public:
    Repository(const Repository&) = delete;
    Repository& operator=(const Repository&) = delete;
    Repository(Repository&&) = delete;
    Repository& operator=(Repository&&) = delete;

    const auto& get_index() const noexcept { return m_index; }
    auto& get_dl_repository() noexcept { return *m_dl_repository; }
    const auto& get_dl_repository() const noexcept { return *m_dl_repository; }
    const auto& get_dl_repository_ptr() const noexcept { return m_dl_repository; }

    void clear() noexcept { m_symbol_repository.clear(); }

    template<typename T>
    std::optional<tyr::View<tyr::Index<T>, Repository>> find(const tyr::Data<T>& data) const noexcept
    {
        if (auto index = m_symbol_repository.template find_local<T>(data))
            return tyr::View<tyr::Index<T>, Repository>(*index, *this);
        return std::nullopt;
    }

    template<typename T>
    std::pair<tyr::View<tyr::Index<T>, Repository>, bool> get_or_create(tyr::Data<T>& data)
    {
        const auto [index, success] = m_symbol_repository.template get_or_create_local<T>(data);
        return { tyr::View<tyr::Index<T>, Repository>(index, *this), success };
    }

    template<typename T>
    const tyr::Data<T>& operator[](tyr::Index<T> index) const noexcept
    {
        return m_symbol_repository.template at_local<T>(index);
    }

    template<typename T>
    size_t size() const noexcept
    {
        return m_symbol_repository.template local_size<T>();
    }

    template<typename T>
    const Repository& get_canonical_context(tyr::Index<T>) const noexcept
    {
        return *this;
    }
};

using RepositoryPtr = std::shared_ptr<Repository>;

class RepositoryFactory
{
private:
    size_t m_next_index = 0;

public:
    Repository create(runir::kr::dl::ConstructorRepositoryPtr dl_repository) { return Repository(m_next_index++, std::move(dl_repository)); }
    RepositoryPtr create_shared(runir::kr::dl::ConstructorRepositoryPtr dl_repository)
    {
        return RepositoryPtr(new Repository(m_next_index++, std::move(dl_repository)));
    }
};

using RepositoryFactoryPtr = std::shared_ptr<RepositoryFactory>;

using RegisterView = tyr::View<tyr::Index<Register>, Repository>;
using MemoryStateView = tyr::View<tyr::Index<MemoryState>, Repository>;
using ModuleView = tyr::View<tyr::Index<Module>, Repository>;
using RuleVariantView = tyr::View<tyr::Index<RuleVariant>, Repository>;

template<RuleKind Kind>
using RuleView = tyr::View<tyr::Index<Rule<Kind>>, Repository>;

inline const Repository& get_repository(const Repository& repository) noexcept { return repository; }
inline Repository& get_repository(Repository& repository) noexcept { return repository; }

}  // namespace runir::kr::mpg

#endif
