#ifndef RUNIR_KR_DL_BOOLEAN_DATA_HPP_
#define RUNIR_KR_DL_BOOLEAN_DATA_HPP_

#include "runir/kr/dl/query_data.hpp"
#include "runir/kr/dl/role_data.hpp"

#include <urd/elements/flat.hpp>
#include <utility>

namespace urd
{

template<typename F, runir::kr::dl::AtomicTag Tag>
struct data<runir::kr::dl::Boolean<F, Tag>>
{
    urd::index<runir::kr::dl::Boolean<F, Tag>> index;
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
};

template<typename F>
struct data<runir::kr::dl::Boolean<F, runir::kr::dl::NonemptyTag>>
{
    urd::index<runir::kr::dl::Boolean<F, runir::kr::dl::NonemptyTag>> index;
    typename F::Argument operand;

    data() = default;

    explicit data(typename F::Argument operand_) : index(), operand(std::move(operand_)) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"operand", &data::operand>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::operand> {}; }
};

template<typename F>
struct data<runir::kr::dl::Boolean<F, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>>
{
    urd::index<runir::kr::dl::Boolean<F, runir::kr::dl::ArgumentTag<runir::kr::dl::BooleanTag>>> index;
    urd::index<runir::kr::dl::Argument<runir::kr::dl::BooleanTag>> argument;

    data() = default;

    explicit data(urd::index<runir::kr::dl::Argument<runir::kr::dl::BooleanTag>> argument_) : index(), argument(argument_) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"argument", &data::argument>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::argument> {}; }
};

template<typename F, runir::kr::dl::ComparisonTag Tag>
struct data<runir::kr::dl::Boolean<F, Tag>>
{
    urd::index<runir::kr::dl::Boolean<F, Tag>> index;
    runir::kr::dl::operand_t<F, runir::kr::dl::comparison_operand_t<Tag>> left_operand;
    runir::kr::dl::operand_t<F, runir::kr::dl::comparison_operand_t<Tag>> right_operand;

    data() = default;

    data(runir::kr::dl::operand_t<F, runir::kr::dl::comparison_operand_t<Tag>> left_operand_,
         runir::kr::dl::operand_t<F, runir::kr::dl::comparison_operand_t<Tag>> right_operand_) :
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
};

template<typename F>
struct data<runir::kr::dl::Boolean<F, runir::kr::dl::BooleanConstantTag>>
{
    urd::index<runir::kr::dl::Boolean<F, runir::kr::dl::BooleanConstantTag>> index;
    bool value = false;

    data() = default;

    explicit data(bool value_) : index(), value(value_) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"value", &data::value>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::value> {}; }
};

template<typename F, runir::kr::dl::LogicalBinaryTag Tag>
struct data<runir::kr::dl::Boolean<F, Tag>>
{
    urd::index<runir::kr::dl::Boolean<F, Tag>> index;
    runir::kr::dl::operand_t<F, runir::kr::dl::BooleanTag> left_operand;
    runir::kr::dl::operand_t<F, runir::kr::dl::BooleanTag> right_operand;

    data() = default;

    data(runir::kr::dl::operand_t<F, runir::kr::dl::BooleanTag> left_operand_, runir::kr::dl::operand_t<F, runir::kr::dl::BooleanTag> right_operand_) :
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
};

template<typename F>
struct data<runir::kr::dl::Boolean<F, runir::kr::dl::NotTag>>
{
    urd::index<runir::kr::dl::Boolean<F, runir::kr::dl::NotTag>> index;
    runir::kr::dl::operand_t<F, runir::kr::dl::BooleanTag> operand;

    data() = default;

    explicit data(runir::kr::dl::operand_t<F, runir::kr::dl::BooleanTag> operand_) : index(), operand(std::move(operand_)) {}

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"operand", &data::operand>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::members_of(fields()); }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::operand> {}; }
};

}  // namespace urd

#endif
