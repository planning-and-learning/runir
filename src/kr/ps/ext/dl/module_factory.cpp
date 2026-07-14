#include "runir/kr/ps/ext/dl/module_factory.hpp"

#include "runir/kr/ps/ext/canonicalization.hpp"
#include "runir/kr/ps/ext/dl/parser.hpp"

#include <fmt/format.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace runir::kr::ps::ext::dl
{

ModuleView ModuleFactory::create_empty(Repository& repository)
{
    auto entry_data = ygg::Data<MemoryState>(std::string("m0"));
    const auto entry = repository.get_or_create(entry_data).first;

    auto symbol_data = ygg::Data<ModuleSymbol>(std::string("empty"));
    const auto symbol = repository.get_or_create(symbol_data).first;
    auto data = ygg::Data<Module>(symbol.get_index());
    data.entry_memory_state = entry.get_index();
    data.memory_states.push_back(entry.get_index());
    canonicalize(data);
    return repository.get_or_create(data).first;
}

ModuleView ModuleFactory::create(ModuleSpecification specification, tyr::formalism::planning::DomainView domain, Repository& repository)
{
    return parse_module(create_description(specification), domain, repository);
}

std::string ModuleFactory::create_empty_description()
{
    return R"RUNIR((:module
    (:symbol empty)
    (:arguments)
    (:registers)
    (:entry m0)
    (:memory m0)
    (:features)
    (:rules)
))RUNIR";
}

std::string ModuleFactory::create_description(ModuleSpecification specification)
{
    switch (specification)
    {
        case ModuleSpecification::ON_BONET_ET_AL_ICAPS2024:
            return create_on_bonet_et_al_icaps2024_description();
        case ModuleSpecification::ON_TABLE_BONET_ET_AL_ICAPS2024:
            return create_on_table_bonet_et_al_icaps2024_description();
        case ModuleSpecification::TOWER_BONET_ET_AL_ICAPS2024:
            return create_tower_bonet_et_al_icaps2024_description();
        case ModuleSpecification::BLOCKS_BONET_ET_AL_ICAPS2024:
            return create_blocks_bonet_et_al_icaps2024_description();
    }

    throw std::runtime_error(fmt::format("Unknown module specification: {}.", static_cast<int>(specification)));
}

std::vector<ModuleView> ModuleFactory::create_bonet_et_al_icaps2024_modules(tyr::formalism::planning::DomainView domain, Repository& repository)
{
    return parse_modules(create_bonet_et_al_icaps2024_descriptions(), domain, repository);
}

std::vector<std::string> ModuleFactory::create_bonet_et_al_icaps2024_descriptions()
{
    return {
        create_on_bonet_et_al_icaps2024_description(),
        create_on_table_bonet_et_al_icaps2024_description(),
        create_tower_bonet_et_al_icaps2024_description(),
        create_blocks_bonet_et_al_icaps2024_description(),
    };
}

