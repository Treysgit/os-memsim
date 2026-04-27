#include <iostream>
#include <algorithm>
#include <iomanip>
#include "mmu.h"

Mmu::Mmu(int memory_size)
{
    _next_pid = 1024;
    _max_size = memory_size;
}

Mmu::~Mmu()
{
    for (int i = 0; i < _processes.size(); i++)
    {
        for (int j = 0; j < _processes[i]->variables.size(); j++)
        {
            delete _processes[i]->variables[j];
        }
        delete _processes[i];
    }
}

uint32_t Mmu::createProcess()
{
    Process *proc = new Process();
    proc->pid = _next_pid;

    Variable *var = new Variable();
    var->name = "<FREE_SPACE>";
    var->type = DataType::FreeSpace;
    var->virtual_address = 0;
    var->size = _max_size;
    proc->variables.push_back(var);

    _processes.push_back(proc);

    _next_pid++;

    return proc->pid;
}

void Mmu::addVariableToProcess(uint32_t pid, std::string var_name, DataType type, uint32_t size, uint32_t address)
{
    Process *proc = getProcess(pid);

    if (proc != NULL)
    {
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
    std::cout << " PID  | Variable Name | Virtual Addr | Size" << std::endl;
    std::cout << "------+---------------+--------------+------------" << std::endl;

    for (int i = 0; i < _processes.size(); i++)
    {
        for (int j = 0; j < _processes[i]->variables.size(); j++)
        {
            Variable *var = _processes[i]->variables[j];

            if (var->type == DataType::FreeSpace)
            {
                continue;
            }

            std::cout << std::setw(5) << _processes[i]->pid << " | "
                      << std::left << std::setw(13) << var->name << std::right << " |   0x"
                      << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << var->virtual_address
                      << std::dec << std::nouppercase << std::setfill(' ') << " | "
                      << std::setw(10) << var->size << std::endl;
        }
    }
}

Process* Mmu::getProcess(uint32_t pid)
{
    std::vector<Process*>::iterator it = std::find_if(
        _processes.begin(),
        _processes.end(),
        [pid](Process* p){ return p != NULL && p->pid == pid; });

    if (it != _processes.end())
    {
        return *it;
    }

    return NULL;
}

Variable* Mmu::getVariable(uint32_t pid, std::string var_name)
{
    Process *proc = getProcess(pid);

    if (proc == NULL)
    {
        return NULL;
    }

    for (int i = 0; i < proc->variables.size(); i++)
    {
        if (proc->variables[i]->name == var_name && proc->variables[i]->type != DataType::FreeSpace)
        {
            return proc->variables[i];
        }
    }

    return NULL;
}

bool Mmu::variableExists(uint32_t pid, std::string var_name)
{
    return getVariable(pid, var_name) != NULL;
}

bool Mmu::removeVariableFromProcess(uint32_t pid, std::string var_name)
{
    Process *proc = getProcess(pid);

    if (proc == NULL)
    {
        return false;
    }

    for (int i = 0; i < proc->variables.size(); i++)
    {
        if (proc->variables[i]->name == var_name && proc->variables[i]->type != DataType::FreeSpace)
        {
            proc->variables[i]->name = "<FREE_SPACE>";
            proc->variables[i]->type = DataType::FreeSpace;
            proc->variables[i]->values.clear();
            return true;
        }
    }

    return false;
}

bool Mmu::removeProcess(uint32_t pid)
{
    for (int i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            for (int j = 0; j < _processes[i]->variables.size(); j++)
            {
                delete _processes[i]->variables[j];
            }

            delete _processes[i];
            _processes.erase(_processes.begin() + i);
            return true;
        }
    }

    return false;
}

void Mmu::printProcesses()
{
    for (int i = 0; i < _processes.size(); i++)
    {
        std::cout << _processes[i]->pid << std::endl;
    }
}

uint32_t Mmu::getMaxVirtual()
{
    return _max_size;
}
