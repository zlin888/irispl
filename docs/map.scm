(define map
  (lambda (f l)
    (if (pair? l)
      (cons (f (car l)) (map f (cdr l)))
      (f (car l)))))

; (define check-types
;   (lambda (_type-list object-list)
;     (eq? _type-list (map type object-list))))

; (define double 
;   (lambda (x) (+ x x)))

; (display (map type (list 1 2)))
; (display (cons 'hi 'hi))
; (display (eq? (list 'NUMBER 'NUMBER) (list 'NUMBER 'NUMBER)))
; (display (check-types (list 'NUMBER 'NUMBER) (list 1 2)))

; (define shit
;   (lambda (f x) (f x)))

; (display (shit type 121))