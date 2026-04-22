#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include "mmu.h"
#include "pagetable.h"

// added
#include <sstream>

// 64 MB (64 * 1024 * 1024)
#define PHYSICAL_MEMORY 67108864

void printStartMessage(int page_size);
void createProcess(int text_size, int data_size, Mmu *mmu, PageTable *page_table);
void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table);
void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, uint8_t *memory);
void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table);
void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table);

int main(int argc, char **argv)
{
    // Ensure user specified page size as a command line parameter
    if (argc < 2)
    {
        std::cerr << "Error: you must specify the page size" << std::endl;
        return 1;
    }

    // Print opening instuction message
    int page_size = std::stoi(argv[1]);
    printStartMessage(page_size);

    // Create physical 'memory' (raw array of bytes)
    uint8_t *memory = new uint8_t[PHYSICAL_MEMORY]; // 2^26 bytes

    // Create MMU and Page Table
    Mmu *mmu = new Mmu(PHYSICAL_MEMORY); 
    PageTable *page_table = new PageTable(page_size);

    // Prompt loop
    std::string command;
    std::cout << "> ";
    std::getline(std::cin, command);
    // command = <cmd> <text_size> <data_size>

    std::string cmd;
    int text_size;
    int data_size;

    std::isstringstream user_input(command); // <cmd> <text_size> <data_size>
    user_input >> cmd >> text_size >> data_size;

    while (command != "exit")
    {
        // Handle command
        // TODO: implement this!
        if(cmd == "create")
        {
            if(text_size < 2048 || text_size > 16384)
            {
                // print error
            }
            else if(data_size < 0 || data_size > 1024)
            {
                // print error
            }
            else
            {
                createProcess(text_size, data_size, mmu, page_table);
            }
        }
        else if(cmd == "allocate")
        {
            //implement
        }
        // more else ifs
        else
        {
            // command not recognized
        }


        // Get next command
        std::cout << "> ";
        std::getline(std::cin, command);
    }

    // Cean up
    delete[] memory;
    delete mmu;
    delete page_table;

    return 0;
}

void printStartMessage(int page_size)
{
    std::cout << "Welcome to the Memory Allocation Simulator! Using a page size of " << page_size << " bytes." << std:: endl;
    std::cout << "Commands:" << std:: endl;
    std::cout << "  * create <text_size> <data_size> (initializes a new process)" << std:: endl;
    std::cout << "  * allocate <PID> <var_name> <data_type> <number_of_elements> (allocated memory on the heap)" << std:: endl;
    std::cout << "  * set <PID> <var_name> <offset> <value_0> <value_1> <value_2> ... <value_N> (set the value for a variable)" << std:: endl;
    std::cout << "  * free <PID> <var_name> (deallocate memory on the heap that is associated with <var_name>)" << std:: endl;
    std::cout << "  * terminate <PID> (kill the specified process)" << std:: endl;
    std::cout << "  * print <object> (prints data)" << std:: endl;
    std::cout << "    * If <object> is \"mmu\", print the MMU memory table" << std:: endl;
    std::cout << "    * if <object> is \"page\", print the page table" << std:: endl;
    std::cout << "    * if <object> is \"processes\", print a list of PIDs for processes that are still running" << std:: endl;
    std::cout << "    * if <object> is a \"<PID>:<var_name>\", print the value of the variable for that process" << std:: endl;
    std::cout << std::endl;
}

