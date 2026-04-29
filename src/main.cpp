// Harrison's compile command: g++ -std=c++17 -Iinclude -o bin/memsim src/*.cpp
// Harrison's run command: ./bin/memsim 4096

#include <iostream>
#include <cstdint>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include "mmu.h"
#include "pagetable.h"

// 64 MB (64 * 1024 * 1024)
#define PHYSICAL_MEMORY 67108864

void printStartMessage(int page_size);
void createProcess(int text_size, int data_size, Mmu *mmu, PageTable *page_table);
void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table);
void setVariable(uint32_t pid, std::string var_name, uint32_t offset, std::vector<std::string> values, Mmu *mmu, PageTable *page_table, uint8_t *memory);
void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table);
void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table);

uint32_t getTypeSize(DataType type);
void printVariable(uint32_t pid, std::string var_name, Mmu *mmu);
void printObject(std::string object, Mmu *mmu, PageTable *page_table);

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
        else if(cmd == "set")
        {
            uint32_t pid;
            std::string var_name;
            uint32_t offset;
            user_input >> pid >> var_name >> offset;

            std::vector<std::string> values;
            std::string value;
            while(user_input >> value)
            {
                values.push_back(value);
            }

            setVariable(pid, var_name, offset, values, mmu, page_table, memory);
        }
        else if(cmd == "print")
        {
            std::string object;
            user_input >> object;
            printObject(object, mmu, page_table);
        }
        else if(cmd == "free")
        {
            uint32_t pid;
            std::string var_name;
            user_input >> pid >> var_name;
            freeVariable(pid, var_name, mmu, page_table);
        }
        else if(cmd == "terminate")
        {
            uint32_t pid;
            user_input >> pid;
            terminateProcess(pid, mmu, page_table);
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
        if(!page_table->addEntry(pid, page_index))
        {
            std::cout << "error: allocation exceeds system memory" << std::endl;
            page_table->removeProcess(pid);
            mmu->removeProcess(pid);
            return;
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

    if(process == NULL)
    {
        std::cout << "error: process not found" << std::endl;
        return;
    }

    if(mmu->variableExists(pid, var_name))
    {
        std::cout << "error: variable already exists" << std::endl;
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

    // //first-fit (see if any deallocated spaces are big enough)
    // for(int i = 0 ; i < process->variables.size() ; i++)
    // {
    //     if(process->variables[i]->type == DataType::FreeSpace && process->variables[i]->size >= var_alloc){
    //         new_addr = process->variables[i]->virtual_address; // assign address of free space
    //         mmu->addVariableToProcess(pid, var_name, type, var_alloc, new_addr);
    //         process->variables[i]->virtual_address += var_alloc; //move free space virtual address forward
    //         process->variables[i]->size -= var_alloc; // decrease size of free space

    //         hole_found = 1;
    //         break;

    //     }
    // }
    //first-fit (see if any deallocated variable spaces are big enough)
    // for(int i = 0 ; i < process->variables.size() ; i++)
    // {
    //     if(process->variables[i]->type == DataType::FreeSpace && process->variables[i]->size >= var_alloc){
    //         new_addr = process->variables[i]->virtual_address; // assign address of free space
    //         mmu->addVariableToProcess(pid, var_name, type, var_alloc, new_addr);
    //         process->variables[i]->virtual_address += var_alloc; //move free space virtual address forward
    //         process->variables[i]->size -= var_alloc; // decrease size of free space

    //         hole_found = 1;
    //         break;

    //     }
    // }

    
    // first-fit within already allocated pages
    for(int i = 0; i < process->variables.size(); i++)
    {
        //1st condition, check if variable is a hole
        if(process->variables[i]->type == DataType::FreeSpace)
        {
            uint32_t var_hole_start = process->variables[i]->virtual_address; //variable hole start va
            uint32_t var_hole_end = var_hole_start + process->variables[i]->size; //end va

            uint32_t page_size = page_table->getPageSize();

            uint32_t start_page_num = var_hole_start / page_size; //page number of first page in hole
            uint32_t last_page_num = (var_hole_end - 1) / page_size;//va of last page in hole, truncation division

            //loop through all pages allocated, and look for non-used space in a page
            //find first page with page hole big enough 
            for(uint32_t page_i = start_page_num; page_i <= last_page_num; page_i++)
            {
                uint32_t page_start = page_i * page_size; //va of first byte in page
                uint32_t page_end = page_start + page_size; //va of last byte in page

                //make sure to use only the part of the start/end page belonging to this FREE_SPACE variable
                uint32_t usable_start = std::max(var_hole_start, page_start); 
                uint32_t usable_end = std::min(var_hole_end, page_end);

                // skip if page space not yet allocated a physical address
                // returns < 0 if not
                if(page_table->getPhysicalAddress(pid, page_start) < 0)
                {
                    continue;
                }
                if(usable_start != var_hole_start){
                    continue; 
                }

                //found first hole in a page big enough
                if(usable_end - usable_start >= var_alloc)
                {
                    
                    new_addr = usable_start; //get VA of start of page hole
                    uint32_t alloc_end = new_addr + var_alloc; //get va to update FREE_SPACE this page is in

                    // case 1: whole hole in page used
                    if(new_addr == var_hole_start && alloc_end == var_hole_end)
                    {
                        delete process->variables[i];
                        process->variables.erase(process->variables.begin() + i); //removes variable in processusing iterator at beginning index
                    }
                    // case 2: part of head page of FREE_SPACE used
                    else if(new_addr == var_hole_start)
                    {
                        process->variables[i]->virtual_address = alloc_end; //update FREE_SPACE va
                        process->variables[i]->size = var_hole_end - alloc_end; //update size left of FREE_SPACE
                    }
                    // case 3: used up entire end page hole
                    else if(alloc_end == var_hole_end)
                    {
                        process->variables[i]->size = new_addr - var_hole_start; //update FREE_SPACE size
                    }
                    

                    mmu->addVariableToProcess(pid, var_name, type, var_alloc, new_addr);

                    hole_found = 1;
                    break;
                }
            }
        }
    

        if(hole_found)
        {
            break;
        }
    }




    uint32_t end = 0; //address of the end point of the last variable in VA space
    //allocate new pages if hole not found
     if(!hole_found)
        {
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

            uint32_t page_size = page_table->getPageSize();

            uint32_t first_page = end / page_size; //convert starting address into a page number
            uint32_t last_page = (end + var_alloc - 1) / page_size; //last page the variable will touch

            int pages_needed = 0; 
            // get pages needed
            for(uint32_t page = first_page; page <= last_page; page++) 
            {
                if(page_table->getPhysicalAddress(pid, page * page_size) < 0) 
                {
                    pages_needed++; 
                }
            }

            int frames_free = 0; 
            for(int i = 0; i < PHYSICAL_MEMORY / page_table->getPageSize(); i++) 
            {
                if(!page_table->frameIsUsed(i))
                {
                    frames_free++;
                }
            }

            if(frames_free < pages_needed) 
            {
                std::cout << "error: allocation exceeds system memory" << std::endl; 
                return; 
            }

            for(uint32_t page = first_page; page <= last_page; page++) 
            {
                if(!page_table->addEntry(pid, page)) 
                {
                    std::cout << "error: allocation exceeds system memory" << std::endl; 
                    return; 
                }
            }

        new_addr = end;

        mmu->addVariableToProcess(pid, var_name, type, var_alloc, new_addr);

        for(int i = 0 ; i < process->variables.size() ; i++)
        {
            if(process->variables[i]->type == DataType::FreeSpace && process->variables[i]->virtual_address == end)
            {
                process->variables[i]->virtual_address += var_alloc;
                process->variables[i]->size -= var_alloc;
                break;
            }
        }
    }

    std::cout << new_addr << std::endl;
}






void setVariable(uint32_t pid, std::string var_name, uint32_t offset, std::vector<std::string> values, Mmu *mmu, PageTable *page_table, uint8_t *memory)
{
    Process *process = mmu->getProcess(pid);
    if(process == NULL)
    {
        std::cout << "error: process not found" << std::endl;
        return;
    }

    Variable *var = mmu->getVariable(pid, var_name);
    if(var == NULL)
    {
        std::cout << "error: variable not found" << std::endl;
        return;
    }

    uint32_t type_size = getTypeSize(var->type);
    uint32_t element_count = var->size / type_size;

    if(offset + values.size() > element_count)
    {
        std::cout << "error: index out of range" << std::endl;
        return;
    }

    if(var->values.size() < element_count)
    {
        var->values.resize(element_count, "0");
    }

    for(int i = 0; i < values.size(); i++)
    {
        uint32_t element_index = offset + i;
        var->values[element_index] = values[i];

        uint32_t virtual_address = var->virtual_address + (element_index * type_size);
        int physical_address = page_table->getPhysicalAddress(pid, virtual_address);

        if(physical_address >= 0)
        {
            std::memset(memory + physical_address, 0, type_size);
            std::memcpy(memory + physical_address, values[i].c_str(), std::min((uint32_t)values[i].size(), type_size));
        }
    }
}

void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table)
{
    Process *process = mmu->getProcess(pid);
    if(process == NULL)
    {
        std::cout << "error: process not found" << std::endl;
        return;
    }

    Variable *var = mmu->getVariable(pid, var_name);
    if(var == NULL)
    {
        std::cout << "error: variable not found" << std::endl;
        return;
    }

    uint32_t page_size = page_table->getPageSize();
    uint32_t start_page = var->virtual_address / page_size;
    uint32_t end_page = (var->virtual_address + var->size - 1) / page_size;

    mmu->removeVariableFromProcess(pid, var_name);

    for(uint32_t page = start_page; page <= end_page; page++)
    {
        bool page_has_variable = false;

        for(int i = 0; i < process->variables.size(); i++)
        {
            Variable *current = process->variables[i];
            if(current->type == DataType::FreeSpace)
            {
                continue;
            }

            uint32_t current_start = current->virtual_address;
            uint32_t current_end = current->virtual_address + current->size - 1;
            uint32_t page_start = page * page_size;
            uint32_t page_end = page_start + page_size - 1;

            if(current_start <= page_end && current_end >= page_start)
            {
                page_has_variable = true;
                break;
            }
        }

        if(!page_has_variable)
        {
            // u_int32_t physical_address = page_table->getPhysicalAddress(pid, page * page_size);

            // if (physical_address != -1) {
            //     u_int32_t frame = physical_address / page_size;
            // }
            page_table->removeEntry(pid, page);
            
        }

        for (int i = 0; i < (int)process->variables.size() - 1; i++) {
            if (process->variables[i]->type == DataType::FreeSpace && 
                process->variables[i+1]->type == DataType::FreeSpace) {
                
                process->variables[i]->size += process->variables[i+1]->size;
                
                delete process->variables[i+1];
                process->variables.erase(process->variables.begin() + i + 1);
            
                i--; 
            }
        }
    }
}

void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table)
{
    Process *process = mmu->getProcess(pid);
    if(process == NULL)
    {
        std::cout << "error: process not found" << std::endl;
        return;
    }

    page_table->removeProcess(pid);
    mmu->removeProcess(pid);
}

