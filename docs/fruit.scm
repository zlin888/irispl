(import object)

(class Fruit (name)
  (super object.base)
  ((get-name (lambda (self) name))))

(class Apple (color name)
  (super (Fruit name))
  ((get-color (lambda (self) color))
  (set-color! (lambda (self new-color)
    (set! color new-color)))))

(define red-apple (Apple "red", "Apple"))

(display (object.=> red-apple 'get-color))
(display (object.=> red-apple 'get-name))
(object.=> red-apple 'set-color! "blue")
(display (object.=> red-apple 'get-color))



