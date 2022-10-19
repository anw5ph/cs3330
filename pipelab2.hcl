######### The PC #############
register fF { 
    pc:64 = 0; 
}

########## Fetch #############
pc = F_pc;

# Handles stall for load-use hazard
wire loadUse: 1;
loadUse = (E_icode == MRMOVQ && (E_dstM == d_srcA || E_dstM == d_srcB));

/* keep the PC the same next cycle */
stall_F = loadUse;

f_icode = i10bytes[4..8];
f_rA = i10bytes[12..16];
f_rB = i10bytes[8..12];

f_valC = [
	f_icode in { JXX } : i10bytes[8..72];
	1 : i10bytes[16..80];
];

wire offset:64, valP:64;
offset = [
	f_icode in { HALT, NOP, RET } : 1;
	f_icode in { RRMOVQ, OPQ, PUSHQ, POPQ } : 2;
	f_icode in { JXX, CALL } : 9;
	1 : 10;
];

valP = F_pc + offset;

f_Stat = [
	f_icode == HALT : STAT_HLT;
	f_icode > 0xb : STAT_INS;
	1 : STAT_AOK;
];

########## Decode #############
register fD {
    icode : 4 = 0;
    rA : 4 = REG_NONE;
    rB : 4 = REG_NONE;
    valC : 64 = 0;
    Stat : 3 = STAT_AOK;
}

/* keep same instruction in decode next cycle */
stall_D = loadUse;

reg_srcA = [
	D_icode in {RMMOVQ} : D_rA;
	1 : REG_NONE;
];

reg_srcB = [
	D_icode in {RMMOVQ, MRMOVQ} : D_rB;
	1 : REG_NONE;
];

d_dstM = [  /* could also be reg_dstE, which might be easier on the homework */
    D_icode in {MRMOVQ} : D_rA;
    1: REG_NONE;
];

d_valA = [
    reg_srcA == REG_NONE : 0; # no forwarding if register is none
    reg_srcA == m_dstM : m_valM; # forward post-memory (M -> E)
    reg_srcA == W_dstM : W_valM; # forward pre-writeback ("register file forwarding")
    1 : reg_outputA; # returned by register file based on reg_srcA
];

d_valB = [
    reg_srcB == REG_NONE : 0; # no forwarding if register is none
    reg_srcB == m_dstM : m_valM; # forward post-memory (M -> E)
    reg_srcB == W_dstM : W_valM; # forward pre-writeback ("register file forwarding")
    1 : reg_outputB; # returned by register file based on reg_srcB
];

d_icode = D_icode;
d_valC = D_valC;
d_Stat = D_Stat;
d_srcA = reg_srcA;
d_srcB = reg_srcB;

########## Execute #############
register dE {
    icode : 4 = 0;
    dstM : 4 = REG_NONE;
    srcA : 4 = REG_NONE;
    srcB : 4 = REG_NONE;
    valA : 64 = 0;
    valB : 64 = 0;
    valC : 64 = 0;
    Stat : 3 = STAT_AOK;
}

/* send nop to execute next cycle */
bubble_E = loadUse;

wire operand1:64, operand2:64;

operand1 = [
	E_icode in { MRMOVQ, RMMOVQ } : E_valC;
	1: 0;
];
operand2 = [
	E_icode in { MRMOVQ, RMMOVQ } : E_valB;
	1: 0;
];

e_valE = [
	E_icode in { MRMOVQ, RMMOVQ } : operand1 + operand2;
	1 : 0;
];

e_icode = E_icode;
e_dstM = E_dstM;
e_valA = E_valA;
e_Stat = E_Stat;

########## Memory #############
register eM {
    icode : 4 = 0;
    dstM : 4 = REG_NONE;
    valA : 64 = 0;
    valE : 64 = 0;
    Stat : 3 = STAT_AOK;
}

mem_readbit = M_icode in { MRMOVQ };
mem_writebit = M_icode in { RMMOVQ };

mem_addr = [
	M_icode in { MRMOVQ, RMMOVQ } : M_valE;
    1: 0xBADBADBAD;
];
mem_input = [
	M_icode in { RMMOVQ } : M_valA;
    1: 0xBADBADBAD;
];

m_valM  = [
    M_icode in { MRMOVQ } : mem_output;
    1 : 0;
];

m_icode = M_icode;
m_dstM = M_dstM;
m_Stat = M_Stat;

########## Writeback #############
register mW {
    icode : 4 = 0;
    dstM : 4 = REG_NONE;
    valM : 64 = 0;
    Stat : 3 = STAT_AOK;
}

reg_dstM = W_dstM;

reg_inputM = [
	W_icode in {MRMOVQ} : W_valM;
    1: 0xBADBADBAD;
];

Stat = W_Stat;
f_pc = valP;