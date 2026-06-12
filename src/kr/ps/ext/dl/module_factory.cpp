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

    auto data = ygg::Data<Module>(std::string("empty"));
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
    return R"(
(:module
  (:symbol empty)
  (:arguments)
  (:description "")
  (:registers)
  (:entry m0)
  (:memory m0)
  (:features)
  (:rules))
)";
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
    return R"((:program
    (:entry "root")
    (:module
        (:symbol root)
        (:arguments
        )
        (:description "")
        (:registers
        )
        (:entry m0)
        (:memory
            m0
            m1
        )
        (:features
        )
        (:rules
            (:rule
                (:symbol auto1)
                (:description "")
                (:expression
                    (:source-memory m0)
                    (:target-memory m1)
                    (:call
                        (:conditions)
                        (:callee "blocks")
                        (:arguments
                            (r_atomic_goal
                                "on"
                                true)
                        )
                    )
                )
            )
        )
    )
    (:module
        (:symbol blocks)
        (:arguments
            (:role "O" 0)
        )
        (:description "")
        (:registers
            (:concept (:symbol 0))
        )
        (:entry m0)
        (:memory
            m0
            m1
        )
        (:features
            (:concept
                (:symbol L)
                (:description "lowest misplaced block")
                (:expression
                    (c_and
                        (c_not
                            (c_same_as
                                (r_atomic_state
                                    "on")
                                (r_argument
                                    0)))
                        (c_all
                            (r_transitive_closure
                                (r_atomic_state
                                    "on"))
                            (c_same_as
                                (r_atomic_state
                                    "on")
                                (r_argument
                                    0))))
                )
            )
        )
        (:rules
            (:rule
                (:symbol auto43)
                (:description "")
                (:expression
                    (:source-memory m0)
                    (:target-memory m1)
                    (:load
                        (:expression
                            (:conditions
                                (:greater_zero L)
                            )
                            (:concept
                                (c_and
                                    (c_not
                                        (c_same_as
                                            (r_atomic_state
                                                "on")
                                            (r_argument
                                                0)))
                                    (c_all
                                        (r_transitive_closure
                                            (r_atomic_state
                                                "on"))
                                        (c_same_as
                                            (r_atomic_state
                                                "on")
                                            (r_argument
                                                0))))
                            )
                            (:register 0)
                        )
                    )
                )
            )
            (:rule
                (:symbol auto45)
                (:description "")
                (:expression
                    (:source-memory m1)
                    (:target-memory m0)
                    (:call
                        (:conditions)
                        (:callee "tower")
                        (:arguments
                            (r_argument
                                0)
                            (c_register
                                0)
                        )
                    )
                )
            )
        )
    )
    (:module
        (:symbol tower)
        (:arguments
            (:concept "X" 0)
            (:role "O" 0)
        )
        (:description "")
        (:registers
            (:concept (:symbol 0))
        )
        (:entry m0)
        (:memory
            m0
            m1
            m2
            m3
        )
        (:features
            (:concept
                (:symbol X)
                (:description "current block argument")
                (:expression
                    (c_argument
                        0)
                )
            )
            (:concept
                (:symbol W)
                (:description "block below r0 in target tower O")
                (:expression
                    (c_some
                        (r_inverse
                            (r_argument
                                0))
                        (c_register
                            0))
                )
            )
            (:concept
                (:symbol M)
                (:description "block above r0 in target tower O")
                (:expression
                    (c_some
                        (r_argument
                            0)
                        (c_register
                            0))
                )
            )
        )
        (:rules
            (:rule
                (:symbol auto38)
                (:description "")
                (:expression
                    (:source-memory m0)
                    (:target-memory m1)
                    (:load
                        (:expression
                            (:conditions
                                (:greater_zero X)
                            )
                            (:concept
                                (c_argument
                                    0)
                            )
                            (:register 0)
                        )
                    )
                )
            )
            (:rule
                (:symbol auto40)
                (:description "")
                (:expression
                    (:source-memory m1)
                    (:target-memory m2)
                    (:call
                        (:conditions
                            (:equal_zero W)
                        )
                        (:callee "on-table")
                        (:arguments
                            (c_register
                                0)
                        )
                    )
                )
            )
            (:rule
                (:symbol auto41)
                (:description "")
                (:expression
                    (:source-memory m1)
                    (:target-memory m2)
                    (:call
                        (:conditions
                            (:greater_zero W)
                        )
                        (:callee "on")
                        (:arguments
                            (c_register
                                0)
                            (c_some
                                (r_inverse
                                    (r_argument
                                        0))
                                (c_register
                                    0))
                        )
                    )
                )
            )
            (:rule
                (:symbol auto42)
                (:description "")
                (:expression
                    (:source-memory m2)
                    (:target-memory m3)
                    (:call
                        (:conditions
                            (:greater_zero M)
                        )
                        (:callee "tower")
                        (:arguments
                            (r_argument
                                0)
                            (c_some
                                (r_argument
                                    0)
                                (c_register
                                    0))
                        )
                    )
                )
            )
        )
    )
    (:module
        (:symbol on-table)
        (:arguments
            (:concept "X" 0)
        )
        (:description "")
        (:registers
        )
        (:entry m0)
        (:memory
            m0
            m1
            m2
        )
        (:features
            (:boolean
                (:symbol H)
                (:description "holding a block")
                (:expression
                    (b_nonempty
                        (c_atomic_state
                            "holding"))
                )
            )
            (:boolean
                (:symbol Tx)
                (:description "X is on the table")
                (:expression
                    (b_nonempty
                        (c_and
                            (c_argument
                                0)
                            (c_atomic_state
                                "on-table")))
                )
            )
        
            (:concept
                (:symbol DO_on_table_1)
                (:description "")
                (:expression
                    (c_argument
                                                        0))
            )
            (:concept
                (:symbol DO_on_table_2)
                (:description "")
                (:expression
                    (c_top))
            )
            (:concept
                (:symbol DO_on_table_3)
                (:description "")
                (:expression
                    (c_argument
                                                        0))
            )
        )
        (:rules
            (:rule
                (:symbol auto32)
                (:description "")
                (:expression
                    (:source-memory m0)
                    (:target-memory m1)
                    (:do
                        (:expression
                            (:conditions
                                (:negative H)
                                (:negative Tx)
                            )
                            (:action "unstack")
                            (:arguments
                                DO_on_table_1
                                DO_on_table_2
                            )
                            (:effects)
                        )
                    )
                )
            )
            (:rule
                (:symbol auto34)
                (:description "")
                (:expression
                    (:source-memory m0)
                    (:target-memory m1)
                    (:sketch
                        (:expression
                            (:conditions
                                (:positive H)
                            )
                            (:effects)
                        )
                    )
                )
            )
            (:rule
                (:symbol auto30)
                (:description "")
                (:expression
                    (:source-memory m0)
                    (:target-memory m2)
                    (:sketch
                        (:expression
                            (:conditions
                                (:positive Tx)
                            )
                            (:effects)
                        )
                    )
                )
            )
            (:rule
                (:symbol auto36)
                (:description "")
                (:expression
                    (:source-memory m1)
                    (:target-memory m2)
                    (:do
                        (:expression
                            (:conditions)
                            (:action "putdown")
                            (:arguments
                                DO_on_table_3
                            )
                            (:effects)
                        )
                    )
                )
            )
        )
    )
    (:module
        (:symbol on)
        (:arguments
            (:concept "X" 0)
            (:concept "Y" 1)
        )
        (:description "")
        (:registers
            (:concept (:symbol 0))
            (:concept (:symbol 1))
        )
        (:entry m0)
        (:memory
            m0
            m1
            m2
            m3
            m4
            m5
            m6
            m7
            m8
        )
        (:features
            (:concept
                (:symbol N)
                (:description "blocks above X or Y")
                (:expression
                    (c_and
                        (c_not
                            (c_atomic_state
                                "clear"))
                        (c_or
                            (c_argument
                                0)
                            (c_argument
                                1)))
                )
            )
            (:concept
                (:symbol T0)
                (:description "block directly above r0")
                (:expression
                    (c_some
                        (r_inverse
                            (r_atomic_state
                                "on"))
                        (c_register
                            0))
                )
            )
            (:concept
                (:symbol T1)
                (:description "block directly above r1")
                (:expression
                    (c_some
                        (r_inverse
                            (r_atomic_state
                                "on"))
                        (c_register
                            1))
                )
            )
            (:boolean
                (:symbol H)
                (:description "holding a block")
                (:expression
                    (b_nonempty
                        (c_atomic_state
                            "holding"))
                )
            )
            (:boolean
                (:symbol Tx)
                (:description "X is on the table")
                (:expression
                    (b_nonempty
                        (c_and
                            (c_argument
                                0)
                            (c_atomic_state
                                "on-table")))
                )
            )
        
            (:concept
                (:symbol DO_on_1)
                (:description "")
                (:expression
                    (c_top))
            )
            (:concept
                (:symbol DO_on_2)
                (:description "")
                (:expression
                    (c_register
                                                        1))
            )
            (:concept
                (:symbol DO_on_3)
                (:description "")
                (:expression
                    (c_top))
            )
            (:concept
                (:symbol DO_on_4)
                (:description "")
                (:expression
                    (c_register
                                                        1))
            )
            (:concept
                (:symbol DO_on_5)
                (:description "")
                (:expression
                    (c_argument
                                                        0))
            )
            (:concept
                (:symbol DO_on_6)
                (:description "")
                (:expression
                    (c_argument
                                                        0))
            )
            (:concept
                (:symbol DO_on_7)
                (:description "")
                (:expression
                    (c_top))
            )
            (:concept
                (:symbol DO_on_8)
                (:description "")
                (:expression
                    (c_argument
                                                        0))
            )
            (:concept
                (:symbol DO_on_9)
                (:description "")
                (:expression
                    (c_argument
                                                        1))
            )
        )
        (:rules
            (:rule
                (:symbol auto6)
                (:description "")
                (:expression
                    (:source-memory m0)
                    (:target-memory m1)
                    (:load
                        (:expression
                            (:conditions
                                (:negative H)
                                (:greater_zero N)
                            )
                            (:concept
                                (c_and
                                    (c_not
                                        (c_atomic_state
                                            "clear"))
                                    (c_or
                                        (c_argument
                                            0)
                                        (c_argument
                                            1)))
                            )
                            (:register 0)
                        )
                    )
                )
            )
            (:rule
                (:symbol auto2)
                (:description "")
                (:expression
                    (:source-memory m0)
                    (:target-memory m4)
                    (:sketch
                        (:expression
                            (:conditions
                                (:positive H)
                            )
                            (:effects)
                        )
                    )
                )
            )
            (:rule
                (:symbol auto4)
                (:description "")
                (:expression
                    (:source-memory m0)
                    (:target-memory m7)
                    (:sketch
                        (:expression
                            (:conditions
                                (:negative H)
                                (:equal_zero N)
                            )
                            (:effects)
                        )
                    )
                )
            )
            (:rule
                (:symbol auto8)
                (:description "")
                (:expression
                    (:source-memory m1)
                    (:target-memory m2)
                    (:load
                        (:expression
                            (:conditions
                                (:greater_zero T0)
                            )
                            (:concept
                                (c_some
                                    (r_inverse
                                        (r_atomic_state
                                            "on"))
                                    (c_register
                                        0))
                            )
                            (:register 1)
                        )
                    )
                )
            )
            (:rule
                (:symbol auto10)
                (:description "")
                (:expression
                    (:source-memory m2)
                    (:target-memory m2)
                    (:load
                        (:expression
                            (:conditions
                                (:greater_zero T1)
                            )
                            (:concept
                                (c_some
                                    (r_inverse
                                        (r_atomic_state
                                            "on"))
                                    (c_register
                                        1))
                            )
                            (:register 1)
                        )
                    )
                )
            )
            (:rule
                (:symbol auto12)
                (:description "")
                (:expression
                    (:source-memory m2)
                    (:target-memory m5)
                    (:sketch
                        (:expression
                            (:conditions
                                (:equal_zero T1)
                            )
                            (:effects)
                        )
                    )
                )
            )
            (:rule
                (:symbol auto16)
                (:description "")
                (:expression
                    (:source-memory m3)
                    (:target-memory m0)
                    (:sketch
                        (:expression
                            (:conditions
                                (:equal_zero T0)
                            )
                            (:effects)
                        )
                    )
                )
            )
            (:rule
                (:symbol auto14)
                (:description "")
                (:expression
                    (:source-memory m3)
                    (:target-memory m1)
                    (:sketch
                        (:expression
                            (:conditions
                                (:greater_zero T0)
                            )
                            (:effects)
                        )
                    )
                )
            )
            (:rule
                (:symbol auto18)
                (:description "")
                (:expression
                    (:source-memory m4)
                    (:target-memory m0)
                    (:do
                        (:expression
                            (:conditions)
                            (:action "putdown")
                            (:arguments
                                DO_on_1
                            )
                            (:effects)
                        )
                    )
                )
            )
            (:rule
                (:symbol auto20)
                (:description "")
                (:expression
                    (:source-memory m5)
                    (:target-memory m6)
                    (:do
                        (:expression
                            (:conditions)
                            (:action "unstack")
                            (:arguments
                                DO_on_2
                                DO_on_3
                            )
                            (:effects)
                        )
                    )
                )
            )
            (:rule
                (:symbol auto22)
                (:description "")
                (:expression
                    (:source-memory m6)
                    (:target-memory m3)
                    (:do
                        (:expression
                            (:conditions)
                            (:action "putdown")
                            (:arguments
                                DO_on_4
                            )
                            (:effects)
                        )
                    )
                )
            )
            (:rule
                (:symbol auto24)
                (:description "")
                (:expression
                    (:source-memory m7)
                    (:target-memory m8)
                    (:do
                        (:expression
                            (:conditions
                                (:positive Tx)
                            )
                            (:action "pickup")
                            (:arguments
                                DO_on_5
                            )
                            (:effects)
                        )
                    )
                )
            )
            (:rule
                (:symbol auto26)
                (:description "")
                (:expression
                    (:source-memory m7)
                    (:target-memory m8)
                    (:do
                        (:expression
                            (:conditions
                                (:negative Tx)
                            )
                            (:action "unstack")
                            (:arguments
                                DO_on_6
                                DO_on_7
                            )
                            (:effects)
                        )
                    )
                )
            )
            (:rule
                (:symbol auto28)
                (:description "")
                (:expression
                    (:source-memory m8)
                    (:target-memory m8)
                    (:do
                        (:expression
                            (:conditions)
                            (:action "stack")
                            (:arguments
                                DO_on_8
                                DO_on_9
                            )
                            (:effects)
                        )
                    )
                )
            )
        )
    )
)
)";
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
    return R"((:module
    (:symbol on)
    (:arguments
        (:concept "X" 0)
        (:concept "Y" 1)
    )
    (:description "")
    (:registers
        (:concept (:symbol 0))
        (:concept (:symbol 1))
    )
    (:entry m0)
    (:memory
        m0
        m1
        m2
        m3
        m4
        m5
        m6
        m7
        m8
    )
    (:features
        (:concept
            (:symbol N)
            (:description "blocks above X or Y")
            (:expression
                (c_and
                    (c_not
                        (c_atomic_state
                            "clear"))
                    (c_or
                        (c_argument
                            0)
                        (c_argument
                            1)))
            )
        )
        (:concept
            (:symbol T0)
            (:description "block directly above r0")
            (:expression
                (c_some
                    (r_inverse
                        (r_atomic_state
                            "on"))
                    (c_register
                        0))
            )
        )
        (:concept
            (:symbol T1)
            (:description "block directly above r1")
            (:expression
                (c_some
                    (r_inverse
                        (r_atomic_state
                            "on"))
                    (c_register
                        1))
            )
        )
        (:boolean
            (:symbol H)
            (:description "holding a block")
            (:expression
                (b_nonempty
                    (c_atomic_state
                        "holding"))
            )
        )
        (:boolean
            (:symbol Tx)
            (:description "X is on the table")
            (:expression
                (b_nonempty
                    (c_and
                        (c_argument
                            0)
                        (c_atomic_state
                            "on-table")))
            )
        )
    
        (:concept
            (:symbol DO_on_1)
            (:description "")
            (:expression
                (c_top))
        )
        (:concept
            (:symbol DO_on_2)
            (:description "")
            (:expression
                (c_register
                                                1))
        )
        (:concept
            (:symbol DO_on_3)
            (:description "")
            (:expression
                (c_top))
        )
        (:concept
            (:symbol DO_on_4)
            (:description "")
            (:expression
                (c_register
                                                1))
        )
        (:concept
            (:symbol DO_on_5)
            (:description "")
            (:expression
                (c_argument
                                                0))
        )
        (:concept
            (:symbol DO_on_6)
            (:description "")
            (:expression
                (c_argument
                                                0))
        )
        (:concept
            (:symbol DO_on_7)
            (:description "")
            (:expression
                (c_top))
        )
        (:concept
            (:symbol DO_on_8)
            (:description "")
            (:expression
                (c_argument
                                                0))
        )
        (:concept
            (:symbol DO_on_9)
            (:description "")
            (:expression
                (c_argument
                                                1))
        )
    )
    (:rules
        (:rule
            (:symbol auto6)
            (:description "")
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:load
                    (:expression
                        (:conditions
                            (:negative H)
                            (:greater_zero N)
                        )
                        (:concept
                            (c_and
                                (c_not
                                    (c_atomic_state
                                        "clear"))
                                (c_or
                                    (c_argument
                                        0)
                                    (c_argument
                                        1)))
                        )
                        (:register 0)
                    )
                )
            )
        )
        (:rule
            (:symbol auto2)
            (:description "")
            (:expression
                (:source-memory m0)
                (:target-memory m4)
                (:sketch
                    (:expression
                        (:conditions
                            (:positive H)
                        )
                        (:effects)
                    )
                )
            )
        )
        (:rule
            (:symbol auto4)
            (:description "")
            (:expression
                (:source-memory m0)
                (:target-memory m7)
                (:sketch
                    (:expression
                        (:conditions
                            (:negative H)
                            (:equal_zero N)
                        )
                        (:effects)
                    )
                )
            )
        )
        (:rule
            (:symbol auto8)
            (:description "")
            (:expression
                (:source-memory m1)
                (:target-memory m2)
                (:load
                    (:expression
                        (:conditions
                            (:greater_zero T0)
                        )
                        (:concept
                            (c_some
                                (r_inverse
                                    (r_atomic_state
                                        "on"))
                                (c_register
                                    0))
                        )
                        (:register 1)
                    )
                )
            )
        )
        (:rule
            (:symbol auto10)
            (:description "")
            (:expression
                (:source-memory m2)
                (:target-memory m2)
                (:load
                    (:expression
                        (:conditions
                            (:greater_zero T1)
                        )
                        (:concept
                            (c_some
                                (r_inverse
                                    (r_atomic_state
                                        "on"))
                                (c_register
                                    1))
                        )
                        (:register 1)
                    )
                )
            )
        )
        (:rule
            (:symbol auto12)
            (:description "")
            (:expression
                (:source-memory m2)
                (:target-memory m5)
                (:sketch
                    (:expression
                        (:conditions
                            (:equal_zero T1)
                        )
                        (:effects)
                    )
                )
            )
        )
        (:rule
            (:symbol auto16)
            (:description "")
            (:expression
                (:source-memory m3)
                (:target-memory m0)
                (:sketch
                    (:expression
                        (:conditions
                            (:equal_zero T0)
                        )
                        (:effects)
                    )
                )
            )
        )
        (:rule
            (:symbol auto14)
            (:description "")
            (:expression
                (:source-memory m3)
                (:target-memory m1)
                (:sketch
                    (:expression
                        (:conditions
                            (:greater_zero T0)
                        )
                        (:effects)
                    )
                )
            )
        )
        (:rule
            (:symbol auto18)
            (:description "")
            (:expression
                (:source-memory m4)
                (:target-memory m0)
                (:do
                    (:expression
                        (:conditions)
                        (:action "putdown")
                        (:arguments
                            DO_on_1
                        )
                        (:effects)
                    )
                )
            )
        )
        (:rule
            (:symbol auto20)
            (:description "")
            (:expression
                (:source-memory m5)
                (:target-memory m6)
                (:do
                    (:expression
                        (:conditions)
                        (:action "unstack")
                        (:arguments
                            DO_on_2
                            DO_on_3
                        )
                        (:effects)
                    )
                )
            )
        )
        (:rule
            (:symbol auto22)
            (:description "")
            (:expression
                (:source-memory m6)
                (:target-memory m3)
                (:do
                    (:expression
                        (:conditions)
                        (:action "putdown")
                        (:arguments
                            DO_on_4
                        )
                        (:effects)
                    )
                )
            )
        )
        (:rule
            (:symbol auto24)
            (:description "")
            (:expression
                (:source-memory m7)
                (:target-memory m8)
                (:do
                    (:expression
                        (:conditions
                            (:positive Tx)
                        )
                        (:action "pickup")
                        (:arguments
                            DO_on_5
                        )
                        (:effects)
                    )
                )
            )
        )
        (:rule
            (:symbol auto26)
            (:description "")
            (:expression
                (:source-memory m7)
                (:target-memory m8)
                (:do
                    (:expression
                        (:conditions
                            (:negative Tx)
                        )
                        (:action "unstack")
                        (:arguments
                            DO_on_6
                            DO_on_7
                        )
                        (:effects)
                    )
                )
            )
        )
        (:rule
            (:symbol auto28)
            (:description "")
            (:expression
                (:source-memory m8)
                (:target-memory m8)
                (:do
                    (:expression
                        (:conditions)
                        (:action "stack")
                        (:arguments
                            DO_on_8
                            DO_on_9
                        )
                        (:effects)
                    )
                )
            )
        )
    )
)
)";
}

