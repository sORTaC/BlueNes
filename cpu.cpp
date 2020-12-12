#include "cpu.h"

cpu6502::cpu6502()
{

}

cpu6502::cpu6502(mem6502 mem_)
{
    cpumem = mem_;
}

//debugging
void cpu6502::state_dump()
{
    printf("<======================================SYSTEM STATE=====================================>\n");
    //printf("CLOCK -------------->\n\n\
    //    Cycles Executed So Far: %d\n\n", total_cycles);
    printf("--------REGISTERS-------\n|\
        A: 0x%X        \n\n|\
        X: 0x%X        \n\n|\
        Y: 0x%X         \n", A, X, Y);
    // printf("|")
    printf("------------------------\n");
    printf("---------STATUS---------\n|   N Z O C B I D _          \n\n|   %d %d %d %d %d %d %d %d         \n\n|Program Counter: 0x%X          \n\n| Stack Pointer:  0x%X          \n",
        NEG_RES,
        ZER_RES,
        OVR_RES,
        CAR_RES,
        BRK_RES,
        INT_RES,
        DEC_RES,
        _RES,
        pc,
        sp
    );
    printf("------------------------\n");
}

void cpu6502::mem_dump(uint16_t from, uint16_t to, uint16_t _offset)
{
    int n = 0;
    int offset;
    if (to != 0)
        offset = to - from;
    else
        offset = _offset;
    to = from + offset;
    if (offset != 0)
    {
        if (to != 0)
            printf("<=================================MEMORY[0x%X: 0x%X]==================================>\n\n", from, to);
        else
            printf("<=================================MEMORY[0x%X: 0x%X]==================================>\n\n", from, from + offset);
        printf("           0x0       0x1       0x2       0x3       0x4       0x5       0x6       0x7\n");
        printf("0x%X ", from);
        for (int i = 0; i < offset; i += 8)
        {
            printf("||");
            for (int j = 0; j < 8; j++)
            {
                printf("  0x%X  ", read(from + j + i));
                printf("||");
            }
            printf("\n");
            printf("      -----------------------------------------------------------------------------------\n");
            int sum = from + 8 + n * 8;
            if (sum != to)
                printf("0x%x:", from + 8 + n * 8);
            n++;
        }
    }
    else
        printf("Error: ending value can't be equal to start value!");
}

//
void cpu6502::write(uint16_t addr, uint8_t data)
{
    //at addr in ram, write data
    cpumem[addr] = data;

    //mirror memory
    if (addr >= 0x000 && addr <= 0x07FF)
    {
        write(addr + 0x0800, data);
        write(addr + 0x1000, data);
        write(addr + 0x1800, data);
    }

    //memory ppu registers
    if (addr >= 0x2000 && addr <= 0x2007)
    {
        for (uint16_t i = 0x00; i < 0x1ff8; i += 8)
        {
            cpumem[addr + i] = data;
        }
    }

    //APU and IO region
    if (addr >= 0x4000 && addr <= 0x401F)
    {
        printf("\nAPU and IO region\n");
    }
}

uint16_t cpu6502::read(uint16_t addr)
{
    //from addr in ram, read data
    return cpumem[addr];
}

//addressing mode
uint16_t cpu6502::accumulator()
{
    return 0;
}

uint16_t cpu6502::immediate()
{
    return pc++;
}

uint16_t cpu6502::zeropage()
{
    return read(pc++);
}

uint16_t cpu6502::zeropage_x()
{
    uint16_t addr = read(pc++) + (uint16_t)X;
    return (addr & 0xff);
}

uint16_t cpu6502::zeropage_y()
{
    uint16_t addr = read(pc++) + (uint16_t)Y;
    return (addr & 0xff);
}

uint16_t cpu6502::implied()
{
    return 0;
}

uint16_t cpu6502::absolute()
{
    uint16_t final = read(pc++) | (read(pc++) << 8);
    return final;
}

uint16_t cpu6502::absolute_x()
{
    uint16_t addr = (read(pc++) | (read(pc++) << 8));
    addr += (uint16_t)X;
    return addr;
}

uint16_t cpu6502::absolute_y()
{
    uint16_t addr = (read(pc++) | (read(pc++) << 8));
    addr += (uint16_t)Y;
    return addr;
}

uint16_t cpu6502::indirect()
{
    uint16_t addr1 = read(pc++);
    uint16_t addr2 = read(pc++);
    uint16_t addr3 = (addr1)|(addr2 << 8);
    //jmp bug
    uint16_t addr4 = read((addr3 & 0xFF00) | ((addr3 + 1) & 0x00FF));
    uint16_t final = read(addr3) | 0x100 * addr4;
    return final;
}

uint16_t cpu6502::indirect_x()
{
    uint16_t addr = (read(pc++) + (uint16_t)X) % 256;
    uint16_t addr2 = (addr + 1) % 256;
    uint16_t addr3 = (read(addr)) | (read(addr2) << 8);
    return addr3;
}

uint16_t cpu6502::indirect_y()
{
    uint16_t addr = read(pc++);
    uint16_t addr1 = (addr + 1) % 256;
    uint16_t addr2 = read(addr) + (read(addr1) << 8) + Y;
    return addr2;
}

