register fF {
	predPC : 64 = 0;
}

register cC {
	SF:1 = 0;
	ZF:1 = 0;
}

########## Fetch ##########
wire loadUse:1, isRETdem: 1, noRun: 1;
loadUse = (E_icode in { MRMOVQ, POPQ } && E_dstM == reg_srcA) || (E_icode in { MRMOVQ, POPQ } && E_dstM == reg_srcB);
isRETdem = (d_icode in { RET }) || (e_icode in { RET }) || (m_icode in { RET });
noRun = (f_Stat != STAT_AOK);
stall_F = loadUse || isRETdem || noRun;

pc = [ 
	M_icode in { JXX } && !M_conditionsMet : M_oldvalP;
	W_icode in { RET } : W_valM;
	1 : F_predPC;
];

f_icode = i10bytes[4..8];
f_ifun = i10bytes[0..4];

f_rA = i10bytes[12..16];
f_rB = i10bytes[8..12];

f_valC = [
	f_icode in { PUSHQ, POPQ, CALL, RET } : 8;
	1 : i10bytes[16..80];
];

f_oldvalP = pc + 9;

f_valP = [
	f_icode in { RRMOVQ, OPQ, PUSHQ, POPQ }	: pc + 2;
	f_icode in { JXX, CALL } : i10bytes[8..72];
	f_icode in { IRMOVQ, RMMOVQ, MRMOVQ } : pc + 10;
	1 : pc + 1;
];

f_predPC = f_valP;

f_Stat = [
	f_icode == HALT	: STAT_HLT;
	f_icode > 0xb : STAT_INS;
	1 : STAT_AOK;
];

########## Decode ##########
register fD {
	icode : 4 = 0;
	ifun : 4 = 0;
	rA : 4 = REG_NONE;
	rB : 4 = REG_NONE;
	valC : 64 = 0;
    valP : 64 = 0;
	oldvalP : 64 = 0;
	Stat : 3 = 0;
}

# Handle stalling and squashing
wire isRETem: 1;
stall_D = loadUse;
isRETem = (e_icode in { RET }) || (m_icode in { RET });
bubble_D = (e_icode in { JXX } && !e_conditionsMet) || isRETem;

# Source A's register
reg_srcA = [
	D_icode in { RRMOVQ, OPQ, RMMOVQ, PUSHQ, POPQ } : D_rA;
	1 : REG_NONE;
];

# Source B's register (can be rB or stack pointer or none)
reg_srcB = [
	D_icode in { RRMOVQ, RMMOVQ, OPQ, MRMOVQ }	: D_rB;
	D_icode in { PUSHQ, POPQ, CALL, RET } : REG_RSP;
	1 : REG_NONE;
];

# Determines current value in A (accounts for forwarding and hazards)
d_valA = [
    reg_srcA == REG_NONE : 0; # no forwarding if register is none
	reg_srcA == e_dstE && e_icode == IRMOVQ : e_valC;
	reg_srcA == e_dstE && e_icode == RRMOVQ : e_valA;
	reg_srcA == e_dstE && e_icode in { OPQ, PUSHQ, POPQ, CALL } : e_valE;
	reg_srcA == m_dstE && m_icode == IRMOVQ : m_valC;
	reg_srcA == m_dstE && m_icode == RRMOVQ : m_valA;
	reg_srcA == m_dstE && m_icode in { OPQ, PUSHQ, POPQ, CALL } : m_valE;
	reg_srcA == m_dstM && m_icode in { MRMOVQ, POPQ } : m_valM;
    reg_srcA == reg_dstE : reg_inputE;
	reg_srcA == reg_dstM : reg_inputM;
	1 : reg_outputA; # returned by register file based on reg_srcA
];

# Determines current value in B (accounts for forwarding and hazards)
d_valB = [
    reg_srcB == REG_NONE : 0; # no forwarding if register is none
	reg_srcB == e_dstE && e_icode == IRMOVQ : e_valC;
	reg_srcB == e_dstE && e_icode == RRMOVQ : e_valA;
	reg_srcB == e_dstE && e_icode in { OPQ, PUSHQ, POPQ, CALL, RET } : e_valE;
	reg_srcB == m_dstE && m_icode == IRMOVQ	: m_valC;
	reg_srcB == m_dstE && m_icode == RRMOVQ : m_valA;
	reg_srcB == m_dstE && m_icode in { OPQ, PUSHQ, POPQ, CALL, RET } : m_valE;
	reg_srcB == m_dstM && m_icode in {MRMOVQ, POPQ}	: m_valM;
    reg_srcB == reg_dstE : reg_inputE;
	reg_srcB == reg_dstM : reg_inputM;
	1 : reg_outputB; # returned by register file based on reg_srcB
];

# Determines current destination register (can be rB or stack pointer or none)
d_dstE = [
	D_icode in { RRMOVQ, IRMOVQ, OPQ } : D_rB;
	D_icode in { PUSHQ, POPQ, CALL, RET } : REG_RSP;
	1 : REG_NONE;
];

# Determines current destination register (can be rA or none)
d_dstM = [ 
	D_icode in { MRMOVQ, POPQ } : D_rA;
	1 : REG_NONE;
];

