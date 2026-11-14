(define (domain choose-path)
    (:requirements :strips)
    (:predicates (at ?x) (edge ?x ?y) (candidate ?x) (bad ?x))
    (:action move
        :parameters (?from ?to)
        :precondition (and (at ?from) (edge ?from ?to))
        :effect (and (not (at ?from)) (at ?to))))
