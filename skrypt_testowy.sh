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
touch virtualdisk
dd if=/dev/urandom of=$name bs=4096 count=256
dd if=/dev/urandom of=$name2 bs=2048 count=123 #dlugosc pliku nie podzielna przez rozmiar bloku 
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
echo "Usuwanie pliku $name"
./lab6 6 $name
./lab6 4
echo "==========================="
echo "===Porownanie md5sum======="
echo "==========================="

md5sum $name1 $name
md5sum $name2 $name3

./lab6 7

for i in $name1 $name $name2 $name3
do
    rm $i
done
