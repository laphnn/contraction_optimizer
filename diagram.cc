#include "diagram.h"

#include <algorithm>
#include <functional>


using namespace std;


  /*
   *
   * 	Diagram implementation
   *
   */


  Diagram::Diagram(const Graph& _graph,
      		   const std::vector<uint>& _tensIdList) :
    	graph(_graph), tensIdList(_tensIdList) {}

  Diagram::Diagram(const Graph& _graph,
      		   const std::vector<uint>& _tensIdList,
		   const std::vector<uint>& _resultIdList) :
    	graph(_graph), tensIdList(_tensIdList), resultIdList(_resultIdList) {}

  Diagram::Diagram(const Diagram& rhs) :
    	graph(rhs.graph), tensIdList(rhs.tensIdList), resultIdList(rhs.resultIdList) {}


  bool Diagram::isDone() const {
#ifdef SAFETY_FLAG
    if (tensIdList.empty() != graph.getContractionList().empty())
      throw("Inconsistent tensIdList and graph.");
#endif
    return tensIdList.empty();
  }

  bool Diagram::replaceSubexpression(const uint graphStep,
      				     const std::pair<uint, uint>& globTensPair,
				     const uint newGlobTensID) {
      // check if the tensors in the proposed step occur in this diagram
    auto tIt1 = std::find(tensIdList.begin(), tensIdList.end(), globTensPair.first);
    if (tIt1 == tensIdList.end()) return false;
    auto tIt2 = std::find(tensIdList.begin(), tensIdList.end(), globTensPair.second);
    if (tIt2 == tensIdList.end()) return false;
    
    iTup tensPair = std::make_pair(
	std::distance(tensIdList.begin(), tIt1),
	std::distance(tensIdList.begin(), tIt2));

    /*uint pos1 = 1234, pos2 = 1234;
    for (auto iP=0u; iP < tensIdList.size(); iP++) {
      if (tensIdList[iP] == globTensPair.first) {
	pos1 = iP;
	if (pos2 < 1234) break;
      }
      else if (tensIdList[iP] == globTensPair.second) {
	pos2 = iP;
	if (pos1 < 1234) break;
      }
    }
    if (pos1 == 1234 || pos2 == 1234) return false;

    iTup tensPair = pair(pos1, pos2);*/

      // check if the proposed contraction occurs in the diagram
    uint replStep = graph.isSubexpression(graphStep, tensPair);

    if (replStep == 0)
      return false;

    bool subcDone = graph.replaceSubexpression(replStep);
    _reorgTensIdList(tensPair, newGlobTensID, subcDone);
    return true;
  }

  void Diagram::getProfit(const uint graphStep,
			  const iTup& globTensPair,
		   	  ContractionCost& result) {
      // check if the tensors in the proposed step occur in this diagram
    auto tIt1 = std::find(tensIdList.begin(), tensIdList.end(), globTensPair.first);
    if (tIt1 == tensIdList.end()) return;
    auto tIt2 = std::find(tensIdList.begin(), tensIdList.end(), globTensPair.second);
    if (tIt2 == tensIdList.end()) return;

    iTup tensPair = std::make_pair(
	std::distance(tensIdList.begin(), tIt1),
	std::distance(tensIdList.begin(), tIt2));

      // check if the proposed contraction occurs in the diagram
    uint replStep = graph.isSubexpression(graphStep, tensPair);

    if (replStep == 0)
      return;

     // it's a subexpression, go get profit from Graph
    graph.getProfit(replStep, result);
  }



  iTup Diagram::_getGlobalTensPair(uint stepCode) const {
    unsigned int tId1 = (stepCode >> 4) & 0xf;
    unsigned int tId2 = stepCode & 0xf;

    if (tId1 < tId2) {
      auto lIt = tensIdList.begin();
      std::advance(lIt, tId1);
      unsigned int globId1 = *lIt;
      std::advance(lIt, tId2-tId1);
      return std::make_pair(globId1, *lIt);
    }
    else {
      auto lIt = tensIdList.begin();
      std::advance(lIt, tId2);
      unsigned int globId2 = *lIt;
      std::advance(lIt, tId1-tId2);
      return std::make_pair(*lIt, globId2);
    }
  }


  std::pair<uint, std::list<std::pair<uint, iTup>>> Diagram::singleTermOpt() const {
    std::vector<uint> stepList;
    uint cost;
    std::tie(cost, stepList) = graph.singleTermOpt();

    std::list<std::pair<uint, iTup>> globStepList;
    for (auto mIt : stepList)
      globStepList.push_back(std::make_pair(mIt & 0xffffff00u, _getGlobalTensPair(mIt)));

    return std::make_pair(cost, globStepList);
  }


  void Diagram::_reorgTensIdList(const iTup& tensPair, uint newGlobId, bool subcDone) {
    auto lIt = tensIdList.begin();
      // remove the two tensors indexed by tensPair
    if (tensPair.first > tensPair.second) {
      std::advance(lIt, tensPair.second);
      lIt = tensIdList.erase(lIt);
      std::advance(lIt, tensPair.first-tensPair.second-1);
      tensIdList.erase(lIt);
    }
    else {
      std::advance(lIt, tensPair.first);
      lIt = tensIdList.erase(lIt);
      std::advance(lIt, tensPair.second-tensPair.first-1);
      tensIdList.erase(lIt);
    }

      // add new tensor ID either to result list or tensIdList
    if (subcDone)
      resultIdList.push_back(newGlobId);
    else
      tensIdList.push_back(newGlobId);
  }
