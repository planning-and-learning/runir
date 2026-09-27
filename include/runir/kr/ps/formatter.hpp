#ifndef RUNIR_KR_PS_FORMATTER_HPP_
#define RUNIR_KR_PS_FORMATTER_HPP_

#include "runir/kr/dl/semantics/formatter.hpp"
#include "runir/kr/ps/condition_view.hpp"
#include "runir/kr/ps/dl/condition_view.hpp"
#include "runir/kr/ps/dl/effect_view.hpp"
#include "runir/kr/ps/dl/feature_view.hpp"
#include "runir/kr/ps/effect_view.hpp"
#include "runir/kr/ps/feature_view.hpp"

#include <concepts>
#include <fmt/format.h>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <yggdrasil/io/iostream.hpp>

namespace runir::kr::ps
{

template<runir::kr::ps::dl::FeatureTag FeatureTag>
constexpr std::string_view feature_type()
{
    if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
        return runir::kr::dl::ConceptTag::name;
    else if constexpr (std::same_as<FeatureTag, runir::kr::dl::RoleTag>)
        return runir::kr::dl::RoleTag::name;
    else
        return FeatureTag::keyword;
}

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename C>
void append_feature(std::ostream& os, ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>>, C> view)
{
    os << ygg::print_indent << "(:" << feature_type<FeatureTag>() << "\n";
    {
        ygg::IndentScope scope(os);
        os << ygg::print_indent << "(:symbol " << view.get_symbol() << ")\n";
        os << ygg::print_indent << "(:expression ";
        fmt::format_to(std::ostream_iterator<char>(os), "{}", view.get_expression());
        os << ")\n";
    }
    os << ygg::print_indent << ')';
}

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename C>
void append_feature(std::ostream& os, ygg::View<ygg::Index<runir::kr::ps::Feature<Family, FeatureTag>>, C> view)
{
    ygg::visit([&](auto concrete) { append_feature(os, concrete); }, view.get_variant());
}

template<typename View>
std::string feature(View view)
{
    auto os = std::ostringstream {};
    append_feature(os, view);
    return os.str();
}

template<runir::kr::FamilyTag Family,
         typename Out,
         runir::kr::ps::dl::FeatureTag FeatureTag,
         runir::kr::ps::dl::ConditionObservationTag<FeatureTag> ObservationTag,
         typename C>
Out condition(ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> view, Out out)
{
    return fmt::format_to(out, "({} {})", ObservationTag::keyword, view.get_feature().get_symbol());
}

template<runir::kr::FamilyTag Family, typename Out, typename LanguageTag, typename C>
Out condition(ygg::View<ygg::Index<runir::kr::ps::ConcreteConditionVariant<Family, LanguageTag>>, C> view, Out out)
{
    return ygg::visit([&](auto concrete) { return condition(concrete, out); }, view.get_variant());
}

template<runir::kr::FamilyTag Family, typename Out, typename C>
Out condition(ygg::View<ygg::Index<runir::kr::ps::ConditionVariant<Family>>, C> view, Out out)
{
    return ygg::visit([&](auto concrete) { return condition(concrete, out); }, view.get_variant());
}

template<runir::kr::FamilyTag Family,
         typename Out,
         runir::kr::ps::dl::FeatureTag FeatureTag,
         runir::kr::ps::dl::EffectObservationTag<FeatureTag> ObservationTag,
         typename C>
Out effect(ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> view, Out out)
{
    return fmt::format_to(out, "({} {})", ObservationTag::keyword, view.get_feature().get_symbol());
}

template<runir::kr::FamilyTag Family, typename Out, typename LanguageTag, typename C>
Out effect(ygg::View<ygg::Index<runir::kr::ps::ConcreteEffectVariant<Family, LanguageTag>>, C> view, Out out)
{
    return ygg::visit([&](auto concrete) { return effect(concrete, out); }, view.get_variant());
}

template<runir::kr::FamilyTag Family, typename Out, typename C>
Out effect(ygg::View<ygg::Index<runir::kr::ps::EffectVariant<Family>>, C> view, Out out)
{
    return ygg::visit([&](auto concrete) { return effect(concrete, out); }, view.get_variant());
}

template<typename View>
std::string condition(View view)
{
    auto result = std::string {};
    condition(view, std::back_inserter(result));
    return result;
}

template<typename View>
std::string effect(View view)
{
    auto result = std::string {};
    effect(view, std::back_inserter(result));
    return result;
}

}  // namespace runir::kr::ps

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::Feature<Family, FeatureTag>>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::feature(view), context); }
};

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<Family, runir::kr::DlTag, FeatureTag>>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::feature(view), context); }
};

template<runir::kr::FamilyTag Family, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConditionVariant<Family>>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::condition(view), context); }
};

template<runir::kr::FamilyTag Family, typename LanguageTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteConditionVariant<Family, LanguageTag>>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::condition(view), context); }
};

template<runir::kr::FamilyTag Family,
         runir::kr::ps::dl::FeatureTag FeatureTag,
         runir::kr::ps::dl::ConditionObservationTag<FeatureTag> ObservationTag,
         typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, FeatureTag, ObservationTag>>, C>> :
    fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::condition(view), context); }
};

template<runir::kr::FamilyTag Family, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::EffectVariant<Family>>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::effect(view), context); }
};

template<runir::kr::FamilyTag Family, typename LanguageTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteEffectVariant<Family, LanguageTag>>, C>> : fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::effect(view), context); }
};

template<runir::kr::FamilyTag Family, runir::kr::ps::dl::FeatureTag FeatureTag, runir::kr::ps::dl::EffectObservationTag<FeatureTag> ObservationTag, typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, FeatureTag, ObservationTag>>, C>> :
    fmt::formatter<std::string_view>
{
    auto format(auto view, format_context& context) const { return fmt::formatter<std::string_view>::format(runir::kr::ps::effect(view), context); }
};

#endif
