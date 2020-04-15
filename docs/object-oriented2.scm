(define error 'error)

(define simple-cell
    (lambda (value)
        (lambda (selector)
            (cond ((eq? selector 'fetch) (lambda () value))
                  ((eq? selector 'store!) (lambda (new-value) (set! value new-value)))
                  ((eq? selector 'cell?) #t)
                  (else #f)))))


(define positive-filter (lambda (value) (>= value 0)))

(define make-filtered-cell
    (lambda (value filter)
        (let ((super (simple-cell value)))
            (lambda (selector)
                (cond ((eq? selector 'store!)
                        (lambda (self new-value) 
                            (if (positive-filter new-value)
                                ((super 'store!) new-value)
                                error)))
                        (else (super selector)))))))

(class filter-cell (value filter)
  (super (simple-cell value))
  ((store 
    (lambda (self new-value)
      (if (positive-filter new-value)
          ((super 'store!) new-value)
          error)))
  (double
    (lambda (self) (set! value (* 2 value)))))
    )

(define =>
  (lambda (instance method . args)
    (let ((_method (instance method)))
      (apply _method args))))

(define a-cell (filter-cell 17 positive-filter))
(define aa-cell (make-filtered-cell 17 positive-filter))

(display ((aa-cell 'fetch)))
(display ((a-cell 'fetch)))
; (=> a-cell 'store 1 2 3 4 5 6 7 8 9)
; (isinstance? a-cell) => true
