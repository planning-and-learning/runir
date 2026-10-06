(define (problem incremental-dl-task)
  (:domain incremental-dl)
  (:init
    (present a) (present b)
    (edge a b) (edge b c)
    (ready)
    (fixed a) (fixed c))
  (:goal (and (present b) (not (present a)) (edge b c) (not (edge a b)))))
