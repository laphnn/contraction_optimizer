#!/usr/bin/env bash
  
g++ -c -g -DNDEBUG -O3 -Wall -std=c++17 -fPIC -DDOUBLEPRECISION -I${LAPH_BACKEND_PATH}/include -L${LAPH_BACKEND_PATH}/lib/ diagram.cc -o diagram.o
g++ -c -g -DNDEBUG -O3 -Wall -std=c++17 -fPIC -DDOUBLEPRECISION -I${LAPH_BACKEND_PATH}/include -L${LAPH_BACKEND_PATH}/lib/ contraction_optimizer.cc -o contraction_optimizer.o
g++ -c -g -DNDEBUG -O3 -Wall -std=c++17 -fPIC -DDOUBLEPRECISION -I${LAPH_BACKEND_PATH}/include -L${LAPH_BACKEND_PATH}/lib/ graph.cc -o graph.o
g++ -g -DNDEBUG -O3 -Wall -std=c++17 -fPIC -DDOUBLEPRECISION -I${LAPH_BACKEND_PATH}/include -L${LAPH_BACKEND_PATH}/lib/ driver.cc contraction_optimizer.o diagram.o graph.o
