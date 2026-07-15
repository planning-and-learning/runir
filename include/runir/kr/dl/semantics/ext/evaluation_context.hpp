#ifndef RUNIR_KR_DL_SEMANTICS_EXT_EVALUATION_CONTEXT_HPP_
#define RUNIR_KR_DL_SEMANTICS_EXT_EVALUATION_CONTEXT_HPP_

#include "runir/kr/dl/declarations.hpp"
#include "runir/kr/dl/register_index.hpp"
#include "runir/kr/dl/semantics/denotation_view.hpp"
#include "runir/kr/dl/semantics/evaluation_context.hpp"

#include <array>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <tuple>
#include <tyr/formalism/planning/repository.hpp>
#include <utility>
#include <yggdrasil/core/dependent_false.hpp>

namespace runir::kr::dl::semantics
{

struct Arguments
{
    std::span<const DenotationView<ConceptTag>> concept_arguments;
    std::span<const DenotationView<RoleTag>> role_arguments;
    std::span<const DenotationView<BooleanTag>> boolean_arguments;
    std::span<const DenotationView<NumericalTag>> numerical_arguments;

    Arguments() noexcept = default;

    template<typename Source>
    explicit Arguments(const Source& source) noexcept :
        concept_arguments(source.template get<ConceptTag>()),
        role_arguments(source.template get<RoleTag>()),
        boolean_arguments(source.template get<BooleanTag>()),
        numerical_arguments(source.template get<NumericalTag>())
    {
    }

    template<CategoryTag Category>
    const auto& get() const noexcept
    {
        if constexpr (std::same_as<Category, ConceptTag>)
            return concept_arguments;
        else if constexpr (std::same_as<Category, RoleTag>)
            return role_arguments;
        else if constexpr (std::same_as<Category, BooleanTag>)
            return boolean_arguments;
        else if constexpr (std::same_as<Category, NumericalTag>)
            return numerical_arguments;
        else
            static_assert(ygg::dependent_false<Category>::value, "unhandled DL argument category");
    }

    template<CategoryTag Category>
    const auto& at(ArgumentIdentifier<Category> arg) const
    {
        return get<Category>()[verify_bounds(arg)];
    }

private:
    template<CategoryTag Category>
    size_t verify_bounds(ArgumentIdentifier<Category> arg) const
    {
        const auto index = static_cast<size_t>(ygg::uint_t(arg));
        if (index >= get<Category>().size())
            throw std::out_of_range(make_out_of_range_message<Category>(index));
        return index;
    }

    template<CategoryTag Category>
    std::string make_out_of_range_message(size_t index) const
    {
        return std::string(Category::name) + " argument identifier " + std::to_string(index) + " is out of range; argument count is "
               + std::to_string(get<Category>().size()) + ".";
    }
};

struct Registers
{
    std::array<std::optional<tyr::formalism::planning::ObjectView>, runir::kr::dl::num_registers> concept_registers;
    std::array<std::optional<std::pair<tyr::formalism::planning::ObjectView, tyr::formalism::planning::ObjectView>>, runir::kr::dl::num_registers>
        role_registers;

    template<CategoryTag Category>
    auto& get() noexcept
    {
        if constexpr (std::same_as<Category, ConceptTag>)
            return concept_registers;
        else if constexpr (std::same_as<Category, RoleTag>)
            return role_registers;
        else
            static_assert(ygg::dependent_false<Category>::value, "unhandled DL register category");
    }

    template<CategoryTag Category>
    const auto& get() const noexcept
    {
        if constexpr (std::same_as<Category, ConceptTag>)
            return concept_registers;
        else if constexpr (std::same_as<Category, RoleTag>)
            return role_registers;
        else
            static_assert(ygg::dependent_false<Category>::value, "unhandled DL register category");
    }

    template<CategoryTag Category>
    auto& at(RegisterIdentifier<Category> reg)
    {
        return get<Category>()[verify_bounds(reg)];
    }

    template<CategoryTag Category>
    const auto& at(RegisterIdentifier<Category> reg) const
    {
        return get<Category>()[verify_bounds(reg)];
    }

    void set(RegisterIdentifier<ConceptTag> reg, tyr::formalism::planning::ObjectView object) { at(reg) = std::move(object); }

    void set(RegisterIdentifier<RoleTag> reg, tyr::formalism::planning::ObjectView source, tyr::formalism::planning::ObjectView target)
    {
        at(reg) = std::pair(std::move(source), std::move(target));
    }

    template<CategoryTag Category>
    void clear(RegisterIdentifier<Category> reg)
    {
        at(reg).reset();
    }

    auto identifying_members() const noexcept { return std::tie(concept_registers, role_registers); }

private:
    template<CategoryTag Category>
    size_t verify_bounds(RegisterIdentifier<Category> reg) const
    {
        const auto index = static_cast<size_t>(ygg::uint_t(reg));
        if (index >= get<Category>().size())
            throw std::out_of_range(make_out_of_range_message<Category>(index));
        return index;
    }

    template<CategoryTag Category>
    static std::string make_out_of_range_message(size_t index)
    {
        return std::string(Category::name) + " register identifier " + std::to_string(index) + " is out of range; max registers is "
               + std::to_string(runir::kr::dl::num_registers) + ".";
    }
};

template<tyr::planning::TaskKind Kind>
class EvaluationContext<runir::kr::ExtFamilyTag, Kind> : public BaseEvaluationContext<EvaluationContext<runir::kr::ExtFamilyTag, Kind>, Kind>
{
private:
    using Base = BaseEvaluationContext<EvaluationContext<runir::kr::ExtFamilyTag, Kind>, Kind>;

    Registers m_registers;
    Arguments m_arguments;

public:
    EvaluationContext(tyr::planning::StateView<Kind> state,
                      Builder& builder,
                      DenotationRepository& denotation_repository,
                      Arguments arguments = {},
                      Registers registers = {}) noexcept :
        Base(std::move(state), builder, denotation_repository),
        m_registers(std::move(registers)),
        m_arguments(arguments)
    {
    }

    auto& registers() noexcept { return m_registers; }
    const auto& registers() const noexcept { return m_registers; }
    const auto& arguments() const noexcept { return m_arguments; }
};

}  // namespace runir::kr::dl::semantics

#endif
