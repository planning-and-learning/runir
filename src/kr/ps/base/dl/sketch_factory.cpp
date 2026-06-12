#include "runir/kr/ps/base/dl/sketch_factory.hpp"

#include "runir/kr/ps/base/dl/parser.hpp"

#include <fmt/format.h>
#include <stdexcept>
#include <string>

namespace runir::kr::ps::base::dl
{

SketchView SketchFactory::create_empty(Repository& repository)
{
    auto data = ygg::Data<runir::kr::ps::base::Sketch> {};
    return repository.get_or_create(data).first;
}

SketchView SketchFactory::create(SketchSpecification specification, tyr::formalism::planning::DomainView domain, Repository& repository)
{
    return parse_sketch(create_description(specification), domain, repository);
}

std::string SketchFactory::create_empty_description()
{
    return R"(
(:sketch
  (:features)
  (:rules))
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

    throw std::runtime_error(fmt::format("Unknown sketch specification: {}.", static_cast<int>(specification)));
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
    return R"((:sketch
    (:features
        (:boolean
            (:symbol r_b)
            (:description "")
            (:expression
                (b_nonempty
                    (c_some
                        (r_atomic_goal
                            "at"
                            true)
                        (c_atomic_state
                            "at-robby")))
            )
        )
        (:numerical
            (:symbol c)
            (:description "")
            (:expression
                (n_count
                    (c_some
                        (r_atomic_state
                            "carry")
                        (c_top)))
            )
        )
        (:numerical
            (:symbol b)
            (:description "")
            (:expression
                (n_count
                    (c_not
                        (c_same_as
                            (r_atomic_goal
                                "at"
                                true)
                            (r_atomic_state
                                "at"))))
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto1)
            (:description "")
            (:expression
                (:conditions
                    (:negative r_b)
                    (:equal_zero c)
                    (:greater_zero b)
                )
                (:effects
                    (:unchanged r_b)
                    (:increases c)
                    (:unchanged b)
                )
            )
        )
        (:rule
            (:symbol auto2)
            (:description "")
            (:expression
                (:conditions
                    (:equal_zero c)
                    (:greater_zero b)
                    (:positive r_b)
                )
                (:effects
                    (:unchanged b)
                    (:negative r_b)
                    (:unchanged c)
                )
            )
        )
        (:rule
            (:symbol auto3)
            (:description "")
            (:expression
                (:conditions
                    (:greater_zero b)
                    (:positive r_b)
                    (:greater_zero c)
                )
                (:effects
                    (:unchanged r_b)
                    (:decreases c)
                    (:decreases b)
                )
            )
        )
        (:rule
            (:symbol auto4)
            (:description "")
            (:expression
                (:conditions
                    (:negative r_b)
                    (:greater_zero b)
                    (:greater_zero c)
                )
                (:effects
                    (:unchanged b)
                    (:unchanged c)
                    (:positive r_b)
                )
            )
        )
    )
)
)";
}

std::string SketchFactory::create_blocks3ops_france_et_al_aaai2021_description()
{
    return R"((:sketch
    (:features
        (:numerical
            (:symbol c)
            (:description "")
            (:expression
                (n_count
                    (c_atomic_state
                        "clear"))
            )
        )
        (:numerical
            (:symbol t)
            (:description "")
            (:expression
                (n_count
                    (c_not
                        (c_same_as
                            (r_atomic_goal
                                "on"
                                true)
                            (r_atomic_state
                                "on"))))
            )
        )
        (:numerical
            (:symbol bwp)
            (:description "")
            (:expression
                (n_count
                    (c_all
                        (r_transitive_closure
                            (r_atomic_state
                                "on"))
                        (c_same_as
                            (r_atomic_goal
                                "on"
                                true)
                            (r_atomic_state
                                "on"))))
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto5)
            (:description "")
            (:expression
                (:conditions
                    (:greater_zero c)
                    (:greater_zero t)
                )
                (:effects
                    (:increases c)
                    (:unchanged t)
                    (:unchanged bwp)
                )
            )
        )
        (:rule
            (:symbol auto6)
            (:description "")
            (:expression
                (:conditions
                    (:greater_zero c)
                    (:greater_zero t)
                )
                (:effects
                    (:increases c)
                    (:increases bwp)
                )
            )
        )
        (:rule
            (:symbol auto7)
            (:description "")
            (:expression
                (:conditions
                    (:greater_zero c)
                    (:greater_zero t)
                )
                (:effects
                    (:increases c)
                    (:unchanged bwp)
                    (:decreases t)
                )
            )
        )
        (:rule
            (:symbol auto8)
            (:description "")
            (:expression
                (:conditions
                    (:greater_zero c)
                    (:greater_zero t)
                )
                (:effects
                    (:unchanged bwp)
                    (:decreases t)
                    (:decreases c)
                )
            )
        )
    )
)
)";
}

