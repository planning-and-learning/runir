(define (domain action-existential)
    (:requirements :strips :existential-preconditions)
    (:predicates (at ?x) (supports ?x ?w) (done))
    (:action finish
        :parameters (?x)
        :precondition (and (at ?x) (exists (?w) (supports ?x ?w)))
        :effect (done)))
