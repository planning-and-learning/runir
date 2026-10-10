#ifndef RUNIR_KR_PS_ICP_PROGRAM_VIEW_HPP_
#define RUNIR_KR_PS_ICP_PROGRAM_VIEW_HPP_

#include "runir/kr/ps/icp/module_view.hpp"
#include "runir/kr/ps/icp/program_data.hpp"

namespace ygg
{

template<typename C>
class View<Index<runir::kr::ps::icp::Program>, C> : public ygg::IndexViewBase<runir::kr::ps::icp::Program, C>
{
public:
    using ygg::IndexViewBase<runir::kr::ps::icp::Program, C>::IndexViewBase;

    auto get_module() const noexcept { return make_view(this->get_data().module, this->get_context()); }
    auto get_entry_module() const noexcept { return get_module(); }
};

}

#endif
