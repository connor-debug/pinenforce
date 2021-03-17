#include <iostream>
#include <fstream>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include "pin.H"
using std::ofstream;
using std::string;
using std::hex;
using std::setw;
using std::cerr;
using std::dec;
using std::endl;

ofstream outFile;

// A structure to represent a stack
struct Stack {
    ADDRINT top;
    ADDRINT capacity;
    ADDRINT* array;
};

//struct Stack* stack = createStack(2000);
 
// function to create a stack of given capacity. It initializes size of
// stack as 0
struct Stack* createStack(ADDRINT capacity)
{
    struct Stack* stack = (struct Stack*)malloc(sizeof(struct Stack));
    stack->capacity = capacity;
    stack->top = -1;
    stack->array = (ADDRINT*)malloc(stack->capacity * sizeof(ADDRINT));
    return stack;
}
 
// Stack is empty when top is equal to -1
ADDRINT isEmpty(struct Stack* stack)
{
    return stack->top == (ADDRINT)-1;
}
 
// Function to add an item to stack.  It increases top by 1
void push(struct Stack* stack, ADDRINT item)
{
    stack->array[++stack->top] = item;
    printf("%lu pushed to stack\n", item);
}
 
// Function to remove an item from stack.  It decreases top by 1
ADDRINT pop(struct Stack* stack)
{
    if (isEmpty(stack))
        return INT_MIN;
    return stack->array[stack->top--];
}
 
// Function to return the top from stack without removing it
ADDRINT peek(struct Stack* stack)
{
    if (isEmpty(stack))
        return INT_MIN;
    return stack->array[stack->top];
}

struct Stack* stack = createStack(20000);

VOID checkStack(ADDRINT addr){
	ADDRINT ch = pop(stack);
	if(ch == addr) { printf("VALID RET!\n"); return; }
	while(!isEmpty(stack)){
			ch = pop(stack);
			if(ch == addr){
				printf("VALID RET!");
				return;
			}
		}
	printf("Invalid return: %lu vs %lu, Terminating program...\n", ch, addr);
	exit(1);
}


VOID pushStack(ADDRINT addr)
{
	push(stack, addr);
   // PIN_SafeCopy(&value, addr, sizeof(ADDRINT));
   // outFile << "Address 0x" << hex << (unsigned long long)addr << dec << ":\t" << value << endl;
   // printf("found an indirect control flow transfer\n");
    return;
}

VOID Instruction(INS ins, VOID *v)
{
	ADDRINT nextAddress;
	
    if (INS_IsCall(ins) && INS_IsControlFlow(ins))
    {
	nextAddress = INS_NextAddress(ins);

        INS_InsertCall(ins,
                       IPOINT_BEFORE,
                       AFUNPTR(pushStack),
		       IARG_ADDRINT,
		       nextAddress,
                       IARG_END);
    }

    if (INS_IsRet(ins) && INS_IsControlFlow(ins))
	{
		INS_InsertCall(ins,
				IPOINT_BEFORE,
				AFUNPTR(checkStack),
				IARG_BRANCH_TARGET_ADDR,
				IARG_END);
	}
}

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
                            "o", "enf.out", "specify output file name");

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    outFile.close();
}

INT32 Usage()
{
    cerr << "This tool dump heap memory information (global variable) ..." << endl;
    cerr << endl
         << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

int main(int argc, char *argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv))
        return Usage();
    outFile.open(KnobOutputFile.Value().c_str());
    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);
    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
