#include "pin.H"
#include <fstream>
#include <iostream>
#include <unordered_map>

KNOB< std::string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "trace", "specify file name for output");

std::unordered_map<THREADID, std::ofstream> output;
std::unordered_map<ADDRINT, std::string> routine_name;
std::unordered_map<ADDRINT, std::string> routine_file;
std::unordered_map<ADDRINT, INT> routine_line;

INT32 Usage()
{
    std::cerr << "This tool prints out the number of dynamically executed " << std::endl
         << "instructions, basic blocks and threads in the application." << std::endl
         << std::endl;

    std::cerr << KNOB_BASE::StringKnobSummary() << std::endl;

    return -1;
}

VOID routineBefore(THREADID threadid, ADDRINT rtn_addr)
{ 
    output[threadid] << "called:" << routine_file[rtn_addr] << ":" << routine_line[rtn_addr] << ":" << routine_name[rtn_addr] << std::endl;
}

VOID routineAfter(THREADID threadid, ADDRINT rtn_addr)
{ 
    output[threadid] << "return:" << routine_file[rtn_addr] << ":" << routine_line[rtn_addr] << ":" << routine_name[rtn_addr] << std::endl;
}

VOID Routine(RTN rtn, VOID* v) 
{
    std::string rtn_name = RTN_Name(rtn);
    ADDRINT rtn_addr = RTN_Address(rtn);
    int line;
    std::string file_name;
    PIN_GetSourceLocation(rtn_addr, NULL, &line, &file_name);
    routine_name[rtn_addr] = rtn_name;
    if (file_name.empty())
        routine_file[rtn_addr] = "??";
    else
        routine_file[rtn_addr] = file_name;
    routine_line[rtn_addr] = line;

    RTN_Open(rtn);
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)routineBefore, IARG_THREAD_ID, IARG_ADDRINT, rtn_addr, IARG_END);
    RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)routineAfter, IARG_THREAD_ID, IARG_ADDRINT, rtn_addr, IARG_END);
    RTN_Close(rtn);
}

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v) 
{ 
    std::string output_file = KnobOutputFile.Value() + "." + std::to_string(PIN_GetPid());
    output[threadid].open(output_file.c_str());
}

VOID ThreadFini(THREADID threadid, const CONTEXT* ctxt, INT32 code, VOID* v) 
{
    output[threadid].close();
}

VOID ForkBefore(THREADID threadid, const CONTEXT *ctxt, VOID *v)
{
}

VOID ForkAfterInChild(THREADID threadid, const CONTEXT *ctxt, VOID *v)
{
    output.clear();
    std::string output_file = KnobOutputFile.Value() + "." + std::to_string(PIN_GetPid());
    output[threadid].open(output_file.c_str());
}

VOID Fini(INT32 code, VOID* v)
{
}

int main(int argc, char* argv[]) {
    if (PIN_Init(argc, argv))
        return Usage(); 

    PIN_InitSymbols();

    RTN_AddInstrumentFunction(Routine, 0);

    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);
    PIN_AddForkFunction(FPOINT_BEFORE, ForkBefore, 0);
    PIN_AddForkFunction(FPOINT_AFTER_IN_CHILD, ForkAfterInChild, 0);

    PIN_AddFiniFunction(Fini, 0);

    PIN_StartProgram();

    return 0;
}