std::string ModuleFactory::create_bonet_et_al_icaps2024_program_description()
{
    return R"RUNIR((:program
    (:entry root)
    (:module
        (:symbol root)
        (:arguments)
        (:registers)
        (:entry m0)
        (:memory m0 m1)
        (:features
            (:role
                (:symbol O)
                (:expression
                    (r_atomic_goal "on" true)
                )
            )
        )
        (:rules
            (:rule
                (:symbol auto1)
                (:expression
                    (:source-memory m0)
                    (:target-memory m1)
                    (:call
                        (:conditions)
                        (:callee blocks)
                        (:arguments O)
                    )
                )
            )
        )
    )
    (:module
        (:symbol blocks)
        (:arguments
            (:role O)
        )
        (:registers
            (:concept r0)
        )
        (:entry m0)
        (:memory m0 m1)
        (:features
            (:concept
                (:symbol L_load)
                (:expression
                    (c_and
                        (c_all
                            (r_transitive_closure
                                (r_atomic_state "on")
                            )
                            (c_same_as
                                (r_argument O)
                                (r_atomic_state "on")
                            )
                        )
                        (c_not
                            (c_same_as
                                (r_argument O)
                                (r_atomic_state "on")
                            )
                        )
                    )
                )
            )
            (:numerical
                (:symbol L)
                (:expression
                    (n_count
                        (c_and
                            (c_all
                                (r_transitive_closure
                                    (r_atomic_state "on")
                                )
                                (c_same_as
                                    (r_argument O)
                                    (r_atomic_state "on")
                                )
                            )
                            (c_not
                                (c_same_as
                                    (r_argument O)
                                    (r_atomic_state "on")
                                )
                            )
                        )
                    )
                )
            )
            (:role
                (:symbol O_arg)
                (:expression
                    (r_argument O)
                )
            )
            (:concept
                (:symbol R0)
                (:expression
                    (c_register r0)
                )
            )
        )
        (:rules
            (:rule
                (:symbol auto43)
                (:expression
                    (:source-memory m0)
                    (:target-memory m1)
                    (:load
                        (:conditions
                            (greater_zero L)
                        )
                        (:concept L_load)
                        (:register
                            (:concept r0)
                        )
                    )
                )
            )
            (:rule
                (:symbol auto45)
                (:expression
                    (:source-memory m1)
                    (:target-memory m0)
                    (:call
                        (:conditions)
                        (:callee tower)
                        (:arguments O_arg R0)
                    )
                )
            )
        )
    )
    (:module
        (:symbol tower)
        (:arguments
            (:concept X)
            (:role O)
        )
        (:registers
            (:concept r0)
        )
        (:entry m0)
        (:memory m0 m1 m2 m3)
        (:features
            (:concept
                (:symbol X)
                (:expression
                    (c_argument X)
                )
            )
            (:numerical
                (:symbol X_count)
                (:expression
                    (n_count
                        (c_argument X)
                    )
                )
            )
            (:concept
                (:symbol W)
                (:expression
                    (c_some
                        (r_inverse
                            (r_argument O)
                        )
                        (c_register r0)
                    )
                )
            )
            (:numerical
                (:symbol W_count)
                (:expression
                    (n_count
                        (c_some
                            (r_inverse
                                (r_argument O)
                            )
                            (c_register r0)
                        )
                    )
                )
            )
            (:concept
                (:symbol M)
                (:expression
                    (c_some
                        (r_argument O)
                        (c_register r0)
                    )
                )
            )
            (:numerical
                (:symbol M_count)
                (:expression
                    (n_count
                        (c_some
                            (r_argument O)
                            (c_register r0)
                        )
                    )
                )
            )
            (:concept
                (:symbol R0)
                (:expression
                    (c_register r0)
                )
            )
            (:role
                (:symbol O_arg)
                (:expression
                    (r_argument O)
                )
            )
        )
        (:rules
            (:rule
                (:symbol auto38)
                (:expression
                    (:source-memory m0)
                    (:target-memory m1)
                    (:load
                        (:conditions
                            (greater_zero X_count)
                        )
                        (:concept X)
                        (:register
                            (:concept r0)
                        )
                    )
                )
            )
            (:rule
                (:symbol auto40)
                (:expression
                    (:source-memory m1)
                    (:target-memory m2)
                    (:call
                        (:conditions
                            (equal_zero W_count)
                        )
                        (:callee on-table)
                        (:arguments R0)
                    )
                )
            )
            (:rule
                (:symbol auto41)
                (:expression
                    (:source-memory m1)
                    (:target-memory m2)
                    (:call
                        (:conditions
                            (greater_zero W_count)
                        )
                        (:callee on)
                        (:arguments R0 W)
                    )
                )
            )
            (:rule
                (:symbol auto42)
                (:expression
                    (:source-memory m2)
                    (:target-memory m3)
                    (:call
                        (:conditions
                            (greater_zero M_count)
                        )
                        (:callee tower)
                        (:arguments O_arg M)
                    )
                )
            )
        )
    )
    (:module
        (:symbol on-table)
        (:arguments
            (:concept X)
        )
        (:registers)
        (:entry m0)
        (:memory m0 m1 m2)
        (:features
            (:concept
                (:symbol DO_on_table_1)
                (:expression
                    (c_argument X)
                )
            )
            (:concept
                (:symbol DO_on_table_2)
                (:expression
                    (c_top)
                )
            )
            (:concept
                (:symbol DO_on_table_3)
                (:expression
                    (c_argument X)
                )
            )
            (:boolean
                (:symbol H)
                (:expression
                    (b_nonempty
                        (c_atomic_state "holding")
                    )
                )
            )
            (:boolean
                (:symbol Tx)
                (:expression
                    (b_nonempty
                        (c_and
                            (c_argument X)
                            (c_atomic_state "on-table")
                        )
                    )
                )
            )
        )
        (:rules
            (:rule
                (:symbol auto32)
                (:expression
                    (:source-memory m0)
                    (:target-memory m1)
                    (:do
                        (:conditions
                            (negative H)
                            (negative Tx)
                        )
                        (:action "unstack")
                        (:arguments DO_on_table_1 DO_on_table_2)
                        (:effects)
                    )
                )
            )
            (:rule
                (:symbol auto34)
                (:expression
                    (:source-memory m0)
                    (:target-memory m1)
                    (:sketch
                        (:conditions
                            (positive H)
                        )
                        (:effects)
                    )
                )
            )
            (:rule
                (:symbol auto30)
                (:expression
                    (:source-memory m0)
                    (:target-memory m2)
                    (:sketch
                        (:conditions
                            (positive Tx)
                        )
                        (:effects)
                    )
                )
            )
            (:rule
                (:symbol auto36)
                (:expression
                    (:source-memory m1)
                    (:target-memory m2)
                    (:do
                        (:conditions)
                        (:action "putdown")
                        (:arguments DO_on_table_3)
                        (:effects)
                    )
                )
            )
        )
    )
    (:module
        (:symbol on)
        (:arguments
            (:concept X)
            (:concept Y)
        )
        (:registers
            (:concept r0)
            (:concept r1)
        )
        (:entry m0)
        (:memory m0 m1 m2 m3 m4 m5 m6 m7 m8)
        (:features
            (:numerical
                (:symbol N)
                (:expression
                    (n_count
                        (c_and
                            (c_or
                                (c_argument X)
                                (c_argument Y)
                            )
                            (c_not
                                (c_atomic_state "clear")
                            )
                        )
                    )
                )
            )
            (:numerical
                (:symbol T0)
                (:expression
                    (n_count
                        (c_some
                            (r_inverse
                                (r_atomic_state "on")
                            )
                            (c_register r0)
                        )
                    )
                )
            )
            (:numerical
                (:symbol T1)
                (:expression
                    (n_count
                        (c_some
                            (r_inverse
                                (r_atomic_state "on")
                            )
                            (c_register r1)
                        )
                    )
                )
            )
            (:concept
                (:symbol N_load)
                (:expression
                    (c_and
                        (c_or
                            (c_argument X)
                            (c_argument Y)
                        )
                        (c_not
                            (c_atomic_state "clear")
                        )
                    )
                )
            )
            (:concept
                (:symbol T0_load)
                (:expression
                    (c_some
                        (r_inverse
                            (r_atomic_state "on")
                        )
                        (c_register r0)
                    )
                )
            )
            (:concept
                (:symbol T1_load)
                (:expression
                    (c_some
                        (r_inverse
                            (r_atomic_state "on")
                        )
                        (c_register r1)
                    )
                )
            )
            (:concept
                (:symbol DO_on_1)
                (:expression
                    (c_top)
                )
            )
            (:concept
                (:symbol DO_on_2)
                (:expression
                    (c_register r1)
                )
            )
            (:concept
                (:symbol DO_on_3)
                (:expression
                    (c_top)
                )
            )
            (:concept
                (:symbol DO_on_4)
                (:expression
                    (c_register r1)
                )
            )
            (:concept
                (:symbol DO_on_5)
                (:expression
                    (c_argument X)
                )
            )
            (:concept
                (:symbol DO_on_6)
                (:expression
                    (c_argument X)
                )
            )
            (:concept
                (:symbol DO_on_7)
                (:expression
                    (c_top)
                )
            )
            (:concept
                (:symbol DO_on_8)
                (:expression
                    (c_argument X)
                )
            )
            (:concept
                (:symbol DO_on_9)
                (:expression
                    (c_argument Y)
                )
            )
            (:boolean
                (:symbol H)
                (:expression
                    (b_nonempty
                        (c_atomic_state "holding")
                    )
                )
            )
            (:boolean
                (:symbol Tx)
                (:expression
                    (b_nonempty
                        (c_and
                            (c_argument X)
                            (c_atomic_state "on-table")
                        )
                    )
                )
            )
        )
        (:rules
            (:rule
                (:symbol auto6)
                (:expression
                    (:source-memory m0)
                    (:target-memory m1)
                    (:load
                        (:conditions
                            (negative H)
                            (greater_zero N)
                        )
                        (:concept N_load)
                        (:register
                            (:concept r0)
                        )
                    )
                )
            )
            (:rule
                (:symbol auto2)
                (:expression
                    (:source-memory m0)
                    (:target-memory m4)
                    (:sketch
                        (:conditions
                            (positive H)
                        )
                        (:effects)
                    )
                )
            )
            (:rule
                (:symbol auto4)
                (:expression
                    (:source-memory m0)
                    (:target-memory m7)
                    (:sketch
                        (:conditions
                            (negative H)
                            (equal_zero N)
                        )
                        (:effects)
                    )
                )
            )
            (:rule
                (:symbol auto8)
                (:expression
                    (:source-memory m1)
                    (:target-memory m2)
                    (:load
                        (:conditions
                            (greater_zero T0)
                        )
                        (:concept T0_load)
                        (:register
                            (:concept r1)
                        )
                    )
                )
            )
            (:rule
                (:symbol auto10)
                (:expression
                    (:source-memory m2)
                    (:target-memory m2)
                    (:load
                        (:conditions
                            (greater_zero T1)
                        )
                        (:concept T1_load)
                        (:register
                            (:concept r1)
                        )
                    )
                )
            )
            (:rule
                (:symbol auto12)
                (:expression
                    (:source-memory m2)
                    (:target-memory m5)
                    (:sketch
                        (:conditions
                            (equal_zero T1)
                        )
                        (:effects)
                    )
                )
            )
            (:rule
                (:symbol auto16)
                (:expression
                    (:source-memory m3)
                    (:target-memory m0)
                    (:sketch
                        (:conditions
                            (equal_zero T0)
                        )
                        (:effects)
                    )
                )
            )
            (:rule
                (:symbol auto14)
                (:expression
                    (:source-memory m3)
                    (:target-memory m1)
                    (:sketch
                        (:conditions
                            (greater_zero T0)
                        )
                        (:effects)
                    )
                )
            )
            (:rule
                (:symbol auto18)
                (:expression
                    (:source-memory m4)
                    (:target-memory m0)
                    (:do
                        (:conditions)
                        (:action "putdown")
                        (:arguments DO_on_1)
                        (:effects)
                    )
                )
            )
            (:rule
                (:symbol auto20)
                (:expression
                    (:source-memory m5)
                    (:target-memory m6)
                    (:do
                        (:conditions)
                        (:action "unstack")
                        (:arguments DO_on_2 DO_on_3)
                        (:effects)
                    )
                )
            )
            (:rule
                (:symbol auto22)
                (:expression
                    (:source-memory m6)
                    (:target-memory m3)
                    (:do
                        (:conditions)
                        (:action "putdown")
                        (:arguments DO_on_4)
                        (:effects)
                    )
                )
            )
            (:rule
                (:symbol auto24)
                (:expression
                    (:source-memory m7)
                    (:target-memory m8)
                    (:do
                        (:conditions
                            (positive Tx)
                        )
                        (:action "pickup")
                        (:arguments DO_on_5)
                        (:effects)
                    )
                )
            )
            (:rule
                (:symbol auto26)
                (:expression
                    (:source-memory m7)
                    (:target-memory m8)
                    (:do
                        (:conditions
                            (negative Tx)
                        )
                        (:action "unstack")
                        (:arguments DO_on_6 DO_on_7)
                        (:effects)
                    )
                )
            )
            (:rule
                (:symbol auto28)
                (:expression
                    (:source-memory m8)
                    (:target-memory m8)
                    (:do
                        (:conditions)
                        (:action "stack")
                        (:arguments DO_on_8 DO_on_9)
                        (:effects)
                    )
                )
            )
        )
    )
))RUNIR";
}

