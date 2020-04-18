(import utils "docs/test.utils.scm")
(import error)
(import object)

; (define _cons
;   (lambda (l1 l2)
;     (if (or (not (list? l1)) (not (list? l2)))
;       (exit error.cons-error)
;       (if (pair)) 
;         )))

(define display-list 
  (lambda (l) 
    (if (pair? l)
      (begin (display-list (cdr l)) (display (car l)) )
      (display (car l)))))

(define _list?
  (lambda (item)
    (if (list? item) #t #f)))

; (display (_cons (list 12) 3))
; (display (_cons 123 12))

; (begin (display 2) (display 1) )
; (display-list (list 3 4 5 6))
; (utils.show error.cons-error)

; ; (display (begin (display 12) 123))
; ; (display (begin (display 12) (display 18) 99 1299))
; ; (display (let ((_ (display 12)) (x 13)) (+ x 1)))
; (display (car (car (list 1))))

(class food (name)
  (super object.object)
  ((fetch (lambda (self) name))))

(class apple (value)
  (super (food "apple"))
  ( (fetch-value (lambda (self) value))
    (store (lambda (self new-value) (set! value new-value)))))

(define my-apple (apple 380))
(display (object.=> my-apple 'fetch))
(object.=> my-apple 'store 800)
((my-apple 'store) my-apple 400)
(display (object.=> my-apple 'fetch-value))
(display (object.=> my-apple 'fetch))
