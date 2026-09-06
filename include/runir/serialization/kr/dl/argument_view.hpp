#ifndef RUNIR_SERIALIZATION_KR_DL_ARGUMENT_VIEW_HPP_
#define RUNIR_SERIALIZATION_KR_DL_ARGUMENT_VIEW_HPP_

#include "runir/kr/dl/argument_view.hpp"
#include "runir/kr/dl/repository.hpp"

#include <yggdrasil/serialization/dictionaries.hpp>

namespace ygg::serialization
{

template<runir::kr::dl::CategoryTag Category, typename C>
struct TypeName<View<Index<runir::kr::dl::Argument<Category>>, C>>
{
    static std::string get() { return std::string(Category::name) + "Argument"; }
};

template<runir::kr::dl::CategoryTag Category, typename C>
void tag_invoke(boost::json::value_from_tag,
                boost::json::value& result,
                const View<Index<runir::kr::dl::Argument<Category>>, C>& value,
                Dictionaries* dictionaries)
{
    dictionaries->object(result, value, [&](auto& ar)
    {
        ar.field("name", value.get_name());
        ar.field("identifier", uint_t(value.get_identifier()));
    });
}

}

#endif
