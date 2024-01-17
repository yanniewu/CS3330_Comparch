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
    icode == CALL : valC;
    icode == JXX  : valC;
    icode == RET  : mem_output;
    icode == NOP : P_pc + 1;
    icode == RRMOVQ : P_pc + 2;
    icode == IRMOVQ : P_pc + 10;
    icode == RMMOVQ : P_pc + 10;
    icode == MRMOVQ : P_pc + 10;
    icode == OPQ : P_pc + 2;
    icode == CMOVXX : P_pc + 2;
    icode == PUSHQ : P_pc + 2;
    icode == POPQ : P_pc + 2;
    1             : P_pc + 1;
];

valC = [
    (icode == IRMOVQ || icode == RMMOVQ || icode == MRMOVQ) : i10bytes[16..80];
    
    icode == JXX && conditionsMet : i10bytes[8..72]; 
    
    icode == CALL : i10bytes[8..72];
    
    icode == JXX && !conditionsMet: P_pc + 9;
    1 : 0xBADBADBAD;
];

rA = [
    (icode == RRMOVQ || icode == OPQ || icode == CMOVXX || icode == RMMOVQ || icode == MRMOVQ || icode == PUSHQ || icode == POPQ) : i10bytes[12..16];
    (icode == CALL || icode == RET) : REG_RSP;
    1 : 0xBADBADBAD;
];

rB = [
    (icode == IRMOVQ || icode == RRMOVQ || icode == OPQ || icode == CMOVXX || icode == RMMOVQ || icode == MRMOVQ) : i10bytes[8..12];
    (icode == PUSHQ || icode == POPQ) : REG_RSP;
    1 : 0xBADBADBAD;
];

reg_dstE = [
    !conditionsMet && icode == CMOVXX: 0xF;
    (icode == IRMOVQ || icode == RRMOVQ || icode == OPQ || icode == CMOVXX || icode == PUSHQ || icode == POPQ) : rB;
    (icode == MRMOVQ || icode == CALL || icode == RET): rA;
    1 : 0xF;
];

reg_dstM = [
    icode == POPQ : rA;
    1 : 0xF;
];

reg_srcA = rA;
reg_srcB = rB;

reg_inputE = [
    icode == IRMOVQ : valC;
    (icode == CMOVXX) : reg_outputA;
    icode == OPQ : valE;
    icode == MRMOVQ: mem_output;
    icode == PUSHQ: valE;
    icode == POPQ: valE;
    icode == CALL : valE;
    icode == RET : valE;
    1 : 0xF;
];

reg_inputM = [
    icode == POPQ: mem_output;
    1 : 0xF;
];

valE = [ // "ALU"
    icode == OPQ && ifun == XORQ : reg_outputA ^ reg_outputB;
    icode == OPQ && ifun == SUBQ : reg_outputB - reg_outputA;
    icode == OPQ && ifun == ANDQ : reg_outputA & reg_outputB;
    icode == OPQ && ifun == ADDQ : reg_outputA + reg_outputB;
    
    icode == RMMOVQ || icode == MRMOVQ : reg_outputB + valC;
    
    (icode == PUSHQ) : reg_outputB - 8;
    icode == CALL : reg_outputA - 8;
    icode == POPQ : reg_outputB + 8;
    icode == RET : reg_outputA + 8;
        
    1 : 0xBADBADBAD;
];

register cC {
    SF:1 = 0;
    ZF:1 = 1;
}

stall_C = (icode != OPQ);
c_ZF = (valE == 0);
c_SF = (valE >= 0x8000000000000000);

conditionsMet = [
    ifun == ALWAYS : 1;
    ifun == LE : C_SF || C_ZF;
    ifun == LT : C_SF;
    ifun == EQ : C_ZF;
    ifun == NE : !C_ZF;
    ifun == GE : !(C_SF);
    ifun == GT : !(C_SF || C_ZF);
    1 : 0xBADBADBAD;
];

mem_readbit = [
    (icode == RMMOVQ || icode == PUSHQ || icode == CALL): 0;
    (icode == MRMOVQ || icode == POPQ || icode == RET): 1;
    
    1 : 0;
];

mem_writebit = [
    (icode == RMMOVQ || icode == PUSHQ || icode == CALL): 1;
    (icode == MRMOVQ || icode == POPQ || icode == RET): 0;
    
    1 : 0;
];

mem_input = [
    (icode == RMMOVQ || icode == PUSHQ || icode == MRMOVQ || icode == POPQ): reg_outputA;
    icode == CALL : P_pc + 9;
    //icode == RET : P_pc + 1;
    1: 0xF;
];
mem_addr = [
    (icode == RMMOVQ || icode == MRMOVQ || icode == PUSHQ || icode == CALL) : valE;
    icode == POPQ : reg_outputB;
    icode == RET : reg_outputA; // value read from pre-added %rsp address is in mem_output
    1 : 0xF;
];
