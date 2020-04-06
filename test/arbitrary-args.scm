(define sum-list 
  (lambda (args)
    (if (pair? args)
        (+ (car args) (sum-list (cdr args)))
        (car args))))

;; should raise error
;; When using arbitrary arguments function, one argument should be put after '.'.
(define sum
  (lambda (arg0 agr1 . )
    (+ (+ arg0 agr1) (sum-list arg0))))

;; should raise error
;; When using arbitrary arguments function, only one argument can be put after '.'.
(define sum
  (lambda (arg0 agr1 . args hi)
    (+ (+ arg0 agr1) (sum-list args))))