uint16_t cpu6502::relative()
{
    uint16_t offset = read(pc++);
    if (offset & 0x80) 
        offset |= 0xFF00;
    return offset;
}
//

void cpu6502::pushtostack(uint8_t data)
{
    write(BASESTACK + sp, data);
    if (sp == 0x00) sp = 0xFF;
    else sp--;
}

uint8_t cpu6502::pullfromstack()
{
    if (sp == 0xFF) sp = 0x00;
    else sp++;
    return (uint8_t)read(BASESTACK + sp);
}

//opcodes

//Load instructions
void cpu6502::OP_LDA(uint16_t addr) {

    A = (uint8_t)(read(addr) & 0x00FF);
    set_zer(!A);
    set_neg(A & 0x80);
}

void cpu6502::OP_LDY(uint16_t addr) {
    Y = (uint8_t)(read(addr) & 0x00FF);
    set_zer(!Y);
    set_neg(Y & 0x80);
}

void cpu6502::OP_LDX(uint16_t addr) {
    X = (uint8_t)(read(addr) & 0x00FF);
    set_zer(!X);
    set_neg(X & 0x80);
}

//Stack operations
void cpu6502::OP_STA(uint16_t addr) {
    write(addr, A);
}

void cpu6502::OP_STY(uint16_t addr) {
    write(addr, Y);
}

void cpu6502::OP_STX(uint16_t addr) {
    write(addr, X);
}

//Flag operations

void cpu6502::OP_CLC(uint16_t) {
    set_car(0);
}

void cpu6502::OP_CLD(uint16_t) {
    set_dec(0);
}

void cpu6502::OP_CLI(uint16_t) {
    set_int(0);
}

void cpu6502::OP_CLV(uint16_t) {
    set_ovr(0);
}

void cpu6502::OP_SEI(uint16_t) {
    set_int(1);
}

void cpu6502::OP_SED(uint16_t) {
    set_dec(1);
}

void cpu6502::OP_SEC(uint16_t) {
    set_car(1);
}

//Subroutines

void cpu6502::OP_JSR(uint16_t addr)
{
    pc--;
    write(BASESTACK + sp, (pc >> 8));
    write(BASESTACK + (sp - 1), pc & 0xFF);
    sp -= 2;
    pc = addr;
}

void cpu6502::OP_RTS(uint16_t addr)
{
    uint16_t info1 = pullfromstack();
    uint16_t info2 = pullfromstack() << 8;
    pc = (info1 | info2) + 1;
}
//Transfer operations

void cpu6502::OP_TXA(uint16_t addr)
{
    A = X;
    set_neg(A & NEG);
    set_zer(!A);
}

void cpu6502::OP_TAX(uint16_t addr)
{
    X = A;
    set_neg(X & NEG);
    set_zer(!X);
}

void cpu6502::OP_TAY(uint16_t addr)
{
    Y = A;
    set_neg(Y & NEG);
    set_zer(!Y);
}

void cpu6502::OP_TSX(uint16_t addr)
{
    X = sp;
    set_neg(X & 0x80);
    set_zer(!X);
}

void cpu6502::OP_TXS(uint16_t addr)
{
    sp = X;
}

void cpu6502::OP_TYA(uint16_t addr)
{
    A = Y;
    set_neg(Y & NEG);
    set_zer(!Y);
}

//NOP

void cpu6502::OP_NOP(uint16_t addr)
{

}

//DECREMENT AND INCREMENT
void cpu6502::OP_DEC(uint16_t addr)
{
    uint8_t result = (read(addr) - 1);
    set_zer(!result);
    set_neg(result & NEG);
    write(addr, result);
}

void cpu6502::OP_DEX(uint16_t addr)
{
    X--;
    set_zer(!X);
    set_neg(X & NEG);
}

void cpu6502::OP_DEY(uint16_t addr)
{
    Y--;
    set_zer(!Y);
    set_neg(Y & NEG);
}


void cpu6502::OP_INX(uint16_t addr)
{
    X++;
    set_zer(!X);
    set_neg(X & NEG);
}

void cpu6502::OP_INY(uint16_t addr)
{
    Y++;
    set_zer(!Y);
    set_neg(Y & NEG);
}

void cpu6502::OP_INC(uint16_t addr)
{
    uint8_t result = (read(addr) + 1) & 0xff;
    set_zer(result == 0);
    set_neg(result & 0x80);
    write(addr, result);
}

//Compare

void cpu6502::OP_CMP(uint16_t addr)
{
    uint16_t M = read(addr);
    set_car(A >= M);
    set_zer(A == M);
    set_neg((A - M) & NEG);
}


void cpu6502::OP_CPX(uint16_t addr)
{
    uint16_t M = read(addr);
    set_car(X >= M);
    set_zer(X == M);
    set_neg((X - M) & NEG);
}

void cpu6502::OP_CPY(uint16_t addr)
{
    uint16_t M = read(addr);
    set_car(Y >= M);
    set_zer(Y == M);
    set_neg((Y - M) & NEG);
}

void cpu6502::OP_JMP(uint16_t addr)
{
    pc = addr;
}

//Branch