uint32_t getTypeSize(DataType type)
{
    if(type == DataType::Char)
    {
        return 1;
    }
    else if(type == DataType::Short)
    {
        return 2;
    }
    else if(type == DataType::Int || type == DataType::Float)
    {
        return 4;
    }
    else if(type == DataType::Long || type == DataType::Double)
    {
        return 8;
    }

    return 1;
}

void printVariable(uint32_t pid, std::string var_name, Mmu *mmu)
{
    Process *process = mmu->getProcess(pid);
    if(process == NULL)
    {
        std::cout << "error: process not found" << std::endl;
        return;
    }

    Variable *var = mmu->getVariable(pid, var_name);
    if(var == NULL)
    {
        std::cout << "error: variable not found" << std::endl;
        return;
    }

    uint32_t element_count = var->size / getTypeSize(var->type);

    if(var->values.size() < element_count)
    {
        var->values.resize(element_count, "0");
    }

    uint32_t values_to_print = element_count;
    if(values_to_print > 4)
    {
        values_to_print = 4;
    }

    for(uint32_t i = 0; i < values_to_print; i++)
    {
        if(i > 0)
        {
            std::cout << ", ";
        }
        std::cout << var->values[i];
    }

    if(element_count > 4)
    {
        std::cout << ", ... [" << element_count << " items]";
    }

    std::cout << std::endl;
}

void printObject(std::string object, Mmu *mmu, PageTable *page_table)
{
    if(object == "mmu")
    {
        mmu->print();
    }
    else if(object == "page")
    {
        page_table->print();
    }
    else if(object == "processes")
    {
        mmu->printProcesses();
    }
    else
    {
        size_t colon_position = object.find(":");
        if(colon_position == std::string::npos)
        {
            std::cout << "error: command not recognized" << std::endl;
            return;
        }

        uint32_t pid = std::stoi(object.substr(0, colon_position));
        std::string var_name = object.substr(colon_position + 1);
        printVariable(pid, var_name, mmu);
    }
}
