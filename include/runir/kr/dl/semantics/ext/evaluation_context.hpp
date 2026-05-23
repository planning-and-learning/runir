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
    template<CategoryTag Category>
        requires(std::same_as<Category, ConceptTag> || std::same_as<Category, RoleTag>)
    using Register = tyr::Index<std::conditional_t<std::same_as<Category, ConceptTag>,
                                                   Concept<runir::kr::ExtFamilyTag, RegisterIdentifierTag>,
                                                   Role<runir::kr::ExtFamilyTag, RegisterIdentifierTag>>>;

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

    template<CategoryTag Category>
        requires(std::same_as<Category, ConceptTag> || std::same_as<Category, RoleTag>)
    void set(Register<Category> reg, Element<Category> element)
    {
        registers<Category>()[verify_bounds<Category>(reg)] = std::move(element);
    }

    template<CategoryTag Category>
        requires(std::same_as<Category, ConceptTag> || std::same_as<Category, RoleTag>)
    void clear(Register<Category> reg)
    {
        registers<Category>()[verify_bounds<Category>(reg)].reset();
    }

    template<CategoryTag Category>
        requires(std::same_as<Category, ConceptTag> || std::same_as<Category, RoleTag>)
    Element<Category> get(Register<Category> reg) const
    {
        const auto index = verify_bounds<Category>(reg);
        return verify_set<Category>(index);
    }

private:
    template<CategoryTag Category>
    size_t verify_bounds(Register<Category> reg) const
    {
        const auto index = static_cast<size_t>(tyr::uint_t(reg));
        if (index >= registers<Category>().size())
            throw std::out_of_range(make_out_of_range_message<Category>(index));
        return index;
    }

    template<CategoryTag Category>
    Element<Category> verify_set(size_t index) const
    {
        const auto& register_array = registers<Category>();
        if (!register_array[index])
            throw std::runtime_error(make_unset_message<Category>(index));
        return *register_array[index];
    }

    template<CategoryTag Category>
    static std::string make_unset_message(size_t index)
    {
        return std::string(Category::name) + " denotation register " + std::to_string(index) + " is not set.";
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