void cpu6502::OP_BCC(uint16_t addr)
{
    if (!CAR_RES)
    {
        uint16_t pc_bef = pc;
        pc += addr;
        if ((pc & 0xff00) != (pc_bef & 0xff00))
            page_crossed = true;
    }
    branch_succeeds = true;
}

void cpu6502::OP_BCS(uint16_t addr)
{
    if (CAR_RES)
    {
        uint16_t pc_bef = pc;
        pc += addr;
        if ((pc & 0xff00) != (pc_bef & 0xff00))
            page_crossed = true;
    }
    branch_succeeds = true;
}

void cpu6502::OP_BEQ(uint16_t addr)
{
    if (ZER_RES)
    {
        uint16_t pc_bef = pc;
        pc += addr;
        if ((pc & 0xff00) != (pc_bef & 0xff00))
            page_crossed = true;
    }
    branch_succeeds = true;
}

void cpu6502::OP_BNE(uint16_t addr)
{
    if (!ZER_RES)
    {
        uint16_t oldpc = pc;
        pc = pc + addr;
        if ((oldpc & 0xFF00) != (pc & 0xFF00))
            page_crossed = true;
    }
}

void cpu6502::OP_BMI(uint16_t addr)
{
    if (NEG_RES)
    {
        uint16_t pc_bef = pc;
        pc += addr;
        if ((pc & 0xff00) != (pc_bef & 0xff00))
            page_crossed = true;
    }
    branch_succeeds = true;
}

void cpu6502::OP_BPL(uint16_t addr)
{
    if (!NEG_RES)
    {
        uint16_t pc_bef = pc;
        pc += addr;
        if ((pc & 0xff00) != (pc_bef & 0xff00))
            page_crossed = true;
    }
    branch_succeeds = true;
}

void cpu6502::OP_BVC(uint16_t addr)
{
    if (!OVR_RES)
    {
        uint16_t pc_bef = pc;
        pc += addr;
        if ((pc & 0xff00) != (pc_bef & 0xff00))
            page_crossed = true;
    }
    branch_succeeds = true;
}

void cpu6502::OP_BVS(uint16_t addr)
{
    if (OVR_RES)
    {
        uint16_t pc_bef = pc;
        pc += addr;
        if ((pc & 0xff00) != (pc_bef & 0xff00))
            page_crossed = true;
    }
    branch_succeeds = true;
}

//Arithmetic

void cpu6502::OP_ADC(uint16_t addr)
{
    uint16_t M = read(addr);
    uint16_t res = (uint16_t)A + M + (uint16_t)(status & CARRY);
    set_car(res & 0xFF00);
    set_zer(!(res & 0x00FF));
    set_ovr( (res ^ (uint16_t)A) & ( res ^ M ) & 0x0080 );
    set_neg(res & 0x0080);
    A = res & 0x00FF;
}

void cpu6502::OP_SBC(uint16_t addr)
{
    uint16_t M = read(addr);
    M = M ^ 0x00FF;//take one's complement
    uint16_t res = (uint16_t)A + M + (uint16_t)(status & CARRY);
    set_car(res & 0xFF00);
    set_zer(!(res & 0x00FF));
    set_ovr((res ^ (uint16_t)A) & (res ^ M) & 0x0080);
    set_neg(res & 0x0080);
    A = res & 0x00FF;
}

void cpu6502::OP_AND(uint16_t addr)
{
    uint8_t res = A & (uint8_t)read(addr);
    set_zer(res == 0);
    set_neg(res & 0x80);
    A = res;
}

void cpu6502::OP_EOR(uint16_t addr)
{
    A ^= read(addr);
    set_zer(A == 0);
    set_neg(A & 0x80);
}

void cpu6502::OP_ORA(uint16_t addr)
{
    uint16_t res = A | read(addr);
    set_zer(res == 0);
    set_neg(res & 0x80);
    A = (uint8_t)res;
}

//SHIFTING

void cpu6502::OP_LSR(uint16_t addr, int mode = 1)
{
    if (mode == 0)
    {
        uint8_t M = A;
        set_car(M & 0x1);
        M = M >> 1;
        set_neg(0);
        set_zer(!M);
        A = M;
    }
    else
    {
        uint8_t M = (uint8_t)read(addr);
        set_car(M & 0x1);
        M = M >> 1;
        set_neg(0);
        set_zer(!M);
        write(addr, (uint8_t)M);
    }
}

void cpu6502::OP_ASL(uint16_t addr, int mode = 1)
{
    if (mode == 0)
    {
        uint8_t M = A;
        set_car(M & 0x80);
        M = M << 1;
        M &= 0xFF;
        set_neg(M & 0x80);
        set_zer(!M);
        A = M;
    }
    else
    {
        uint8_t M = (uint8_t)read(addr);
        set_car(M & 0x80);
        M = M << 1;
        M &= 0xFF;
        set_neg(M & 0x80);
        set_zer(!M);
        write(addr, M);
    }
}

