#include "graph.h"
#include "diagram.h"
#include "contraction_optimizer.h"

#include <iostream>
#include <fstream>
#include <chrono>

  template <typename T>
    std::ostream& operator<<(std::ostream& output, std::vector<T> const& values)
    {
      output<< "[";
      for (auto const& value : values)
      {
        output << value << ", ";
      }
      output<< "]";
      return output;
    }

int main() {
  ContractionCost::setDilutionRange(64);

  auto graph1 = Graph({
                {{0,2}, {{0,0}, {1,1}}},
                {{1,3}, {{1,1}, {2,2}}},
                {{1,2}, {{0,2}}},
                {{0,3}, {{2,0}}},
        });
  auto graph2 = Graph({
		  {{0,2}, {{0,0}, {1,1}}},
		  {{1,3}, {{1,2}, {2,1}}},
		  {{1,2}, {{0,2}}},
		  {{0,3}, {{2,0}}},
	  });
  std::list<Diagram> diagList;
  diagList.push_back(Diagram(graph1, {0,1,2,3}));
  diagList.push_back(Diagram(graph2, {0,1,2,4}));

  ContractionOptimizer cOp(diagList);

  auto begin = std::chrono::high_resolution_clock::now();
  cOp.tune();
  auto end = std::chrono::high_resolution_clock::now();

  std::cout << "Tuning done, total time = "<<double(std::chrono::duration_cast<std::chrono::milliseconds>(end-begin).count())/1000.<<"s"<<std::endl;
  std::cout << "cost w/o CSE = "<<cOp.getNoCSECost().getCostArray() << std::endl;
  std::cout << "cost w   CSE = "<<cOp.getCSECost().getCostArray() << std::endl<<std::endl;

  std::cout<<"Expected:"<<std::endl
    << "cost w/o CSE = [0, 2, 0, 4, 0, ]"<<std::endl
    << "cost w   CSE = [0, 2, 0, 3, 0, ]"<<std::endl;

}
