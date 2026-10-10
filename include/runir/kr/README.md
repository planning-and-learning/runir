# Template dimensions

Runir separates the policy representation from the language used to express its
features. A policy family selects the policy vocabulary and its execution
semantics; a DL family selects the available feature constructors. These choices
are connected by traits, rather than forming an unrestricted Cartesian product.

## Policy family and feature language

The tags are declared in [declarations.hpp](declarations.hpp).
[PsFamilyTraits](ps/family_traits.hpp) specifies each policy family's feature
categories, condition/effect types, and associated `DlFamily`.

| Policy family | Representation | `DlFamily` | Named feature categories |
| --- | --- | --- | --- |
| `BaseFamilyTag` | Sketches (`ps::base`) | `BaseFamilyTag` | Boolean, numerical |
| `ExtFamilyTag` | Module programs (`ps::ext`) | `ExtFamilyTag` | Concept, role, Boolean, numerical, query |
| `IcpFamilyTag` | Indexical concept policies (`ps::icp`) | `ExtFamilyTag` | Concept, role, Boolean, numerical, query |
| `UnsFamilyTag` | Unsolvability classifiers (`uns`) | `UnsFamilyTag` | Boolean |

`UnsFamilyTag` participates in the shared feature infrastructure but does not
define another policy executor. Its condition and effect inventories are empty.
Its Boolean features may contain numerical, concept, and role expressions.

ICP reuses the Ext feature language while retaining its own policy types,
repository, rules, and execution history. Consequently, `IcpFamilyTag` satisfies
`kr::FamilyTag` but not `kr::dl::FamilyTag`. Sharing a DL family does not imply
sharing an execution model or a particular repository instance.

At the policy layer, the main feature types are:

```cpp
ps::Feature<Family, FeatureTag>
ps::ConcreteFeature<Family, LanguageTag, FeatureTag>
```

`Feature` wraps the language-specific concrete feature. `DlTag` is currently the
only implemented `LanguageTag`; its expressions include DL constructors and
relational queries. There is no second interchangeable feature language today.

[FeatureExpression](ps/dl/feature_data.hpp) maps a concrete feature to an expression
in `DlFamilyFor<Family>`:

| Policy `FeatureTag` | Expression type in `kr::dl` |
| --- | --- |
| `kr::dl::ConceptTag` | `Constructor<DlFamily, ConceptTag>` |
| `kr::dl::RoleTag` | `Constructor<DlFamily, RoleTag>` |
| `kr::dl::BooleanTag` | `Constructor<DlFamily, BooleanTag>` |
| `kr::dl::NumericalTag` | `Constructor<DlFamily, NumericalTag>` |
| `ps::dl::QueryFeature` | `Query<DlFamily>` |

This mapping describes expression types; the policy family inventory determines
which named features belong in its default repository. Generic records and views
are constrained by their semantics, not by that inventory. For example, a Base
role feature or an Uns numerical feature can be stored in a custom repository
with the corresponding DL repository. DL feature categories and valid
condition/effect observation pairs have explicit concepts; the storage context
`C` remains generic.

## DL family, category, and constructor

[DlFamilyTraits](dl/family_traits.hpp) lists the concrete constructor tags for
each DL family and category.

| DL family | Constructor vocabulary |
| --- | --- |
| Base | Atomic state/goal expressions, concept and role operations, Boolean nonemptiness, numerical count and distance |
| Ext | Base plus concept/role registers, typed call arguments, numerical constants and arithmetic |
| Uns | Base plus Boolean constants, logic and comparisons, numerical constants and arithmetic |

The four denotation categories are `ConceptTag`, `RoleTag`, `BooleanTag`, and
`NumericalTag`. `Concept<Family, Tag>`, `Role<Family, Tag>`,
`Boolean<Family, Tag>`, and `Numerical<Family, Tag>` identify concrete operators;
`Constructor<Family, Category>` wraps the alternatives for a category.
The `Family*ConstructorTag` concepts check membership in the corresponding
inventory.

`Query<Family, Tag>` represents a relational operator, and `Query<Family>` wraps
the query alternatives. A query evaluates to a relation, not a fifth DL
denotation category. Queries can embed family-specific DL expressions; query
projections produce concepts or roles. `ConceptOrRoleTag` restricts projection
and register types to these two categories.

The `dl::grammar` and `dl::cnf_grammar` namespaces represent generation grammars,
separately from evaluable expressions. Their vocabularies exclude relational
queries and projections. The compiled `cnf_grammar::generate` implementation is
currently available for Base with both task kinds.

## Evaluation and storage

State evaluation uses the **DL family**, while transition evaluation uses the
**policy family** and derives its DL family through `ps::DlFamilyFor<Family>`:

```cpp
// Type composition for a lifted ICP numerical feature.
namespace kr = runir::kr;
using Family = kr::IcpFamilyTag;
using DlFamily = kr::ps::DlFamilyFor<Family>;  // ExtFamilyTag

using Feature = kr::ps::ConcreteFeature<Family, kr::DlTag,
                                      kr::dl::NumericalTag>;
using StateContext = kr::dl::semantics::StateEvaluationContext<DlFamily, tyr::LiftedTag>;
using TransitionContext = kr::ps::dl::TransitionEvaluationContext<Family, tyr::LiftedTag>;
```

`DenotationCaches<DlFamily>` follows the expression family as well.
[TransitionEvaluationContext](ps/dl/transition_evaluation_context.hpp) contains
source and target state contexts borrowing separate caches. Ext DL contexts also
carry register values and call arguments; this applies to both Ext and ICP
policies. ICP history belongs to policy execution, not to the DL context.

[BasicRepository](ps/repository.hpp) derives its DL repository type from the same
mapping. Its policy storage combines the shared family inventory with the
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

Shared code should derive the expression family through `ps::DlFamilyFor<Family>`. Use the
weakest constraint required by the implementation: a default repository's
inventory does not define every valid generic instantiation. Family-specific
data and execution behavior remain specialized where their semantics differ.
