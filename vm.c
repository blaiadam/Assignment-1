#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm.h"
#include "data.h"


//===== Declarations =====

void initVM(VirtualMachine*);
int readInstructions(FILE*, Instruction*);
void dumpInstructions(FILE*, Instruction*, int numOfIns);
int getBasePointer(int* stack, int currentBP, int L);
void dumpStack(FILE*, int* stack, int sp, int bp);
int executeInstruction(VirtualMachine* vm, Instruction ins, FILE* vmIn, FILE* vmOut);


//===== Global Data and misc structs & enums =====

const char *opcodes[] = 
{
    "illegal", // opcode 0 is illegal
    "lit", "rtn", "lod", "sto", "cal", // 1, 2, 3 ..
    "inc", "jmp", "jpc", "sio", "sio",
    "sio", "neg", "add", "sub", "mul",
    "div", "odd", "mod", "eql", "neq",
    "lss", "leq", "gtr", "geq"
};

enum { CONT, HALT };


//===== Definitions =====

// INIT VM
void initVM(VirtualMachine* vm)
{
	// Initialize the counter and pointers.
    if(vm)
    {
        vm->BP = 1;
		vm->SP = 0;
		vm->PC = 0;
    }
}

// READ INSTRUCTIONS
int readInstructions(FILE* in, Instruction* ins)
{
    // Instruction index
    int i = 0;
    
    while(fscanf(in, "%d %d %d %d", &ins[i].op, &ins[i].r, &ins[i].l, &ins[i].m) == 4)
        i++;

    // Return the number of instructions read
    return i;
}

// DUMP INSTRUCTIONS
void dumpInstructions(FILE* out, Instruction* ins, int numOfIns)
{
    // Header
    fprintf(out,
        "***Code Memory***\n%3s %3s %3s %3s %3s \n",
        "#", "OP", "R", "L", "M"
        );

    // Instructions
    int i;
    for(i = 0; i < numOfIns; i++)
    {
        fprintf(
            out,
            "%3d %3s %3d %3d %3d \n", // formatting
            i, opcodes[ins[i].op], ins[i].r, ins[i].l, ins[i].m
        );
    }
}

// GET BASE POINTER
int getBasePointer(int* stack, int currentBP, int L)
{
	// Create new BP.
	int newBP = currentBP;
	
	// Use while loop to find newBP L levels down.
	while(L-- > 0)
		newBP = stack[newBP + 1];
	
	return newBP;
}

// DUMP STACK
void dumpStack(FILE* out, int* stack, int sp, int bp)
{
    if(bp == 0)
        return;

    // bottom-most level, where a single zero value lies
    if(bp == 1)
    {
        fprintf(out, "%3d ", 0);
    }

    // former levels - if exists
    if(bp != 1)
    {
        dumpStack(out, stack, bp - 1, stack[bp + 2]);            
    }

    // top level: current activation record
    if(bp <= sp)
    {
        // indicate a new activation record
        fprintf(out, "| ");

        // print the activation record
        int i;
        for(i = bp; i <= sp; i++)
        {
            fprintf(out, "%3d ", stack[i]);
        }
    }
}

