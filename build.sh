#!/usr/bin/env bash
  
g++ -c -g -DNDEBUG -O3 -Wall -std=c++17 diagram.cc -o diagram.o
g++ -c -g -DNDEBUG -O3 -Wall -std=c++17 contraction_optimizer.cc -o contraction_optimizer.o
g++ -c -g -DNDEBUG -O3 -Wall -std=c++17 graph.cc -o graph.o
g++ -g -DNDEBUG -O3 -Wall -std=c++17 driver.cc contraction_optimizer.o diagram.o graph.o