ModuleProgramView ModuleFactory::create_bonet_et_al_icaps2024_program(tyr::formalism::planning::DomainView domain, Repository& repository)
{
    return parse_module_program(create_bonet_et_al_icaps2024_program_description(), domain, repository);
}

ModuleView ModuleFactory::create_on_bonet_et_al_icaps2024(tyr::formalism::planning::DomainView domain, Repository& repository)
{
    return parse_module(create_on_bonet_et_al_icaps2024_description(), domain, repository);
}

ModuleView ModuleFactory::create_on_table_bonet_et_al_icaps2024(tyr::formalism::planning::DomainView domain, Repository& repository)
{
    return parse_module(create_on_table_bonet_et_al_icaps2024_description(), domain, repository);
}

ModuleView ModuleFactory::create_tower_bonet_et_al_icaps2024(tyr::formalism::planning::DomainView domain, Repository& repository)
{
    return parse_module(create_tower_bonet_et_al_icaps2024_description(), domain, repository);
}

ModuleView ModuleFactory::create_blocks_bonet_et_al_icaps2024(tyr::formalism::planning::DomainView domain, Repository& repository)
{
    return parse_module(create_blocks_bonet_et_al_icaps2024_description(), domain, repository);
}

std::string ModuleFactory::create_on_bonet_et_al_icaps2024_description()
{
    return R"RUNIR((:module
    (:symbol on)
    (:arguments
        (:concept X)
        (:concept Y)
    )
    (:registers
        (:concept r0)
        (:concept r1)
    )
    (:entry m0)
    (:memory m0 m1 m2 m3 m4 m5 m6 m7 m8)
    (:features
        (:numerical
            (:symbol N)
            (:expression
                (n_count
                    (c_and
                        (c_or
                            (c_argument X)
                            (c_argument Y)
                        )
                        (c_not
                            (c_atomic_state "clear")
                        )
                    )
                )
            )
        )
        (:numerical
            (:symbol T0)
            (:expression
                (n_count
                    (c_some
                        (r_inverse
                            (r_atomic_state "on")
                        )
                        (c_register r0)
                    )
                )
            )
        )
        (:numerical
            (:symbol T1)
            (:expression
                (n_count
                    (c_some
                        (r_inverse
                            (r_atomic_state "on")
                        )
                        (c_register r1)
                    )
                )
            )
        )
        (:concept
            (:symbol N_load)
            (:expression
                (c_and
                    (c_or
                        (c_argument X)
                        (c_argument Y)
                    )
                    (c_not
                        (c_atomic_state "clear")
                    )
                )
            )
        )
        (:concept
            (:symbol T0_load)
            (:expression
                (c_some
                    (r_inverse
                        (r_atomic_state "on")
                    )
                    (c_register r0)
                )
            )
        )
        (:concept
            (:symbol T1_load)
            (:expression
                (c_some
                    (r_inverse
                        (r_atomic_state "on")
                    )
                    (c_register r1)
                )
            )
        )
        (:concept
            (:symbol DO_on_1)
            (:expression
                (c_top)
            )
        )
        (:concept
            (:symbol DO_on_2)
            (:expression
                (c_register r1)
            )
        )
        (:concept
            (:symbol DO_on_3)
            (:expression
                (c_top)
            )
        )
        (:concept
            (:symbol DO_on_4)
            (:expression
                (c_register r1)
            )
        )
        (:concept
            (:symbol DO_on_5)
            (:expression
                (c_argument X)
            )
        )
        (:concept
            (:symbol DO_on_6)
            (:expression
                (c_argument X)
            )
        )
        (:concept
            (:symbol DO_on_7)
            (:expression
                (c_top)
            )
        )
        (:concept
            (:symbol DO_on_8)
            (:expression
                (c_argument X)
            )
        )
        (:concept
            (:symbol DO_on_9)
            (:expression
                (c_argument Y)
            )
        )
        (:boolean
            (:symbol H)
            (:expression
                (b_nonempty
                    (c_atomic_state "holding")
                )
            )
        )
        (:boolean
            (:symbol Tx)
            (:expression
                (b_nonempty
                    (c_and
                        (c_argument X)
                        (c_atomic_state "on-table")
                    )
                )
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto6)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:load
                    (:conditions
                        (negative H)
                        (greater_zero N)
                    )
                    (:concept N_load)
                    (:register
                        (:concept r0)
                    )
                )
            )
        )
        (:rule
            (:symbol auto2)
            (:expression
                (:source-memory m0)
                (:target-memory m4)
                (:sketch
                    (:conditions
                        (positive H)
                    )
                    (:effects)
                )
            )
        )
        (:rule
            (:symbol auto4)
            (:expression
                (:source-memory m0)
                (:target-memory m7)
                (:sketch
                    (:conditions
                        (negative H)
                        (equal_zero N)
                    )
                    (:effects)
                )
            )
        )
        (:rule
            (:symbol auto8)
            (:expression
                (:source-memory m1)
                (:target-memory m2)
                (:load
                    (:conditions
                        (greater_zero T0)
                    )
                    (:concept T0_load)
                    (:register
                        (:concept r1)
                    )
                )
            )
        )
        (:rule
            (:symbol auto10)
            (:expression
                (:source-memory m2)
                (:target-memory m2)
                (:load
                    (:conditions
                        (greater_zero T1)
                    )
                    (:concept T1_load)
                    (:register
                        (:concept r1)
                    )
                )
            )
        )
        (:rule
            (:symbol auto12)
            (:expression
                (:source-memory m2)
                (:target-memory m5)
                (:sketch
                    (:conditions
                        (equal_zero T1)
                    )
                    (:effects)
                )
            )
        )
        (:rule
            (:symbol auto16)
            (:expression
                (:source-memory m3)
                (:target-memory m0)
                (:sketch
                    (:conditions
                        (equal_zero T0)
                    )
                    (:effects)
                )
            )
        )
        (:rule
            (:symbol auto14)
            (:expression
                (:source-memory m3)
                (:target-memory m1)
                (:sketch
                    (:conditions
                        (greater_zero T0)
                    )
                    (:effects)
                )
            )
        )
        (:rule
            (:symbol auto18)
            (:expression
                (:source-memory m4)
                (:target-memory m0)
                (:do
                    (:conditions)
                    (:action "putdown")
                    (:arguments DO_on_1)
                    (:effects)
                )
            )
        )
        (:rule
            (:symbol auto20)
            (:expression
                (:source-memory m5)
                (:target-memory m6)
                (:do
                    (:conditions)
                    (:action "unstack")
                    (:arguments DO_on_2 DO_on_3)
                    (:effects)
                )
            )
        )
        (:rule
            (:symbol auto22)
            (:expression
                (:source-memory m6)
                (:target-memory m3)
                (:do
                    (:conditions)
                    (:action "putdown")
                    (:arguments DO_on_4)
                    (:effects)
                )
            )
        )
        (:rule
            (:symbol auto24)
            (:expression
                (:source-memory m7)
                (:target-memory m8)
                (:do
                    (:conditions
                        (positive Tx)
                    )
                    (:action "pickup")
                    (:arguments DO_on_5)
                    (:effects)
                )
            )
        )
        (:rule
            (:symbol auto26)
            (:expression
                (:source-memory m7)
                (:target-memory m8)
                (:do
                    (:conditions
                        (negative Tx)
                    )
                    (:action "unstack")
                    (:arguments DO_on_6 DO_on_7)
                    (:effects)
                )
            )
        )
        (:rule
            (:symbol auto28)
            (:expression
                (:source-memory m8)
                (:target-memory m8)
                (:do
                    (:conditions)
                    (:action "stack")
                    (:arguments DO_on_8 DO_on_9)
                    (:effects)
                )
            )
        )
    )
))RUNIR";
}

