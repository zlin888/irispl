(define make-simple-cell
    (lambda (value)
        (lambda (selector)
            (cond ((eq? selector 'fetch) (lambda () value))
                  ((eq? selector 'store!) (lambda (new-value) (set! value new-value)))
                  ((eq? selector 'cell?) (lambda () #t))
                  (else #f)))))

(define a-cell (make-simple-cell 13))
((a-cell 'store!) 21)
(display ((a-cell 'fetch)))