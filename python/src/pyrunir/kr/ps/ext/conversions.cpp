// The conversions from Python of the builders of the ext family's programs and of its execution control's records (conversions.hpp), with what their builders
// hold complete (the program executors' bindings bind them).

#include "pyrunir/kr/ps/ext/conversions.hpp"

#include <functional>
#include <nanobind/stl/chrono.h>
#include <nanobind/stl/function.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/string_view.h>
#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>
#include <optional>
#include <runir/kr/database/syntax/query_repository.hpp>
#include <runir/kr/dl/repository.hpp>
#include <runir/kr/dl/semantics/denotation_repository.hpp>
#include <runir/kr/dl/semantics/evaluation_policy.hpp>
#include <runir/kr/ps/dl/declarations.hpp>
#include <runir/kr/ps/dl/evaluation.hpp>
#include <runir/kr/ps/ext/detail/successor_expander_impl.hpp>
#include <runir/kr/ps/ext/evaluation_environment.hpp>
#include <runir/kr/ps/ext/formatter.hpp>
#include <runir/kr/ps/ext/program_executor.hpp>
#include <runir/kr/ps/ext/repository.hpp>
#include <runir/kr/ps/icp/evaluation_environment.hpp>
#include <runir/kr/ps/icp/formatter.hpp>
#include <runir/kr/ps/icp/program_executor.hpp>
#include <runir/kr/ps/icp/repository.hpp>
#include <runir/kr/ps/icp/successor_expander.hpp>
#include <runir/kr/task_context.hpp>
#include <string>
#include <tyr/planning/declarations.hpp>
#include <urd/dot.hpp>
#include <urd/python/bindings.hpp>
#include <urd/python/graphs.hpp>
#include <utility>
#include <vefr/python/bindings.hpp>
#include <yggdrasil/python/bindings.hpp>

URD_PYTHON_CONVERSIONS(, runir::kr::ps::ext::Repository, ygg::formalism::symbol_types_t<runir::kr::ps::ext::Repository>, 51)
URD_PYTHON_CONVERSIONS(, runir::kr::ps::ext::ControlRepository, ygg::formalism::symbol_types_t<runir::kr::ps::ext::ControlRepository>, 2)
