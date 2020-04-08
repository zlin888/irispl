; check closure

(define x (lambda (x) (* x x)))
(display x)
(display x)
(display (lambda (x) (* x x)))