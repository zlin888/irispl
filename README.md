# typed-scheme
Yet another lisp-language. Typed-scheme in C++ VM

# User Manual

## Let
lexical binding: variable will be defined and has init as default value. the scope of these varibles is inside the let 
application
```
(let ((variable init)
      (variable init)
      (expression)))
```

## Function Arbitrary Arguments Supported
general form:
```
(lambda (arg0 arg1 . agrs)
  (use-agrs-as-list)))
```
example:
```
(define sum-list 
  (lambda (args)
    (if (pair? args)
        (+ (car args) (sum-list (cdr args)))
        (car args))))

(define sum
  (lambda (arg0 agr1 . args)
    (+ (+ arg0 agr1) (sum-list args))))

(sum 1 2 3 4 5) -> 15
```

## Apply
```
(apply function argument-list)
```
equivalent to:
```
(function arg0 arg1 ... argX)
```


# Develop Manual
## some bullets 
* Compile and VM push-store principle: always push inversely and store in order
