(set-logic QF_ABV)
(declare-fun _substvar_81_ () (_ BitVec 1))
(declare-fun _substvar_85_ () (_ BitVec 32))
(declare-fun _substvar_89_ () (_ BitVec 32))
(declare-fun _substvar_94_ () (_ BitVec 32))
(declare-fun _substvar_359_ () (_ BitVec 1))
(declare-fun _substvar_360_ () (_ BitVec 32))
(declare-fun _substvar_372_ () (_ BitVec 1))
(declare-fun _substvar_378_ () (_ BitVec 32))
(declare-fun _substvar_379_ () (_ BitVec 2))
(declare-fun _substvar_383_ () (_ BitVec 1))
(declare-fun _substvar_389_ () (_ BitVec 32))
(declare-fun _substvar_397_ () (_ BitVec 32))
(declare-fun _substvar_403_ () (_ BitVec 32))
(declare-fun _substvar_611_ () Bool)
(declare-fun _substvar_640_ () Bool)
(declare-fun _substvar_814_ () (_ BitVec 32))
(declare-fun _substvar_815_ () (_ BitVec 1))
(declare-fun _substvar_824_ () (_ BitVec 32))
(declare-fun _substvar_839_ () (_ BitVec 32))
(declare-fun _substvar_849_ () (_ BitVec 32))
(declare-fun _substvar_1200_ () (_ BitVec 32))
(declare-fun |UNROLL#2334| () (Array (_ BitVec 5) (_ BitVec 32)))
(assert (= (ite _substvar_611_ #b0 _substvar_359_) _substvar_81_))
(assert (and (= (ite (= ((_ extract 0 0) (ite (= ((_ extract 0 0) _substvar_81_) #b1) #b1 #b0)) #b1) #b1 #b0) _substvar_372_) (= (select |UNROLL#2334| ((_ extract 19 15) (ite (= ((_ extract 0 0) (ite (= ((_ extract 0 0) _substvar_81_) #b1) #b1 #b0)) #b1) (_ bv0 32) _substvar_85_))) _substvar_89_) (= (ite (= ((_ extract 0 0) (ite (= ((_ extract 0 0) _substvar_81_) #b1) #b1 #b0)) #b1) (_ bv0 32) _substvar_85_) _substvar_360_) true))
(define-fun |UNROLL#3338| () (_ BitVec 32) _substvar_89_)
(assert (and (= _substvar_372_ (_ bv0 1)) (= (concat (_ bv0 1) (ite (= (bvand _substvar_360_ #b00000000000000000000000001001000) #b00000000000000000000000001000000) #b1 #b0)) _substvar_379_) (= |UNROLL#3338| _substvar_378_) true))
(assert (= (ite (not (= ((_ extract 0 0) _substvar_379_) #b1)) #b0 (ite (= _substvar_378_ (_ bv0 32)) #b1 #b0)) _substvar_383_))
(assert (= (ite (= ((_ extract 0 0) (ite (or false false (= ((_ extract 0 0) _substvar_383_) #b1) false) #b1 #b0)) #b1) (_ bv0 32) _substvar_389_) _substvar_1200_))
(assert (= _substvar_1200_ _substvar_849_))
(assert (= _substvar_849_ _substvar_839_))
(assert (= _substvar_839_ _substvar_397_))
(define-fun |UNROLL#6749| () (_ BitVec 32) _substvar_397_)
(assert (= |UNROLL#6749| _substvar_94_))
(define-fun |UNROLL#7307| () (_ BitVec 32) _substvar_94_)
(assert (= (ite _substvar_640_ #b00000000000000000000000000000000 |UNROLL#7307|) _substvar_824_))
(assert (= _substvar_824_ _substvar_814_))
(assert (= _substvar_814_ _substvar_403_))
(push 1)
(assert false)
(set-info :status unsat)
(check-sat)
(pop 1)
(assert (= (ite (= _substvar_403_ (_ bv0 32)) #b1 #b0) _substvar_815_))
(push 1)
(assert (not (= ((_ extract 0 0) _substvar_815_) #b1)))
(set-info :status sat)
(check-sat)
(exit)