std::string SketchFactory::create_spanner_france_et_al_aaai2021_description()
{
    return R"((:sketch
    (:features
        (:boolean
            (:symbol e)
            (:description "")
            (:expression
                (b_nonempty
                    (c_some
                        (r_atomic_state
                            "at")
                        (c_all
                            (r_inverse
                                (r_atomic_state
                                    "at"))
                            (c_atomic_state
                                "man"))))
            )
        )
        (:numerical
            (:symbol n)
            (:description "")
            (:expression
                (n_count
                    (c_and
                        (c_atomic_goal
                            "tightened"
                            true)
                        (c_not
                            (c_atomic_state
                                "tightened"))))
            )
        )
        (:numerical
            (:symbol h)
            (:description "")
            (:expression
                (n_count
                    (c_some
                        (r_atomic_state
                            "at")
                        (c_top)))
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto9)
            (:description "")
            (:expression
                (:conditions
                    (:greater_zero n)
                    (:greater_zero h)
                    (:positive e)
                )
                (:effects
                    (:unchanged n)
                    (:unchanged h)
                )
            )
        )
        (:rule
            (:symbol auto10)
            (:description "")
            (:expression
                (:conditions
                    (:greater_zero n)
                    (:greater_zero h)
                    (:negative e)
                )
                (:effects
                    (:unchanged n)
                    (:decreases h)
                )
            )
        )
        (:rule
            (:symbol auto11)
            (:description "")
            (:expression
                (:conditions
                    (:greater_zero n)
                    (:greater_zero h)
                    (:negative e)
                )
                (:effects
                    (:unchanged h)
                    (:decreases n)
                    (:unchanged e)
                )
            )
        )
    )
)
)";
}

std::string SketchFactory::create_delivery_france_et_al_aaai2021_description()
{
    return R"((:sketch
    (:features
        (:boolean
            (:symbol not_H)
            (:description "")
            (:expression
                (b_nonempty
                    (c_atomic_state
                        "empty"))
            )
        )
        (:numerical
            (:symbol u)
            (:description "")
            (:expression
                (n_count
                    (c_not
                        (c_same_as
                            (r_atomic_state
                                "at")
                            (r_atomic_goal
                                "at"
                                true))))
            )
        )
        (:numerical
            (:symbol p)
            (:description "")
            (:expression
                (n_distance
                    (c_some
                        (r_inverse
                            (r_atomic_state
                                "at"))
                        (c_atomic_state
                            "truck"))
                    (r_atomic_state
                        "adjacent")
                    (c_and
                        (c_all
                            (r_inverse
                                (r_atomic_goal
                                    "at"
                                    true))
                            (c_bot))
                        (c_some
                            (r_inverse
                                (r_atomic_state
                                    "at"))
                            (c_atomic_state
                                "package"))))
            )
        )
        (:numerical
            (:symbol t)
            (:description "")
            (:expression
                (n_distance
                    (c_some
                        (r_inverse
                            (r_atomic_state
                                "at"))
                        (c_atomic_state
                            "truck"))
                    (r_atomic_state
                        "adjacent")
                    (c_some
                        (r_inverse
                            (r_atomic_goal
                                "at"
                                true))
                        (c_top)))
            )
        )
    )
    (:rules
        (:rule
            (:symbol auto12)
            (:description "")
            (:expression
                (:conditions
                    (:greater_zero u)
                    (:positive not_H)
                    (:greater_zero p)
                )
                (:effects
                    (:unchanged not_H)
                    (:decreases p)
                )
            )
        )
        (:rule
            (:symbol auto13)
            (:description "")
            (:expression
                (:conditions
                    (:greater_zero u)
                    (:positive not_H)
                    (:equal_zero p)
                )
                (:effects
                    (:negative not_H)
                )
            )
        )
        (:rule
            (:symbol auto14)
            (:description "")
            (:expression
                (:conditions
                    (:greater_zero u)
                    (:negative not_H)
                    (:greater_zero t)
                )
                (:effects
                    (:unchanged not_H)
                    (:unchanged u)
                    (:decreases t)
                )
            )
        )
        (:rule
            (:symbol auto15)
            (:description "")
            (:expression
                (:conditions
                    (:greater_zero u)
                    (:negative not_H)
                    (:equal_zero t)
                )
                (:effects
                    (:decreases u)
                    (:positive not_H)
                    (:unchanged p)
                )
            )
        )
    )
)
)";
}

}  // namespace runir::kr::ps::base::dl
