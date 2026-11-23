#ifndef RUNIR_KR_PS_EXT_PROGRAM_DATA_HPP_
#define RUNIR_KR_PS_EXT_PROGRAM_DATA_HPP_

#include "runir/kr/ps/ext/declarations.hpp"

#include <tuple>
#include <urd/elements/flat.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>

namespace urd
{

template<>
struct data<runir::kr::ps::ext::Program>
{
    urd::key<runir::kr::ps::ext::Program> index;
    urd::index<runir::kr::ps::ext::Module> entry_mod;
    urd::flat_vector<urd::index<runir::kr::ps::ext::Module>> modules;

    data() = default;

    data(urd::index<runir::kr::ps::ext::Module> entry_mod_, urd::flat_vector<urd::index<runir::kr::ps::ext::Module>> modules_) :
        index(),
        entry_mod(entry_mod_),
        modules(std::move(modules_))
    {
    }

    data(const data& other) = delete;
    data& operator=(const data& other) = delete;
    data(data&& other) = default;
    data& operator=(data&& other) = default;
    ~data() = default;

    static constexpr auto fields() noexcept { return urd::Fields<urd::field<"entry_mod", &data::entry_mod>, urd::field<"modules", &data::modules>> {}; }

    static constexpr auto identifying_members() noexcept { return urd::Members<&data::entry_mod, &data::modules> {}; }

    static constexpr auto cista_members() noexcept { return urd::Members<&data::index, &data::entry_mod, &data::modules> {}; }

    [[nodiscard]] urd::key<runir::kr::ps::ext::Program> key() const noexcept { return urd::key<runir::kr::ps::ext::Program>(index); }
};

}  // namespace urd

#endif