// EXECUTE INSTRUCTIONS
int executeInstruction(VirtualMachine* vm, Instruction ins, FILE* vmIn, FILE* vmOut)
{	
    switch(ins.op)
    {
		case 1:
			// lit
			vm->RF[ins.r] = ins.m;
			break;
			
		case 2:
			// rtn
			vm->SP = vm->BP - 1;
			vm->PC = vm->stack[vm->SP + 4];
			vm->BP = vm->stack[vm->SP + 3];
			break;
			
		case 3:
			// lod
			vm->RF[ins.r] = vm->stack[getBasePointer(vm->stack, vm->BP, ins.l) + ins.m];
			break;
			
		case 4:
			// sto
			vm->stack[getBasePointer(vm->stack, vm->BP, ins.l) + ins.m] = vm->RF[ins.r];
			break;
			
		case 5:
			// cal
			vm->stack[vm->SP + 1] = 0;
			vm->stack[vm->SP + 2] = getBasePointer(vm->stack, vm->BP, ins.l);
			vm->stack[vm->SP + 3] = vm->BP;
			vm->stack[vm->SP + 4] = vm->PC;
			vm->BP = vm->SP + 1;
			vm->PC = ins.m;
			break;
			
		case 6:
			// INC
			vm->SP += ins.m;
			break;
			
		case 7:
			// JMP
			vm->PC = ins.m;
			break;
			
		case 8:
			// JPC
			if(vm->RF[ins.r] == 0)
				vm->PC = ins.m;
			break;
			
		case 9:
			// SIO
			fprintf(vmOut, "%d ", vm->RF[ins.r]);
			break;
			
		case 10:
			// SIO
			fscanf(vmIn, "%d", &vm->RF[ins.r]);
			break;
			
		case 11:
			// SIO
			return 0;
			
		case 12:
			// NEG
			vm->RF[ins.r] *= -1;
			break;
			
		case 13:
			// ADD
			vm->RF[ins.r] = vm->RF[ins.l] + vm->RF[ins.m];
			break;
			
		case 14:
			// SUB
			vm->RF[ins.r] = vm->RF[ins.l] - vm->RF[ins.m];
			break;
			
		case 15:
			// MUL
			vm->RF[ins.r] = vm->RF[ins.l] * vm->RF[ins.m];
			break;
			
		case 16:
			// DIV
			vm->RF[ins.r] = vm->RF[ins.l] / vm->RF[ins.m];
			break;
			
		case 17:
			// ODD
			vm->RF[ins.r] %= 2;
			break;
			
		case 18:
			// MOD
			vm->RF[ins.r] = vm->RF[ins.l] % vm->RF[ins.m];
			break;
			
		case 19:
			// EQL
			vm->RF[ins.r] = vm->RF[ins.l] == vm->RF[ins.m];
			break;
			
		case 20:
			// NEQ
			vm->RF[ins.r] = vm->RF[ins.l] != vm->RF[ins.m];
			break;
			
		case 21:
			// LSS
			vm->RF[ins.r] = vm->RF[ins.l] < vm->RF[ins.m];
			break;
			
		case 22:
			// LEQ
			vm->RF[ins.r] = vm->RF[ins.l] <= vm->RF[ins.m];
			break;
			
		case 23:
			// GTR
			vm->RF[ins.r] = vm->RF[ins.l] > vm->RF[ins.m];
			break;
			
		case 24:
			// GEQ
			vm->RF[ins.r] = vm->RF[ins.l] >= vm->RF[ins.m];
			break;
			
        default:
            fprintf(stderr, "Illegal instruction?");
            return 0;
    }

    return 1;
}

// SIMULATE VM
void simulateVM(FILE* inp, FILE* outp, FILE* vmIn, FILE* vmOut)
{
    // Read instructions from file.
	Instruction* ins = malloc(sizeof(Instruction) * MAX_CODE_LENGTH);
	int numOfIns = readInstructions(inp, ins);
	
	// Dump instructions to the output file.
	dumpInstructions(outp, ins, numOfIns);
	
    // Before starting the code execution on the virtual machine, write the header for the simulation part 
	// (***Execution***)
    fprintf(outp, "\n***Execution***\n");
    fprintf(
        outp,
        "%3s %3s %3s %3s %3s %3s %3s %3s %3s \n",         // formatting
        "#", "OP", "R", "L", "M", "PC", "BP", "SP", "STK" // titles
    );

    // Create and initialize the virtual machine.
	VirtualMachine *vm = malloc(sizeof(VirtualMachine));
	initVM(vm);
	
    // Fetch&Execute the instructions on the virtual machine until halting.
	// i is the counter for printing out the stack.
	int i = 0, numOfAR = 0, currAR = 0, halt = 1, currInst = 0, prevCall = 0;
	int AR[MAX_STACK_HEIGHT];
    while(halt)
    {	
        // Advance PC - before execution and execute the instruction.
		currInst = vm->PC++;
        halt = executeInstruction(vm, ins[currInst], vmIn, vmOut);
		
		// Checks if inc has created a new activation record. If there was a call in the previous instruction,
		// this is ignored.
		if((ins[currInst].op == 6) && (prevCall == 0) && (ins[currInst].m > 1))
			AR[numOfAR++] = vm->BP;
		
		// Check if current instruction is cal. If so, add the location of the BP for the new activation 
		// record to the AR[] array.
		if(ins[currInst].op == 5)
		{
			AR[numOfAR++] = vm->BP;
			prevCall = 1;
		} else
			prevCall = 0;
		
        // Print current state
        // Following is a possible way of printing the current state where instrBeingExecuted is the 
		// address of the instruction at vm memory and instr is the instruction being executed.
        fprintf(
            outp,
            "%3d %3s %3d %3d %3d %3d %3d %3d ",
            currInst, // place of instruction at memory
            opcodes[ins[currInst].op], ins[currInst].r, ins[currInst].l, ins[currInst].m, // instruction info
            vm->PC, vm->BP, vm->SP // vm info
        );
		
		// Print stack
		dumpStack(outp, vm->stack, vm->SP, vm->BP);
		
        fprintf(outp, "\n");
    }

    // Above loop ends when machine halts. Therefore, dump halt message and free malloced memory.
    fprintf(outp, "HLT\n");
	free(ins);
	free(vm);
}