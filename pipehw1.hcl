########## the PC and condition codes registers #############
register fF { pc:64 = 0; }

register fD {
    valC:64 = 0;
    ifun:4 = 0;
    icode:4 = 0;
    Stat:3 = 0;
    rA:4 = REG_NONE;
    rB:4 = REG_NONE;
    
}

register dE {
    icode:4 = 0; 
    ifun:4 = 0;
    valC:64 = 0;
    Stat:3 = 0;
    dstE:4 = REG_NONE;
    rB:4 = REG_NONE; #initial
    rA:4 = REG_NONE; #initial
    rvalA:64 = 0; # value in register rA
    rvalB:64 = 0; # value in register rB
    dstM:4 = REG_NONE;
}

register eM {
    icode:4 = 0;
    ifun:4 = 0;
    valC:64 = 0; 
    Stat:3 = 0;
    dstE:4 = REG_NONE;
    conditionsMet:1 = 0;
    dstM:4 = REG_NONE;
    rvalA:64 = 0; # the value in register rA
    rvalB:64 = 0; # the value in register rB
    valE:64 = 0;
}

register mW {
    icode:4 = 0;
    ifun:4 = 0;
    valC:64 = 0;
    Stat:3 = 0;
    dstE:4 = REG_NONE;
    conditionsMet:1 = 0;
    dstM:4 = REG_NONE;
    valM:64=0;
    rvalA:64 = 0; # the value in register rA
    rvalB:64 = 0; # the value in register rB
    valE:64 = 0;
}

register cC {
    ZF:1 = 1;
    SF:1 = 0;
}

########## Fetch #############
pc = F_pc;


f_icode = i10bytes[4..8];
f_ifun = i10bytes[0..4];
f_rA = i10bytes[12..16];
f_rB = i10bytes[8..12];
# valC = immediate

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

#valP => update pc with valP

valP = F_pc + offset;

########## Decode #############

d_icode = D_icode;
d_Stat = D_Stat;
d_ifun = D_ifun;
d_rB = D_rB;
d_rA = D_rA;

reg_srcA = [
    d_icode in {RRMOVQ, OPQ, CMOVXX, RMMOVQ} : d_rA;
    1 : REG_NONE;
];

reg_srcB = [
    d_icode in {OPQ, CMOVXX, RMMOVQ, MRMOVQ} : d_rB;
    1 : REG_NONE;
];

d_dstE = [
    d_icode in {IRMOVQ, RRMOVQ, OPQ, CMOVXX} : d_rB;
    d_icode in {NOP, HALT} : REG_NONE;
    1 : REG_NONE;
];

d_dstM = [
	D_icode in { MRMOVQ } : D_rA;
	1 : REG_NONE;
];

# mux for forwarding rvalA
d_rvalA = [
	reg_srcA == REG_NONE: 0;
    (m_icode == OPQ && reg_srcA == m_dstE) : m_valE;
    (e_icode == OPQ && reg_srcA == e_dstE) : e_valE;
    (W_icode == OPQ && reg_srcA == W_dstE) : W_valE;
	reg_srcA == m_dstM : m_valM; # forward post-memory
	reg_srcA == W_dstM : W_valM; # forward pre-writeback
    reg_srcA == e_dstE: e_valC;
    reg_srcA == m_dstE: m_valC;
    reg_srcA == W_dstE: W_valC;
    1 : reg_outputA;
];

# mux for forwarding rvalB
d_rvalB = [
	reg_srcB == REG_NONE: 0;
    (m_icode == OPQ && reg_srcB == m_dstE) : m_valE;
    (e_icode == OPQ && reg_srcB == e_dstE) : e_valE;
    (W_icode == OPQ && reg_srcB == W_dstE) : W_valE;
	reg_srcB == m_dstM : m_valM; # forward post-memory
	reg_srcB == W_dstM : W_valM; # forward pre-writeback
    reg_srcB == e_dstE: e_valC;
    reg_srcB == m_dstE: m_valC;
    reg_srcB == W_dstE: W_valC;
    1 : reg_outputB;
];

