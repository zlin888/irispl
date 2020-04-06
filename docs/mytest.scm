(import MoB "/Users/bytedance/CLionProjects/typed-scheme/docs/test.utils.scm")

(define pass 'pass)
(define error 'error)

(define make-simple-cell
    (lambda (value)
        (lambda (selector)
            (cond ((eq? selector 'fetch) (lambda () value))
                  ((eq? selector 'store!) (lambda (new-value) (set! value new-value)))
                  ((eq? selector 'cell?) #t)
                  (else #f)))))

(define x (make-simple-cell 77))

(display ((x 'fetch)))
((x 'store!) 88)
(display ((x 'fetch)))

(define sum-list 
  (lambda (args)
    (if (pair? args)
        (+ (car args) (sum-list (cdr args)))
        (car args))))


(define sum
  (lambda (arg0 agr1 . args)
    (+ (+ arg0 agr1) (sum-list args))))

(+ 12 1)

; (display (sum 1 2 (cons 3 4)))
(display (sum 1 2 3 4))
