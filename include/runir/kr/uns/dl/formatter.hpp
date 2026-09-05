#ifndef RUNIR_KR_UNS_DL_FORMATTER_HPP_
#define RUNIR_KR_UNS_DL_FORMATTER_HPP_

#include "runir/kr/dl/semantics/formatter.hpp"
#include "runir/kr/uns/dl/feature_view.hpp"

#include <fmt/format.h>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <yggdrasil/io/iostream.hpp>

namespace runir::kr::uns::dl
{

template<typename C>
void append_feature(std::ostream& os,
                    ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>>, C> view)
{
    os << "(:" << runir::kr::ps::dl::BooleanFeature::keyword << "\n";
    {
        ygg::IndentScope scope(os);
        os << ygg::print_indent << fmt::format("(:symbol {})", view.get_symbol().str()) << "\n";
        os << ygg::print_indent << "(:expression ";
        fmt::format_to(std::ostream_iterator<char>(os), "{}", view.get_expression());
        os << ")\n";
    }
    os << ygg::print_indent << ")";
}

template<typename C>
std::string feature(ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>>, C> view)
{
    auto os = std::ostringstream {};
    append_feature(os, view);
    return os.str();
}

}  // namespace runir::kr::uns::dl

template<typename C>
struct fmt::formatter<ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>>, C>> :
    fmt::formatter<std::string_view>
{
    using View = ygg::View<ygg::Index<runir::kr::ps::ConcreteFeature<runir::kr::UnsFamilyTag, runir::kr::DlTag, runir::kr::ps::dl::BooleanFeature>>, C>;
    auto format(View view, format_context& ctx) const { return fmt::formatter<std::string_view>::format(runir::kr::uns::dl::feature(view), ctx); }
};

#endif
