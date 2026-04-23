#include <iostream>
#include <algorithm>
#include "mmu.h"

Mmu::Mmu(int memory_size)
{
    _next_pid = 1024;
    _max_size = memory_size;
}

Mmu::~Mmu()
{
}

uint32_t Mmu::createProcess()
{
    Process *proc = new Process(); // create heap process object
    proc->pid = _next_pid; // assign current PID

    // Initial variable -- just a reference of memory left in RAM
    Variable *var = new Variable(); // create variable object
    var->name = "<FREE_SPACE>"; 
    var->type = DataType::FreeSpace; // label free space for future allocations
    var->virtual_address = 0; // first VA is address 0
    var->size = _max_size; // hard upper bound of virtual memory the process can allocate
    proc->variables.push_back(var); //attach to process

    _processes.push_back(proc); // put new process in vector of processes

    _next_pid++;
    
    return proc->pid;
}

void Mmu::addVariableToProcess(uint32_t pid, std::string var_name, DataType type, uint32_t size, uint32_t address)
{
    std::vector<Process*>::iterator it = std::find_if(
        _processes.begin(), // start of vector
        _processes.end(), // end of vector
        [pid](Process* p) { return p != NULL && p->pid == pid; }); //use pid as capture list to find process

    if (it != _processes.end())
    {
        Process *proc = *it; // store process being assigned variable

        Variable *var = new Variable();
        var->name = var_name;
        var->type = type;
        var->virtual_address = address;
        var->size = size;
        proc->variables.push_back(var);
    }
}

void Mmu::print()
{
    int i, j;

    std::cout << " PID  | Variable Name | Virtual Addr | Size" << std::endl;
    std::cout << "------+---------------+--------------+------------" << std::endl;
    for (i = 0; i < _processes.size(); i++)
    {
        for (j = 0; j < _processes[i]->variables.size(); j++)
        {
            // TODO: print all variables (excluding those of type DataType::FreeSpace)
        }
    }
}
Process* Mmu::getProcess(uint32_t pid)
{
    std::vector<Process*>::iterator it = std::find_if(
        _processes.begin(), //start of vector
        _processes.end(),  //end of vector
        [pid](Process* p){  return p != NULL && p->pid == pid; }); //use pid as capture list to find process

    if (it != _processes.end())
    {
        Process *proc = *it; // store process of pid
    }

    return NULL; // process of pid not found

}

uint32_t Mmu::getMaxVirtual()
{
    return _max_size;
}


