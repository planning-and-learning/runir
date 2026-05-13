#include "runir/kr/gp/dl/policy_factory.hpp"

#include "runir/kr/gp/dl/parser.hpp"

#include <stdexcept>
#include <string>

namespace runir::kr::gp::dl
{

PolicyView PolicyFactory::create(PolicySpecification specification, tyr::formalism::planning::DomainView domain, Repository& repository)
{
    return parse_policy(create_description(specification), domain, repository);
}

std::string PolicyFactory::create_description(PolicySpecification specification)
{
    switch (specification)
    {
        case PolicySpecification::GRIPPER_FRANCE_ET_AL_AAAI2021:
            return create_gripper_france_et_al_aaai2021_description();
        case PolicySpecification::BLOCKS3OPS_FRANCE_ET_AL_AAAI2021:
            return create_blocks3ops_france_et_al_aaai2021_description();
        case PolicySpecification::SPANNER_FRANCE_ET_AL_AAAI2021:
            return create_spanner_france_et_al_aaai2021_description();
        case PolicySpecification::DELIVERY_FRANCE_ET_AL_AAAI2021:
            return create_delivery_france_et_al_aaai2021_description();
    }

    throw std::runtime_error("Unknown policy specification.");
}

PolicyView PolicyFactory::create_gripper_france_et_al_aaai2021(tyr::formalism::planning::DomainView domain, Repository& repository)
{
    return parse_policy(create_gripper_france_et_al_aaai2021_description(), domain, repository);
}

PolicyView PolicyFactory::create_blocks3ops_france_et_al_aaai2021(tyr::formalism::planning::DomainView domain, Repository& repository)
{
    return parse_policy(create_blocks3ops_france_et_al_aaai2021_description(), domain, repository);
}

PolicyView PolicyFactory::create_spanner_france_et_al_aaai2021(tyr::formalism::planning::DomainView domain, Repository& repository)
{
    return parse_policy(create_spanner_france_et_al_aaai2021_description(), domain, repository);
}

PolicyView PolicyFactory::create_delivery_france_et_al_aaai2021(tyr::formalism::planning::DomainView domain, Repository& repository)
{
    return parse_policy(create_delivery_france_et_al_aaai2021_description(), domain, repository);
}

std::string PolicyFactory::create_gripper_france_et_al_aaai2021_description()
{
    return R"(
(
  (:features
    (boolean r_b "r_b" "" (b_nonempty (c_existential_quantification (r_atomic_goal "at" true) (c_atomic_state "at-robby"))))
    (numerical c "c" "" (n_count (c_existential_quantification (r_atomic_state "carry") (c_top))))
    (numerical numerical_b "b" "" (n_count (c_negation (c_role_value_map_equality (r_atomic_state "at") (r_atomic_goal "at" true)))))
  )
  (:rules
    (
      (:conditions (negative r_b) (equal_zero c) (greater_zero numerical_b))
      (:effects (unchanged r_b) (increases c) (unchanged numerical_b))
    )
    (
      (:conditions (positive r_b) (equal_zero c) (greater_zero numerical_b))
      (:effects (negative r_b) (unchanged c) (unchanged numerical_b))
    )
    (
      (:conditions (positive r_b) (greater_zero c) (greater_zero numerical_b))
      (:effects (unchanged r_b) (decreases c) (decreases numerical_b))
    )
    (
      (:conditions (negative r_b) (greater_zero c) (greater_zero numerical_b))
      (:effects (positive r_b) (unchanged c) (unchanged numerical_b))
    )
  )
)
)";
}