# Update values to be transferred through next register bank
d_icode = D_icode;
d_ifun = D_ifun;
d_valC = D_valC;
d_oldvalP = D_oldvalP;
d_Stat = D_Stat;

########## Execute ##########
register dE {
	icode : 4 = 0;
    ifun : 4 = 0;
    dstE : 4 = REG_NONE;
	dstM : 4 = REG_NONE;
	valA : 64 = 0;
	valB : 64 = 0;
    valC : 64 = 0;
	oldvalP	: 64 = 0;
	Stat : 3 = 0;
}

# Handles squashing 
wire isRETm: 1;
isRETm = (m_icode in { RET });
bubble_E = loadUse || (e_icode in { JXX } && !e_conditionsMet) || isRETm;

# Performs ALU operations for provided input values
e_valE = [
	E_icode == OPQ && E_ifun == ADDQ : E_valA + E_valB;
	E_icode == OPQ && E_ifun == SUBQ : E_valB - E_valA;
	E_icode == OPQ && E_ifun == ANDQ : E_valA & E_valB;
	E_icode == OPQ && E_ifun == XORQ : E_valA ^ E_valB;
	E_icode in { RMMOVQ, MRMOVQ, POPQ, RET } : E_valC + E_valB;
	E_icode in { PUSHQ, CALL } : E_valB - E_valC;
	1 : 0;
];

# Handles condition codes and proper stalling
stall_C = !(E_icode == OPQ);
c_ZF = (e_valE == 0);
c_SF = (e_valE >= 0x8000000000000000);

e_conditionsMet = [
	E_ifun == 0: 1; // Always
	E_ifun == 1 : (C_SF || C_ZF); // Less than or equal to
	E_ifun == 2 : C_SF; // Less than
	E_ifun == 3 : C_ZF; // Equal to
	E_ifun == 4 : !C_ZF; // Not equal to
	E_ifun == 5 : (!C_SF || C_ZF); // Greater than or equal to
	E_ifun == 6 : (!C_SF & !C_ZF); // Greater than
	1 : 0; // Base case
];

# Sets destination register to previous destination register or none
e_dstE = [
	E_icode in { RRMOVQ } && !e_conditionsMet : REG_NONE;
	1 : E_dstE;
];

# Update values to be transferred through next register bank
e_icode = E_icode;
e_ifun = E_ifun;
e_dstM = E_dstM;
e_valA = E_valA;
e_valB = E_valB;
e_valC = E_valC;
e_oldvalP = E_oldvalP;
e_Stat = E_Stat;

########## Memory ##########
register eM {
	icode : 4 = 0;
	ifun : 4 = 0;
    dstE : 4 = REG_NONE;
	dstM: 4 = REG_NONE;
	valA : 64 = 0;
    valB : 64 = 0;
    valC : 64 = 0;
	valE : 64 = 0;
	oldvalP	: 64 = 0;
	conditionsMet : 1 = 0;
	Stat : 3 = 0;
}

# Handles squashing
bubble_M = isRETm;

# Determines whether we are reading memory with the current instruction
mem_readbit = [
	M_icode in { RMMOVQ, PUSHQ, CALL } : 0;
	1 : 1;
];

# Determines whether we are writing to memory with the current instruction
mem_writebit = [
	M_icode in { RMMOVQ, PUSHQ, CALL } : 1;
	1 : 0;
];

# Determines what the value to write to memory is
mem_input = [
	M_icode in { RMMOVQ, PUSHQ } : M_valA;
	M_icode in { CALL }	: M_oldvalP;
	1 : 0;
];

# Determines what the memory address to visit is
mem_addr = [
	M_icode in { RMMOVQ, MRMOVQ, PUSHQ, CALL } : M_valE;
	M_icode in { POPQ, RET } : M_valB;
	1 : 0;
];

# Update values to be transferred through next register bank
m_icode = M_icode;
m_ifun = M_ifun;
m_dstE = M_dstE;
m_dstM = M_dstM;
m_valA = M_valA;
m_valC = M_valC;
m_valE = M_valE;
m_valM = mem_output;
m_conditionsMet = M_conditionsMet;
m_Stat = M_Stat;

########## Writeback ##########
register mW {
	icode : 4 = 0;
	ifun : 4 = 0;
    dstE : 4 = REG_NONE;
	dstM : 4 = REG_NONE;
	valA : 64 = 0;
	valC : 64 = 0;
	valE : 64 = 0;
	valM : 64 = 0;
    conditionsMet : 1 = 0;
	Stat : 3 = 0;
}

# Determines what the input for the destination is
reg_inputM = [
	W_icode in { MRMOVQ, POPQ } : W_valM;
    1 : 0;
];

# Determines what the destination register is given the current instruction
reg_dstE = [
	((!W_conditionsMet) && W_icode in { RRMOVQ }) || (W_icode in { HALT }) : REG_NONE;
	1 : W_dstE;
];

# Determines the input for the destination register given the current instruction and condition codes
reg_inputE = [
	W_icode in { RRMOVQ } && W_conditionsMet : W_valA;
	W_icode in { OPQ, PUSHQ, POPQ, CALL, RET } : W_valE;
	W_icode in { IRMOVQ } : W_valC;
	1 : 0;
];

# Update values to be transferred through next register bank
reg_dstM = W_dstM;

########## Status Update ##########
Stat = W_Stat;