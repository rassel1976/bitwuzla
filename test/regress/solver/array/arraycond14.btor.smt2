(set-logic QF_ABV)
(set-info :status unsat)
(declare-const a0 (Array (_ BitVec 1) (_ BitVec 1) ))
(declare-const a1 (Array (_ BitVec 1) (_ BitVec 1) ))
(declare-const v0 (_ BitVec 1))
(declare-const v1 (_ BitVec 1))
(declare-const v2 (_ BitVec 1))
(declare-const v3 (_ BitVec 1))
(declare-const v4 (_ BitVec 1))
(assert (= #b1 (bvand (ite (= a0 a1) #b1 #b0) (bvnot (ite (= (select (ite (= v2 #b1) (ite (= v0 #b1) a0 a1) (ite (= v1 #b1) a1 a0)) v4) (select (ite (= v3 #b1) (ite (= v1 #b1) a1 a0) (ite (= v0 #b1) a0 a1)) v4)) #b1 #b0)))))
(check-sat)