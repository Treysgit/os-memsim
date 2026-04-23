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


    while (command != "exit")
    {
        std::string cmd;

        std::istringstream user_input(command); // <cmd> <text_size> <data_size>
        user_input >> cmd; // first arg

        if(cmd == "create")
        {
            int text_size;
            int data_size;
            user_input >> text_size >> data_size; //2nd and 3rd args

            if(text_size < 2048 || text_size > 16384)
            {
                std::cout << "text size out of range" << std::endl;

            }
            else if(data_size < 0 || data_size > 1024)
            {
                std::cout << "data size out of range" << std::endl;
            }
            else
            {
                createProcess(text_size, data_size, mmu, page_table);
            }
        }
        else if(cmd == "allocate")
        {
            uint32_t pid;
            std::string var_name;
            std::string data_type;
            uint32_t number_of_elements;

            user_input >> pid >> var_name >> data_type >> number_of_elements;

            DataType var_type;
            if(data_type == "char")
            {
                var_type = DataType::Char;
            }
            else if(data_type == "short")
            {
                var_type = DataType::Short;
            }
            else if(data_type == "int")
            {
                var_type = DataType::Int;
            }
            else if(data_type == "long")
            {
                var_type = DataType::Long;
            }
            else if(data_type == "float")
            {
                var_type = DataType::Float;
            }
            else if(data_type == "double")
            {
                var_type = DataType::Double;
            }
            else
            {
                std::cout << "Invalid variable data type" << std::endl;

                //reprompt and restart loop
                std::cout << "> ";
                std::getline(std::cin, command);
                continue;
            }

            allocateVariable(pid, var_name, var_type, number_of_elements, mmu, page_table);
            

        }

        //  else ifs for other commands
        else if(cmd == "set")
        {
            // implement
        }
        else if(cmd == "print")
        {
            // implement
        }
        else if(cmd == "exit")
        {
            // implement
        }
        else if(cmd == "free")
        {
            // implement
        }
        else if(cmd == "terminate")
        {
            // implement
        }
        else if(cmd != "exit")
        {
            std::cout << "error: command not recognized" << std::endl;
        }


        // reprompt
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
    Process *process = mmu->getProcess(pid); 

    uint32_t  stack_size = 65536;
    uint32_t full_alloc = text_size + data_size + stack_size;

    //get virtual addresses 
    uint32_t text_va = 0;
    uint32_t globals_va = text_va + text_size;
    uint32_t stack_va = globals_va + data_size;

    // assign virtual addresses to each variable
    DataType default_type = Int;
    mmu->addVariableToProcess(pid, "<TEXT>", default_type, text_size, text_va);
    mmu->addVariableToProcess(pid, "<GLOBALS>", default_type, data_size, globals_va);
    mmu->addVariableToProcess(pid, "<STACK>", default_type, stack_size, stack_va);



    uint32_t page_size = page_table->getPageSize();
    uint32_t total_pages = (full_alloc + page_size - 1) / page_size; // get total pages for process, round up division

    // loop addEntry() to allocate needed pages for process
    for(int page_index = 0 ; page_index < total_pages ; page_index++)
    {
        page_table->addEntry(pid, page_index);
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
                // Free space left is total virtual - full allocation
                process->variables[i]->virtual_address = full_alloc;
                process->variables[i]->size = mmu->getMaxVirtual() - full_alloc;
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
    Process *process = mmu->getProcess(pid);

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
    for(int i = 0 ; i < process->variables.size() ; i++)
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
    uint32_t end = 0; //address of the end point of the last variable in VA space
    //allocate new pages if hole not found
    if(!hole_found)
    {

        //loop through each variable, skipping FreeSpace holes. Largest VA + size is the end point.
        for(int i = 0 ; i < process->variables.size() ; i++ )
        {
            if(process->variables[i]->type != DataType::FreeSpace)
            {
                uint32_t end_guess = process->variables[i]->virtual_address + process->variables[i]->size;

                if(end_guess > end)
                {
                    end = end_guess;
                }
            }


        }
        //allocate enough new pages for new variable allocation
        uint32_t page_size = page_table->getPageSize();
        uint32_t total_pages = (var_alloc + page_size - 1) / page_size; // get total new pages for process, round up division
        uint32_t end_page = end / page_size; //get the end page to append new pages to

        // loop addEntry() to allocate new pages to process, starting at end page
        for(int i = 0 ; i < total_pages ; i++)
        {
            page_table->addEntry(pid, end_page + i); // addEntry(pid, page_number)
        }

        //update variables virtual address to where we started allocation
        new_addr = end;

        //put actual variable in process
        mmu->addVariableToProcess(pid, var_name, type, var_alloc, new_addr);

        //update free space area
        for(int i = 0 ; i < process->variables.size() ; i++)
            {
                if(process->variables[i]->type == DataType::FreeSpace && process->variables[i]->virtual_address == end)
                {
                    //change start index of free space area to the end of variable allocation. 
                    process->variables[i]->virtual_address += var_alloc;
                    process->variables[i]->size -= var_alloc; //decrememnt memory left in RAM
                    break;
                }
            }
        
    
    }
    std::cout << new_addr << std::endl;






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
