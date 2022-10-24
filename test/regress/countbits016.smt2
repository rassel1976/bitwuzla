(set-option :incremental false)
(set-info :source "Verifies correcntess of Peter Wegner's algorithm:
P. Wegner.
A technique for counting ones in a binary computer.
CACM 3(5), 1960.
Bit-width: 16

Contributed by Armin Biere (armin.biere@jku.at).")
(set-info :status unsat)
(set-info :category "crafted")
(set-logic QF_BV)
(declare-fun v1 () (_ BitVec 16))
(check-sat)