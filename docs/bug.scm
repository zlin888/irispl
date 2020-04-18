(define sum-list
  (lambda (args)
    (if (pair? args)
        (+ (car args) (sum-list (cdr args)))
        (car args))))

(define sum
  (lambda (arg0 agr1 . args)
    (+ (+ arg0 agr1) (sum-list args))))

; (define x (list 55 56 57))

; (display (apply sum x))
(display (apply sum (list 99 98 97)))