std::string ModuleFactory::create_on_table_bonet_et_al_icaps2024_description()
{
    return R"((:module
    (:symbol on-table)
    (:arguments
        (:concept "X" 0)
    )
    (:description "")
    (:registers
    )
    (:entry m0)
    (:memory
        m0
        m1
        m2
    )
    (:features
        (:boolean
            (:symbol H)
            (:description "holding a block")
            (:expression
                (b_nonempty
                    (c_atomic_state
                        "holding"))
            )
        )
        (:boolean
            (:symbol Tx)
            (:description "X is on the table")
            (:expression
                (b_nonempty
                    (c_and
                        (c_argument
                            0)
                        (c_atomic_state
                            "on-table")))
            )
        )
    
        (:concept
            (:symbol DO_on_table_1)
            (:description "")
            (:expression
                (c_argument
                                                0))
        )
        (:concept
            (:symbol DO_on_table_2)
            (:description "")
            (:expression
                (c_top))
        )
        (:concept
            (:symbol DO_on_table_3)
            (:description "")
            (:expression
                (c_argument
                                                0))
        )
    )
    (:rules
        (:rule
            (:symbol auto32)
            (:description "")
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:do
                    (:expression
                        (:conditions
                            (:negative H)
                            (:negative Tx)
                        )
                        (:action "unstack")
                        (:arguments
                            DO_on_table_1
                            DO_on_table_2
                        )
                        (:effects)
                    )
                )
            )
        )
        (:rule
            (:symbol auto34)
            (:description "")
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:sketch
                    (:expression
                        (:conditions
                            (:positive H)
                        )
                        (:effects)
                    )
                )
            )
        )
        (:rule
            (:symbol auto30)
            (:description "")
            (:expression
                (:source-memory m0)
                (:target-memory m2)
                (:sketch
                    (:expression
                        (:conditions
                            (:positive Tx)
                        )
                        (:effects)
                    )
                )
            )
        )
        (:rule
            (:symbol auto36)
            (:description "")
            (:expression
                (:source-memory m1)
                (:target-memory m2)
                (:do
                    (:expression
                        (:conditions)
                        (:action "putdown")
                        (:arguments
                            DO_on_table_3
                        )
                        (:effects)
                    )
                )
            )
        )
    )
)
)";
}

