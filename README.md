# LHEtest

The program "prova01" extracts data from a .lhe file, creating a TTree.

To compile it: 
```
c++ -o prova01 prova01.cpp `root-config --cflags --glibs`
```
To run the program:
```
./prova01 file.lhe
```
The .root output file is named LHETree.root
