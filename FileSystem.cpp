#include "FileSystem.h"

//**************************************************************************
//PUBLIC
//**************************************************************************

FileSystem::FileSystem(unsigned int numberOfBlocks)
{
    superBlock = new SuperBlock();
    superBlock->amountBlocks = numberOfBlocks;
    FAT = new int[numberOfBlocks];
    fileDescriptors = new DirectoryEntry[MAX_FILE_COUNT];
}
//**************************************************************************
//**************************************************************************

FileSystem::~FileSystem()
{
    if (superBlock != nullptr)
        delete superBlock;
    if (FAT != nullptr)
        delete[] FAT;
    if (fileDescriptors != nullptr)
        delete[] fileDescriptors;
}

//**************************************************************************
//**************************************************************************

int FileSystem::createVirtualDisk()
{
    FILE *file_ptr;
    file_ptr = fopen("virtualdisk", "wb");

    if (file_ptr == nullptr)
    {
        std::cout << "Blad otwarcia pliku!\n";
        return -1;
    }

    if (superBlock == nullptr || FAT == nullptr || fileDescriptors == nullptr)
    {
        std::cout << "Blad alokacji pamieci!\n";
        fclose(file_ptr);
        return -2;
    }
    unsigned int numberOfBlocks = superBlock->amountBlocks;

    superBlock->diskSize = sizeof(SuperBlock) + numberOfBlocks * sizeof(int) + MAX_FILE_COUNT * sizeof(DirectoryEntry) + numberOfBlocks * sizeof(Block);
    superBlock->userSpace = numberOfBlocks;
    superBlock->userSpaceInUse = 0;
    superBlock->fatTableOffset = sizeof(SuperBlock);
    superBlock->dentryOffset = sizeof(SuperBlock) + numberOfBlocks * sizeof(int);
    superBlock->blocksOffset = sizeof(SuperBlock) + numberOfBlocks * sizeof(int) + MAX_FILE_COUNT * sizeof(DirectoryEntry);

    for (unsigned int i = 0; i < numberOfBlocks; ++i)
    {
        FAT[i] = -2;
    }

    if (fwrite(superBlock, sizeof(SuperBlock), 1, file_ptr) != 1)
    {
        std::cout << "Blad zapisu super bloku do pliku!\n";
        fclose(file_ptr);
        return -1;
    }
    if (fwrite(FAT, sizeof(int), numberOfBlocks, file_ptr) != numberOfBlocks)
    {
        std::cout << "Blad zapisu tablicy FAT do pliku!\n";
        fclose(file_ptr);
        return -1;
    }
    if (fwrite(fileDescriptors, sizeof(DirectoryEntry), MAX_FILE_COUNT, file_ptr) != MAX_FILE_COUNT)
    {
        std::cout << "Blad zapisu tablicy dentry do pliku!\n";
        fclose(file_ptr);
        return -1;
    }

    std::cout << "Pomyslnie utworzono wirtualny dysk.\n";
    fclose(file_ptr);
    return 0;
}

//**************************************************************************
//**************************************************************************

int FileSystem::loadVirtualDisk()
{
    FILE *file_ptr;
    file_ptr = fopen("virtualdisk", "rb");
    if (file_ptr == nullptr)
    {
        std::cout << "Blad otwarcia pliku!\n";
        return -1;
    }
    if (superBlock == nullptr || FAT == nullptr || fileDescriptors == nullptr)
    {
        std::cout << "Blad alokacji pamieci!\n";
        fclose(file_ptr);
        return -2;
    }
    unsigned int numberOfBlocks = superBlock->amountBlocks;
    fseek(file_ptr, 0, 0);

    if (fread(superBlock, sizeof(SuperBlock), 1, file_ptr) != 1)
    {
        std::cout << "Blad odczytu super bloku z pliku!\n";
        fclose(file_ptr);
        return -1;
    }
    if (fread(FAT, sizeof(int), numberOfBlocks, file_ptr) != numberOfBlocks)
    {
        std::cout << "Blad zapisu tablicy FAT z pliku!\n";
        fclose(file_ptr);
        return -1;
    }
    if (fread(fileDescriptors, sizeof(DirectoryEntry), MAX_FILE_COUNT, file_ptr) != MAX_FILE_COUNT)
    {
        std::cout << "Blad zapisu tablicy dentry z pliku!\n";
        fclose(file_ptr);
        return -1;
    }
    std::cout << "Pomyslnie odczytano wirtualny dysk.\n";
    fclose(file_ptr);
    fseek(file_ptr, 0, 0);
    return 0;
}
//**************************************************************************
//**************************************************************************

