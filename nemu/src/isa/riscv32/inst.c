/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define CALL 0
#define RET 1

#ifdef CONFIG_FTRACE
void add_ftrace(vaddr_t pc, int type, vaddr_t address);
#endif

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write
#define CSR(i) (cpu.csr[i])

enum {
    TYPE_I, TYPE_U, TYPE_S,
    TYPE_N, TYPE_J, TYPE_R,
    TYPE_B // none
};

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)
#define immJ() do { *imm = (SEXT(BITS(i, 31, 31), 1) << 20) | (BITS(i, 19, 12) << 12) | (BITS(i, 20, 20) << 11) | (BITS(i, 30, 21) << 1) | 0; } while(0)
#define immB() do { *imm = (SEXT(BITS(i, 31, 31), 1) << 12) | (BITS(i, 7, 7) << 11) | (BITS(i, 30, 25) << 5) | (BITS(i, 11, 8) << 1) | 0;} while(0)

static void decode_operand(Decode *s, word_t *rd, word_t *rs1_call_ret, word_t *src1, word_t *src2, word_t *imm, word_t *shamt, int type) {
    uint32_t i = s->isa.inst.val;
    word_t rs1 = BITS(i, 19, 15);
    word_t rs2 = BITS(i, 24, 20);
    *rs1_call_ret = rs1;
    *rd     = BITS(i, 11, 7);
    *shamt  = BITS(i, 24, 20);
    switch (type) {
        case TYPE_I: src1R();          immI(); break;
        case TYPE_U:                   immU(); break;
        case TYPE_S: src1R(); src2R(); immS(); break;
        case TYPE_J:                   immJ(); break;
        case TYPE_R: src1R(); src2R();         break;
        case TYPE_B: src1R(); src2R(); immB(); break;
    }
}

#ifdef CONFIG_FTRACE
static void identity_cal_ret(const char *name, Decode *s, int rd, int rs1) {
    if (strcmp("jal", name) == 0) {
        if (rd == 1 || rd == 5) {
            add_ftrace(s->pc, CALL, s->dnpc);
        } 
        /* else if (rd == 0) {*/
        /*     tail_call(s->pc);*/
        /* }*/
    }
    if (strcmp("jalr", name) == 0) {
        if ((rd != 1 && rd != 5) && (rs1 == 1 || rs1 == 5))
            add_ftrace(s->pc, RET, s->dnpc);
        else if ((rd == 1 || rd == 5) && (rs1 != 1 && rs1 != 5))
            add_ftrace(s->pc, CALL, s->dnpc);
        /* else if ((rd == 0) && (rs1 != 1 && rs1 != 5)) {*/
        /*     tail_call(s->pc);*/
        /* }*/
    }
}
#endif

