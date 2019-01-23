#include "FileSystem.h"
#include <iostream>

int main(int argc, char *argv[])
{

    const unsigned int numberOfBlocks = 512;
    FileSystem fs(numberOfBlocks);

    if (argc < 2)
        return 0;

    switch (atoi(argv[1]))
    {
    case 1:
        return fs.createVirtualDisk();
    case 2:
        if (argc != 3)
        {
            return -1;
        }
        fs.loadVirtualDisk();
        return fs.importFile(argv[2]);
    case 3:
        if (argc != 4)
        {
            return -1;
        }

        fs.loadVirtualDisk();
        return fs.exportFile(argv[2], argv[3]);

    case 4:
        fs.loadVirtualDisk();
        fs.displayVirtualDisk();
        return 0;
    case 5:
        fs.loadVirtualDisk();
        fs.displayFATtable();
        return 0;

    case 6:
        if (argc != 3)
        {
            return -1;
        }
        fs.loadVirtualDisk();
        return fs.deleteFile(argv[2]);
    case 7:
        fs.deleteDisk();
    }
    return 0;
}