int FileSystem::importFile(char *name)
{
    FILE *virtualDisk, *newFile;
    int i, firstBlockNumber, numberInDentry;
    for (i = 0; i < MAX_FILE_COUNT; ++i)
    {
        if (!strcmp(fileDescriptors[i].name, name))
        {
            std::cout << "Podany plik istnieje na dysku.\n";
            return 0;
        }
    }
    newFile = fopen(name, "r+b");
    virtualDisk = fopen("virtualdisk", "r+b");
    if (virtualDisk == nullptr)
    {
        std::cout << "Blad otwarcia pliku virtualdisk!\n";
        return -1;
    }
    if (newFile == nullptr)
    {
        std::cout << "Blad otwarcia pliku importowanego!\n";
        return -1;
    }

    int length = strlen(name);
    if (length >= MAX_NAME)
    {
        std::cout << "Za dluga nazwa pliku\n";
        fclose(newFile);
        fclose(virtualDisk);
        return -1;
    }

    int fileSize = FileSystem::fileSize(newFile);
    int howMuchBlocks;
    if (fileSize % BLOCK_SIZE == 0)
        howMuchBlocks = fileSize / BLOCK_SIZE;
    else
        howMuchBlocks = fileSize / BLOCK_SIZE + 1;

    if (howMuchBlocks > superBlock->userSpace)
    {
        std::cout << "Plik za duzy" << std::endl;
        fclose(newFile);
        fclose(virtualDisk);
        return -2;
    }

    firstBlockNumber = findFirstEmptyBlock();
    if (firstBlockNumber == -1)
    {
        std::cout << "Brak wolnych blokow" << std::endl;
        fclose(newFile);
        fclose(virtualDisk);
        return -2;
    }
    numberInDentry = -1;
    for (i = 0; i < MAX_FILE_COUNT; ++i)
    {
        if (fileDescriptors[i].valid == false)
        {
            numberInDentry = i;
            break;
        }
    }
    if (numberInDentry == -1)
    {
        std::cout << "Zbyt duzo plikow na dysku.\n"
                  << std::endl;
        fclose(newFile);
        fclose(virtualDisk);
        return -2;
    }
    fileDescriptors[numberInDentry].firstBlock = firstBlockNumber;
    strcpy(fileDescriptors[numberInDentry].name, name);
    fileDescriptors[numberInDentry].valid = true;
    fileDescriptors[numberInDentry].size = fileSize;
    fileDescriptors[numberInDentry].creationDate = fileDescriptors[numberInDentry].modificationDate = t;

    int tmp = firstBlockNumber;
    Block *tmpBlock = new Block();

    for (i = 0; i < howMuchBlocks; ++i)
    {
        firstBlockNumber = findFirstEmptyBlock();

        if (firstBlockNumber == -1)
        {
            std::cout << "Brak wolnych blokow" << std::endl;
            fclose(newFile);
            fclose(virtualDisk);
            return -2;
        }
        fseek(virtualDisk, superBlock->blocksOffset + firstBlockNumber * sizeof(Block), 0);
        if (fread(tmpBlock->block, BLOCK_SIZE, 1, newFile) != 1 && !feof(newFile))
        {
            printf("Error. Failed to read data from file\n");
            fclose(newFile);
            fclose(virtualDisk);
            delete tmpBlock;
            return -2;
        }
        if (fwrite(tmpBlock->block, sizeof(Block), 1, virtualDisk) != 1)
        {
            printf("Error. Failed to write data from file\n");
            fclose(newFile);
            fclose(virtualDisk);
            delete tmpBlock;
            return -2;
        }
        if (i != 0)
            FAT[tmp] = firstBlockNumber;
        superBlock->userSpace--;
        superBlock->userSpaceInUse++;
        FAT[firstBlockNumber] = -1;
        tmp = firstBlockNumber;
    }
    if (saveVirtualDisk(virtualDisk))
        std::cout << "Nie udalo sie zapisac\n";
    delete tmpBlock;
    fseek(virtualDisk, 0, 0);
    fclose(newFile);
    fclose(virtualDisk);
    std::cout << "Import zakonczony sukcesem\n";
    return 0;
}

//**************************************************************************
//**************************************************************************