std::string ModuleFactory::create_tower_bonet_et_al_icaps2024_description()
{
    return R"((:module
    (:symbol tower)
    (:arguments
        (:concept "X" 0)
        (:role "O" 0)
    )
    (:description "")
    (:registers
        (:concept (:symbol 0))
    )
    (:entry m0)
    (:memory
        m0
        m1
        m2
        m3
    )
    (:features
        (:concept
            (:symbol X)
            (:description "current block argument")
            (:expression
                (c_argument
                    0)
            )
        )
        (:concept
            (:symbol W)
            (:description "block below r0 in target tower O")
            (:expression
                (c_some
                    (r_inverse
                        (r_argument
                            0))
                    (c_register
                        0))
            )
        )
        (:concept
            (:symbol M)
            (:description "block above r0 in target tower O")
            (:expression
                (c_some
                    (r_argument
                        0)
                    (c_register
                        0))
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto38)
            (:description "")
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:load
                    (:expression
                        (:conditions
                            (:greater_zero X)
                        )
                        (:concept
                            (c_argument
                                0)
                        )
                        (:register 0)
                    )
                )
            )
        )
        (:rule
            (:symbol auto40)
            (:description "")
            (:expression
                (:source-memory m1)
                (:target-memory m2)
                (:call
                    (:conditions
                        (:equal_zero W)
                    )
                    (:callee "on-table")
                    (:arguments
                        (c_register
                            0)
                    )
                )
            )
        )
        (:rule
            (:symbol auto41)
            (:description "")
            (:expression
                (:source-memory m1)
                (:target-memory m2)
                (:call
                    (:conditions
                        (:greater_zero W)
                    )
                    (:callee "on")
                    (:arguments
                        (c_register
                            0)
                        (c_some
                            (r_inverse
                                (r_argument
                                    0))
                            (c_register
                                0))
                    )
                )
            )
        )
        (:rule
            (:symbol auto42)
            (:description "")
            (:expression
                (:source-memory m2)
                (:target-memory m3)
                (:call
                    (:conditions
                        (:greater_zero M)
                    )
                    (:callee "tower")
                    (:arguments
                        (r_argument
                            0)
                        (c_some
                            (r_argument
                                0)
                            (c_register
                                0))
                    )
                )
            )
        )
    )
)
)";
}