std::string PolicyFactory::create_blocks3ops_france_et_al_aaai2021_description()
{
    return R"(
(
  (:features
    (numerical c "c" "" (n_count (c_atomic_state "clear")))
    (numerical t "t" "" (n_count (c_negation (c_role_value_map_equality (r_atomic_state "on") (r_atomic_goal "on" true)))))
    (numerical bwp "bwp" "" (n_count (c_value_restriction (r_transitive_closure (r_atomic_state "on")) (c_role_value_map_equality (r_atomic_state "on") (r_atomic_goal "on" true)))))
  )
  (:rules
    (
      (:conditions (greater_zero c) (greater_zero t))
      (:effects (increases c) (unchanged t) (unchanged bwp))
    )
    (
      (:conditions (greater_zero c) (greater_zero t))
      (:effects (increases c) (increases bwp))
    )
    (
      (:conditions (greater_zero c) (greater_zero t))
      (:effects (increases c) (decreases t) (unchanged bwp))
    )
    (
      (:conditions (greater_zero c) (greater_zero t))
      (:effects (decreases c) (decreases t) (unchanged bwp))
    )
  )
)
)";
}

std::string PolicyFactory::create_spanner_france_et_al_aaai2021_description()
{
    return R"(
(
  (:features
    (boolean e "e" "" (b_nonempty (c_existential_quantification (r_atomic_state "at") (c_value_restriction (r_inverse (r_atomic_state "at")) (c_atomic_state "man")))))
    (numerical h "h" "" (n_count (c_existential_quantification (r_atomic_state "at") (c_top))))
    (numerical n "n" "" (n_count (c_intersection (c_negation (c_atomic_state "tightened")) (c_atomic_goal "tightened" true))))
  )
  (:rules
    (
      (:conditions (greater_zero n) (greater_zero h) (positive e))
      (:effects (unchanged n) (unchanged h))
    )
    (
      (:conditions (greater_zero n) (greater_zero h) (negative e))
      (:effects (unchanged n) (decreases h))
    )
    (
      (:conditions (greater_zero n) (greater_zero h) (negative e))
      (:effects (decreases n) (unchanged h) (unchanged e))
    )
  )
)
)";
}

std::string PolicyFactory::create_delivery_france_et_al_aaai2021_description()
{
    return R"(
(
  (:features
    (boolean not_H "not_H" "" (b_nonempty (c_atomic_state "empty")))
    (numerical u "u" "" (n_count (c_negation (c_role_value_map_equality (r_atomic_goal "at" true) (r_atomic_state "at")))))
    (numerical t "t" "" (n_distance (c_existential_quantification (r_inverse (r_atomic_state "at")) (c_atomic_state "truck")) (r_atomic_state "adjacent") (c_existential_quantification (r_inverse (r_atomic_goal "at" true)) (c_top))))
    (numerical p "p" "" (n_distance (c_existential_quantification (r_inverse (r_atomic_state "at")) (c_atomic_state "truck")) (r_atomic_state "adjacent") (c_intersection (c_existential_quantification (r_inverse (r_atomic_state "at")) (c_atomic_state "package")) (c_value_restriction (r_inverse (r_atomic_goal "at" true)) (c_bot)))))
    (numerical x "x" "" (n_distance (c_existential_quantification (r_inverse (r_atomic_state "at")) (c_atomic_state "empty")) (r_atomic_state "adjacent") (c_existential_quantification (r_inverse (r_atomic_state "at")) (c_intersection (c_atomic_state "package") (c_negation (c_role_value_map_equality (r_atomic_state "at") (r_atomic_goal "at" true)))))))
  )
  (:rules
    (
      (:conditions (greater_zero u) (positive not_H) (greater_zero p))
      (:effects (unchanged not_H) (decreases p))
    )
    (
      (:conditions (greater_zero u) (positive not_H) (equal_zero p))
      (:effects (negative not_H))
    )
    (
      (:conditions (greater_zero u) (negative not_H) (greater_zero t))
      (:effects (unchanged u) (unchanged not_H) (decreases t))
    )
    (
      (:conditions (greater_zero u) (negative not_H) (equal_zero t))
      (:effects (decreases u) (positive not_H) (unchanged p))
    )
  )
)
)";
}

}  // namespace runir::kr::gp::dl