int FileSystem::exportFile(char *name, char* exportName)
{
    FILE *virtualDisk, *newFile;
    int numberInDentry = -1;
    int i;
    int howMuchBlocks;
    for (i = 0; i < MAX_FILE_COUNT; ++i)
    {
        if (!strcmp(fileDescriptors[i].name, name))
        {
            std::cout<<"Wchodzi"<<std::endl;
            if (fileDescriptors[i].valid == true)
            {
                std::cout<<"Wchodzi222"<<std::endl;
                numberInDentry = i;
                break;
            }
        }
    }
    if (numberInDentry == -1)
    {
        std::cout << "Plik nie istnieje!\n";
        return -1;
    }

    if (fileDescriptors[numberInDentry].size % BLOCK_SIZE == 0)
        howMuchBlocks = fileDescriptors[numberInDentry].size / BLOCK_SIZE;
    else
        howMuchBlocks = fileDescriptors[numberInDentry].size / BLOCK_SIZE + 1;

    virtualDisk = fopen("virtualdisk", "r+b");
    if (virtualDisk == nullptr)
    {
        std::cout << "Blad otwarcia pliku virtualdisk!\n";
        return -1;
    }
    newFile = fopen(exportName, "wb");
    fseek(newFile, 0, 0);
    if (newFile == nullptr)
    {
        std::cout << "Blad otwarcia pliku do exportu!\n";
        return -1;
    }
    Block *tmpBlock = new Block();

    if (tmpBlock == nullptr)
    {
        std::cout << "Blad alokacji!\n";
        return -2;
    }
    int firstBlock = fileDescriptors[numberInDentry].firstBlock;
    int tmp = firstBlock;
    for (i = 0; i < howMuchBlocks; ++i)
    {
        fseek(virtualDisk, superBlock->blocksOffset + tmp * sizeof(Block), 0);
        fread(tmpBlock->block, sizeof(Block), 1, virtualDisk);
        if (tmp != -1)
            tmp = FAT[tmp];

        if (fwrite(tmpBlock->block, sizeof(Block), 1, newFile) != 1)
        {
            std::cout << "Blad exportu do pliku!\n";
            fclose(virtualDisk);
            fclose(newFile);
            return -1;
        }
    }
    delete tmpBlock;
    fseek(virtualDisk, 0, 0);
    fclose(virtualDisk);
    fclose(newFile);
    std::cout << "Export zakonczony sukcesem\n";
    return 0;
}

//**************************************************************************
//**************************************************************************
void FileSystem::displayVirtualDisk()
{
    if (superBlock == nullptr || FAT == nullptr || fileDescriptors == nullptr)
    {
        std::cout << "Blad alokacji pamieci!\n";
        return;
    }
    std::cout << "//**************************************************************************//" << std::endl;
    std::cout << "Rozmiar dysku:" << superBlock->diskSize << std::endl;
    std::cout << "Ilosc blokow:" << superBlock->amountBlocks << std::endl;
    std::cout << "Ilosc wolnych blokow:" << superBlock->userSpace << std::endl;
    std::cout << "Ilosc zajetych blokow:" << superBlock->userSpaceInUse << std::endl;
    std::cout << "Wszystkie pliki:" << std::endl;
    for (unsigned int i = 0; i < MAX_FILE_COUNT; ++i)
    {
        if (fileDescriptors[i].valid != false)
        {
            std::cout << "Nazwa pliku: " << fileDescriptors[i].name;
            std::cout << " Rozmiar: " << fileDescriptors[i].size;
            std::cout << " Data utworzenia: " << fileDescriptors[i].creationDate;
            std::cout << " Data modyfikacji: " << fileDescriptors[i].modificationDate;
            std::cout << " Pierwszy blok: " << fileDescriptors[i].firstBlock << "\n";
        }
    }
    std::cout << "//**************************************************************************//" << std::endl;
}

//**************************************************************************
//**************************************************************************

void FileSystem::displayFATtable()
{
    if (superBlock == nullptr || FAT == nullptr || fileDescriptors == nullptr)
    {
        std::cout << "Blad alokacji pamieci!\n";
        return;
    }

    std::cout << "TABLICA FAT:" << std::endl;
    for (unsigned int i = 0; i < superBlock->amountBlocks; ++i)
    {
        if (FAT[i] != -2)
        {
            std::cout << "Numer w tablicy: " << i << ", wartosc: " << FAT[i] << "\n";
        }
    }
}

//**************************************************************************
//**************************************************************************

