#ifndef RUNIR_REPOSITORY_HPP_
#define RUNIR_REPOSITORY_HPP_

#include "runir/canonicalization.hpp"
#include "runir/declarations.hpp"
#include "runir/semantics/datas.hpp"
#include "runir/semantics/views.hpp"

#include <memory>
#include <tyr/common/type_list.hpp>
#include <tyr/common/types.hpp>
#include <tyr/formalism/planning/declarations.hpp>
#include <tyr/formalism/planning/repository.hpp>
#include <tyr/formalism/relation_repository.hpp>
#include <tyr/formalism/repository.hpp>
#include <tyr/formalism/repository_factory.hpp>
#include <tyr/formalism/symbol_repository.hpp>
#include <vector>

namespace runir
{

using ConceptTypes = tyr::MapTypeListT<Concept, ConceptConstructorTags>;
using RoleTypes = tyr::MapTypeListT<Role, RoleConstructorTags>;
using BooleanTypes = tyr::MapTypeListT<Boolean, BooleanConstructorTags>;
using NumericalTypes = tyr::MapTypeListT<Numerical, NumericalConstructorTags>;
using ConstructorTypes = tyr::MapTypeListT<Constructor, CategoryTags>;

using ConstructorRepositoryTypes =
    tyr::ConcatTypeListsT<tyr::formalism::planning::SymbolRepositoryTypes, ConceptTypes, RoleTypes, BooleanTypes, NumericalTypes, ConstructorTypes>;

using ConstructorSymbolRepository = tyr::ApplyTypeListT<tyr::formalism::SymbolRepository, ConstructorRepositoryTypes>;
using ConstructorRelationRepository = tyr::ApplyTypeListT<tyr::formalism::RelationRepository, tyr::formalism::planning::RelationRepositoryTypes>;

using ConstructorRepository = tyr::formalism::Repository<ConstructorSymbolRepository, ConstructorRelationRepository>;
using ConstructorRepositoryPtr = std::shared_ptr<ConstructorRepository>;

using ConstructorRepositoryFactory = tyr::formalism::RepositoryFactory<ConstructorSymbolRepository, ConstructorRelationRepository>;
using ConstructorRepositoryFactoryPtr = std::shared_ptr<ConstructorRepositoryFactory>;

template<ConceptConstructorTag Tag>
using ConceptView = tyr::View<tyr::Index<Concept<Tag>>, ConstructorRepository>;
template<ConceptConstructorTag Tag>
using ConceptListView = tyr::View<tyr::IndexList<Concept<Tag>>, ConstructorRepository>;
template<ConceptConstructorTag Tag>
using ConceptViewList = std::vector<ConceptView<Tag>>;

template<RoleConstructorTag Tag>
using RoleView = tyr::View<tyr::Index<Role<Tag>>, ConstructorRepository>;
template<RoleConstructorTag Tag>
using RoleListView = tyr::View<tyr::IndexList<Role<Tag>>, ConstructorRepository>;
template<RoleConstructorTag Tag>
using RoleViewList = std::vector<RoleView<Tag>>;

template<BooleanConstructorTag Tag>
using BooleanView = tyr::View<tyr::Index<Boolean<Tag>>, ConstructorRepository>;
template<BooleanConstructorTag Tag>
using BooleanListView = tyr::View<tyr::IndexList<Boolean<Tag>>, ConstructorRepository>;
template<BooleanConstructorTag Tag>
using BooleanViewList = std::vector<BooleanView<Tag>>;

template<NumericalConstructorTag Tag>
using NumericalView = tyr::View<tyr::Index<Numerical<Tag>>, ConstructorRepository>;
template<NumericalConstructorTag Tag>
using NumericalListView = tyr::View<tyr::IndexList<Numerical<Tag>>, ConstructorRepository>;
template<NumericalConstructorTag Tag>
using NumericalViewList = std::vector<NumericalView<Tag>>;

template<CategoryTag Category>
using ConstructorView = tyr::View<tyr::Index<Constructor<Category>>, ConstructorRepository>;
template<CategoryTag Category>
using ConstructorListView = tyr::View<tyr::IndexList<Constructor<Category>>, ConstructorRepository>;
template<CategoryTag Category>
using ConstructorViewList = std::vector<ConstructorView<Category>>;

}

#endif
