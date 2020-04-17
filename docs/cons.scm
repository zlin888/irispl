(import utils "docs/test.utils.scm")
(import error)

; (define _cons
;   (lambda (l1 l2)
;     (if (or (not (list? l1)) (not (list? l2)))
;       (exit error.cons-error)
;       (if (pair)) 
;         )))

(define display-list 
  (lambda (l) 
    (if (pair? l)
      (begin (display (car l)) (display-list (cdr l)))
      (display (car l)))))

(define _list?
  (lambda (item)
    (if (list? item) #t #f)))

; (display (_cons (list 12) 3))
; (display (_cons 123 12))

(display-list (list 3 4 5 6))
(begin (display 1) (display 2))
(utils.show error.cons-error)

; (display (begin (display 12) 123))
; (display (begin (display 12) (display 18) 99 1299))
(display (let ((_ (display 12)) (x 13)) (+ x 1)))