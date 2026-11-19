#include "runir/kr/dl/cnf_grammar/constructor_repository.hpp"

#include <yggdrasil/formalism/builder_instantiations.hpp>

// The builder storage of the repository, which the library's other sources only declare (yggdrasil/formalism/builder.hpp).
// A source of its own: instantiated in constructor_repository.cpp together with the repository's symbols and interning,
// GCC 13 writes LTO bytecode it cannot read back (lto1: internal compiler error in lto_tag_to_tree_code); each part
// alone, or any two, is read back.
YGG_FORMALISM_BUILDER(, runir::kr::dl::cnf_grammar::Builder, 62)
