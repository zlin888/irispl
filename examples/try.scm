(import type)
(import object)

(define object-type
  (lambda (o)
    (object.=> o '__type__)))

(class Apple
  (value)
  (super object.base)
  ((fetch-value (lambda (self) value))
    ))
    
(define plus 
  (type.#lambda 
    (list 'NUMBER 'NUMBER 'NUMBER) 
    'NUMBER
    (lambda (x y z)
      (+ x (+ y z)))))

; (define apple (Apple 50))

(class Fruit 
  (price)
  (super object.base)
  ((fetch-price (lambda (self) price))
   (store-price (lambda (self new-price) (set! price new-price)))))

(define apple (Fruit 3))
(display (object.=> apple 'fetch-price))

; (display (plus 1 2 3))
; (display (object-type apple))
; (display (type apple))

; (define jack 12)
; (display 12)