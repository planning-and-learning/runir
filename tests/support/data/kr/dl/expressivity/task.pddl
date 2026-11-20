(define (problem expressivity-1)
  (:domain expressivity)
  (:init (p a) (q b) (e a b) (e b c) (f a b) (f c c) (flag))
  (:goal (and (p a) (not (p b)) (q c) (e a b) (not (e b c)) (f c a) (not (flag)))))
