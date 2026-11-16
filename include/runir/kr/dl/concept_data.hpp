#ifndef RUNIR_KR_DL_CONCEPT_DATA_HPP_
#define RUNIR_KR_DL_CONCEPT_DATA_HPP_

#include "runir/kr/dl/declarations.hpp"

#include <tyr/formalism/declarations.hpp>
#include <urd/elements/flat.hpp>
#include <utility>
#include <yggdrasil/core/config.hpp>

namespace urd
{

template<typename F, runir::kr::dl::NullaryConceptTag Tag>
struct data<runir::kr::dl::Concept<F, Tag>>
{
    urd::index<runir::kr::dl::Concept<F, Tag>> index;

    data() = default;
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<> {}; }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index> {}; }
};

template<typename F, typename Tag>
    requires runir::kr::dl::is_atomic_state_tag_v<Tag>
struct data<runir::kr::dl::Concept<F, Tag>>
{
    urd::index<runir::kr::dl::Concept<F, Tag>> index;
    urd::index<tyr::formalism::Predicate<runir::kr::dl::TagFactKind<Tag>>> predicate;

    data() = default;
    explicit data(urd::index<tyr::formalism::Predicate<runir::kr::dl::TagFactKind<Tag>>> predicate_) : index(), predicate(std::move(predicate_)) {}
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"predicate", &data::predicate>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::predicate> {}; }
};

template<typename F, typename Tag>
    requires runir::kr::dl::is_atomic_goal_tag_v<Tag>
struct data<runir::kr::dl::Concept<F, Tag>>
{
    urd::index<runir::kr::dl::Concept<F, Tag>> index;
    urd::index<tyr::formalism::Predicate<runir::kr::dl::TagFactKind<Tag>>> predicate;
    bool polarity = false;

    data() = default;
    data(urd::index<tyr::formalism::Predicate<runir::kr::dl::TagFactKind<Tag>>> predicate_, bool polarity_) :
        index(),
        predicate(std::move(predicate_)),
        polarity(std::move(polarity_))
    {
    }
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"predicate", &data::predicate>, urd::field<"polarity", &data::polarity>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::predicate, &data::polarity> {}; }
};

template<typename F, runir::kr::dl::ConceptBinaryTag Tag>
struct data<runir::kr::dl::Concept<F, Tag>>
{
    urd::index<runir::kr::dl::Concept<F, Tag>> index;
    runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag> left_operand;
    runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag> right_operand;

    data() = default;
    data(runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag> left_operand_, runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag> right_operand_) :
        index(),
        left_operand(std::move(left_operand_)),
        right_operand(std::move(right_operand_))
    {
    }
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"left_operand", &data::left_operand>, urd::field<"right_operand", &data::right_operand>> {};
    }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::left_operand, &data::right_operand> {}; }
};

template<typename F>
struct data<runir::kr::dl::Concept<F, runir::kr::dl::NegationTag>>
{
    urd::index<runir::kr::dl::Concept<F, runir::kr::dl::NegationTag>> index;
    runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag> operand;

    data() = default;
    explicit data(runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag> operand_) : index(), operand(std::move(operand_)) {}
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"operand", &data::operand>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::operand> {}; }
};

template<typename F, runir::kr::dl::QuantificationTag Tag>
struct data<runir::kr::dl::Concept<F, Tag>>
{
    urd::index<runir::kr::dl::Concept<F, Tag>> index;
    runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag> role;
    runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag> qualifier;

    data() = default;
    data(runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag> role_, runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag> qualifier_) :
        index(),
        role(std::move(role_)),
        qualifier(std::move(qualifier_))
    {
    }
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"role", &data::role>, urd::field<"qualifier", &data::qualifier>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::role, &data::qualifier> {}; }
};

template<typename F, runir::kr::dl::NumberRestrictionTag Tag>
struct data<runir::kr::dl::Concept<F, Tag>>
{
    urd::index<runir::kr::dl::Concept<F, Tag>> index;
    ygg::uint_t n = 0;
    runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag> role;

    data() = default;
    data(ygg::uint_t n_, runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag> role_) : index(), n(std::move(n_)), role(std::move(role_)) {}
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"n", &data::n>, urd::field<"role", &data::role>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::n, &data::role> {}; }
};

