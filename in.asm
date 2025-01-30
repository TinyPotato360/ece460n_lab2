        .ORIG x3000
        ADD R0 R0 x7
        ADD R1 R0 R0
        AND R2 R0 x3
        AND R3 R1 R0
        BRnzp next

next    ;;;
        HALT
        .END