void cpu6502::OP_ROL(uint16_t addr, int mode = 1)
{
    if (mode == 0)
    {
        uint8_t flg = A;
        uint8_t M = (A << 1) & 0xfe;
        M |= (status & CARRY);
        set_car(flg & NEG);
        set_neg(M & 0x80);
        set_zer(M == 0);
        A = M;
    }
    else
    {
        uint8_t flg = (uint8_t)read(addr);
        uint8_t M = (read(addr) << 1) & 0xfe;
        M |= (status & CARRY);
        set_car(flg & NEG);
        set_neg(M & 0x80);
        set_zer(M == 0);
        write(addr, M);
    }
}

void cpu6502::OP_ROR(uint16_t addr, int mode = 1)
{
    if (mode == 0)
    {
        uint8_t M = (A >> 1) | ((status & CARRY) << 7);
        set_car(A & CARRY);
        set_neg(M & 0x80);
        set_zer(M == 0);
        A = M;
    }
    else
    {
        uint8_t flg = (uint8_t)read(addr);
        uint8_t M = (read(addr) >> 1) | ((status & CARRY) << 7);
        set_car(flg & CARRY);
        set_neg(M & 0x80);
        set_zer(M == 0);
        write(addr, M);
    }
}

void cpu6502::OP_BRK(uint16_t addr)
{
    pc++;
    pushtostack((pc >> 8) & 0xFF);
    pushtostack(pc & 0xFF);
    pushtostack(status | BRK);
    set_int(1);
    uint16_t l = read(0xFFFE);
    uint16_t h = (read(0xFFFF) << 8);
    pc = h | l;
}

void cpu6502::OP_RTI(uint16_t addr)
{
    status = pullfromstack();
    uint16_t l = pullfromstack();
    uint16_t h = (pullfromstack() << 8);
    pc = h | l;
}

//STACK PUSH AND PULL

void cpu6502::OP_PHA(uint16_t addr)
{
    pushtostack(A);
}

void cpu6502::OP_PHP(uint16_t addr)
{
    pushtostack(status | BRK);
    set_unused(1);
}

void cpu6502::OP_PLA(uint16_t addr)
{
    A = pullfromstack();
    set_neg(A & 0x80);
    set_zer(A == 0);
}

void cpu6502::OP_PLP(uint16_t addr)
{
    status = pullfromstack()|_;
}

//BIT TEST
void cpu6502::OP_BIT(uint16_t addr)
{
    uint16_t M = read(addr);
    uint16_t res = A & M;
    set_zer(res == 0);
    set_ovr(M & 0x40);
    set_neg(M & 0x80);
}

//RESET VECTORS

void cpu6502::nmi()
{
    write(BASESTACK + sp, (pc >> 8) & 0xFF);
    write(BASESTACK + (sp - 1), pc & 0xFF);
    sp -= 2;
    write(BASESTACK + sp, status);
    sp--;
    set_int(1);
    pc = (read(0xFFFA)) + (read(0xFFFB) << 8);
}

void cpu6502::irq()
{
    write(BASESTACK + sp, (pc >> 8) & 0xFF);
    write(BASESTACK + (sp - 1), pc & 0xFF);
    sp -= 2;
    write(BASESTACK + sp, status);
    sp--;
    set_int(1);
    pc = (read(0xFFFE)) + (read(0xFFFF) << 8);
}

void cpu6502::reset()
{
    A = X = Y = 0;
    status = 0x20;
    uint16_t lo = (read(0xFFFC));
    uint16_t hi = (read(0xFFFD) << 8);
    pc = hi | lo;
    sp = 0xFD;
}

//Execute and decode

void cpu6502::runfor(int cycles)
{
    //init
    int remaining_clks = cycles;

    cycleCount = 0;

    while (remaining_clks > 0)
    {
        //fetch

        uint8_t op = (uint8_t)read(pc++);

        //decode, execute, and get cycles

        int opcycles = decode(op);

        remaining_clks -= opcycles ? opcycles : 1;

        cycleCount += opcycles ? opcycles : 1;

    }
    printf("CycleCount: %d\n\n", cycleCount);
}