template<typename F, runir::kr::dl::QualifiedNumberRestrictionTag Tag>
struct data<runir::kr::dl::Concept<F, Tag>>
{
    urd::index<runir::kr::dl::Concept<F, Tag>> index;
    ygg::uint_t n = 0;
    runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag> role;
    runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag> qualifier;

    data() = default;
    data(ygg::uint_t n_, runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag> role_, runir::kr::dl::operand_t<F, runir::kr::dl::ConceptTag> qualifier_) :
        index(),
        n(std::move(n_)),
        role(std::move(role_)),
        qualifier(std::move(qualifier_))
    {
    }
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"n", &data::n>, urd::field<"role", &data::role>, urd::field<"qualifier", &data::qualifier>> {};
    }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::n, &data::role, &data::qualifier> {}; }
};

template<typename F, runir::kr::dl::RoleComparisonTag Tag>
struct data<runir::kr::dl::Concept<F, Tag>>
{
    urd::index<runir::kr::dl::Concept<F, Tag>> index;
    runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag> left_operand;
    runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag> right_operand;

    data() = default;
    data(runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag> left_operand_, runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag> right_operand_) :
        index(),
        left_operand(std::move(left_operand_)),
        right_operand(std::move(right_operand_))
    {
    }
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept
    {
        return urd::Fields<urd::field<"left_operand", &data::left_operand>, urd::field<"right_operand", &data::right_operand>> {};
    }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::left_operand, &data::right_operand> {}; }
};

template<typename F>
struct data<runir::kr::dl::Concept<F, runir::kr::dl::RoleFillersTag>>
{
    urd::index<runir::kr::dl::Concept<F, runir::kr::dl::RoleFillersTag>> index;
    runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag> role;
    urd::flat_vector<urd::index<tyr::formalism::Object>> objects;

    data() = default;
    data(runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag> role_, urd::flat_vector<urd::index<tyr::formalism::Object>> objects_) :
        index(),
        role(std::move(role_)),
        objects(std::move(objects_))
    {
    }
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"role", &data::role>, urd::field<"objects", &data::objects>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::role, &data::objects> {}; }
};

template<typename F>
struct data<runir::kr::dl::Concept<F, runir::kr::dl::OneOfTag>>
{
    urd::index<runir::kr::dl::Concept<F, runir::kr::dl::OneOfTag>> index;
    urd::flat_vector<urd::index<tyr::formalism::Object>> objects;

    data() = default;
    explicit data(urd::flat_vector<urd::index<tyr::formalism::Object>> objects_) : index(), objects(std::move(objects_)) {}
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"objects", &data::objects>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::objects> {}; }
};

template<typename F>
struct data<runir::kr::dl::Concept<F, runir::kr::dl::NominalTag>>
{
    urd::index<runir::kr::dl::Concept<F, runir::kr::dl::NominalTag>> index;
    urd::index<tyr::formalism::Object> object;

    data() = default;
    explicit data(urd::index<tyr::formalism::Object> object_) : index(), object(std::move(object_)) {}
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"object", &data::object>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::object> {}; }
};

template<typename F>
struct data<runir::kr::dl::Concept<F, runir::kr::dl::RegisterTag>>
{
    urd::index<runir::kr::dl::Concept<F, runir::kr::dl::RegisterTag>> index;
    urd::index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>> slot;

    data() = default;
    explicit data(urd::index<runir::kr::dl::Register<runir::kr::dl::ConceptTag>> slot_) : index(), slot(std::move(slot_)) {}
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"slot", &data::slot>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::slot> {}; }
};

template<typename F>
struct data<runir::kr::dl::Concept<F, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>>
{
    urd::index<runir::kr::dl::Concept<F, runir::kr::dl::ArgumentTag<runir::kr::dl::ConceptTag>>> index;
    urd::index<runir::kr::dl::Argument<runir::kr::dl::ConceptTag>> argument;

    data() = default;
    explicit data(urd::index<runir::kr::dl::Argument<runir::kr::dl::ConceptTag>> argument_) : index(), argument(std::move(argument_)) {}
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"argument", &data::argument>> {}; }
    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }
    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::argument> {}; }
};

}  // namespace urd

#endif
