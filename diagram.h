#ifndef DIAGRAM_H
#define DIAGRAM_H

#include <map>
#include <set>
#include <vector>
#include <list>
#include <numeric>

#include "graph.h"



class Diagram {

  private:
    Graph graph;
      // need to erase elements from tensIdList all the time,
      // so use list instead of vector
    std::vector<uint> tensIdList;
    std::vector<uint> resultIdList;


  public:
    Diagram(const Graph& _graph, const std::vector<uint>& _tensIdList);
    Diagram(const Graph& _graph, const std::vector<uint>& _tensIdList, const std::vector<uint>& _resultIdList);
    Diagram(const Diagram& rhs);

      // ContractionOptimizer::tune relies on this returning a reference
      // to find the maximum tensor ID
    const std::vector<uint>& getRemainingTensors() const { return tensIdList; }
    std::vector<uint> getResultIdList() const { return resultIdList; }
    Graph getGraph() const { return graph; }
    bool isDone() const;

    std::pair<uint, std::list<std::pair<uint, iTup>>> singleTermOpt() const;
    bool replaceSubexpression(uint graphStep,
      			      const std::pair<uint, uint>& globTensPair,
			      uint newGlobTensID);
    void getProfit(const uint graphStep,
		   const iTup& globTensPair,
		   ContractionCost& result);

    //bool operator==(const Diagram& rhs) const {
    //  return LE::multiEqual(graph, rhs.graph,
    //      		    tensIdList, rhs.tensIdList);
    //}

    //bool operator<(const Diagram& rhs) const {
    //  return LE::multiLessThan(graph, rhs.graph,
    //      		       tensIdList, rhs.tensIdList);
    //}

  private:
    void _reorgTensIdList(const iTup& tensPair, uint newGlobId, bool subcDone);
    iTup _getGlobalTensPair(uint stepCode) const;

};




// ***************************************************************
#endif  
