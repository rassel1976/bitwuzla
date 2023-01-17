(set-option :produce-models true)
(set-info :status sat)
(set-logic QF_ABV)

(declare-fun |UNROLL#30| () (_ BitVec 1))
(declare-fun |UNROLL#36| () (_ BitVec 1))

(declare-fun |UNROLL#42| () (_ BitVec 32))
(define-fun |UNROLL#41| () Bool
    (= |UNROLL#42| #b11111111111111111111111111111111))
(assert |UNROLL#41|)

(declare-fun |UNROLL#106| () Bool)
(assert |UNROLL#106|)
(check-sat)

(define-fun |UNROLL#294| () Bool
    (not (or
        (= ((_ extract 0 0) |UNROLL#30|) #b1)
        (= ((_ extract 0 0) |UNROLL#36|) #b1))))

(define-fun |UNROLL#454| () (_ BitVec 32) (bvnot |UNROLL#42|))
(define-fun |UNROLL#300| () (_ BitVec 32)
    (bvor |UNROLL#454|
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
        (concat (ite |UNROLL#294| #b1 #b0)
            (ite |UNROLL#294| #b1 #b0))))))))))))))))))))))))))))))))))

(define-fun |UNROLL#298| () (_ BitVec 32) (bvnot |UNROLL#300|))
(define-fun |UNROLL#297| () Bool (distinct
    (concat #b00000000000000000000000000000000 |UNROLL#298|)
    #b0000000000000000000000000000000000000000000000000000000000000000))