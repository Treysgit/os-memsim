#include <algorithm>
#include <iomanip>
#include "pagetable.h"

#define PHYSICAL_MEMORY 67108864

PageTable::PageTable(int page_size)
{
    _page_size = page_size;
    _max_frames = PHYSICAL_MEMORY / page_size;
}

PageTable::~PageTable()
{
}

std::vector<std::string> PageTable::sortedKeys()
{
    std::vector<std::string> keys;

    std::map<std::string, int>::iterator it;
    for (it = _table.begin(); it != _table.end(); it++)
    {
        keys.push_back(it->first);
    }

    std::sort(keys.begin(), keys.end(), PageTableKeyComparator());

    return keys;
}

bool PageTable::frameIsUsed(int frame)
{
    std::map<std::string, int>::iterator it;
    for (it = _table.begin(); it != _table.end(); it++)
    {
        if (it->second == frame)
        {
            return true;
        }
    }

    return false;
}

bool PageTable::addEntry(uint32_t pid, int page_number)
{
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);

    if (_table.count(entry) > 0)
    {
        return true;
    }

    for (int frame = 0; frame < _max_frames; frame++)
    {
        if (!frameIsUsed(frame))
        {
            _table[entry] = frame;
            return true;
        }
    }

    return false;
}

void PageTable::removeEntry(uint32_t pid, int page_number)
{
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);
    _table.erase(entry);
}

void PageTable::removeProcess(uint32_t pid)
{
    std::vector<std::string> keys_to_delete;

    std::map<std::string, int>::iterator it;
    for (it = _table.begin(); it != _table.end(); it++)
    {
        size_t sep = it->first.find("|");
        uint32_t entry_pid = std::stoi(it->first.substr(0, sep));

        if (entry_pid == pid)
        {
            keys_to_delete.push_back(it->first);
        }
    }

    for (int i = 0; i < keys_to_delete.size(); i++)
    {
        _table.erase(keys_to_delete[i]);
    }
}

int PageTable::getPhysicalAddress(uint32_t pid, uint32_t virtual_address)
{
    int page_number = virtual_address / _page_size;
    int page_offset = virtual_address % _page_size;

    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);

    int address = -1;
    if (_table.count(entry) > 0)
    {
        int frame_number = _table[entry];
        address = frame_number * _page_size + page_offset;
    }

    return address;
}

void PageTable::print()
{
    std::cout << " PID  | Page Number | Frame Number" << std::endl;
    std::cout << "------+-------------+--------------" << std::endl;

    std::vector<std::string> keys = sortedKeys();

    for (int i = 0; i < keys.size(); i++)
    {
        size_t sep = keys[i].find("|");
        uint32_t pid = std::stoi(keys[i].substr(0, sep));
        int page_number = std::stoi(keys[i].substr(sep + 1));
        int frame_number = _table[keys[i]];

        std::cout << std::setw(5) << pid << " | "
                  << std::setw(11) << page_number << " | "
                  << std::setw(12) << frame_number << std::endl;
    }
}

int PageTable::getPageSize()
{
    return _page_size;
}
