(define make-filtered-cell
    (lambda (value filter)
        (let ((super (make-simple-cell value)))
            (lambda (selector)
                (cond ((eq? selector 'store!)
                        (lambda (self new-value) 
                            (if (positive-filter new-value)
                                ((super 'store!) new-value)
                                error)))
                        (else (super selector))))))

(class filter-cell (value filter)
  (super (simple-cell value))
  (store 
    (lambda (self new-value)
      (if (positive-filter new-value)
          ((super 'store!) new-value)
          error)))
  (double
    (lambda (self) (set! value (* 2 value)))))

(define a-cell (filter-cell 17 filter))
(=> a-cell.store 1 2 3 4 5 6 7 8 9)
; (isinstance? a-cell) => true

(define isinstance? 
  (lambda (instance)
    (a-cell)