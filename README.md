# LHEtest

The program "prova01" extracts data from a set of .lhe files, creating a TTree.

To compile it: 
```
c++ -o prova01 prova01.cpp `root-config --cflags --glibs`
```
To run the program:
```
./prova01 output.root file1.lhe file2.lhe ..
```
A .lhe file is given in test.lhe.tar.xz
