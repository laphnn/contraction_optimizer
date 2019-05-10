#ifndef DIAGRAM_H
#define DIAGRAM_H

#include <map>
#include <set>
#include <vector>
#include <numeric>

#include "graph.h"



class Diagram {

  private:
    Graph graph;
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

    std::pair<uint, std::vector<std::pair<uint, iTup>>> singleTermOpt() const;
    bool replaceSubexpression(uint graphStep,
      			      const std::pair<uint, uint>& globTensPair,
			      uint newGlobTensID);
    bool getProfit(const uint graphStep,
		   const iTup& globTensPair,
		   ContractionCost& result);

    bool operator==(const Diagram& rhs) const {
      return ((graph == rhs.graph) && (tensIdList == rhs.tensIdList));
    }

    bool operator<(const Diagram& rhs) const {
      return ((graph < rhs.graph) ||
	      (graph == rhs.graph && tensIdList < rhs.tensIdList));
    }

    void _sortTensorList();

  private:
    void _reorgTensIdList(const iTup& tensPair, uint newGlobId, bool subcDone);
    iTup _getGlobalTensPair(uint stepCode) const;
    inline bool _getLocalTensorIDs(const std::pair<uint, uint>& globTensPair, iTup& res);

};




// ***************************************************************
#endif  
