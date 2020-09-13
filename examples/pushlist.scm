(define sum-list
  (lambda (args)
    (if (pair? args)
        (+ (car args) (sum-list (cdr args)))
        (car args))))

(define sum
  (lambda (arg0 agr1 . args)
    (+ (+ arg0 agr1) (sum-list args))))

(display (apply + (list 1 2 3)))
(display (apply sum (list 99 98 97)))