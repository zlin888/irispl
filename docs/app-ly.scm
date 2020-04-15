(define sum-list
  (lambda (args)
    (if (pair? args)
        (+ (car args) (sum-list (cdr args)))
        (car args))))

(define sum
  (lambda (arg0 agr1 . args)
    (+ (+ arg0 agr1) (sum-list args))))

(define _sum
    (lambda (x y z)
        (+ x (+ y z))))

(define shit _sum)

; (display (sum 1 2 (list 3 4)))
(display (sum 1 2 3 4))
(display (_sum 1 2 3))
(display (shit 1 2 3))

(define x (list 1 2 3))
(define complex-application
    (lambda (x)
        (lambda (y)
          (lambda (z) (+ x (+ y z))))))

(display (apply _sum x))
(display (apply + x))
(display (apply sum (list 99 98 97)))
(display (apply sum (list 99 98 97)))
(display (((complex-application 12) 13) 14))

(display (apply + x))
(display (apply + x))