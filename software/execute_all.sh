#!/bin/bash

echo '------------IB35---------------\n' > salida.txt
./program data/ibex_35.csv 0.005 0.08 50 42 ../ib35.csv >> salida.txt
echo '------------S&P100---------------\n' >> salida.txt
./program data/syp_100.csv 0.005 0.05 50 42 ../syp100.csv >> salida.txt
echo '------------S&P500---------------\n' >> salida.txt
./program data/syp_500.csv 0.005 0.02 50 42 ../syp500.csv >> salida.txt

