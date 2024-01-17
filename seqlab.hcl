# An example file in our custom HCL variant, with lots of comments

register pP {  
    # our own internal register. P_pc is its output, p_pc is its input.
	pc:64 = 0; # 64-bits wide; 0 is its default value.
	
	# we could add other registers to the P register bank
	# register bank should be a lower-case letter and an upper-case letter, in that order.
	
	# there are also two other signals we can optionally use:
	# "bubble_P = true" resets every register in P to its default value
	# "stall_P = true" causes P_pc not to change, ignoring p_pc's value
} 

# "pc" is a pre-defined input to the instruction memory and is the 
# address to fetch 6 bytes from (into pre-defined output "i10bytes").
pc = P_pc;

# we can define our own input/output "wires" of any number of 0<bits<=80
wire opcode:8, icode:4, valC:64, rB:4, rA:4, valE:64, ifun:4, conditionsMet:1;

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

# some named constants are built-in: the icodes, ifuns, STAT_??? and REG_???


# Stat is a built-in output; STAT_HLT means "stop", STAT_AOK means 
# "continue".  The following uses the mux syntax described in the 
# textbook
Stat = [
	icode == HALT : STAT_HLT;
    ( icode > 11 ) : STAT_INS;
	1             : STAT_AOK;
];

# to make progress, we have to update the PC...

p_pc = [    
    icode == HALT : P_pc + 1;
    icode == CALL : P_pc + 9;
    icode == JXX  : valC;
    icode == RRMOVQ : P_pc + 2;
    icode == IRMOVQ : P_pc + 10;
    icode == RMMOVQ : P_pc + 10;
    icode == MRMOVQ : P_pc + 10;
    icode == OPQ : P_pc + 2;
    icode == RET  : P_pc + 1;
    icode == NOP : P_pc + 1;
    icode == CMOVXX : P_pc + 2;
    icode == PUSHQ : P_pc + 2;
    icode == POPQ : P_pc + 2;
    1             : P_pc + 1;
];

valC = [
    (icode == IRMOVQ || icode == RMMOVQ) : i10bytes[16..80];
    icode == JXX : i10bytes[8..72];
    1 : 0xBADBADBAD;
];

rA = [
    ( icode == OPQ || icode == CMOVXX || icode == RRMOVQ || icode == RMMOVQ) : i10bytes[12..16];
    1 : 0xBADBADBAD;
];

rB = [
    (icode == RRMOVQ || icode == OPQ || icode == IRMOVQ || icode == CMOVXX || icode == RMMOVQ) : i10bytes[8..12];
    1 : 0xBADBADBAD;
];

reg_dstE = [
    !conditionsMet && icode == CMOVXX: 0xF;
    (icode == IRMOVQ || icode == RRMOVQ || icode == OPQ || icode == CMOVXX) : rB;
    1 : 0xF;
];

reg_srcA = rA;
reg_srcB = rB;

reg_inputE = [
    icode == OPQ : valE;
    icode == IRMOVQ : valC;
    (icode == CMOVXX) : reg_outputA;
    1 : 0xF;
];

valE = [
    icode == OPQ && ifun == ANDQ : reg_outputA & reg_outputB;
    icode == OPQ && ifun == ADDQ : reg_outputA + reg_outputB;
    icode == OPQ && ifun == XORQ : reg_outputA ^ reg_outputB;
    icode == OPQ && ifun == SUBQ : reg_outputB - reg_outputA;
    
    
    icode == RMMOVQ : reg_outputB + valC;
    
    1 : 0xBADBADBAD;
];

register cC {
    ZF:1 = 1;
    SF:1 = 0;
    
}

stall_C = (icode != OPQ);
c_SF = (valE >= 0x8000000000000000);
c_ZF = (valE == 0);


conditionsMet = [
    ifun == ALWAYS : 1;
    ifun == EQ : C_ZF;
    ifun == LE : C_SF || C_ZF;
    ifun == GT : !(C_SF || C_ZF);
    ifun == NE : !C_ZF;
    ifun == LT : C_SF;
    ifun == GE : !(C_SF);
    1 : 0xBADBADBAD;
];

mem_readbit = !(icode == RMMOVQ);
mem_writebit = (icode == RMMOVQ);
mem_input = reg_outputA;
mem_addr = valE;
