########## the PC and condition codes registers #############
register cC {
    SF : 1 = 0;
    ZF : 1 = 1;
}

register pF { 
    pc : 64 = 0;
}

########## Fetch #############
pc = F_pc;

f_icode = i10bytes[4..8];
f_ifun = i10bytes[0..4];

# Register A
f_rA = [
    f_icode == RRMOVQ : i10bytes[12..16];
    f_icode == IRMOVQ : i10bytes[12..16];
    f_icode == OPQ : i10bytes[12..16];
    f_icode == CMOVXX : i10bytes[12..16];
    1 : REG_NONE;
];

# Register B
f_rB = [
    f_icode == RRMOVQ : i10bytes[8..12];
    f_icode == IRMOVQ : i10bytes[8..12];
    f_icode == OPQ : i10bytes[8..12];
    f_icode == CMOVXX : i10bytes[8..12];
    1 : REG_NONE;
];

# Val C
f_valC = [
	f_icode == IRMOVQ : i10bytes[16..80];
	1 : 0;
];

wire valP:64;

# Handles pc change
valP = [
	f_icode == HALT : pc + 1;
    f_icode == NOP : pc + 1;
	f_icode == RRMOVQ : pc + 2;
    f_icode == OPQ : pc + 2;
    f_icode == CMOVXX : pc + 2;
	f_icode == IRMOVQ : pc + 10;
	1 : pc + 9;
];

p_pc = valP;

# Handles Stat
f_Stat = [
    f_icode == HALT : STAT_HLT;
    f_icode == NOP : STAT_AOK;
    f_icode == RRMOVQ : STAT_AOK;
    f_icode == IRMOVQ : STAT_AOK;
    f_icode == OPQ : STAT_AOK;
    f_icode == CMOVXX : STAT_AOK;
    1 : STAT_INS;
];

# Stall through remaining stages if STAT is not AOK
stall_F = [
    f_Stat != STAT_AOK: 1;
    1 : 0
]; 

######### Fetch output and decode input register #############
register fD {
    icode : 4 = 0;
    ifun : 4 = 0;
    rA : 4 = REG_NONE;
    rB : 4 = REG_NONE;
    valC : 64 = 0;
    Stat : 3 = STAT_AOK;
}

########## Decode #############
d_icode = D_icode;
d_ifun = D_ifun;
d_valC = D_valC;
d_Stat = D_Stat;

# What is the source for register A?
reg_srcA = [
	D_icode == RRMOVQ : D_rA;
    D_icode == OPQ : D_rA;
    D_icode == CMOVXX : D_rA;
	1 : REG_NONE;
];

d_srcA = reg_srcA;

# What is the source for register B?
reg_srcB = [
	D_icode == OPQ : D_rB;
	1 : REG_NONE;
];

d_srcB = reg_srcB;

# Current register A value
d_valA = [
    # Forwarding to execute stage
    reg_srcA == e_dstE : e_valE;
    
    # Forwarding to memory stage
    reg_srcA == m_dstE : m_valE;

    # Forwarding to writeback stage
    reg_srcA == reg_dstE : reg_inputE;

    # None
    1 : reg_outputA;
];

# Current register B value
d_valB = [
    # Forwarding to execute stage
    reg_srcB == e_dstE : e_valE;

    # Forwarding to memory stage
    reg_srcB == m_dstE : m_valE;

    # Forwarding to writeback stage
    reg_srcB == reg_dstE : reg_inputE;

    # None
    1 : reg_outputB;
];

# Destination register
d_dstE = [
    D_icode == RRMOVQ : D_rB;
    D_icode == IRMOVQ : D_rB;
    D_icode == OPQ : D_rB;
    D_icode == CMOVXX : D_rB;
    1 : REG_NONE;

];

######### Decode output and encode input register #############
register dE {
    icode : 4 = 0;
    ifun : 4 = 0;
    valA : 64 = 0;
    valB : 64 = 0;
    valC : 64 = 0;
    dstE : 4 = REG_NONE;
    srcA : 4 = REG_NONE;
    srcB : 4 = REG_NONE;
    Stat : 3 = STAT_AOK;
}

########## Execute #############
e_icode = E_icode;
e_valA = E_valA;
e_valB = E_valB;
e_valC = E_valC;

# ALU
e_valE = [
    # Addition
    (E_icode == OPQ)  && (E_ifun == ADDQ) : E_valA + E_valB;
    # SUbtraction
    (E_icode == OPQ)  && (E_ifun == SUBQ) : E_valB - E_valA;
    # AND
    (E_icode == OPQ)  && (E_ifun == ANDQ) : E_valA & E_valB;
    # XOR
    (E_icode == OPQ)  && (E_ifun == XORQ) : E_valA ^ E_valB;
    # Val C
    E_icode == IRMOVQ : E_valC;
    # Val A
    E_icode == RRMOVQ : E_valA;
    1 : 0;
];

wire conditionsMet: 1;

c_ZF = (e_valE == 0);
c_SF = (e_valE >= 0x8000000000000000);
stall_C = (E_icode != OPQ);

# Checks condition flags
conditionsMet = [
    E_ifun == 0   : 1;
    E_ifun == 1   : (C_SF || C_ZF);
    E_ifun == 2   : (C_SF & !C_ZF);
    E_ifun == 3   : C_ZF;
    E_ifun == 4   : !C_ZF;
    E_ifun == 5   : (!C_SF || C_ZF);
    E_ifun == 6   : (!C_SF & !C_ZF);  
    1           : 0;
];

e_srcA = E_srcA;
e_srcB = E_srcB;

# Determines if destination register when there is CMOVXX
e_dstE = [
    (E_icode == CMOVXX) && (!conditionsMet): REG_NONE;
    1 : E_dstE;
];

e_Stat = E_Stat;

######### Encode output and memory input register #############
register eM {
    icode : 4 = 0;
    valA : 64 = 0;
    valB : 64 = 0;
    valC : 64 = 0;
    valE : 64 = 0;
    dstE : 4 = REG_NONE;
    srcA : 4 = REG_NONE;
    srcB : 4 = REG_NONE;
    Stat : 3 = STAT_AOK;
}

########## Memory #############
m_icode = M_icode;
m_valA = M_valA;
m_valB = M_valB;
m_valC = M_valC;
m_valE = M_valE;
m_dstE = M_dstE;
m_Stat = M_Stat;

######### Memory output and writeback input register #############

# Nothing happens here right now

register mW {
    icode:4 = 0;
    valA : 64 = 0;
    valB : 64 = 0;
    valC : 64 = 0;
    valE : 64 = 0;
    dstE : 4 = REG_NONE;
    Stat : 3 = STAT_AOK;
}

########## Writeback #############

# Determines what to write into the destination register
reg_inputE = [
	W_icode == RRMOVQ : W_valA;
	W_icode == IRMOVQ : W_valC;
    W_icode == OPQ : W_valE;
    1 : 0 ;
];

reg_dstE = W_dstE;

########## PC and Status updates #############
Stat = W_Stat;