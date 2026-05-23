#ifndef RUNIR_KR_DL_SEMANTICS_EXT_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_DL_SEMANTICS_EXT_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/ext/register_index.hpp"
#include "runir/kr/dl/semantics/evaluation_context.hpp"

#include <array>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace runir::kr::dl::semantics
{

template<tyr::planning::TaskKind Kind>
class EvaluationContext<runir::kr::ExtFamilyTag, Kind> : public BaseEvaluationContext<EvaluationContext<runir::kr::ExtFamilyTag, Kind>, Kind>
{
public:
    using Self = EvaluationContext<runir::kr::ExtFamilyTag, Kind>;

    template<CategoryTag Category>
    using Element = DenotationElement<Category, Self>;

    template<CategoryTag Category>
    using Registers = std::array<std::optional<Element<Category>>, runir::kr::dl::num_registers>;

private:
    using Base = BaseEvaluationContext<EvaluationContext<runir::kr::ExtFamilyTag, Kind>, Kind>;

    Registers<ConceptTag> m_concept_registers;
    Registers<RoleTag> m_role_registers;

public:
    EvaluationContext(tyr::planning::StateView<Kind> state, Builder& builder, DenotationRepository& denotation_repository) noexcept :
        Base(std::move(state), builder, denotation_repository),
        m_concept_registers(),
        m_role_registers()
    {
    }

    template<CategoryTag Category>
        requires(std::same_as<Category, ConceptTag> || std::same_as<Category, RoleTag>)
    auto& registers() noexcept
    {
        if constexpr (std::same_as<Category, ConceptTag>)
            return m_concept_registers;
        else if constexpr (std::same_as<Category, RoleTag>)
            return m_role_registers;
    }

    template<CategoryTag Category>
        requires(std::same_as<Category, ConceptTag> || std::same_as<Category, RoleTag>)
    const auto& registers() const noexcept
    {
        if constexpr (std::same_as<Category, ConceptTag>)
            return m_concept_registers;
        else if constexpr (std::same_as<Category, RoleTag>)
            return m_role_registers;
    }

    auto& at(RegisterIdentifier<ConceptTag> reg) { return registers<ConceptTag>()[verify_bounds(reg)]; }

    const auto& at(RegisterIdentifier<ConceptTag> reg) const { return registers<ConceptTag>()[verify_bounds(reg)]; }

    auto& at(RegisterIdentifier<RoleTag> reg) { return registers<RoleTag>()[verify_bounds(reg)]; }

    const auto& at(RegisterIdentifier<RoleTag> reg) const { return registers<RoleTag>()[verify_bounds(reg)]; }

    void set(RegisterIdentifier<ConceptTag> reg, Element<ConceptTag> element) { at(reg) = std::move(element); }

    void set(RegisterIdentifier<RoleTag> reg, Element<RoleTag> element) { at(reg) = std::move(element); }

    void clear(RegisterIdentifier<ConceptTag> reg) { at(reg).reset(); }

    void clear(RegisterIdentifier<RoleTag> reg) { at(reg).reset(); }

private:
    size_t verify_bounds(RegisterIdentifier<ConceptTag> reg) const
    {
        const auto index = static_cast<size_t>(tyr::uint_t(reg));
        if (index >= registers<ConceptTag>().size())
            throw std::out_of_range(make_out_of_range_message<ConceptTag>(index));
        return index;
    }

    size_t verify_bounds(RegisterIdentifier<RoleTag> reg) const
    {
        const auto index = static_cast<size_t>(tyr::uint_t(reg));
        if (index >= registers<RoleTag>().size())
            throw std::out_of_range(make_out_of_range_message<RoleTag>(index));
        return index;
    }

    template<CategoryTag Category>
    static std::string make_out_of_range_message(size_t index)
    {
        return std::string(Category::name) + " register identifier " + std::to_string(index) + " is out of range; max registers is "
               + std::to_string(runir::kr::dl::num_registers) + ".";
    }
};

}  // namespace runir::kr::dl::semantics

#endif
