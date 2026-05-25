#include "runir/kr/ps/ext/dl/module_factory.hpp"

#include "runir/kr/ps/ext/canonicalization.hpp"
#include "runir/kr/ps/ext/dl/parser.hpp"

#include <stdexcept>
#include <string>
#include <vector>

namespace runir::kr::ps::ext::dl
{

ModuleView ModuleFactory::create_empty(Repository& repository)
{
    auto entry_data = tyr::Data<MemoryState>(std::string("m0"));
    const auto entry = repository.get_or_create(entry_data).first;

    auto data = tyr::Data<Module>(std::string("empty"));
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
    return R"((module "empty"
  (:arguments)
  (:registers)
  (:entry "m0")
  (:memory "m0")
  (:features)
  (:transitions)
)
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

    throw std::runtime_error("Unknown module specification.");
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
    return std::string(R"((program
  (:entry "root")
  (module "root"
    (:arguments)
    (:registers)
    (:entry "m0")
    (:memory "m0" "m1")
    (:features)
    (:transitions
      ("m0" "m1" (call (:conditions) (:callee "blocks") (:arguments (r_atomic_goal "on" true))))
    )
  )
)") + create_blocks_bonet_et_al_icaps2024_description()
           + create_tower_bonet_et_al_icaps2024_description() + create_on_table_bonet_et_al_icaps2024_description()
           + create_on_bonet_et_al_icaps2024_description() + ")\n";
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
    return R"((module "on"
  (:arguments
    (concept "X" 0)
    (concept "Y" 1)
  )
  (:registers
    (concept "r0" 0)
    (concept "r1" 1)
  )
  (:entry "m0")
  (:memory "m0" "m1" "m2" "m3" "m4" "m5" "m6" "m7" "m8")
  (:features
    (concept B "B" "all blocks" (c_top))
    (concept N "N" "blocks above X or Y" (c_and (c_not (c_atomic_state "clear")) (c_or (c_argument 0) (c_argument 1))))
    (concept T0 "T0" "block directly above r0" (c_some (r_inverse (r_atomic_state "on")) (c_register 0)))
    (concept T1 "T1" "block directly above r1" (c_some (r_inverse (r_atomic_state "on")) (c_register 1)))
    (boolean H "H" "holding a block" (b_nonempty (c_atomic_state "holding")))
    (boolean Tx "Tx" "X is on the table" (b_nonempty (c_and (c_argument 0) (c_atomic_state "on-table"))))
  )
  (:transitions
    ("m0" "m4" (sketch (:conditions (positive H)) (:effects)))
    ("m0" "m7" (sketch (:conditions (negative H) (equal_zero N)) (:effects)))
    ("m0" "m1" (load (:conditions (negative H) (greater_zero N)) (:concept (c_and (c_not (c_atomic_state "clear")) (c_or (c_argument 0) (c_argument 1)))) (:register "r0")))
    ("m1" "m2" (load (:conditions (greater_zero T0)) (:concept (c_some (r_inverse (r_atomic_state "on")) (c_register 0))) (:register "r1")))
    ("m2" "m2" (load (:conditions (greater_zero T1)) (:concept (c_some (r_inverse (r_atomic_state "on")) (c_register 1))) (:register "r1")))
    ("m2" "m5" (sketch (:conditions (equal_zero T1)) (:effects)))
    ("m3" "m1" (sketch (:conditions (greater_zero T0)) (:effects)))
    ("m3" "m0" (sketch (:conditions (equal_zero T0)) (:effects)))
    ("m4" "m0" (do (:conditions) (:action "putdown") (:arguments (c_top))))
    ("m5" "m6" (do (:conditions) (:action "unstack") (:arguments (c_register 1) (c_top))))
    ("m6" "m3" (do (:conditions) (:action "putdown") (:arguments (c_register 1))))
    ("m7" "m8" (do (:conditions (positive Tx)) (:action "pickup") (:arguments (c_argument 0))))
    ("m7" "m8" (do (:conditions (negative Tx)) (:action "unstack") (:arguments (c_argument 0) B)))
    ("m8" "m8" (do (:conditions) (:action "stack") (:arguments (c_argument 0) (c_argument 1))))
  )
)
)";
}

std::string ModuleFactory::create_on_table_bonet_et_al_icaps2024_description()
{
    return R"((module "on-table"
  (:arguments
    (concept "X" 0)
  )
  (:registers)
  (:entry "m0")
  (:memory "m0" "m1" "m2")
  (:features
    (boolean H "H" "holding a block" (b_nonempty (c_atomic_state "holding")))
    (boolean Tx "Tx" "X is on the table" (b_nonempty (c_and (c_argument 0) (c_atomic_state "on-table"))))
  )
  (:transitions
    ("m0" "m2" (sketch (:conditions (positive Tx)) (:effects)))
    ("m0" "m1" (do (:conditions (negative Tx) (negative H)) (:action "unstack") (:arguments (c_argument 0) (c_top))))
    ("m0" "m1" (sketch (:conditions (positive H)) (:effects)))
    ("m1" "m2" (do (:conditions) (:action "putdown") (:arguments (c_argument 0))))
  )
)
)";
}

std::string ModuleFactory::create_tower_bonet_et_al_icaps2024_description()
{
    return R"((module "tower"
  (:arguments
    (role "O" 0)
    (concept "X" 0)
  )
  (:registers
    (concept "r0" 0)
  )
  (:entry "m0")
  (:memory "m0" "m1" "m2" "m3")
  (:features
    (concept X "X" "current block argument" (c_argument 0))
    (concept M "M" "block above r0 in target tower O" (c_some (r_argument 0) (c_register 0)))
    (concept W "W" "block below r0 in target tower O" (c_some (r_inverse (r_argument 0)) (c_register 0)))
  )
  (:transitions
    ("m0" "m1" (load (:conditions (greater_zero X)) (:concept (c_argument 0)) (:register "r0")))
    ("m1" "m2" (call (:conditions (equal_zero W)) (:callee "on-table") (:arguments (c_register 0))))
    ("m1" "m2" (call (:conditions (greater_zero W)) (:callee "on") (:arguments (c_register 0) (c_some (r_inverse (r_argument 0)) (c_register 0)))))
    ("m2" "m3" (call (:conditions (greater_zero M)) (:callee "tower") (:arguments (r_argument 0) (c_some (r_argument 0) (c_register 0)))))
  )
)
)";
}

std::string ModuleFactory::create_blocks_bonet_et_al_icaps2024_description()
{
    return R"((module "blocks"
  (:arguments
    (role "O" 0)
  )
  (:registers
    (concept "r0" 0)
  )
  (:entry "m0")
  (:memory "m0" "m1")
  (:features
    (concept L "L" "lowest misplaced block" (c_and (c_not (c_same_as (r_atomic_state "on") (r_argument 0))) (c_all (r_transitive_closure (r_atomic_state "on")) (c_same_as (r_atomic_state "on") (r_argument 0)))))
  )
  (:transitions
    ("m0" "m1" (load (:conditions (greater_zero L)) (:concept L) (:register "r0")))
    ("m1" "m0" (call (:conditions) (:callee "tower") (:arguments (r_argument 0) (c_register 0))))
  )
)
)";
}

}  // namespace runir::kr::ps::ext::dl
