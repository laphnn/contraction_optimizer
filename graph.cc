#include <algorithm>
#include <iostream>
#include "graph.h"


using namespace std;


  /*
   *
   * 	ContractionCost implementation
   *
   * 	We are assuming that
   *
   * 	1) There will never be a contraction with cost N_dil^0 stored here
   * 	2) In operator-= we assume that this > rhs for the result to remain
   * 		an 'unsigned int'. In fact we have the stronger requirement
   * 		that there always be sufficient higher-order contractions
   * 		to borrow from, without having to promote many lower-order
   * 		contractions first.
   *
   */

  ContractionCost& ContractionCost::operator+=(const ContractionCost& rhs) {
    for (unsigned int iC=0; iC < store.size(); ++iC)
      store[iC] += rhs.store[iC];

    return *this;
  }

  ContractionCost& ContractionCost::operator+=(const unsigned int& rhs) {
    store[rhs-1]++;
    return *this;
  }

  ContractionCost& ContractionCost::operator-=(const ContractionCost& rhs) {
    for (unsigned int iC=0; iC < store.size(); ++iC) {
      if (store[iC] < rhs.store[iC]) borrow(iC+1);
      store[iC] -= rhs.store[iC];
    }

    return *this;
  }

  bool ContractionCost::operator<(const ContractionCost& rhs) const {
    auto tmp(store), rhs_tmp(rhs.store);
    for (unsigned int iC = 0; iC < store.size() - 1; ++iC) {
      tmp[iC+1] += tmp[iC] / nDil;
      tmp[iC] %= nDil;
      rhs_tmp[iC+1] += rhs.store[iC] / nDil;
      rhs_tmp[iC] %= nDil;
    }

    return std::lexicographical_compare(tmp.rbegin(), tmp.rend(),
					rhs_tmp.rbegin(), rhs_tmp.rend());
  }

  void ContractionCost::borrow(unsigned int pos) {
      // this will segfault if there no higher positions to borrow from,
      // i.e. don't subtract a bigger number from a smaller number :-)
    if (store[pos] == 0) borrow(pos+1);
    store[pos]--;
    store[pos-1] += nDil;
  }


  uint ContractionCost::nDil = 0;


  /*
   *
   * 	Graph implementation
   *
   */

  Graph::Graph(const std::map<iTup, std::set<iTup>>& contrList) {
    encode(contrList);
  }

  map<iTup, std::set<iTup>> Graph::getContractionList() const {
    map<iTup, std::set<iTup>> ret;
    decode(ret);
    return ret;
  }


    // a graph is encoded in a set of unsigned int, each unsigned int
    // corresponds to one (tens1, tens2) : { <contraction pairs> }
    // and is encoded as follows:
    // 7 x 3 bits to encode the contraction pairs (i1, i2) such that
    // i1 gets encoded by the position in the bit string, and the index
    // it connects to i2 (one-based internally) is written in that position
    // '000' is interpreted as an uncontracted/nonexistent index
    //
    // 4 bits for tens1
    // 4 bits for tens2
    //
    // we define the canonical contraction such that the contraction
    // pair is ordered, tens1 < tens2
  unsigned int Graph::canonicalize(unsigned int aC) const {
    if ((aC & 0xfu) > ((aC >> 4) & 0xfu)) return aC;
    else return reverse(aC);
  }

  unsigned int Graph::reverse(unsigned int aC) const {
    // swap tensor pair
    unsigned int ret = (((aC & 0xfu) << 4) | ((aC >> 4) & 0xfu));
    // swap indices
    for (unsigned int cInd1 = 0; cInd1 < 7; ++cInd1) {
      unsigned int cInd2PlOne = (aC >> (8 + 3*cInd1)) & 0x7;
      if (cInd2PlOne != 0)
	ret |= (cInd1+1) << (8 + (cInd2PlOne-1)*3);
    }
    return ret;
  }

  void Graph::encode(const std::map<iTup, std::set<iTup>>& contrList) {
    icode.clear();

    for (auto mIt : contrList) {
      //if (mIt.second.size() > 7)
	//throw(std::runtime_error("Can't encode contractions for more than 7 quark fields"));

      unsigned int aCode = 0;
      for (auto cIt : mIt.second) {
	aCode |= ((cIt.second+1) & 0x7) << (8 + cIt.first*3);
      }
      aCode |= (mIt.first.first & 0xf) << 4;
      aCode |= mIt.first.second & 0xf;

      icode.insert(canonicalize(aCode));
    }
  }

  std::set<unsigned int> Graph::getTensorIDSet() const {
    std::set<unsigned int> retSet;

    for (auto mIt : icode) {
      retSet.insert(mIt & 0xf);
      retSet.insert((mIt >> 4) & 0xf);
      //unsigned int aCode = mIt;

      //unsigned int tens2 = mIt & 0xf;
      //unsigned int tens1 = (mIt >> 4) & 0xf;
    }

    return retSet;
  }

  std::set<iTup> Graph::decodeElement(unsigned int aC) {
    std::set<iTup> retSet;

    for (unsigned int cInd1 = 0; cInd1 < 7; ++cInd1) {
      unsigned int cInd2 = (aC >> (8+3*cInd1)) & 0x7;
      if (cInd2 != 0)
	retSet.insert(std::make_pair(cInd1, cInd2-1));
    }

    return retSet;
  }

  void Graph::decode(map<iTup, std::set<iTup>>& contrList) const {
    contrList.clear();

    for (auto mIt : icode) {
      //unsigned int aCode = mIt;

      unsigned int tens2 = mIt & 0xf;
      unsigned int tens1 = (mIt >> 4) & 0xf;

      std::set<iTup> contrIndSet = decodeElement(mIt);

      contrList.emplace(std::make_pair(tens1, tens2), contrIndSet);
    }
  }

  /*void Graph::decode(map<iTup, std::set<iTup>>& contrList) const {
    contrList.clear();

    for (auto mIt : icode) {
      //unsigned int aCode = mIt;

      unsigned int tens2 = mIt & 0xf;
      unsigned int tens1 = (mIt >> 4) & 0xf;

      auto iIt = contrList.emplace(std::make_pair(tens1, tens2), std::set<iTup>()).first;

      for (unsigned int cInd1 = 0; cInd1 < 7; ++cInd1) {
	unsigned int cInd2 = (mIt >> (8+3*cInd1)) & 0x7;
	if (cInd2 != 0) 
	  iIt->second.insert(std::make_pair(cInd1, cInd2-1));
      }
    }
  }*/

  unsigned int Graph::isSubexpression(
		  const unsigned int aStep,
		  const std::pair<uint, uint>& tensPair) const {
    unsigned int theStep = (aStep & 0xffffff00u);
    theStep |= (tensPair.first & 0xf) << 4;
    theStep |= tensPair.second & 0xf;
    theStep = canonicalize(theStep);

    if (icode.find(theStep) != icode.end())
      return theStep;
    else
      return 0;
  }


  std::pair<unsigned int, std::vector<unsigned int>> Graph::singleTermOpt() const {
    unsigned int bestReduction = 0, bestCost = 0;
    std::vector<uint> retList;

    for (auto mIt : icode) {
      size_t tensId1 = (mIt >> 4) & 0xfu, tensId2 = mIt & 0xfu;
	// find the reduction that removes as many indices as possible
      unsigned int reduction = decodeElement(mIt).size();

      if (reduction >= bestReduction) {
	  // break ties using cost
	std::vector<unsigned int> tensSizeList = getAllNumInds();
	unsigned int cost = tensSizeList[tensId1] + tensSizeList[tensId2] - reduction;

	if (reduction == bestReduction && cost < bestCost) {
	  retList.clear();
	  bestCost = cost;
	  retList.push_back(mIt);
	}
	else if (reduction > bestReduction) {
	  retList.clear();
	  bestReduction = reduction;
	  bestCost = cost;
	  retList.push_back(mIt);
	}
	else if (cost == bestCost) {
	  retList.push_back(mIt);
	}
      }
    }

    return std::make_pair(bestCost, retList);
  }




  std::vector<unsigned int> Graph::getAllNumInds() const {
    std::map< unsigned int,  unsigned int> tensSizeDict;

    for (auto mIt : icode) {
      unsigned int tensId1 = (mIt >> 4) & 0xfu, tensId2 = mIt & 0xfu;
      auto tIt1 = tensSizeDict.emplace(tensId1, 0).first;
      auto tIt2 = tensSizeDict.emplace(tensId2, 0).first;
      for ( unsigned int cInd1 = 0; cInd1 < 7; ++cInd1) {
	 unsigned int cInd2 = (mIt >> (8+3*cInd1)) & 0x7;
	if (cInd2 != 0) {
	  if (cInd1 >= tIt1->second) tIt1->second = cInd1 + 1;
	  if (cInd2 > tIt2->second) tIt2->second = cInd2;
	}
      }
    }

    std::vector<unsigned int> retVec;

    for (auto tIt : tensSizeDict) {
      retVec.push_back(tIt.second);
    }

    return retVec;
  }

  unsigned int Graph::getNumInds(unsigned int tensId) const {
    unsigned int nInds = 0;

    for (auto mIt : icode) {
        // required tensId is second tensor in pair, find largest contraction
	// index
      if ((mIt & 0xfu) == tensId) {
	for (unsigned int cInd1 = 0; cInd1 < 7; ++cInd1) {
	  if (((mIt >> (8+3*cInd1)) & 0x7) > nInds) {
	    nInds = (mIt >> (8+3*cInd1)) & 0x7;
	  }
	}
      }
        // required tensId is first tensor in pair, find largest nonzero
	// position that's contracted with something
      else if (((mIt >> 4) & 0xfu) == tensId) {
	unsigned int largestActiveInd = (mIt >> 26) ? 7 : (mIt >> 23) ? 6 : (mIt >> 20) ? 5 : (mIt >> 17) ? 4 : (mIt >> 14) ? 3 : (mIt >> 11) ? 2 : 1;
	if (largestActiveInd > nInds) nInds = largestActiveInd;
      }
    }

    return nInds;
  }


  bool Graph::replaceSubexpression(uint replStep) {
      // try the cache
    auto cIt = replCache.find(std::make_pair(*this, replStep));

    if (cIt == replCache.end()) {
	// now we actually have to do the replacement
      replCacheMiss++;
      doReplacement(replStep);
      cIt = replCache.find(std::make_pair(*this, replStep));
    }
    else
      replCacheHit++;


    bool subcDone;
    std::tie(*this, subcDone) = cIt->second;
    return subcDone;
  }

  void Graph::doReplacement(uint replStep) const {
    iTup tensPair((replStep >> 4) & 0xfu, replStep & 0xfu);

      // function returning the new position of a tensor in tensIdList
      // given the old position: the contracted tensors are removed, and
      // the newly added result is appended at the end
    auto getNewTensID = [this,&tensPair] (uint tId) {
      if (tId == tensPair.first || tId == tensPair.second)
	return (unsigned int)getAllNumInds().size()-2;
      uint retId = tId;
      if (tId > tensPair.first) retId--;
      if (tId > tensPair.second) retId--;
      return retId;
    };

    auto contrSet = decodeElement(replStep);
    std::set<uint> idx_remove1, idx_remove2;
    unsigned int nInds1 = getNumInds(tensPair.first);

    for (auto cIt : contrSet) {
      idx_remove1.insert(cIt.first);
      idx_remove2.insert(cIt.second);
    }

      // function returning the new index position of the index in position
      // iInd on tensor tId before the contraction: first come all indices of
      // tensor1 not removed by the contraction, then all such indices of
      // tensor2. If tId is not involved in the current contraction, its index
      // positions are unchanged
    auto getNewIndexPos = [&] (uint tId, uint iInd) {
      if (tId == tensPair.first)
	return iInd - (unsigned int)std::count_if(idx_remove1.begin(),
	    			    idx_remove1.end(),
				    [&](uint el) { return el < iInd; });
      else if (tId == tensPair.second)
	return iInd - (unsigned int)std::count_if(idx_remove2.begin(),
	    			    idx_remove2.end(),
				    [&](uint el) { return el < iInd; }) + nInds1 - (unsigned int)idx_remove1.size();
      else
	return iInd;
    };

     // TODO: I believe this will always yield true, because it only gets called
     // on indices that were not involved in the contraction
    auto isAliveIndex = [&] (uint tId, uint iInd) {
      if (tId == tensPair.first) return (idx_remove1.find(iInd) == idx_remove1.end());
      else if (tId == tensPair.second) return (idx_remove2.find(iInd) == idx_remove2.end());
      else return true;
    };

    auto contrList = getContractionList();
    GraphFactory newGraphFac;
      // flag to check if this is the last (sub)contraction in this graph
    bool subcDone = true;

    for (auto mIt : contrList) {
      iTup cTensPair;
      set<iTup> cSet;
      std::tie(cTensPair, cSet) = mIt;

        // quick check if the tensors involved in mIt are touched by the
	// proposed contraction:
	// 	if mIt is the proposed contraction, simply remove it
	// 	if mIt is between two tensors not involved in the proposed
	// 		contraction, rewrite only the tensor ids
	// 	else we need to rewrite the contraction indices as well
      uint nUntouchedTensors = 2;
      if (cTensPair.first == tensPair.first || cTensPair.first == tensPair.second)
	nUntouchedTensors--;
      if (cTensPair.second == tensPair.first || cTensPair.second == tensPair.second)
	nUntouchedTensors--;

      if (nUntouchedTensors == 0) continue;

      uint newtId1 = getNewTensID(cTensPair.first),
	   newtId2 = getNewTensID(cTensPair.second);

      if (nUntouchedTensors == 2) {
	for (auto cIt : cSet)
	  newGraphFac.addContraction(newtId1, newtId2, cIt.first, cIt.second);
	continue;
      }

        // if we are here, that means that exactly one of the current tensors
	// is involved in the proposed step, so there are other contractions
	// left to do involving the newly formed result
      subcDone = false;

      for (auto cIt : cSet) {
	if (isAliveIndex(cTensPair.first, cIt.first))
	  newGraphFac.addContraction(newtId1, newtId2,
	      			     getNewIndexPos(cTensPair.first, cIt.first),
	      			     getNewIndexPos(cTensPair.second, cIt.second));
	else std::cout << "unexpected not-alive index" << std::endl;
      }
    }

      // add newly found replacement to cache
    Graph newGraph = newGraphFac.getGraph();
    replCache.emplace(std::make_pair(*this, replStep), std::make_pair(newGraph, subcDone));
  }


  void Graph::getProfit(const uint replStep, ContractionCost& result) const {
      // try the cache
    auto cIt = replCache.find(std::make_pair(*this, replStep));

    if (cIt == replCache.end()) {
	// now we actually have to do the replacement
      replCacheMiss++;
      doReplacement(replStep);
      cIt = replCache.find(std::make_pair(*this, replStep));
    }
    else
      replCacheHit++;

    result += getRemainingCost();
    result -= cIt->second.first.getRemainingCost();
  }

  ContractionCost Graph::getRemainingCost() const {
    ContractionCost retCost;

    auto cacheIt = costCache.find(*this);
    if (cacheIt != costCache.end()) {
      costCacheHit++;
      return cacheIt->second;
    }

    costCacheMiss++;

    Graph tmpGraph(*this);

    while (tmpGraph.icode.size() > 0) {
      std::vector<uint> stepList;
      uint cost;

      std::tie(cost, stepList) = tmpGraph.singleTermOpt();

      tmpGraph.replaceSubexpression(stepList.front());
      retCost += cost;
    }

    costCache[*this] = retCost;
    return retCost;
  }

    // replacement cache
  std::map<std::pair<Graph, unsigned int>, std::pair<Graph, bool>> Graph::replCache = std::map<std::pair<Graph, unsigned int>, std::pair<Graph, bool>>();
  uint Graph::replCacheHit = 0;
  uint Graph::replCacheMiss = 0;

    // cost cache
  std::map<Graph, ContractionCost> Graph::costCache = std::map<Graph, ContractionCost>();
  uint Graph::costCacheHit = 0;
  uint Graph::costCacheMiss = 0;

// ***************************************************************