void createProcess(int text_size, int data_size, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - create new process in the MMU
    //   - allocate new variables for the <TEXT>, <GLOBALS>, and <STACK>
    //   - print pid

    uint32_t pid = mmu->createProcess(); //allocate virtual address space, and get unique pid 

    // get created process object
    Process *process = NULL;
    for(int i = 0 ; i < mmu->_processes() ; i++)
    {
        if(mmu->_processes[i]->pid == 0)
        {
            process = mmu->_processes[i];
            break;
        }
    }

    uint32_t  stack_size = 65536;
    uint32_t full_alloc = text_size + data_size + stack_size;

    //get virtual addresses 
    uint32_t text_va = 0;
    uint32_t globals_va = text_va + text_size;
    uint32_t stack_va = global_va + data_size;

    // assign virtual addresses to each variable
    mmu->addVariableToProcess(pid, "<TEXT>", DataType::int, text_size, text_va);
    mmu->addVariableToProcess(process, "<GLOBALS>", DataType::int, date_size, data_va);
    mmu->addVariableToProcess(process, "<STACK>", DataType::int, date_size, stack_va);



    int page_size = page_table->getPageSize();
    int total_pages = (full_alloc + page_size - 1) / page_size; // get total pages for process, round up division

    // loop addEntry() to allocate needed pages for process
    for(int page_index = 0 ; page_index < total_pages ; page_index++)
    {
        page_table->addEntry(pid, page_number);
    }

    // get created process object
    Process *process = NULL;
    for(int i = 0 ; i < mmu->_processes.size() ; i++)
    {
        if(mmu->_processes[i]->pid == pid && mmu->_processes[i] != NULL)
        {
            process = mmu->_processes[i];
            break;
        }
    }
    //update free space for future virtual addresses
    if(process != NULL)
    {
        // find variable given to process for free space
        for(int i = 0 ; i < process->variables.size() ; i++)
        {
            if(process->variables[i]->type == DataType::FreeSpace)
            {
                //change start index of free space to the end of full allocation. 
                // Free space left is total RAM - full allocation
                process->variables[i]->virtual_address = full_alloc;
                process->variables[i]->size = mmu->_max_size - full_alloc;
                break;
            }
        }
    }

    std::cout << pid << std::endl;

}

void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - find first free space within a page already allocated to this process that is large enough to fit the new variable
    //   - if no hole is large enough, allocate new page(s)
    //   - insert variable into MMU
    //   - print virtual memory address

    // find process object in vector
    Process *process = NULL;
    for(int i = 0 ; i < mmu->_processes.size() ; i++)
    {
        if(mmu->_processes[i]->pid == pid && mmu->_processes[i] != NULL)
        {
            process = mmu->_processes[i];
            break;
        }
    }

    //return if not found
    if(process == NULL)
    {
        return;
    }

    //get size of allocation
    uint32_t type_size = 0;
    if(type == DataType::Char)
    {
        type_size = 1;
    }
    else if(type == DataType::Short)
    {
        type_size = 2;
    }
    else if(type == DataType::Int || type == DataType::Float)
    {
        type_size = 4;
    }
    else if(type == DataType::Long || type == DataType::Double)
    {
        type_size = 8;
    }

    //calculate the type of variable by its quantity
    uint32_t var_alloc = type_size * num_elements;
    uint32_t new_addr = 0; //initial address
    int hole_found = 0;

    //first-fit (see if any deallocated spaces are big enough)
    for(int i = 0 ; process->variables.size() ; i++)
    {
        if(process->variables[i]->type == DataType::FreeSpace && process->variables[i]->size >= var_alloc){
            new_addr = process->variables[i]->virtual_address; // assign address of free space
            mmu->addVariableToProcess(pid, var_name, type, var_alloc, new_addr);
            process->variables[i]->virtual_address += var_alloc; //move free space virtual address forward
            process->variables[i]->size -= var_alloc; // decrease size of free space

            hole_found = 1;
            break;

        }
    }

    //allocate new pages if hole not found
    if(!hole_found){
        for(int i = 0 ; i < process->variables.size() ; i++ )
        //find furthest virtual address given to a variable 
        {
            if(process.variables[i] != DataType::FreeSpace)
            {
                new_addr = process->variables[i]->virtual_address;
            }
        }

    
    }
    //allocate enough new pages for new variable allocation
    

    mmu->addVariableToProcess(process, "<VAR>", DataType::type, var_alloc, new_addr);
    







}

void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, uint8_t *memory)
{
    // TODO: implement this!
    //   - look up physical address for variable based on its virtual address / offset
    //   - insert `value` into `memory` at physical address
    //   * note: this function only handles a single element (i.e. you'll need to call this within a loop when setting
    //           multiple elements of an array)
}

void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - remove entry from MMU
    //   - free page if this variable was the only one on a given page
}

void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - remove process from MMU
    //   - free all pages associated with given process
}
