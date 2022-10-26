######### The PC and condition code registers #############
register fF { 
    pc : 64 = 0;
    predPC: 64 = 0;
}

register cC {
    SF : 1 = 0;
    ZF : 1 = 1;
}

########## Fetch #############

# Handles stall for load-use hazard
wire loadUse: 1, isJXX: 1, isRET: 1;
loadUse = (E_icode in { MRMOVQ } && (E_dstM == d_srcA || E_dstM == d_srcB));
isJXX = (M_conditionsMet == 0 && M_ifun != 0 && M_icode in { JXX });
isRET = (D_icode in { RET } || E_icode in { RET } || M_icode in { RET });

/* keep the PC the same next cycle */
stall_F = loadUse || ( isRET && !isJXX);

f_icode = i10bytes[4..8];
f_ifun = i10bytes[0..4];

f_rA = [
    f_icode in { RMMOVQ, MRMOVQ, IRMOVQ, OPQ, RRMOVQ, CMOVXX, PUSHQ, POPQ }: i10bytes[12..16];
    1: REG_NONE;
];

f_rB = [
    f_icode in { PUSHQ, POPQ, CALL, RET }: REG_RSP;
    f_icode in { RMMOVQ, MRMOVQ, IRMOVQ, OPQ, RRMOVQ, CMOVXX, JXX }: i10bytes[8..12];
    1 : REG_NONE;
];

f_valC = [
	f_icode in { JXX, CALL } : i10bytes[8..72];
    f_icode in { RMMOVQ, MRMOVQ, IRMOVQ } : i10bytes[16..80];
	1 : 0;
];

f_valP = [
    f_icode in { RMMOVQ, MRMOVQ, IRMOVQ } : pc + 10;
    f_icode in { JXX, CALL } : pc + 9;
    f_icode in { OPQ, RRMOVQ, CMOVXX, PUSHQ, POPQ } : pc + 2;
    1 : pc + 1;
];

pc = [
    W_icode in { RET } : W_valM;
    M_conditionsMet == 0 && M_icode in { JXX } && M_ifun != 0 :  F_pc;
    1 : F_predPC;
];


f_pc = [
    f_icode in { JXX } && f_ifun != 0: f_valP;
    1 : F_pc;
];

f_predPC = [
    f_icode in { CALL, JXX } : f_valC;
    f_icode in { HALT } : pc;
    1 : f_valP;
];

f_Stat = [
	f_icode == HALT : STAT_HLT;
	f_icode > 0xb : STAT_INS;
	1 : STAT_AOK;
];

########## Decode #############
register fD {
    icode : 4 = 0;
    ifun : 4 = 0;
    rA : 4 = REG_NONE;
    rB : 4 = REG_NONE;
    valC : 64 = 0;
    valP : 64 = 0;
    Stat : 3 = STAT_AOK;
}

/* keep same instruction in decode next cycle */
stall_D = loadUse || (E_icode in { POPQ } && (E_rA == d_srcA || E_rB == d_srcB));
bubble_D = (isRET && !isJXX && !loadUse && !(E_icode in { POPQ } && (E_rA == d_srcA || E_rB == d_srcB)));

reg_srcA = [
	D_icode in { RMMOVQ, RRMOVQ, OPQ, CMOVXX, PUSHQ } : D_rA;
	1 : REG_NONE;
];

reg_srcB = [
	D_icode in { RMMOVQ, MRMOVQ, OPQ, PUSHQ, POPQ, CALL, RET } : D_rB;
	1 : REG_NONE;
];

d_dstM = [
    D_icode in { IRMOVQ, RRMOVQ, OPQ, CMOVXX } : D_rB;
    D_icode in { MRMOVQ, POPQ } : D_rA;
    1: REG_NONE;
];

d_dstE = [
    D_icode in { PUSHQ, POPQ, CALL, RET } : D_rB;
    1 : REG_NONE;
];