d_valC = [
    (reg_srcA == m_dstE && m_dstE != 0xf): m_valC;
    1 : D_valC;
];

########## Execute #############

e_icode = E_icode;
e_valC = E_valC;
e_Stat = E_Stat;
e_dstE = [
    (e_icode == CMOVXX && !e_conditionsMet) : REG_NONE;
    1 : E_dstE;
];
e_ifun = E_ifun;
e_rvalA = E_rvalA;
e_rvalB = E_rvalB;
e_dstM = E_dstM;

e_valE = [ // "ALU"
    e_icode == OPQ && e_ifun == XORQ : e_rvalA ^ e_rvalB;
    e_icode == OPQ && e_ifun == SUBQ : e_rvalB - e_rvalA;
    e_icode == OPQ && e_ifun == ANDQ : e_rvalA & e_rvalB;
    e_icode == OPQ && e_ifun == ADDQ : e_rvalA + e_rvalB;
	e_icode in { RMMOVQ, MRMOVQ } : E_valC + E_rvalB;
    1 : 0xBADBADBAD;
];

e_conditionsMet = [
    e_ifun == ALWAYS : 1;
    e_ifun == LE : C_SF || C_ZF;
    e_ifun == LT : C_SF;
    e_ifun == EQ : C_ZF;
    e_ifun == NE : !C_ZF;
    e_ifun == GE : !(C_SF);
    e_ifun == GT : !(C_SF || C_ZF);
    1 : 0xBADBADBAD;
];

stall_C = (e_icode != OPQ);
c_ZF = (e_valE == 0);
c_SF = (e_valE >= 0x8000000000000000);

########## Memory #############

m_icode = M_icode;
m_valC = M_valC;
m_Stat = M_Stat;
m_dstE = M_dstE;
m_ifun = M_ifun;
m_rvalA = M_rvalA;
m_rvalB = M_rvalB;
m_valE = M_valE;
m_conditionsMet = M_conditionsMet;
m_dstM = M_dstM;

mem_addr = [ # output to memory system
	M_icode in { RMMOVQ, MRMOVQ } : M_valE;
	1 : 0; # Other instructions don't need address
];
mem_readbit =  M_icode in { MRMOVQ }; # output to memory system
mem_writebit = M_icode in { RMMOVQ }; # output to memory system
mem_input = M_rvalA;

m_valM = mem_output; # input from mem_readbit and mem_addr


########## Writeback #############

# destination selection
reg_dstE = W_dstE;

# rvalA = value in rA

reg_inputE = [ # unlike book, we handle the "forwarding" actions (something + 0) here
	W_icode in {IRMOVQ} : W_valC;
    W_icode in {RRMOVQ} : W_rvalA;
    W_icode in {OPQ} : W_valE;
    (W_icode in {CMOVXX} && W_conditionsMet) : W_valC;
    1: 0xBADBADBAD;
];

reg_inputM = W_valM; # output: sent to register file
reg_dstM = W_dstM; # output: sent to register file

Stat = W_Stat; 
# set Stat in the fetch stage, then pass it through the registers and set the final Stat value to the value after being passed through the registers

########## PC and Status updates #############

f_Stat = [
	f_icode == HALT : STAT_HLT;
	f_icode > 0xb : STAT_INS;
	1 : STAT_AOK;
];

f_pc = [
    f_Stat != STAT_AOK: pc;
    1 : valP;
];

################ Pipeline Register Control #########################

wire loadUse:1;

loadUse = (E_icode in {MRMOVQ}) && (E_dstM in {reg_srcA, reg_srcB}); 

### Fetch
stall_F = loadUse || f_Stat != STAT_AOK;

### Decode
stall_D = loadUse;

### Execute
bubble_E = loadUse;

### Memory

### Writeback