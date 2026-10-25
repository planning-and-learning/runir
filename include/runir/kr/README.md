# Template dimensions

Runir separates the policy representation from the language used to express its
features. A policy family selects the policy vocabulary and its execution
semantics. All policy families share one DL constructor set; each language's
parser rejects the constructors it does not support.

## Policy family and feature language

The tags are declared in [declarations.hpp](declarations.hpp).
[PsFamilyTraits](ps/family_traits.hpp) specifies each policy family's feature
categories and condition/effect types.

| Policy family | Representation | Named feature categories |
| --- | --- | --- |
| `BaseFamilyTag` | Sketches (`ps::base`) | Boolean, numerical |
| `ExtFamilyTag` | Module programs (`ps::ext`) | Concept, role, Boolean, numerical, query |
| `IcpFamilyTag` | Indexical concept policies (`ps::icp`) | Concept, role, Boolean, numerical, query |
| `UnsFamilyTag` | Unsolvability classifiers (`uns`) | Boolean |

`UnsFamilyTag` participates in the shared feature infrastructure but does not
define another policy executor. Its condition and effect inventories are empty.
Its Boolean features may contain numerical, concept, and role expressions.

ICP shares the Ext feature categories while retaining its own policy types,
repository, rules, and execution history. Sharing DL expressions does not imply
sharing an execution model or a particular repository instance.

At the policy layer, the main feature types are:

```cpp
ps::Feature<Family, FeatureTag>
ps::ConcreteFeature<Family, LanguageTag, FeatureTag>
```

`Feature` wraps the language-specific concrete feature. `DlTag` is currently the
only implemented `LanguageTag`; its expressions include DL constructors and
relational queries. There is no second interchangeable feature language today.

[FeatureExpression](ps/dl/feature_data.hpp) maps a concrete feature to an expression:

| Policy `FeatureTag` | Expression type in `kr::dl` |
| --- | --- |
| `kr::dl::ConceptTag` | `Constructor<ConceptTag>` |
| `kr::dl::RoleTag` | `Constructor<RoleTag>` |
| `kr::dl::BooleanTag` | `Constructor<BooleanTag>` |
| `kr::dl::NumericalTag` | `Constructor<NumericalTag>` |
| `ps::dl::QueryFeature` | `Query<>` |

This mapping describes expression types; the policy family inventory determines
which named features belong in its default repository. Generic records and views
are constrained by their semantics, not by that inventory. For example, a Base
role feature or an Uns numerical feature can be stored in a custom repository
with the DL repository. DL feature categories and valid condition/effect
observation pairs have explicit concepts; the storage context `C` remains
generic.

## Category and constructor

[declarations.hpp](dl/declarations.hpp) lists the constructor tags of each
category: atomic state/goal expressions, concept and role operations, Boolean
nonemptiness, numerical count and distance, concept/role registers, typed call
arguments, numerical constants and arithmetic, and Boolean constants, logic and
comparisons.

| Language | Registers | Call arguments |
| --- | --- | --- |
| Module programs (`ps::ext`) | yes | yes |
| Indexical concept policies (`ps::icp`) | yes | no |
| Sketches (`ps::base`) | no | no |
| Unsolvability classifiers (`uns`) | no | no |

Every language accepts the other constructors. A rejected constructor is a parse
error at its source position.

The four denotation categories are `ConceptTag`, `RoleTag`, `BooleanTag`, and
`NumericalTag`. `Concept<Tag>`, `Role<Tag>`, `Boolean<Tag>`, and `Numerical<Tag>`
identify concrete operators; `Constructor<Category>` wraps the alternatives for a
category. The `*ConstructorTag` concepts check membership in the corresponding
list.

`Query<Tag>` represents a relational operator, and `Query<>` wraps the query
alternatives. A query evaluates to a relation, not a fifth DL denotation
category. Queries can embed DL expressions; query projections produce concepts
or roles. `ConceptOrRoleTag` restricts projection and register types to these
two categories.

The `dl::grammar` and `dl::cnf_grammar` namespaces represent generation grammars,
separately from evaluable expressions. They contain the `Generation*ConstructorTags`
only: no registers, arguments, arithmetic, Boolean operators, relational queries,
or projections; the grammar parser rejects these. `cnf_grammar::generate` writes
the generated constructors into the DL repository.

## Evaluation and storage

State evaluation is independent of the policy family; transition evaluation uses
the policy family:

```cpp
// Type composition for a lifted ICP numerical feature.
namespace kr = runir::kr;
using Family = kr::IcpFamilyTag;

using Feature = kr::ps::ConcreteFeature<Family, kr::DlTag,
                                      kr::dl::NumericalTag>;
using StateContext = kr::dl::semantics::StateEvaluationContext<tyr::LiftedTag>;
using TransitionContext = kr::ps::dl::TransitionEvaluationContext<Family, tyr::LiftedTag>;
```

[TransitionEvaluationContext](ps/dl/transition_evaluation_context.hpp) contains
source and target state contexts borrowing separate caches. A state context can
carry register values and call arguments; evaluating a register or argument
expression without them throws. ICP history belongs to policy execution, not to
the DL context.

[BasicRepository](ps/repository.hpp) refers to a DL repository; a
[DomainContext](domain_context.hpp) shares one DL repository between all
languages. Its policy storage combines the shared feature inventory with the
family's rules and program types.

The index/data/view pattern is a separate representation choice:

- `ygg::Index<T>` is a typed handle to an interned object.
- `ygg::Data<T>` is its stored record.
- `ygg::View<ygg::Index<T>, C>` combines the handle with a repository context.

The view parameter `C` identifies the storage context; it is not another policy
or feature-language dimension.

## Other tag parameters

| Parameter | Purpose |
| --- | --- |
| `tyr::TaskKind Kind` | Selects ground or lifted planning (`GroundTag`, `LiftedTag`), independently of the policy family. |
| `tyr::formalism::FactKind` | Selects static, fluent, or derived facts for atomic state/goal constructors. Compound-expression staticness is tracked by `is_static()`, not by a separate template family. |
| `ObservationTag` | Selects Boolean or numerical conditions/effects, such as `Positive`, `EqualZero`, or `Decreases`. Valid pairs depend on the feature category and condition/effect inventory. |
| Rule tag | Selects a family's concrete rule body. Shared feature types do not imply shared rule semantics. |

Use the weakest constraint required by the implementation: a default
repository's inventory does not define every valid generic instantiation.
Family-specific data and execution behavior remain specialized where their
semantics differ.