int FileSystem::deleteFile(char *name)
{
    FILE *virtualDisk;
    int numberInDentry = -1;
    int i;
    int howMuchBlocks;
    for (i = 0; i < MAX_FILE_COUNT; ++i)
    {
        if (!strcmp(fileDescriptors[i].name, name))
        {
            numberInDentry = i;
            break;
        }
    }
    if (numberInDentry == -1)
    {
        std::cout << "Plik nie istnieje!\n";
        return -1;
    }
    int fileSize = fileDescriptors[numberInDentry].size;
    std::cout<<"Rozmiar "<<fileSize<<std::endl;
    if (fileSize % BLOCK_SIZE == 0)
        howMuchBlocks = fileSize / BLOCK_SIZE;
    else
        howMuchBlocks = fileSize / BLOCK_SIZE + 1;

    virtualDisk = fopen("virtualdisk", "r+b");
    if (virtualDisk == nullptr)
    {
        std::cout << "Blad otwarcia pliku virtualdisk!\n";
        return -1;
    }
    Block *tmpBlock = new Block();

    if (tmpBlock == nullptr)
    {
        std::cout << "Blad alokacji!\n";
        return -2;
    }
    for (int j = 0; j < BLOCK_SIZE; ++j)
    {
        tmpBlock->block[j] = '\0';
    }
    int firstBlock = fileDescriptors[numberInDentry].firstBlock;
    fileDescriptors[numberInDentry].valid = false;
    int tmp = firstBlock;
    int xtmp;
    std::cout<<"Ilosc blokow"<<howMuchBlocks<<std::endl;
    for (int j = 0; j < howMuchBlocks; ++j)
    {
        fseek(virtualDisk, superBlock->blocksOffset + tmp * sizeof(Block), 0);
        if (tmp != -1)
        {
            xtmp = FAT[tmp];
            FAT[tmp] = -2;
            tmp = xtmp;
        }
        else
        {
            FAT[tmp] = -2;
            break; 
        }
        superBlock->userSpace++;
        superBlock->userSpaceInUse--;

        if (fwrite(tmpBlock->block, sizeof(Block), 1, virtualDisk) != 1)
        {
            std::cout << "Blad usuwania pliku!\n";
            fclose(virtualDisk);
            return -1;
        }
    }
    if (saveVirtualDisk(virtualDisk))
        std::cout << "Nie udalo sie zapisac\n";
    delete tmpBlock;
    fclose(virtualDisk);

    std::cout << "Usuwanie elementu sukcesem!\n";
    return 0;
}

//**************************************************************************
//**************************************************************************

void FileSystem::deleteDisk()
{
    FILE *file_ptr;
    file_ptr = fopen("virtualdisk", "wb");
    fclose(file_ptr);
}

//**************************************************************************
//PRIVATE
//**************************************************************************

unsigned long FileSystem::fileSize(FILE *file)
{
    unsigned long size;
    fseek(file, 0, SEEK_END);
    size = (unsigned long)ftell(file);
    rewind(file);
    return size;
}

//**************************************************************************
//**************************************************************************

int FileSystem::saveVirtualDisk(FILE *file_ptr)
{
    if (superBlock == nullptr || FAT == nullptr || fileDescriptors == nullptr)
    {
        std::cout << "Blad alokacji pamieci!\n";
        fclose(file_ptr);
        return -2;
    }
    unsigned int numberOfBlocks = superBlock->amountBlocks;
    fseek(file_ptr, 0, 0);
    if (fwrite(superBlock, sizeof(SuperBlock), 1, file_ptr) != 1)
    {
        std::cout << "Blad zapisu super bloku do pliku!\n";
        fclose(file_ptr);
        return -1;
    }
    fseek(file_ptr, superBlock->fatTableOffset, 0);
    if (fwrite(FAT, sizeof(int), numberOfBlocks, file_ptr) != numberOfBlocks)
    {
        std::cout << "Blad zapisu tablicy FAT do pliku!\n";
        fclose(file_ptr);
        return -1;
    }
    fseek(file_ptr, superBlock->dentryOffset, 0);
    if (fwrite(fileDescriptors, sizeof(DirectoryEntry), MAX_FILE_COUNT, file_ptr) != MAX_FILE_COUNT)
    {
        std::cout << "Blad zapisu tablicy dentry do pliku!\n";
        fclose(file_ptr);
        return -1;
    }
    fseek(file_ptr, 0, 0);
    std::cout << "Pomyslnie zapisano dane na wirtualny dysk.\n";
    return 0;
}
//**************************************************************************
//**************************************************************************

int FileSystem::findFirstEmptyBlock()
{
    for (int i = 0; i < superBlock->amountBlocks; ++i)
    {
        if (FAT[i] == -2) //pierwszy wolny blok
        {
            return i;
        }
    }
    return -1;
}