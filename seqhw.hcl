register pP {  
    # our own internal register. P_pc is its output, p_pc is its input.
	pc:64 = 0; # 64-bits wide; 0 is its default value.
	
	# we could add other registers to the P register bank
	# register bank should be a lower-case letter and an upper-case letter, in that order.
	
	# there are also two other signals we can optionally use:
	# "bubble_P = true" resets every register in P to its default value
	# "stall_P = true" causes P_pc not to change, ignoring p_pc's value
} 

register cC {
    SF:1 = 0;
    ZF:1 = 1;
}

# "pc" is a pre-defined input to the instruction memory and is the 
# address to fetch 10 bytes from (into pre-defined output "i10bytes").
pc = P_pc;

# we can define our own input/output "wires" of any number of 0<bits<=80
wire opcode:8, icode:4, rA:4, rB:4, valC:64, valE: 64, ifun: 4, conditionsMet: 1;

# the x[i..j] means "just the bits between i and j".  x[0..1] is the 
# low-order bit, similar to what the c code "x&1" does; "x&7" is x[0..3]
opcode = i10bytes[0..8];   # first byte read from instruction memory
icode = opcode[4..8];      # top nibble of that byte
ifun = opcode[0..4]; 

/* we could also have done i10bytes[4..8] directly, but I wanted to
 * demonstrate more bit slicing... and all 3 kinds of comments      */
// this is the third kind of comment

# named constants can help make code readable
const TOO_BIG = 0xC; # the first unused icode in Y86-64

# Stat is a built-in output; STAT_HLT means "stop", STAT_AOK means 
# "continue".  The following uses the mux syntax described in the 
# textbook
Stat = [
    icode == HALT    : STAT_HLT;
    icode > 11       : STAT_INS;
	1                : STAT_AOK;
];

p_pc = [
    icode == HALT                       : P_pc + 1;
    icode == NOP                        : P_pc + 1;
    icode == RRMOVQ                     : P_pc + 2;
    icode == OPQ                        : P_pc + 2;
    icode == PUSHQ                      : P_pc + 2;
    icode == POPQ                       : P_pc + 2;
    icode == CMOVXX                     : P_pc + 2;
    (icode == JXX) && conditionsMet     : valC;
    (icode == JXX) && !conditionsMet    : P_pc + 9;
    icode == IRMOVQ                     : P_pc + 10;
    icode == MRMOVQ                     : P_pc + 10;
    icode == RMMOVQ                     : P_pc + 10;
    icode == CALL                       : valC;
    icode == RET                        : mem_output;
    1                                   : 9;
];

rA = [
    icode == RRMOVQ     : i10bytes[12..16];
    icode == OPQ        : i10bytes[12..16];
    icode == CMOVXX     : i10bytes[12..16];
    icode == RMMOVQ     : i10bytes[12..16];
    icode == MRMOVQ     : i10bytes[12..16];
    icode == PUSHQ      : i10bytes[12..16];
    icode == POPQ       : i10bytes[12..16];
    icode == CALL       : REG_RSP;
    icode == RET        : REG_RSP;
    1                   : REG_NONE;
];

rB = [
    icode == IRMOVQ     : i10bytes[8..12];
    icode == RRMOVQ     : i10bytes[8..12];
    icode == OPQ        : i10bytes[8..12];
    icode == CMOVXX     : i10bytes[8..12];
    icode == RMMOVQ     : i10bytes[8..12];
    icode == MRMOVQ     : i10bytes[8..12];
    icode == PUSHQ      : REG_RSP;
    icode == POPQ       : REG_RSP;
    1                   : REG_NONE;
];

reg_srcA = rA;

reg_srcB = rB;

valC = [
    icode == IRMOVQ : i10bytes[16..80];
    icode == RMMOVQ : i10bytes[16..80];
    icode == MRMOVQ : i10bytes[16..80];
    icode == JXX    : i10bytes[8..72];
    icode == CALL   : i10bytes[8..72];
    1 : 0;           
];

reg_inputE = [
    icode == IRMOVQ                     : valC;
    icode == RRMOVQ                     : reg_outputA;
    icode == MRMOVQ                     : mem_output;
    icode == OPQ                        : valE;
    icode == PUSHQ                      : valE;
    icode == POPQ                       : mem_output;
    icode == CALL                       : valE;
    icode == RET                        : valE;
    1: 0;
];

reg_inputM = [
    icode == POPQ                       : valE;
    1 : 0;
];

reg_dstE = [
    !conditionsMet && (icode == CMOVXX) : REG_NONE;
    icode == IRMOVQ                     : rB;
    icode == RRMOVQ                     : rB;
    icode == OPQ                        : rB;
    icode == MRMOVQ                     : rA;
    icode == PUSHQ                      : rB;
    icode == POPQ                       : rA;
    icode == CALL                       : rA;
    icode == RET                        : rA;
    1                                   : REG_NONE;
];

reg_dstM = [
    icode == POPQ                       : rB;
    1 : REG_NONE;
];

valE = [
    icode == OPQ && ifun == ADDQ : reg_outputA + reg_outputB;
    icode == OPQ && ifun == SUBQ : reg_outputB - reg_outputA;
    icode == OPQ && ifun == ANDQ : reg_outputA & reg_outputB;
    icode == OPQ && ifun == XORQ : reg_outputA ^ reg_outputB;
    icode == RMMOVQ              : reg_outputB + valC;
    icode == MRMOVQ              : reg_outputB + valC;
    icode == PUSHQ               : reg_outputB - 8;
    icode == POPQ                : reg_outputB + 8;
    icode == CALL                : reg_outputA - 8;
    icode == RET                 : reg_outputA + 8;
    1                            : 0;
];

mem_readbit = [
    icode == RMMOVQ : 0;
    icode == MRMOVQ : 1;
    icode == PUSHQ  : 0;
    icode == POPQ   : 1;
    icode == CALL   : 0;
    icode == RET    : 1;
    1 : 0;
];

mem_writebit = [
   icode == RMMOVQ : 1;
   icode == MRMOVQ : 0;
   icode == PUSHQ  : 1;
   icode == POPQ   : 0;
   icode == CALL   : 1;
   icode == RET    : 0;
   1 : 0;
];

mem_addr = [
    icode == RMMOVQ : valE;
    icode == MRMOVQ : valE;
    icode == PUSHQ  : valE;
    icode == POPQ   : reg_outputB;
    icode == CALL   : valE;
    icode == RET    : reg_outputA;
    1 : 0;
];

mem_input = [
    icode == RMMOVQ : reg_outputA;
    icode == PUSHQ  : reg_outputA;
    icode == CALL   : P_pc + 9;
    1 : 0;
];

c_ZF = (valE == 0);
c_SF = (valE >= 0x8000000000000000);
stall_C = (icode != OPQ);

conditionsMet = [
    ifun == 0   : 1;
    ifun == 1   : (C_SF || C_ZF);
    ifun == 2   : (C_SF & !C_ZF);
    ifun == 3   : C_ZF;
    ifun == 4   : !C_ZF;
    ifun == 5   : (!C_SF || C_ZF);
    ifun == 6   : (!C_SF & !C_ZF);  
    1           : 0;
];