std::string ModuleFactory::create_blocks_bonet_et_al_icaps2024_description()
{
    return R"((:module
    (:symbol blocks)
    (:arguments
        (:role "O" 0)
    )
    (:description "")
    (:registers
        (:concept (:symbol 0))
    )
    (:entry m0)
    (:memory
        m0
        m1
    )
    (:features
        (:concept
            (:symbol L)
            (:description "lowest misplaced block")
            (:expression
                (c_and
                    (c_not
                        (c_same_as
                            (r_atomic_state
                                "on")
                            (r_argument
                                0)))
                    (c_all
                        (r_transitive_closure
                            (r_atomic_state
                                "on"))
                        (c_same_as
                            (r_atomic_state
                                "on")
                            (r_argument
                                0))))
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto43)
            (:description "")
            (:expression
                (:source-memory m0)
                (:target-memory m1)
                (:load
                    (:expression
                        (:conditions
                            (:greater_zero L)
                        )
                        (:concept
                            (c_and
                                (c_not
                                    (c_same_as
                                        (r_atomic_state
                                            "on")
                                        (r_argument
                                            0)))
                                (c_all
                                    (r_transitive_closure
                                        (r_atomic_state
                                            "on"))
                                    (c_same_as
                                        (r_atomic_state
                                            "on")
                                        (r_argument
                                            0))))
                        )
                        (:register 0)
                    )
                )
            )
        )
        (:rule
            (:symbol auto45)
            (:description "")
            (:expression
                (:source-memory m1)
                (:target-memory m0)
                (:call
                    (:conditions)
                    (:callee "tower")
                    (:arguments
                        (r_argument
                            0)
                        (c_register
                            0)
                    )
                )
            )
        )
    )
)
)";
}

}  // namespace runir::kr::ps::ext::dl