std::string ModuleFactory::create_on_table_bonet_et_al_icaps2024_description()
{
    return R"RUNIR((:module
    (:symbol on-table)
    (:arguments
        (:concept X)
    )
    (:registers)
    (:entry m0)
    (:memory m0 m1 m2)
    (:features
        (:concept
            (:symbol DO_on_table_1)
            (:expression
                (c_argument X)
            )
        )
        (:concept
            (:symbol DO_on_table_2)
            (:expression
                (c_top)
            )
        )
        (:concept
            (:symbol DO_on_table_3)
            (:expression
                (c_argument X)
            )
        )
        (:boolean
            (:symbol H)
            (:expression
                (b_nonempty
                    (c_atomic_state "holding")
                )
            )
        )
        (:boolean
            (:symbol Tx)
            (:expression
                (b_nonempty
                    (c_and
                        (c_argument X)
                        (c_atomic_state "on-table")
                    )
                )
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto32)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:do
                    (:conditions
                        (negative H)
                        (negative Tx)
                    )
                    (:action "unstack")
                    (:arguments DO_on_table_1 DO_on_table_2)
                    (:effects)
                )
            )
        )
        (:rule
            (:symbol auto34)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:sketch
                    (:conditions
                        (positive H)
                    )
                    (:effects)
                )
            )
        )
        (:rule
            (:symbol auto30)
            (:expression
                (:source-memory m0)
                (:target-memory m2)
                (:sketch
                    (:conditions
                        (positive Tx)
                    )
                    (:effects)
                )
            )
        )
        (:rule
            (:symbol auto36)
            (:expression
                (:source-memory m1)
                (:target-memory m2)
                (:do
                    (:conditions)
                    (:action "putdown")
                    (:arguments DO_on_table_3)
                    (:effects)
                )
            )
        )
    )
))RUNIR";
}

