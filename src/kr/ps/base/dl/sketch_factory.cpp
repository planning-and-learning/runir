#include "runir/kr/ps/base/dl/sketch_factory.hpp"

#include "runir/kr/ps/base/dl/parser.hpp"

#include <stdexcept>
#include <string>

namespace runir::kr::ps::base::dl
{

SketchView SketchFactory::create_empty(Repository& repository)
{
    auto data = tyr::Data<runir::kr::ps::Sketch<runir::kr::BaseFamilyTag>> {};
    return repository.get_or_create(data).first;
}

SketchView SketchFactory::create(SketchSpecification specification, tyr::formalism::planning::DomainView domain, Repository& repository)
{
    return parse_sketch(create_description(specification), domain, repository);
}

std::string SketchFactory::create_empty_description()
{
    return R"(
(
  (:features)
  (:rules)
)
)";
}

std::string SketchFactory::create_description(SketchSpecification specification)
{
    switch (specification)
    {
        case SketchSpecification::GRIPPER_FRANCE_ET_AL_AAAI2021:
            return create_gripper_france_et_al_aaai2021_description();
        case SketchSpecification::BLOCKS3OPS_FRANCE_ET_AL_AAAI2021:
            return create_blocks3ops_france_et_al_aaai2021_description();
        case SketchSpecification::SPANNER_FRANCE_ET_AL_AAAI2021:
            return create_spanner_france_et_al_aaai2021_description();
        case SketchSpecification::DELIVERY_FRANCE_ET_AL_AAAI2021:
            return create_delivery_france_et_al_aaai2021_description();
    }

    throw std::runtime_error("Unknown sketch specification.");
}

SketchView SketchFactory::create_gripper_france_et_al_aaai2021(tyr::formalism::planning::DomainView domain, Repository& repository)
{
    return parse_sketch(create_gripper_france_et_al_aaai2021_description(), domain, repository);
}

SketchView SketchFactory::create_blocks3ops_france_et_al_aaai2021(tyr::formalism::planning::DomainView domain, Repository& repository)
{
    return parse_sketch(create_blocks3ops_france_et_al_aaai2021_description(), domain, repository);
}

SketchView SketchFactory::create_spanner_france_et_al_aaai2021(tyr::formalism::planning::DomainView domain, Repository& repository)
{
    return parse_sketch(create_spanner_france_et_al_aaai2021_description(), domain, repository);
}

SketchView SketchFactory::create_delivery_france_et_al_aaai2021(tyr::formalism::planning::DomainView domain, Repository& repository)
{
    return parse_sketch(create_delivery_france_et_al_aaai2021_description(), domain, repository);
}

std::string SketchFactory::create_gripper_france_et_al_aaai2021_description()
{
    return R"(
(
  (:features
    (boolean r_b "r_b" "" (b_nonempty (c_some (r_atomic_goal "at" true) (c_atomic_state "at-robby"))))
    (numerical c "c" "" (n_count (c_some (r_atomic_state "carry") (c_top))))
    (numerical numerical_b "b" "" (n_count (c_not (c_same_as (r_atomic_state "at") (r_atomic_goal "at" true)))))
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

std::string SketchFactory::create_blocks3ops_france_et_al_aaai2021_description()
{
    return R"(
(
  (:features
    (numerical c "c" "" (n_count (c_atomic_state "clear")))
    (numerical t "t" "" (n_count (c_not (c_same_as (r_atomic_state "on") (r_atomic_goal "on" true)))))
    (numerical bwp "bwp" "" (n_count (c_all (r_transitive_closure (r_atomic_state "on")) (c_same_as (r_atomic_state "on") (r_atomic_goal "on" true)))))
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

std::string SketchFactory::create_spanner_france_et_al_aaai2021_description()
{
    return R"(
(
  (:features
    (boolean e "e" "" (b_nonempty (c_some (r_atomic_state "at") (c_all (r_inverse (r_atomic_state "at")) (c_atomic_state "man")))))
    (numerical h "h" "" (n_count (c_some (r_atomic_state "at") (c_top))))
    (numerical n "n" "" (n_count (c_and (c_not (c_atomic_state "tightened")) (c_atomic_goal "tightened" true))))
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

std::string SketchFactory::create_delivery_france_et_al_aaai2021_description()
{
    return R"(
(
  (:features
    (boolean not_H "not_H" "" (b_nonempty (c_atomic_state "empty")))
    (numerical u "u" "" (n_count (c_not (c_same_as (r_atomic_goal "at" true) (r_atomic_state "at")))))
    (numerical t "t" "" (n_distance (c_some (r_inverse (r_atomic_state "at")) (c_atomic_state "truck")) (r_atomic_state "adjacent") (c_some (r_inverse (r_atomic_goal "at" true)) (c_top))))
    (numerical p "p" "" (n_distance (c_some (r_inverse (r_atomic_state "at")) (c_atomic_state "truck")) (r_atomic_state "adjacent") (c_and (c_some (r_inverse (r_atomic_state "at")) (c_atomic_state "package")) (c_all (r_inverse (r_atomic_goal "at" true)) (c_bot)))))
    (numerical x "x" "" (n_distance (c_some (r_inverse (r_atomic_state "at")) (c_atomic_state "empty")) (r_atomic_state "adjacent") (c_some (r_inverse (r_atomic_state "at")) (c_and (c_atomic_state "package") (c_not (c_same_as (r_atomic_state "at") (r_atomic_goal "at" true)))))))
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

}  // namespace runir::kr::ps::base::dl
