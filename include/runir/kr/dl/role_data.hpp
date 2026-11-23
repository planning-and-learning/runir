#ifndef RUNIR_KR_DL_ROLE_DATA_HPP_
#define RUNIR_KR_DL_ROLE_DATA_HPP_

#include "runir/kr/dl/concept_data.hpp"

#include <urd/elements/flat.hpp>
#include <utility>

namespace urd
{

template<typename F>
struct data<runir::kr::dl::Role<F, runir::kr::dl::UniversalTag>>
{
    urd::key<runir::kr::dl::Role<F, runir::kr::dl::UniversalTag>> index;

    data() = default;
    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index> {}; }

    [[nodiscard]] urd::key<runir::kr::dl::Role<F, runir::kr::dl::UniversalTag>> key() const noexcept
    { return urd::key<runir::kr::dl::Role<F, runir::kr::dl::UniversalTag>>(index); }
};

template<typename F, typename Tag>
    requires runir::kr::dl::is_atomic_state_tag_v<Tag>
struct data<runir::kr::dl::Role<F, Tag>>
{
    urd::key<runir::kr::dl::Role<F, Tag>> index;
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

    [[nodiscard]] urd::key<runir::kr::dl::Role<F, Tag>> key() const noexcept { return urd::key<runir::kr::dl::Role<F, Tag>>(index); }
};

template<typename F, typename Tag>
    requires runir::kr::dl::is_atomic_goal_tag_v<Tag>
struct data<runir::kr::dl::Role<F, Tag>>
{
    urd::key<runir::kr::dl::Role<F, Tag>> index;
    urd::index<tyr::formalism::Predicate<runir::kr::dl::TagFactKind<Tag>>> predicate;
    bool polarity = false;

    data() = default;

    data(urd::index<tyr::formalism::Predicate<runir::kr::dl::TagFactKind<Tag>>> predicate_, bool polarity_) :
        index(),
        predicate(std::move(predicate_)),
        polarity(polarity_)
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

    [[nodiscard]] urd::key<runir::kr::dl::Role<F, Tag>> key() const noexcept { return urd::key<runir::kr::dl::Role<F, Tag>>(index); }
};

template<typename F, runir::kr::dl::RoleBinaryTag Tag>
struct data<runir::kr::dl::Role<F, Tag>>
{
    urd::key<runir::kr::dl::Role<F, Tag>> index;
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
    { return urd::Fields<urd::field<"left_operand", &data::left_operand>, urd::field<"right_operand", &data::right_operand>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::left_operand, &data::right_operand> {}; }

    [[nodiscard]] urd::key<runir::kr::dl::Role<F, Tag>> key() const noexcept { return urd::key<runir::kr::dl::Role<F, Tag>>(index); }
};

template<typename F, runir::kr::dl::RoleUnaryTag Tag>
struct data<runir::kr::dl::Role<F, Tag>>
{
    urd::key<runir::kr::dl::Role<F, Tag>> index;
    runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag> operand;

    data() = default;

    explicit data(runir::kr::dl::operand_t<F, runir::kr::dl::RoleTag> operand_) : index(), operand(std::move(operand_)) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"operand", &data::operand>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::operand> {}; }

    [[nodiscard]] urd::key<runir::kr::dl::Role<F, Tag>> key() const noexcept { return urd::key<runir::kr::dl::Role<F, Tag>>(index); }
};

template<typename F>
struct data<runir::kr::dl::Role<F, runir::kr::dl::RestrictionTag>>
{
    urd::key<runir::kr::dl::Role<F, runir::kr::dl::RestrictionTag>> index;
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

    [[nodiscard]] urd::key<runir::kr::dl::Role<F, runir::kr::dl::RestrictionTag>> key() const noexcept
    { return urd::key<runir::kr::dl::Role<F, runir::kr::dl::RestrictionTag>>(index); }
};

template<typename F>
struct data<runir::kr::dl::Role<F, runir::kr::dl::IdentityTag>>
{
    urd::key<runir::kr::dl::Role<F, runir::kr::dl::IdentityTag>> index;
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

    [[nodiscard]] urd::key<runir::kr::dl::Role<F, runir::kr::dl::IdentityTag>> key() const noexcept
    { return urd::key<runir::kr::dl::Role<F, runir::kr::dl::IdentityTag>>(index); }
};

template<typename F>
struct data<runir::kr::dl::Role<F, runir::kr::dl::RegisterTag>>
{
    urd::key<runir::kr::dl::Role<F, runir::kr::dl::RegisterTag>> index;
    urd::index<runir::kr::dl::Register<runir::kr::dl::RoleTag>> slot;

    data() = default;

    explicit data(urd::index<runir::kr::dl::Register<runir::kr::dl::RoleTag>> slot_) : index(), slot(slot_) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"slot", &data::slot>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::slot> {}; }

    [[nodiscard]] urd::key<runir::kr::dl::Role<F, runir::kr::dl::RegisterTag>> key() const noexcept
    { return urd::key<runir::kr::dl::Role<F, runir::kr::dl::RegisterTag>>(index); }
};

template<typename F>
struct data<runir::kr::dl::Role<F, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>>
{
    urd::key<runir::kr::dl::Role<F, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>> index;
    urd::index<runir::kr::dl::Argument<runir::kr::dl::RoleTag>> argument;

    data() = default;

    explicit data(urd::index<runir::kr::dl::Argument<runir::kr::dl::RoleTag>> argument_) : index(), argument(argument_) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"argument", &data::argument>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::argument> {}; }

    [[nodiscard]] urd::key<runir::kr::dl::Role<F, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>> key() const noexcept
    { return urd::key<runir::kr::dl::Role<F, runir::kr::dl::ArgumentTag<runir::kr::dl::RoleTag>>>(index); }
};

}  // namespace urd

#endif