std::string ModuleFactory::create_tower_bonet_et_al_icaps2024_description()
{
    return R"RUNIR((:module
    (:symbol tower)
    (:arguments
        (:concept X)
        (:role O)
    )
    (:registers
        (:concept r0)
    )
    (:entry m0)
    (:memory m0 m1 m2 m3)
    (:features
        (:concept
            (:symbol X)
            (:expression
                (c_argument X)
            )
        )
        (:numerical
            (:symbol X_count)
            (:expression
                (n_count
                    (c_argument X)
                )
            )
        )
        (:concept
            (:symbol W)
            (:expression
                (c_some
                    (r_inverse
                        (r_argument O)
                    )
                    (c_register r0)
                )
            )
        )
        (:numerical
            (:symbol W_count)
            (:expression
                (n_count
                    (c_some
                        (r_inverse
                            (r_argument O)
                        )
                        (c_register r0)
                    )
                )
            )
        )
        (:concept
            (:symbol M)
            (:expression
                (c_some
                    (r_argument O)
                    (c_register r0)
                )
            )
        )
        (:numerical
            (:symbol M_count)
            (:expression
                (n_count
                    (c_some
                        (r_argument O)
                        (c_register r0)
                    )
                )
            )
        )
        (:concept
            (:symbol R0)
            (:expression
                (c_register r0)
            )
        )
        (:role
            (:symbol O_arg)
            (:expression
                (r_argument O)
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto38)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:load
                    (:conditions
                        (greater_zero X_count)
                    )
                    (:concept X)
                    (:register
                        (:concept r0)
                    )
                )
            )
        )
        (:rule
            (:symbol auto40)
            (:expression
                (:source-memory m1)
                (:target-memory m2)
                (:call
                    (:conditions
                        (equal_zero W_count)
                    )
                    (:callee on-table)
                    (:arguments R0)
                )
            )
        )
        (:rule
            (:symbol auto41)
            (:expression
                (:source-memory m1)
                (:target-memory m2)
                (:call
                    (:conditions
                        (greater_zero W_count)
                    )
                    (:callee on)
                    (:arguments R0 W)
                )
            )
        )
        (:rule
            (:symbol auto42)
            (:expression
                (:source-memory m2)
                (:target-memory m3)
                (:call
                    (:conditions
                        (greater_zero M_count)
                    )
                    (:callee tower)
                    (:arguments O_arg M)
                )
            )
        )
    )
))RUNIR";
}

