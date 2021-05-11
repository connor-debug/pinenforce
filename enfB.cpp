#include <iostream>
#include <fstream>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include "pin.H"
#define MAXRET 60
#define MAXNJMP 8
#define MAXSJMP 24
#define STACK 2000
using std::ofstream;
using std::string;
using std::hex;
using std::setw;
using std::cerr;
using std::dec;
using std::endl;

ofstream outFile;

double run_avg;
int jcount = 0;
int rets = 0;
int icount = 0;
int bcount = 0;

struct Stack {
    ADDRINT top;
    ADDRINT capacity;
    ADDRINT* array;
};

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
 
void push(struct Stack* stack, ADDRINT item)
{
    stack->array[++stack->top] = item;
    printf("%lu pushed to stack\n", item);
}
 
ADDRINT pop(struct Stack* stack)
{
    if (isEmpty(stack))
        return INT_MIN;
    return stack->array[stack->top--];
}
 
ADDRINT peek(struct Stack* stack)
{
    if (isEmpty(stack))
        return INT_MIN;
    return stack->array[stack->top];
}

struct Stack* stack = createStack(STACK);

VOID checkStack(ADDRINT addr){
	ADDRINT ch = pop(stack);
	if(ch == addr) { printf("VALID RET!\n"); rets++;
       		if(rets > MAXRET)
			printf("4 returns in a row\n");
		return;	}
	while(!isEmpty(stack)){
			ch = pop(stack);
			if(ch == addr){
				printf("VALID RET!\n");
				rets++;
				if(rets > MAXRET)
					printf("4 returns in a row");
				return;
			}
		}
	printf("Invalid return: %lu vs %lu, Terminating program...\n", ch, addr);
//	exit(1);
}


VOID pushStack(ADDRINT addr)
{
	push(stack, addr);
	rets = 0;
    return;
}

VOID checkBranch(){
	bcount++;
	run_avg = run_avg + (icount/bcount);
	icount = 0;
	if(bcount >= MAXSJMP){
		bcount = 0;
		if(run_avg <= MAXNJMP){
			printf("fond avg \n");
			jcount++;
		}
		else{run_avg = 0;}
	}
	return;
}

VOID countins(){icount++;/* printf("%d on the ins counter....\n", icount);*/}

VOID Instruction(INS ins, VOID *v)
{
	ADDRINT nextAddress;

	INS_InsertCall(ins, IPOINT_BEFORE,
			AFUNPTR(countins),
			IARG_END);


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

    if (INS_IsBranch(ins))
	{
		INS_InsertCall(ins,
				IPOINT_BEFORE,
				AFUNPTR(checkBranch),
				IARG_END);
	}
}

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
                            "o", "enfB.out", "specify output file name");

VOID Fini(INT32 code, VOID *v)
{
	outFile << "number of potential JOPs: " << dec << jcount;
    outFile.close();
}

INT32 Usage()
{
    cerr << "This tool is for enforcing control flow integrity." << endl;
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
