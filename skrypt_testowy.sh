#!/bin/bash
name="duzy_plik.txt"
name1="nowyduzy.txt"
name2="plikTrzeci.txt"
name3="nowyTrzeci.txt"
echo "==================="
echo "======TESTING======"
echo "==================="
echo "==================="
echo "====KOMPILACJA====="
echo "==================="
make all
echo "======================"
echo "===TWORZENIE PLIKOW==="
echo "======================"
dd if=/dev/urandom of=$name bs=4096 count=256
dd if=/dev/urandom of=$name2 bs=4096 count=128
echo "==========================="
echo "TWORZENIE WIRTUALNEGO DYSKU"
echo "==========================="
./lab6 1
echo "==========================="
echo "==========IMPORT==========="
echo "==========================="
./lab6 2 $name
./lab6 2 $name2
./lab6 4
echo "==========================="
echo "==========EXPORT==========="
echo "==========================="
./lab6 3 $name $name1
./lab6 3 $name2 $name3
./lab6 4
echo "==========================="
echo "========USUWANIE==========="
echo "==========================="

./lab6 6 $name
./lab6 4
echo "==========================="
echo "===Porownanie md5sum======="
echo "==========================="

md5sum $name1 $name
md5sum $name2 $name3

./lab6 7