uint8_t cpu6502::decode(uint8_t op)
{
    int cycles = 0;
    zpg_cross = false;
    branch_succeeds = false;
    page_crossed = false;
    switch (op)
    {
        //LDA
    case 0xA9:
    {
        uint16_t addr = immediate();
        OP_LDA(addr);
        cycles = 2;
        break;
    }
    case 0xA5:
    {
        uint16_t addr = zeropage();
        OP_LDA(addr);
        cycles = 3;
        break;
    }
    case 0xB5:
    {
        uint16_t addr = zeropage_x();
        OP_LDA(addr);
        cycles = 4;
        break;
    }
    case 0xAD:
    {
        uint16_t addr = absolute();
        OP_LDA(addr);
        cycles = 4;
        break;
    }
    case 0xBD:
    {
        uint16_t addr = absolute_x();
        OP_LDA(addr);
        cycles = CYCLE(4);
        break;
    }
    case 0xB9:
    {
        uint16_t addr = absolute_y();
        OP_LDA(addr);
        cycles = CYCLE(4);
        break;
    }
    case 0xA1:
    {
        uint16_t addr = indirect_x();
        OP_LDA(addr);
        cycles = 6;
        break;
    }
    case 0xB1:
    {
        uint16_t addr = indirect_y();
        OP_LDA(addr);
        cycles = CYCLE(5);
        break;
    }
    //LDY
    case 0xA0:
    {
        uint16_t addr = immediate();
        OP_LDY(addr);
        cycles = 2;
        break;
    }
    case 0xA4:
    {
        uint16_t addr = zeropage();
        OP_LDY(addr);
        cycles = 3;
        break;
    }
    case 0xB4:
    {
        uint16_t addr = zeropage_x();
        OP_LDY(addr);
        cycles = 4;
        break;
    }
    case 0xAC:
    {
        uint16_t addr = absolute();
        OP_LDY(addr);
        cycles = 4;
        break;
    }
    case 0xBC:
    {
        uint16_t addr = absolute_x();
        OP_LDY(addr);
        cycles = CYCLE(4);
        break;
    }
    //LDX
    case 0xA2:
    {
        uint16_t addr = immediate();
        OP_LDX(addr);
        cycles = 2;
        break;
    }
    case 0xA6:
    {
        uint16_t addr = zeropage();
        OP_LDX(addr);
        cycles = 3;
        break;
    }
    case 0xB6:
    {
        uint16_t addr = zeropage_y();
        OP_LDX(addr);
        cycles = 4;
        break;
    }
    case 0xAE:
    {
        uint16_t addr = absolute();
        OP_LDX(addr);
        cycles = 4;
        break;
    }
    case 0xBE:
    {
        uint16_t addr = absolute_y();
        OP_LDX(addr);
        cycles = CYCLE(4);
        break;
    }
    //STA
    case 0x85://zeropage
    {
        uint16_t addr = zeropage();
        OP_STA(addr);
        cycles = 3;
        break;
    }
    case 0x95://zeropage, x
    {
        uint16_t addr = zeropage_x();
        OP_STA(addr);
        cycles = 4;
        break;
    }
    case 0x8D://abs
    {
        uint16_t addr = absolute();
        OP_STA(addr);
        cycles = 4;
        break;
    }
    case 0x9D://abs, x
    {
        uint16_t addr = absolute_x();
        OP_STA(addr);
        cycles = 5;
        break;
    }
    case 0x99://abs, y
    {
        uint16_t addr = absolute_y();
        OP_STA(addr);
        cycles = 5;
        break;
    }
    case 0x91://indr, y
    {
        uint16_t addr = indirect_y();
        OP_STA(addr);
        cycles = 6;
        break;
    }
    case 0x81://indr, x
    {
        uint16_t addr = indirect_x();
        OP_STA(addr);
        cycles = 6;
        break;
    }
    //STX
    case 0x86://zeropage
    {
        uint16_t addr = zeropage();
        OP_STX(addr);
        cycles = 3;
        break;
    }
    case 0x8E://abs
    {
        uint16_t addr = absolute();
        OP_STX(addr);
        cycles = 4;
        break;
    }
    case 0x96://zeropage, y
    {
        uint16_t addr = zeropage_y();
        OP_STX(addr);
        cycles = 4;
        break;
    }
    //STY
    case 0x84:
    {
        uint16_t addr = zeropage();
        OP_STY(addr);
        cycles = 3;
        break;
    }
    case 0x8C://abs
    {
        uint16_t addr = absolute();
        OP_STY(addr);
        cycles = 4;
        break;
    }
    case 0x94://zeropage, x
    {
        uint16_t addr = zeropage_x();
        OP_STY(addr);
        cycles = 4;
        break;
    }
    //FLAG OPERATIONS
    case 0x78:
    {
        uint16_t addr = implied();
        OP_SEI(addr);
        cycles = 2;
        break;
    }
    case 0xF8:
    {
        uint16_t addr = implied();
        OP_SED(addr);
        cycles = 2;
        break;
    }
    case 0x38:
    {
        uint16_t addr = implied();
        OP_SEC(addr);
        cycles = 2;
        break;
    }
    case 0x58:
    {
        uint16_t addr = implied();
        OP_CLI(addr);
        cycles = 2;
        break;
    }
    case 0xB8:
    {
        uint16_t addr = implied();
        OP_CLV(addr);
        cycles = 2;
        break;
    }
    case 0xD8:
    {
        uint16_t addr = implied();
        OP_CLD(addr);
        cycles = 2;
        break;
    }
    case 0x18:
    {
        uint16_t addr = implied();
        OP_CLC(addr);
        cycles = 2;
        break;
    }
    //Subroutine
    case 0x20:
    {
        uint16_t addr = absolute();
        OP_JSR(addr);
        cycles = 6;
        break;
    }
    case 0x60:
    {
        uint16_t addr = implied();
        OP_RTS(addr);
        cycles = 6;
        break;
    }
    //Transfer
    case 0x98:
    {
        uint16_t addr = implied();
        OP_TYA(addr);
        cycles = 2;
        break;
    }
    case 0x9A:
    {
        uint16_t addr = implied();
        OP_TXS(addr);
        cycles = 2;
        break;
    }
    case 0x8A:
    {
        uint16_t addr = implied();
        OP_TXA(addr);
        cycles = 2;
        break;
    }
    case 0xBA:
    {
        uint16_t addr = implied();
        OP_TSX(addr);
        cycles = 2;
        break;
    }
    case 0xA8:
    {
        uint16_t addr = implied();
        OP_TAY(addr);
        cycles = 2;
        break;
    }
    case 0xAA:
    {
        uint16_t addr = implied();
        OP_TAX(addr);
        cycles = 2;
        break;
    }
    //NOP
    case 0xEA:
    {
        uint16_t addr = implied();
        OP_NOP(addr);
        cycles = 2;
        break;
    }
    //Increment and decrement
    case 0xCA:
    {
        uint16_t addr = implied();
        OP_DEX(addr);
        cycles = 2;
        break;
    }
    case 0x88:
    {
        uint16_t addr = implied();
        OP_DEY(addr);
        cycles = 2;
        break;
    }
    case 0xC8:
    {
        uint16_t addr = implied();
        OP_INY(addr);
        cycles = 2;
        break;
    }
    case 0xE8:
    {
        uint16_t addr = implied();
        OP_INX(addr);
        cycles = 2;
        break;
    }
    case 0xE6:
    {
        uint16_t addr = zeropage();
        OP_INC(addr);
        cycles = 5;
        break;
    }
    case 0xF6:
    {
        uint16_t addr = zeropage_x();
        OP_INC(addr);
        cycles = 6;
        break;
    }
    case 0xEE:
    {
        uint16_t addr = absolute();
        OP_INC(addr);
        cycles = 6;
        break;
    }
    case 0xFE:
    {
        uint16_t addr = absolute_x();
        OP_INC(addr);
        cycles = 7;
        break;
    }
    case 0xC6:
    {
        uint16_t addr = zeropage();
        OP_DEC(addr);
        cycles = 5;
        break;
    }
    case 0xD6:
    {
        uint16_t addr = zeropage_x();
        OP_DEC(addr);
        cycles = 6;
        break;
    }
    case 0xCE:
    {
        uint16_t addr = absolute();
        OP_DEC(addr);
        cycles = 6;
        break;
    }
    case 0xDE:
    {
        uint16_t addr = absolute_x();
        OP_DEC(addr);
        cycles = 7;
        break;
    }
    //COMPARE
    //CPX
    case 0xEC:
    {
        uint16_t addr = absolute();
        OP_CPX(addr);
        cycles = 4;
        break;
    }
    case 0xE0:
    {
        uint16_t addr = immediate();
        OP_CPX(addr);
        cycles = 2;
        break;
    }
    case 0xE4:
    {
        uint16_t addr = zeropage();
        OP_CPX(addr);
        cycles = 3;
        break;
    }
    //CPY
    case 0xCC:
    {
        uint16_t addr = absolute();
        OP_CPY(addr);
        cycles = 4;
        break;
    }
    case 0xC0:
    {
        uint16_t addr = immediate();
        OP_CPY(addr);
        cycles = 2;
        break;
    }
    case 0xC4:
    {
        uint16_t addr = zeropage();
        OP_CPY(addr);
        cycles = 3;
        break;
    }
    //CPM
    case 0xCD:
    {
        uint16_t addr = absolute();
        OP_CMP(addr);
        cycles = 4;
        break;
    }
    case 0xDD:
    {
        uint16_t addr = absolute_x();
        OP_CMP(addr);
        cycles = CYCLE(4);
        break;
    }
    case 0xD9:
    {
        uint16_t addr = absolute_y();
        OP_CMP(addr);
        cycles = 4;
        break;
    }
    case 0xC9:
    {
        uint16_t addr = immediate();
        OP_CMP(addr);
        cycles = 2;
        break;
    }
    case 0xC5:
    {
        uint16_t addr = zeropage();
        OP_CMP(addr);
        cycles = 3;
        break;
    }
    case 0xD5:
    {
        uint16_t addr = zeropage_x();
        OP_CMP(addr);
        cycles = 4;
        break;
    }
    case 0xC1:
    {
        uint16_t addr = indirect_x();
        OP_CMP(addr);
        cycles = 6;
        break;
    }
    case 0xD1:
    {
        uint16_t addr = indirect_y();
        OP_CMP(addr);
        cycles = CYCLE(5);
        break;
    }
    //JMP
    case 0x4C:
    {
        uint16_t addr = absolute();
        OP_JMP(addr);
        cycles = 3;
        break;
    }
    case 0x6C:
    {
        uint16_t addr = indirect();
        OP_JMP(addr);
        cycles = 5;
        break;
    }
    //BRANCH
    case 0x90:
    {
        uint16_t addr = relative();
        OP_BCC(addr);
        cycles = 2;
        break;
    }
    case 0xB0:
    {
        uint16_t addr = relative();
        OP_BCS(addr);
        cycles = 2;
        break;
    }
    case 0xF0:
    {
        uint16_t addr = relative();
        OP_BEQ(addr);
        cycles = 2;
        break;
    }
    case 0x30:
    {
        uint16_t addr = relative();
        OP_BMI(addr);
        cycles = 2;
        break;
    }
    case 0xD0:
    {
        uint16_t addr = relative();
        OP_BNE(addr);
        cycles = 2;
        if (page_crossed)
            cycles += 2;
        break;
    }
    case 0x10:
    {
        uint16_t addr = relative();
        OP_BPL(addr);
        cycles = 2;
        break;
    }
    case 0x50:
    {
        uint16_t addr = relative();
        OP_BVC(addr);
        cycles = 2;
        break;
    }
    case 0x70:
    {
        uint16_t addr = relative();
        OP_BVS(addr);
        cycles = 2;
        break;
    }
    //ADC
    case 0x69:
    {
        uint16_t addr = immediate();
        OP_ADC(addr);
        cycles = 2;
        break;
    }
    case 0x65:
    {
        uint16_t addr = zeropage();
        OP_ADC(addr);
        cycles = 3;
        break;
    }
    case 0x75:
    {
        uint16_t addr = zeropage_x();
        OP_ADC(addr);
        cycles = 4;
        break;
    }
    case 0x6D:
    {
        uint16_t addr = absolute();
        OP_ADC(addr);
        cycles = 4;
        break;
    }
    case 0x7D:
    {
        uint16_t addr = absolute_x();
        OP_ADC(addr);
        cycles = CYCLE(4);
        break;
    }
    case 0x79:
    {
        uint16_t addr = absolute_y();
        OP_ADC(addr);
        cycles = CYCLE(4);
        break;
    }
    case 0x61:
    {
        uint16_t addr = indirect_x();
        OP_ADC(addr);
        cycles = 6;
        break;
    }
    case 0x71:
    {
        uint16_t addr = indirect_y();
        OP_ADC(addr);
        cycles = CYCLE(5);
        break;
    }
    //SBC
    case 0xE9:
    {
        uint16_t addr = immediate();
        OP_SBC(addr);
        cycles = 2;
        break;
    }
    case 0xE5:
    {
        uint16_t addr = zeropage();
        OP_SBC(addr);
        cycles = 3;
        break;
    }
    case 0xF5:
    {
        uint16_t addr = zeropage_x();
        OP_SBC(addr);
        cycles = 4;
        break;
    }
    case 0xED:
    {
        uint16_t addr = absolute();
        OP_SBC(addr);
        cycles = 4;
        break;
    }
    case 0xFD:
    {
        uint16_t addr = absolute_x();
        OP_SBC(addr);
        cycles = CYCLE(4);
        break;
    }
    case 0xF9:
    {
        uint16_t addr = absolute_y();
        OP_SBC(addr);
        cycles = CYCLE(4);
        break;
    }
    case 0xE1:
    {
        uint16_t addr = indirect_x();
        OP_SBC(addr);
        cycles = 6;
        break;
    }
    case 0xF1:
    {
        uint16_t addr = indirect_y();
        OP_SBC(addr);
        cycles = CYCLE(5);
        break;
    }
    //RTI AND BRK
    case 0x40:
    {
        uint16_t addr = implied();
        OP_RTI(addr);
        cycles = 6;
        break;
    }
    case 0x00:
    {
        uint16_t addr = implied();
        OP_BRK(addr);
        cycles = 7;
        break;
    }
    //ASL
    case 0x0A:
    {
        uint16_t addr = accumulator();
        OP_ASL(addr, 0);
        cycles = 2;
        break;
    }
    case 0x06:
    {
        uint16_t addr = zeropage();
        OP_ASL(addr);
        cycles = 5;
        break;
    }
    case 0x16:
    {
        uint16_t addr = zeropage_x();
        OP_ASL(addr);
        cycles = 6;
        break;
    }
    case 0x0E:
    {
        uint16_t addr = absolute();
        OP_ASL(addr);
        cycles = 6;
        break;
    }
    case 0x1E:
    {
        uint16_t addr = absolute_x();
        OP_ASL(addr);
        cycles = 7;
        break;
    }
    //AND
    case 0x29:
    {
        uint16_t addr = immediate();
        OP_AND(addr);
        cycles = 2;
        break;
    }
    case 0x25:
    {
        uint16_t addr = zeropage();
        OP_AND(addr);
        cycles = 3;
        break;
    }
    case 0x35:
    {
        uint16_t addr = zeropage_x();
        OP_AND(addr);
        cycles = 4;
        break;
    }
    case 0x2D:
    {
        uint16_t addr = absolute();
        OP_AND(addr);
        cycles = 4;
        break;
    }
    case 0x3D:
    {
        uint16_t addr = absolute_x();
        OP_AND(addr);
        cycles = 4;
        break;
    }
    case 0x39:
    {
        uint16_t addr = absolute_y();
        OP_AND(addr);
        cycles = 4;
        break;
    }
    case 0x21:
    {
        uint16_t addr = indirect_x();
        OP_AND(addr);
        cycles = 6;
        break;
    }
    case 0x31:
    {
        uint16_t addr = indirect_y();
        OP_AND(addr);
        cycles = 5;
        break;
    }
    //BIT
    case 0x24:
    {
        uint16_t addr = zeropage();
        OP_BIT(addr);
        cycles = 3;
        break;
    }
    case 0x2C:
    {
        uint16_t addr = absolute();
        OP_BIT(addr);
        cycles = 4;
        break;
    }
    //EOR
    case 0x49:
    {
        uint16_t addr = immediate();
        OP_EOR(addr);
        cycles = 2;
        break;
    }
    case 0x45:
    {
        uint16_t addr = zeropage();
        OP_EOR(addr);
        cycles = 3;
        break;
    }
    case 0x55:
    {
        uint16_t addr = zeropage_x();
        OP_EOR(addr);
        cycles = 4;
        break;
    }
    case 0x4D:
    {
        uint16_t addr = absolute();
        OP_EOR(addr);
        cycles = 4;
        break;
    }
    case 0x5D:
    {
        uint16_t addr = absolute_x();
        OP_EOR(addr);
        cycles = CYCLE(4);
        break;
    }
    case 0x59:
    {
        uint16_t addr = absolute_y();
        OP_EOR(addr);
        cycles = CYCLE(4);
        break;
    }
    case 0x41:
    {
        uint16_t addr = indirect_x();
        OP_EOR(addr);
        cycles = 6;
        break;
    }
    case 0x51:
    {
        uint16_t addr = indirect_y();
        OP_EOR(addr);
        cycles = CYCLE(5);
        break;
    }
    //LSR
    case 0x4A:
    {
        uint16_t addr = accumulator();
        OP_LSR(addr, 0);
        cycles = 2;
        break;
    }
    case 0x46:
    {
        uint16_t addr = zeropage();
        OP_LSR(addr);
        cycles = 5;
        break;
    }
    case 0x56:
    {
        uint16_t addr = zeropage_x();
        OP_LSR(addr);
        cycles = 6;
        break;
    }
    case 0x4E:
    {
        uint16_t addr = absolute();
        OP_LSR(addr);
        cycles = 6;
        break;
    }
    case 0x5E:
    {
        uint16_t addr = absolute_x();
        OP_LSR(addr);
        cycles = 7;
        break;
    }
    //ORA
    case 0x09:
    {
        uint16_t addr = immediate();
        OP_ORA(addr);
        cycles = 2;
        break;
    }
    case 0x05:
    {
        uint16_t addr = zeropage();
        OP_ORA(addr);
        cycles = 3;
        break;
    }
    case 0x15:
    {
        uint16_t addr = zeropage_x();
        OP_ORA(addr);
        cycles = 4;
        break;
    }
    case 0x0D:
    {
        uint16_t addr = absolute();
        OP_ORA(addr);
        cycles = 4;
        break;
    }
    case 0x1D:
    {
        uint16_t addr = absolute_x();
        OP_ORA(addr);
        cycles = CYCLE(4);
        break;
    }
    case 0x19:
    {
        uint16_t addr = absolute_y();
        OP_ORA(addr);
        cycles = CYCLE(4);
        break;
    }
    case 0x01:
    {
        uint16_t addr = indirect_x();
        OP_ORA(addr);
        cycles = 6;
        break;
    }
    case 0x11:
    {
        uint16_t addr = indirect_y();
        OP_ORA(addr);
        cycles = CYCLE(5);
        break;
    }
    //PHA
    case 0x48:
    {
        uint16_t addr = implied();
        OP_PHA(addr);
        cycles = 3;
        break;
    }
    //PHP
    case 0x08:
    {
        uint16_t addr = implied();
        OP_PHP(addr);
        cycles = 3;
        break;
    }
    //PLA
    case 0x68:
    {
        uint16_t addr = implied();
        OP_PLA(addr);
        cycles = 4;
        break;
    }
    //PLP
    case 0x28:
    {
        uint16_t addr = implied();
        OP_PLP(addr);
        cycles = 4;
        break;
    }
    //ROL
    case 0x2A:
    {
        uint16_t addr = accumulator();
        OP_ROL(addr, 0);
        cycles = 2;
        break;
    }
    case 0x26:
    {
        uint16_t addr = zeropage();
        OP_ROL(addr);
        cycles = 5;
        break;
    }
    case 0x36:
    {
        uint16_t addr = zeropage_x();
        OP_ROL(addr);
        cycles = 6;
        break;
    }
    case 0x2E:
    {
        uint16_t addr = absolute();
        OP_ROL(addr);
        cycles = 6;
        break;
    }
    case 0x3E:
    {
        uint16_t addr = absolute_x();
        OP_ROL(addr);
        cycles = 7;
        break;
    }
    //ROR
    case 0x6A:
    {
        uint16_t addr = accumulator();
        OP_ROR(addr, 0);
        cycles = 2;
        break;
    }
    case 0x66:
    {
        uint16_t addr = zeropage();
        OP_ROR(addr);
        cycles = 5;
        break;
    }
    case 0x76:
    {
        uint16_t addr = zeropage_x();
        OP_ROR(addr);
        cycles = 6;
        break;
    }
    case 0x6E:
    {
        uint16_t addr = absolute();
        OP_ROR(addr);
        cycles = 6;
        break;
    }
    case 0x7E:
    {
        uint16_t addr = absolute_x();
        OP_ROR(addr);
        cycles = 7;
        break;
    }
    default:
    {
       printf("\nERROR for OPCODE[ 0x%X ]: Instruction is illegal, could not be completed, or doesn't exist. An error with the clock cycle might also be possible\n", op);
    }
    }
    return cycles;
}