std::string ModuleFactory::create_blocks_bonet_et_al_icaps2024_description()
{
    return R"RUNIR((:module
    (:symbol blocks)
    (:arguments
        (:role O)
    )
    (:registers
        (:concept r0)
    )
    (:entry m0)
    (:memory m0 m1)
    (:features
        (:concept
            (:symbol L_load)
            (:expression
                (c_and
                    (c_all
                        (r_transitive_closure
                            (r_atomic_state "on")
                        )
                        (c_same_as
                            (r_argument O)
                            (r_atomic_state "on")
                        )
                    )
                    (c_not
                        (c_same_as
                            (r_argument O)
                            (r_atomic_state "on")
                        )
                    )
                )
            )
        )
        (:numerical
            (:symbol L)
            (:expression
                (n_count
                    (c_and
                        (c_all
                            (r_transitive_closure
                                (r_atomic_state "on")
                            )
                            (c_same_as
                                (r_argument O)
                                (r_atomic_state "on")
                            )
                        )
                        (c_not
                            (c_same_as
                                (r_argument O)
                                (r_atomic_state "on")
                            )
                        )
                    )
                )
            )
        )
        (:role
            (:symbol O_arg)
            (:expression
                (r_argument O)
            )
        )
        (:concept
            (:symbol R0)
            (:expression
                (c_register r0)
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto43)
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:load
                    (:conditions
                        (greater_zero L)
                    )
                    (:concept L_load)
                    (:register
                        (:concept r0)
                    )
                )
            )
        )
        (:rule
            (:symbol auto45)
            (:expression
                (:source-memory m1)
                (:target-memory m0)
                (:call
                    (:conditions)
                    (:callee tower)
                    (:arguments O_arg R0)
                )
            )
        )
    )
))RUNIR";
}

}  // namespace runir::kr::ps::ext::dl