d_valA = [
    reg_srcA == REG_NONE : 0; # no forwarding if register is none
    reg_srcA == e_dstE : e_valE;
    (reg_srcA == e_dstM)  && (e_icode != POPQ) : e_valE;
    (reg_srcA == m_dstM) && (M_icode in { MRMOVQ }) : m_valM;
    (reg_srcA == W_dstM) && (W_icode in { MRMOVQ }) : W_valM;
    reg_srcA == m_dstM : m_valE; # forward post-memory (M -> E)
    reg_srcA == m_dstE : m_valE; # forward pre-writeback ("register file forwarding")
    reg_srcA == reg_dstE : reg_inputE;
    reg_srcA == reg_dstM : reg_inputM;
    1 : reg_outputA; # returned by register file based on reg_srcA
];

d_valB = [
    reg_srcB == REG_NONE : 0; # no forwarding if register is none
    reg_srcB == e_dstE : e_valE;
    reg_srcB == e_dstM : e_valE;
    (reg_srcB == m_dstM) && (M_icode in { MRMOVQ }) : m_valM; # forward post-memory (M -> E)
    (reg_srcB == W_dstM) && (W_icode in { MRMOVQ }) : W_valM; # forward pre-writeback ("register file forwarding")
    reg_srcB == m_dstM : m_valE;
    reg_srcB == m_dstE : m_valE;
    reg_srcB == reg_dstE : reg_inputE;
    reg_srcB == reg_dstM : reg_inputM;
    1 : reg_outputB; # returned by register file based on reg_srcB
];

d_icode = D_icode;
d_ifun = D_ifun;
d_valC = D_valC;
d_valP = D_valP;
d_Stat = D_Stat;
d_rA = D_rA;
d_rB = D_rB;
d_srcA = reg_srcA;
d_srcB = reg_srcB;

########## Execute #############
register dE {
    icode : 4 = 0;
    ifun : 4 = 0;
    dstM : 4 = REG_NONE;
    dstE : 4 = REG_NONE;
    rA : 4 = REG_NONE;
    rB : 4 = REG_NONE;
    srcA : 4 = REG_NONE;
    srcB : 4 = REG_NONE;
    valA : 64 = 0;
    valB : 64 = 0;
    valC : 64 = 0;
    valP : 64 = 0;
    Stat : 3 = STAT_AOK;
}

/* send nop to execute next cycle */
bubble_E = loadUse || (E_icode in { RET } || M_icode in { RET }) || isJXX;

wire operand1:64, operand2:64;

operand1 = [
    E_icode in { OPQ, RRMOVQ } : E_valA;
    E_icode in { PUSHQ, POPQ, CALL, RET } : E_valB;
	E_icode in { MRMOVQ, RMMOVQ, IRMOVQ } : E_valC;
	1: 0;
];
operand2 = [
	E_icode in { MRMOVQ, RMMOVQ, OPQ } : E_valB;
    E_icode in { PUSHQ, POPQ, CALL, RET } : 8;
	1: 0;
];

e_valE = [
	E_icode in { MRMOVQ, RMMOVQ } : operand1 + operand2;
    (E_icode == OPQ) && (E_ifun == ADDQ): operand1 + operand2;
    (E_icode == OPQ) && (E_ifun == SUBQ): operand2 - operand1;
    (E_icode == OPQ) && (E_ifun == ANDQ): operand1 & operand2;
    (E_icode == OPQ) && (E_ifun == XORQ): operand1 ^ operand2;
    E_icode == IRMOVQ : operand1;
    E_icode == RRMOVQ : operand1;
    E_icode == PUSHQ : operand1 - operand2;
    E_icode == POPQ : operand1 + operand2;
    E_icode == CALL : operand1 - operand2;
    E_icode == RET : operand1 + operand2;
	1 : 0;
];

c_ZF = (e_valE == 0);
c_SF = (e_valE >= 0x8000000000000000);
stall_C = (E_icode != OPQ);

