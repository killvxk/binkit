#pragma once
#pragma warning(disable:4200)
#include <windows.h>
#include <unordered_map>
#include <unordered_set>
#include <list>

#include "Structures.h"
#include "DisassemblyReader.h"

using namespace std;
using namespace stdext;

class Binaries
{
private:
    int m_fileID;
    string m_originalFilePath;

    DisassemblyReader *m_pdisassemblyReader;

public:
    Binaries(DisassemblyReader *DisassemblyReader = NULL);
    ~Binaries();
    void SetFileID(int FileID = 1);
    int GetFileID();
    string GetOriginalFilePath();
};
