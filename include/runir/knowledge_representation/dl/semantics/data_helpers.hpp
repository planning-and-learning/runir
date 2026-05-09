#ifndef RUNIR_SEMANTICS_DATA_HELPERS_HPP_
#define RUNIR_SEMANTICS_DATA_HELPERS_HPP_

#include "runir/knowledge_representation/dl/indices.hpp"

#include <tuple>
#include <tyr/common/types.hpp>
#include <tyr/common/types_utils.hpp>
#include <tyr/formalism/object_index.hpp>
#include <tyr/formalism/predicate_index.hpp>
#include <utility>

namespace runir::kr::dl::semantics
{

template<typename Self>
struct NullaryData
{
    tyr::Index<Self> index;

    NullaryData() = default;

    void clear() noexcept { tyr::clear(index); }

    auto cista_members() const noexcept { return std::tie(index); }
    auto identifying_members() const noexcept { return std::tie(); }
};

template<typename Self, typename Arg>
struct UnaryData
{
    tyr::Index<Self> index;
    tyr::Index<Arg> arg;

    UnaryData() = default;
    UnaryData(tyr::Index<Arg> arg_) : index(), arg(std::move(arg_)) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(arg);
    }

    auto cista_members() const noexcept { return std::tie(index, arg); }
    auto identifying_members() const noexcept { return std::tie(arg); }
};

template<typename Self, typename Lhs, typename Rhs>
struct BinaryData
{
    tyr::Index<Self> index;
    tyr::Index<Lhs> lhs;
    tyr::Index<Rhs> rhs;

    BinaryData() = default;
    BinaryData(tyr::Index<Lhs> lhs_, tyr::Index<Rhs> rhs_) : index(), lhs(std::move(lhs_)), rhs(std::move(rhs_)) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(lhs);
        tyr::clear(rhs);
    }

    auto cista_members() const noexcept { return std::tie(index, lhs, rhs); }
    auto identifying_members() const noexcept { return std::tie(lhs, rhs); }
};

template<typename Self, typename Lhs, typename Mid, typename Rhs>
struct TernaryData
{
    tyr::Index<Self> index;
    tyr::Index<Lhs> lhs;
    tyr::Index<Mid> mid;
    tyr::Index<Rhs> rhs;

    TernaryData() = default;
    TernaryData(tyr::Index<Lhs> lhs_, tyr::Index<Mid> mid_, tyr::Index<Rhs> rhs_) : index(), lhs(std::move(lhs_)), mid(std::move(mid_)), rhs(std::move(rhs_)) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(lhs);
        tyr::clear(mid);
        tyr::clear(rhs);
    }

    auto cista_members() const noexcept { return std::tie(index, lhs, mid, rhs); }
    auto identifying_members() const noexcept { return std::tie(lhs, mid, rhs); }
};

template<typename Self, tyr::formalism::FactKind T>
struct PredicateData
{
    tyr::Index<Self> index;
    tyr::Index<tyr::formalism::Predicate<T>> predicate;
    bool polarity;

    PredicateData() = default;
    PredicateData(tyr::Index<tyr::formalism::Predicate<T>> predicate_) : index(), predicate(predicate_), polarity(true) {}
    PredicateData(tyr::Index<tyr::formalism::Predicate<T>> predicate_, bool polarity_) : index(), predicate(predicate_), polarity(polarity_) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(predicate);
        tyr::clear(polarity);
    }

    auto cista_members() const noexcept { return std::tie(index, predicate, polarity); }
    auto identifying_members() const noexcept { return std::tie(predicate, polarity); }
};

template<typename Self>
struct ObjectData
{
    tyr::Index<Self> index;
    tyr::Index<tyr::formalism::Object> object;

    ObjectData() = default;
    ObjectData(tyr::Index<tyr::formalism::Object> object_) : index(), object(object_) {}

    void clear() noexcept
    {
        tyr::clear(index);
        tyr::clear(object);
    }

    auto cista_members() const noexcept { return std::tie(index, object); }
    auto identifying_members() const noexcept { return std::tie(object); }
};

}

#endif