wire conditionsNotMet:1;
conditionsNotMet = (E_icode == CMOVXX && E_ifun == 3 && !C_ZF) || (E_icode == CMOVXX && E_ifun == 2 && !C_SF) || (E_icode == CMOVXX && E_ifun == 6 && (C_ZF || C_SF)) || (E_icode == CMOVXX && E_ifun == LE && !C_ZF && !C_SF) || (E_icode == CMOVXX && E_ifun == GE && C_SF) || (E_icode == CMOVXX && E_ifun == NE && C_ZF) || (E_icode == JXX && E_ifun == 3 && !C_ZF) || (E_icode == JXX && E_ifun == 2 && !C_SF) || (E_icode == JXX && E_ifun == 6 && (C_ZF || C_SF)) || (E_icode == JXX && E_ifun == LE && !C_ZF && !C_SF) || (E_icode == JXX && E_ifun == GE && C_SF) || (E_icode == JXX && E_ifun == NE && C_ZF);

# Checks condition flags
e_conditionsMet = [
    (E_icode in { JXX }) && (E_ifun != 0): !conditionsNotMet;
    1 : M_conditionsMet;
];

e_dstM = [
    (E_icode in { CMOVXX }) && (E_ifun != 0) && conditionsNotMet: REG_NONE;
    1 : E_dstM;
];

e_icode = E_icode;
e_ifun = E_ifun;
e_dstE = E_dstE;
e_srcA = E_srcA;
e_srcB = E_srcB;
e_valA = E_valA;
e_valB = E_valB;
e_valC = E_valC;
e_valP = E_valP;
e_Stat = E_Stat;

########## Memory #############
register eM {
    icode : 4 = 0;
    ifun : 4 = 0;
    dstM : 4 = REG_NONE;
    dstE : 4 = REG_NONE;
    srcA : 4 = REG_NONE;
    srcB : 4 = REG_NONE;
    valA : 64 = 0;
    valB : 64 = 0;
    valC : 64 = 0;
    valE : 64 = 0;
    valP : 64 = 0;
    conditionsMet : 1 = 0;
    Stat : 3 = STAT_AOK;
}

bubble_M = (M_icode in { RET }) || isJXX;

mem_readbit = M_icode in { MRMOVQ, POPQ, RET };
mem_writebit = M_icode in { RMMOVQ, PUSHQ, CALL };

mem_addr = [
    M_icode in { POPQ, RET } : M_valB;
	M_icode in { MRMOVQ, RMMOVQ, PUSHQ, CALL } : M_valE;
    1: 0xBADBADBAD;
];
mem_input = [
	M_icode in { RMMOVQ, PUSHQ } : M_valA;
    M_icode in { CALL } : M_valP;
    1: 0xBADBADBAD;
];

m_valM  = [
    M_icode in { MRMOVQ, POPQ, RET } : mem_output;
    1: 0xBADBADBAD;
];

m_icode = M_icode;
m_ifun = M_ifun;
m_dstM = M_dstM;
m_dstE = M_dstE;
m_valA = M_valA;
m_valB = M_valB;
m_valC = M_valC;
m_valE = M_valE;
m_Stat = M_Stat;

########## Writeback #############
register mW {
    icode : 4 = 0;
    ifun : 4 = 0;
    dstM : 4 = REG_NONE;
    dstE : 4 = REG_NONE;
    valA : 64 = 0;
    valB : 64 = 0;
    valC : 64 = 0;
    valE : 64 = 0;
    valM : 64 = 0;
    Stat : 3 = STAT_AOK;
}

reg_dstM = W_dstM;

reg_inputM = [
    W_icode in { RRMOVQ } : W_valA;
    W_icode in { IRMOVQ } : W_valC;
    W_icode in { OPQ } : W_valE;
	W_icode in { MRMOVQ, POPQ } : W_valM;
    1: 0xBADBADBAD;
];

reg_inputE = [
	W_icode in { PUSHQ, POPQ, CALL, RET } : W_valE;
    1: 0;
];

reg_dstE = [
    W_icode in { PUSHQ, POPQ, CALL, RET } && (W_dstM != W_dstE) : W_dstE;
    1 : REG_NONE;
];

Stat = W_Stat;