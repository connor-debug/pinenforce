# PINenforce

Information
-----------

PINenforce is a code re-use attack mitigation system built with the intel PIN framework. Benefits of this include the fact that there is no binary re-writing involved in the scheme, and it can be used straight out of the box with little to no setup. This tool works by shadowing the processor instructions of the target program in real time. The main idea of PINenforce revolves around the implementation and synthesis of three separate mechanisms inspired by the return oriented programming security literature:

  * _**Ret-Based  Shadow  Stack**_:  During  the  just-in-time instrumentation  of  the  target  program,  the  pintool examines  and  identifies  the  instruction  type.  If  the instruction  is  a  call,  the  address  is  stored  on  the shadow  stack.  Otherwise,  the  algorithm  checks  for instructions  of  the  ret  type.  If  it  is  a  ret,  then  the top  of  the  shadow  stack  is  checked  for  the  return addresses. If the return addresses match, then this is a valid control flow transfer. If there is a mismatch,then a corruption has been identified. If the instruction is not a ret, then the algorithm moves on to the next  instruction  of  the  target  program.  It  should  be noted that this method only identifies return based memory corruption exceptions.
  
  * _**Abnormal  Control  Flow  Checking**_:  As  the  instrumentation and instruction type checking takes place, the tool will identify a ret that does not correspond to any call as ”abnormal”, note that this is different than  a  mismatch,  as  that  would  be  categorized  as an  exception  or  corruption.  abnormal  control  flow is  a  heuristic  approach  and  the  parameter  for  howmany abnormal instructions must be tolerated can vary. It is also important to check malicious attacks for syscalls.  Malicious  payloads  necessarily  invoke system calls to spawn root shells, so any syscall can raise a red flag.
  
  * _**Jump-Oriented Programming Defense**_: PINenforceuses  a  heuristic  approach  for  identifying  gadgets. The main approach is to use a run time algorithm to dynamically verify that the consecutive number of sequences of instruction of length n does not exceed a threshold s. There are many ways of doing this, and this tool identifies and implements two.

Usage
-----

```
pin -t obj-intel64/enfA.so -- [application] or 
pin -t obj-intel64/enfB.so -- [applicaiton] for the two algorithms

configure PINenforce (paramters, ret based shadow stack, etc.) by editing source and doing
make obj_intel64/enfAorB.so
```
