
#include <stdio.h>
#include <stdlib.h>
#include "tracer.h"

void main() {
  int i = 3;
  int* p1 = &i;
  printf("p1: %p\n", p1);

  int* p2 = malloc(sizeof(int));
  *p2 = 3;
  printf("p2: %p, *p2: %d\n", p2, *p2);
}

foo () {
    int i = 0;
    int n = 300;
    int low = 0;
    int MVL = 64;
    int END = 0;
    VLR = 64;

    int vl = MVL;
    while (!END) {
        if (low + vl > 300) {  // jump if low+vl-300 <= 0
            vl = 300 - low;
            END = 1;
        }

        for (i = low; i < low+vl; i++) {
            c_re[i] = a_re[i] * b_re[i] - a_im[i] * b_im[i];
            c_im[i] = a_re[i] * b_im[i] + a_im[i] * b_re[i];
        }
        low = i;
    }


    LW Ri, #0; load i
    LW Rj, #0; load j
    LW Rn, #300; load n
    LW Rlow, #0; load low
    LW Rmvl, #64;
    LW VLR, #64;
    LW Rend, #0;

    L1:
    BNEZ Rend DONE;
    ADD R1, Rlow, Rvl;
    SUB R1, R1, Rn;
    BLEZ R1, L2
    SUB VLR, Rn, Rlow;
    LW Rend, #1;

    L2:
    LW Ri, Rlow;
    LV V1, (Ri + Ra_re);
    LV V2, (Ri + Rb_re);
    LV V3, (Ri + Ra_im);
    LV V4, (Ri + Rb_im);
    LV V5, (Ri + Rc_re);
    LV V6, (Ri + Rc_im);
    MULVV V7, V1, V2;
    MULVV V8, V3, V4;
    SUBVV V9, V7, V8



    ADD Rlow, VLR

    DONE:
    ...







    ;; assume all the following instructions that have a floating point destination register use single-precision floating point opcode

    ADD RclP, RclP, Rh; suppose Rh has value of h*4
    LW Rk, seq_length; suppose seq_length is a constant

    OuterLoop:
    BLEZ Rk, Exit;
    LW Ri, 4; inner loop counter

    AssignLoop:
    BLEZ Ri, L1;
    LW Fs, #0;
    LW Ft, #0;
    LW F1, 0(RclL);
    LW F2, 4(RclL);
    LW F3, 8(RclL);
    LW F4, 12(RclL);
    LW F5, 0(RclR);
    LW F6, 4(RclR);
    LW F7, 8(RclR);
    LW F8, 12(RclR);
    LW Fa, 0(RtiPL);
    LW Fb, 4(RtiPL);
    LW Fc, 8(RtiPL);
    LW Fd, 12(RtiPL);
    MUL Fa, F1, Fa;
    MUL Fb, F2, Fb;
    MUL Fc, F3, Fc;
    MUL Fd, F4, Fd;
    ADD Fs, Fs, Fa;
    ADD Fs, Fs, Fb;
    ADD Fs, Fs, Fc;
    ADD Fs, Fs, Fd;
    LW Fa, 0(RtiPR);
    LW Fb, 4(RtiPR);
    LW Fc, 8(RtiPR);
    LW Fd, 12(RtiPR);
    MUL Fa, F5, Fa;
    MUL Fb, F6, Fb;
    MUL Fc, F7, Fc;
    MUL Fd, F8, Fd;
    ADD Ft, Ft, Fa;
    ADD Ft, Ft, Fb;
    ADD Ft, Ft, Fc;
    ADD Ft, Ft, Fd;
    MUL Fs, Fs, Ft;
    SW Fs, 0(RclP);
    ADD RtiPL, RtiPL, #16;
    ADD RtiPR, RtiPR, #16;
    ADD RclP, RclP, #4;
    ADD Ri, Ri, #-1;
    J AssignLoop;
    ADD RclL, RclL, #16;
    ADD RclR, RclR, #16;
    ADD Rk, Rk, #-1;
    Exit:















    ;; assume all the following instructions that have a floating point destination register use single-precision floating point opcode

    ADD RclP, RclP, Rh; suppose Rh has value of h*4
    LW Rk, seq_length; suppose seq_length is a constant

OuterLoop:
    BLEZ Rk, Exit;
    LW Ri, 4; inner loop counter

AssignLoop:
    BLEZ Ri, L1;

    LW Fs, #0;
    LW Ft, #0;

    ; inviants in one loop iteration
    LW F1, 0(RclL);
    LW F2, 4(RclL);
    LW F3, 8(RclL);
    LW F4, 12(RclL);
    LW F5, 0(RclR);
    LW F6, 4(RclR);
    LW F7, 8(RclR);
    LW F8, 12(RclR);

    ; add four products tiPL[0]*clL[0] + tiPL[1]*clL[1] + tiPL[2]*clL[2] + tiPL[3]*clL[3]
    LW Fa, 0(RtiPL);
    LW Fb, 4(RtiPL);
    LW Fc, 8(RtiPL);
    LW Fd, 12(RtiPL);

    MUL Fa, F1, Fa;
    MUL Fb, F2, Fb;
    MUL Fc, F3, Fc;
    MUL Fd, F4, Fd;

    ADD Fs, Fs, Fa;
    ADD Fs, Fs, Fb;
    ADD Fs, Fs, Fc;
    ADD Fs, Fs, Fd;

    ; add four products tiPR[0]*clR[0] + tiPR[1]*clR[1] + tiPR[2]*clR[2] + tiPR[3]*clR[3]
    LW Fa, 0(RtiPR);
    LW Fb, 4(RtiPR);
    LW Fc, 8(RtiPR);
    LW Fd, 12(RtiPR);

    MUL Fa, F5, Fa;
    MUL Fb, F6, Fb;
    MUL Fc, F7, Fc;
    MUL Fd, F8, Fd;

    ADD Ft, Ft, Fa;
    ADD Ft, Ft, Fb;
    ADD Ft, Ft, Fc;
    ADD Ft, Ft, Fd;

    MUL Fs, Fs, Ft;
    SW Fs, 0(RclP);

    ; increment addresses for next assignment
    ; only tiPL and tiPR should increment here
    ADD RtiPL, RtiPL, #16;
    ADD RtiPR, RtiPR, #16;
    ADD RclP, RclP, #4;
    ADD Ri, Ri, #-1;
    J AssignLoop;

L1:
    ADD RclL, RclL, #16;
    ADD RclR, RclR, #16;

    ADD Rk, Rk, #-1;
    J OuterLoop;
Exit:










    ADD RclP, RclP, Rh; suppose Rh has value of h*4
    LW Rk, seq_length; suppose seq_length is a constant
    LW VL, #4

OuterLoop:
    BLEZ Rk, Exit;
    LW Ri, 4; inner loop counter

AssignLoop:
    BLEZ Ri, L1;

    LW Fs, #0;
    LW Ft, #0;

    ; inviants in one loop iteration
    LW V1, 0(RclL);
    LW V2, 0(RclR);

    ; add four products tiPL[0]*clL[0] + tiPL[1]*clL[1] + tiPL[2]*clL[2] + tiPL[3]*clL[3]
    LW Va, 0(RtiPL);

    MULVV Va, V1, Va;
    SUMR.S Fs, Va;

    ; add four products tiPR[0]*clR[0] + tiPR[1]*clR[1] + tiPR[2]*clR[2] + tiPR[3]*clR[3]
    LW Va, 0(RtiPR);

    MULVV Va, V2, Va;
    SUMR.S Ft, Va;

    MUL Fs, Fs, Ft;
    SW Fs, 0(RclP);

    ; increment addresses for next assignment
    ; only tiPL and tiPR should increment here
    ADD RtiPL, RtiPL, #16;
    ADD RtiPR, RtiPR, #16;
    ADD RclP, RclP, #4;
    ADD Ri, Ri, #-1;
    J AssignLoop;

    L1:
    ADD RclL, RclL, #16;
    ADD RclR, RclR, #16;

    ADD Rk, Rk, #-1;
    Exit:

}