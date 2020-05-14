(import functools)

(define double-list
  (lambda (l)
    (functools.map
      (lambda (x) (* x 2))
      l)))

(display (double-list (list 1 2 3)))



