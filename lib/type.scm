(import functools)
(import object)

(define check-types
  (lambda (_type-list object-list)
    (eq? _type-list (functools.map type object-list))))

(define check-type
  (lambda (_type object)
    (eq? _type (type object))))

(define typed-apply
  (lambda (types return-type f args)
    (if (check-types types args)
      (let ((result (apply f args)))
        (if (check-type return-type result)
          result
          'shit))
      'bad)))

(define #lambda
  (lambda (types return-type f)
    (lambda (. args)
      (typed-apply types return-type f args))))

; (class #object-lambda
;   (closure)
;   (super object.base)
;   ((input (lambda (self . args) (apply closure args)))))

(class #object-lambda
  (closure)
  (super object.base)
  ((fetch (lambda (self) 12))))