static int decode_exec(Decode *s) {
    word_t rd = 0;
    word_t rs1 = 0;
    word_t src1 = 0, src2 = 0, imm = 0, shamt;
    s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
    decode_operand(s, &rd, &rs1, &src1, &src2, &imm, &shamt, concat(TYPE_, type)); \
    __VA_ARGS__ ; \
}

    INSTPAT_START();
    //RV32I Base Integer Instruction Set
    INSTPAT("???????????????????? ????? 0010111",     auipc  , U, R(rd) = s->pc + imm);
    INSTPAT("???????????????????? ????? 0110111",     lui    , U, R(rd) = imm);

    INSTPAT("???????????? ????? 100 ????? 0000011",   lbu    , I, R(rd) = Mr(src1 + imm, 1));
    INSTPAT("???????????? ????? 000 ????? 0000011",   lb     , I, R(rd) = SEXT(Mr(src1 + imm, 1), 8));
    INSTPAT("???????????? ????? 001 ????? 0000011",   lh     , I, R(rd) = SEXT(Mr(src1 + imm, 2), 16));
    INSTPAT("???????????? ????? 101 ????? 0000011",   lhu    , I, R(rd) = Mr(src1 + imm, 2));
    INSTPAT("???????????? ????? 010 ????? 0000011",   lw     , I, R(rd) = Mr(src1 + imm, 4));
    INSTPAT("???????????? ????? 000 ????? 0010011",   addi   , I, R(rd) = src1 + imm);
    INSTPAT("???????????? ????? 000 ????? 1100111",   jalr   , I, R(rd) = s->pc + 4; s->dnpc = (src1 + imm) & 0xfffffffe;
                                                                  IFDEF(CONFIG_FTRACE,
                                                                      {identity_cal_ret("jalr", s, rd, rs1);}
                                                                  )
            );
    INSTPAT("???????????? ????? 011 ????? 0010011",   sltiu  , I, R(rd) = src1 < imm ? 1 : 0);
    INSTPAT("???????????? ????? 010 ????? 0010011",   slti   , I, R(rd) = (sword_t)src1 < (sword_t)imm ? 1 : 0);
    INSTPAT("???????????? ????? 111 ????? 0010011",   andi   , I, R(rd) = src1 & imm);
    INSTPAT("???????????? ????? 100 ????? 0010011",   xori   , I, R(rd) = src1 ^ imm);
    INSTPAT("???????????? ????? 110 ????? 0010011",   ori    , I, R(rd) = src1 | imm);
    INSTPAT("0000000 ????? ????? 101 ????? 0010011",  srli   , I, R(rd) = src1 >> shamt);
    INSTPAT("0000000 ????? ????? 001 ????? 0010011",  slli   , I, R(rd) = src1 << shamt);
    INSTPAT("0100000 ????? ????? 101 ????? 0010011",  srai   , I, R(rd) = (sword_t)src1 >> shamt);

    INSTPAT("??????? ????? ????? 000 ????? 0100011",  sb     , S, Mw(src1 + imm, 1, src2));
    INSTPAT("??????? ????? ????? 001 ????? 0100011",  sh     , S, Mw(src1 + imm, 2, src2));
    INSTPAT("??????? ????? ????? 010 ????? 0100011",  sw     , S, Mw(src1 + imm, 4, src2));

    INSTPAT("0000000 ????? ????? 000 ????? 0110011",  add    , R, R(rd) = src1 + src2);
    INSTPAT("0100000 ????? ????? 000 ????? 0110011",  sub    , R, R(rd) = src1 - src2);
    INSTPAT("0000000 ????? ????? 001 ????? 0110011",  sll    , R, R(rd) = src1 << (src2 & 0x0000001f));
    INSTPAT("0000000 ????? ????? 010 ????? 0110011",  slt    , R, R(rd) = (sword_t)src1 < (sword_t)src2 ? 1 : 0);
    INSTPAT("0000000 ????? ????? 011 ????? 0110011",  sltu   , R, R(rd) = src1 < src2 ? 1 : 0);
    INSTPAT("0000000 ????? ????? 101 ????? 0110011",  srl    , R, R(rd) = src1 >> (src2 & 0x0000001f));
    INSTPAT("0100000 ????? ????? 101 ????? 0110011",  sra    , R, R(rd) = (sword_t)src1 >> (src2 & 0x0000001f));
    INSTPAT("0000000 ????? ????? 100 ????? 0110011",  xor    , R, R(rd) = src1 ^ src2);
    INSTPAT("0000000 ????? ????? 110 ????? 0110011",  or     , R, R(rd) = src1 | src2);
    INSTPAT("0000000 ????? ????? 111 ????? 0110011",  and    , R, R(rd) = src1 & src2);

    INSTPAT("???????????????????? ????? 1101111",     jal    , J, R(rd) = s->pc + 4; s->dnpc = s->pc + imm;
                                                                  IFDEF(CONFIG_FTRACE, 
                                                                      {identity_cal_ret("jal", s, rd, rs1);}
                                                                  )
            );

    INSTPAT("??????? ????? ????? 000 ????? 1100011",  beq    , B, if (src1 == src2) s->dnpc = s->pc + imm);
    INSTPAT("??????? ????? ????? 001 ????? 1100011",  bne    , B, if (src1 != src2) s->dnpc = s->pc + imm);
    INSTPAT("??????? ????? ????? 111 ????? 1100011",  bgeu   , B, if (src1 >= src2) s->dnpc = s->pc + imm);
    INSTPAT("??????? ????? ????? 101 ????? 1100011",  bge    , B, if ((sword_t)src1 >= (sword_t)src2) s->dnpc = s->pc + imm);
    INSTPAT("??????? ????? ????? 100 ????? 1100011",  blt    , B, if ((sword_t)src1 < (sword_t)src2) s->dnpc = s->pc + imm);
    INSTPAT("??????? ????? ????? 110 ????? 1100011",  bltu   , B, if (src1 < src2) s->dnpc = s->pc + imm);

    INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
    INSTPAT("000000000000 00000 000 00000 1110011",   ecall  , N, ECALL(s->dnpc));

    //“M” Standard Extension for Integer Multiplication and Division
    INSTPAT("0000001 ????? ????? 110 ????? 0110011",  rem    , R, R(rd) = (sword_t)src1 % (sword_t)src2);
    INSTPAT("0000001 ????? ????? 111 ????? 0110011",  remu   , R, R(rd) = src1 % src2);
    INSTPAT("0000001 ????? ????? 000 ????? 0110011",  mul    , R, R(rd) = src1 * src2);
    INSTPAT("0000001 ????? ????? 001 ????? 0110011",  mulh   , R, R(rd) = (((int64_t)(sword_t)src1) * ((int64_t)(sword_t)src2) >> 32));
    INSTPAT("0000001 ????? ????? 011 ????? 0110011",  mulhu  , R, R(rd) = (((uint64_t)src1 * ((uint64_t)src2)) >> 32));
    INSTPAT("0000001 ????? ????? 101 ????? 0110011",  divu   , R, R(rd) = src1 / src2);
    INSTPAT("0000001 ????? ????? 100 ????? 0110011",  div    , R, R(rd) = (sword_t)src1 / (sword_t)src2);

    //“Zicsr” Extension
    INSTPAT("???????????? ????? 001 ????? 1110011",   csrrw  , I, R(rd) = CSR(imm); CSR(imm) = src1);
    INSTPAT("???????????? ????? 010 ????? 1110011",   csrrs  , I, R(rd) = CSR(imm); CSR(imm) = src1 | CSR(imm));
    INSTPAT("???????????? ????? 011 ????? 1110011",   csrrc  , I, R(rd) = CSR(imm); CSR(imm) = (~src1) & CSR(imm));
    INSTPAT("???????????? ????? 101 ????? 1110011",   csrrwi , I, R(rd) = CSR(imm); CSR(imm) = rs1);
    INSTPAT("???????????? ????? 110 ????? 1110011",   csrrsi , I, R(rd) = CSR(imm); CSR(imm) = rs1 | CSR(imm));
    INSTPAT("???????????? ????? 111 ????? 1110011",   csrrci , I, R(rd) = CSR(imm); CSR(imm) = (~rs1) | CSR(imm));

    INSTPAT("001100000010 00000 000 00000 1110011",   mret   , N, s->dnpc = CSR(MEPC));


    INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
    INSTPAT_END();

    R(0) = 0; // reset $zero to 0

    return 0;
}

int isa_exec_once(Decode *s) {
    s->isa.inst.val = inst_fetch(&s->snpc, 4);
    return decode_exec(s);
}
