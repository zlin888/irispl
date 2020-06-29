# typed-scheme
Yet another lisp-language. Typed-scheme in C++ VM

✅ List \
✅ String \
✅ Number \
✅ Quote \
✅ Function (Of course) \
✅ Class & Instance - Inheritance

❎ C++ Interface \
❎ Garbage Collection \
❎ Package Manager

# Usage
## Compile
It, at lease, can be compiled with clang 11.0.0 under the MacOS Catalina.
```
git@github.com:zlin888/irispl.git
cd irispl
make
```
## Run
`./main` is the REPL program. \
`./main path/to/your/iris.scm/file` will compile your iris code and execute it via the VM.

# User Manual

## Class
```
(class class-name (init-variable0 init-variable1 ...)
  (super (father-class-name value)
    ((method-name0 (self variable0) method-body0)
     (method-name1 (self variable0) method-body1))))
```

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
