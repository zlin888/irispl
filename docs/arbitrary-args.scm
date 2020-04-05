(define sum-list 
  (lambda (args)
    (if (pair? args)
        (+ (car args) (sum-list (cdr args)))
        (car args))))

(define sum
  (lambda (arg0 agr1 . args)
    (+ (+ arg0 agr1) (sum-list args))))

(display (sum 1 2 'point (cons 3 4)))
;; (display (sum 1 2 3 4))