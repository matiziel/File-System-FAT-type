#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <iostream>

const unsigned int MAX_NAME = 64;
const unsigned int BLOCK_SIZE = 4096;
const unsigned int MAX_FILE_COUNT = 20;

class FileSystem
{
private:
    struct DirectoryEntry
    {
        char name[MAX_NAME];
        unsigned int size;
        time_t creationDate, modificationDate;
        unsigned int firstBlock;
        bool valid;

        DirectoryEntry()
        {
            valid = false;
        }
    };

    struct SuperBlock
    {
        unsigned int diskSize;
        unsigned int userSpace;         //ilosc wolnych blokow
        unsigned int userSpaceInUse;    //ilosc zajetych blokow
        unsigned int amountBlocks;      //ilosc blokow
        unsigned int fatTableOffset;
        unsigned int dentryOffset;
        unsigned int blocksOffset;
    };

    struct Block
    {
        char block[BLOCK_SIZE];
    };

    SuperBlock *superBlock;
    int *FAT;                                   //-2 pusty, -1 ostatni blok pliku, 0,1,2,... nastepny blok pliku
    DirectoryEntry *fileDescriptors;
    time_t t = time(NULL);

    unsigned long fileSize(FILE* file);
    int saveVirtualDisk(FILE *file_ptr); //return 0 ok, -1 blad pliku, -2 blad alokacji
    int findFirstEmptyBlock();
public:
    FileSystem(): superBlock(nullptr), FAT(nullptr), fileDescriptors(nullptr)
    { }
    FileSystem(unsigned int numberOfBlocks);
    ~FileSystem();

    int createVirtualDisk(); //return 0 -ok, -1 blad pliku, -2 blad alokacji
    int loadVirtualDisk(); //return 0 ok, -1 blad pliku, -2 blad alokacji

    int importFile(char* name);
    int exportFile(char* name, char* exportName);
    //int modifyFile(char* newName);
    int deleteFile(char* name);
    void deleteDisk();

    void displayVirtualDisk();
    void displayFATtable();


    

};

#endif