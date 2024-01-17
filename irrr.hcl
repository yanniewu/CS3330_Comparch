register pP {  
    # our own internal register. P_pc is its output, p_pc is its input.
	pc:64 = 0; # 64-bits wide; 0 is its default value.
	
	# we could add other registers to the P register bank
	# register bank should be a lower-case letter and an upper-case letter, in that order.
	
	# there are also two other signals we can optionally use:
	# "bubble_P = true" resets every register in P to its default value
	# "stall_P = true" causes P_pc not to change, ignoring p_pc's value
} 

pc = P_pc;

wire opcode:8, icode:4;
opcode = i10bytes[0..8];   # first byte read from instruction memory
icode = opcode[4..8];      # top nibble of that byte


Stat = [
	icode == HALT        : STAT_HLT;
	icode > 11			 : STAT_INS;
	1                    : STAT_AOK;
];



p_pc = [    
    icode == HALT : P_pc + 1;
    icode == NOP : P_pc + 1;
    icode == CMOVXX : P_pc + 2;
    icode == IRMOVQ : P_pc + 10;
    icode == RMMOVQ : P_pc + 10;
    icode == MRMOVQ : P_pc + 10;
    icode == RRMOVQ : P_pc + 2;
    icode == OPQ : P_pc + 2;
    icode == JXX  : valC;
    icode == CALL : P_pc + 9;
    icode == RET  : P_pc + 1;
    icode == PUSHQ : P_pc + 2;
    icode == POPQ : P_pc + 2;
    1             : P_pc + 1;
];


# IMPLEMENTATION OF HW
wire rA:4, rB:4, valC:64;

rA = [
	icode == RRMOVQ : i10bytes[12..16];
	1 				: 0;
];


rB = [
	(icode == IRMOVQ || icode == RRMOVQ) : i10bytes[8..12];
	1 									 : 0;
];


valC = [
	icode == IRMOVQ  	: i10bytes[16..80];
	icode == JXX    	: i10bytes[8..72];
	1 				 	: 0;
];

reg_dstM = [
	(icode == IRMOVQ || icode == RRMOVQ)    : rB;
	1            						    : REG_NONE ;
];

reg_srcA = rA;

reg_inputM = [
	icode == IRMOVQ    : valC;  
	icode == RRMOVQ    : reg_outputA;
	1                  : 0xF;
];

