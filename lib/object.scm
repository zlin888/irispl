(define selector-undefined-error 'selector-undefined-error)

(define base 
  (lambda (selector self) selector-undefined-error))

(define =>
  (lambda (instance method . args)
    (let ((_method (instance method)))
      (apply _method (cons instance args)))))