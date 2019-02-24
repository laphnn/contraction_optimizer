#include "contraction_optimizer.h"

#include <tuple>
#include <algorithm>
#include <iostream>
//#include <functional>


using namespace std;

  ContractionOptimizer::ContractionOptimizer(const std::list<Diagram>& _diagList) :
    	diagList(_diagList), CSECost(), noCSECost() {};


  void _printContractionList(const map<iTup, set<iTup>>& cList) {
    cout<<"contractionList:"<<endl;
    for (auto cIt : cList) {
      cout<<"("<<cIt.first.first<<","<<cIt.first.second<<"): ";
      for (auto aC : cIt.second)
	cout << "["<<aC.first<<","<<aC.second<<"] ";
      cout<<endl;
    }
    cout<<endl;
  }


  void ContractionOptimizer::tune() {
    if (ContractionCost::getDilutionRange()==0)
      throw(std::string("Must set dilution range first."));

      // determine cost without CSE and 
      // smallest global tensor ID we can use for intermediaries
    unsigned int maxTensId = 0;

    for (auto dIt : diagList) {
      noCSECost += dIt.getGraph().getRemainingCost();
      maxTensId = max(maxTensId, *std::max_element(
	    		std::begin(dIt.getRemainingTensors()),
	    		std::end(dIt.getRemainingTensors())));
    }

    unsigned int iDiag = 1, nDiag = diagList.size();
    for (auto dIt = diagList.begin(); dIt != diagList.end(); ++dIt) {
      cout<<"Diagram "<<iDiag<<"/"<<nDiag<<endl;
      ++iDiag;

      while (!dIt->isDone()) {

	  // obtain list of good next steps
	unsigned int stepCost;
	list<pair<uint, iTup>> stepList;
	tie(stepCost, stepList) = dIt->singleTermOpt();

	CSECost += stepCost;

	  // reserve ID for new intermediary
	maxTensId++;

	compStep_t globOptStep;

	  // if there's more than one suggested next step
	if (stepList.size() > 1) {
	  ContractionCost globOptProfit;

	  for (auto sIt : stepList) {
	    ContractionCost globProfit;

	    for (auto ddIt = dIt; ddIt != diagList.end(); ++ddIt) {
	      if (ddIt->isDone()) continue;
	      ddIt->getProfit(sIt.first, sIt.second, globProfit);
	    }

	    if (globOptProfit < globProfit) {
	      globOptProfit = globProfit;
	      globOptStep = make_tuple(sIt.first, sIt.second, maxTensId);
	    }
	  }
	}
	  // if there's only one suggested next step
	else
	  globOptStep = make_tuple(stepList.front().first,
				   stepList.front().second, maxTensId);

	  // store compStep
	compStepList.push_back(globOptStep);

	  // replace the subexpression everywhere
	for (auto ddIt = dIt; ddIt != diagList.end(); ++ddIt) {
	  if (ddIt->isDone()) continue;
	  ddIt->replaceSubexpression(std::get<0>(globOptStep),
				     std::get<1>(globOptStep),
				     std::get<2>(globOptStep));
	}
      } // diagram done
    } // diagram list done
  }
