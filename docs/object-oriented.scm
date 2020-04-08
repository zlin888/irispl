
(define pass 'pass)
(define error 'error)

(define make-simple-cell
    (lambda (value)
        (lambda (selector)
            (cond ((eq? selector 'fetch) (lambda () value))
                  ((eq? selector 'store!) (lambda (new-value) (set! value new-value)))
                  ((eq? selector 'cell?) #t)
                  (else #f)))))

(define make-named-cell
    (lambda (value the-name)
        (let ((s-cell (make-simple-cell value)))
            (lambda (selector)
                (cond ((eq? selector 'name) (lambda () the-name))
                    (else (s-cell selector)))))))

(define positive-filter (lambda (value) (>= value 0)))

(define make-filtered-cell
    (lambda (value filter)
        (let ((super (make-simple-cell value)))
            (lambda (selector)
                (cond ((eq? selector 'store!)
                        (lambda (self new-value) 
                            (if (positive-filter new-value)
                                ((super 'store!) new-value)
                                error)))
                        (else (super selector)))))))

(define sub (lambda (x y) (- x y)))
(display (sub 30 99))


(define positive-cell (make-filtered-cell 55 positive-filter))
(display ((positive-cell 'fetch)))
((positive-cell 'store!) positive-cell 33)
(display ((positive-cell 'fetch)))
(display ((positive-cell 'store!) positive-cell -33))

(display (cons (cons 1 13 88) 99))
(display make-filtered-cell)
(display sub)

(define =>
  (lambda (instance method . args)
    (let ((_method (instance method)))
      (apply _method args))))

(define simple-cell (make-simple-cell 33))

(display (=> simple-cell 'fetch))
(=> simple-cell 'store! 77)
(display (=> simple-cell 'fetch))