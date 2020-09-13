(import type)

; test constant
(display (eq? (type 1) type.number))
(display (eq? (type "s") type.string))

; test type.check-type
(display 
  (type.check-types 
    (list type.number, type.string)
    (list 1 "SHI")))

; test typed-apply
(display
  (type.typed-apply
    (list type.number type.number)
     type.number
     + 
     (list 1 2)))

(define typed-add
  (type.#lambda
    (list type.number type.number)
    type.number
    +))

(display (typed-add 3 2))
(display (typed-add 3 "two"))

