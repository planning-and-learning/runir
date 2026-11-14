(define (domain action-nullary)
    (:requirements :strips)
    (:predicates (ready) (done) (truth))
    (:action finish
        :parameters ()
        :precondition (ready)
        :effect (and (not (ready)) (done))))
