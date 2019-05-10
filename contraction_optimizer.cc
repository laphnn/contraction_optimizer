#include "contraction_optimizer.h"

#include <tuple>
#include <algorithm>
#include <iostream>
//#include <functional>


using namespace std;

  ContractionOptimizer::ContractionOptimizer(const std::vector<Diagram>& _diagList) :
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
	    		dIt.getRemainingTensors().begin(),
	    		dIt.getRemainingTensors().end()));
    }

    unsigned int iDiag = 1, nDiag = diagList.size();
    for (auto dIt = diagList.begin(); dIt != diagList.end(); ++dIt) {
      cout<<"Diagram "<<iDiag<<"/"<<nDiag<<"\n";
      ++iDiag;

      while (!dIt->isDone()) {

	  // obtain list of good next steps
	unsigned int stepCost;
	vector<pair<uint, iTup>> stepList;
	tie(stepCost, stepList) = dIt->singleTermOpt();

	CSECost += stepCost;

	  // reserve ID for new intermediary
	maxTensId++;

	compStep_t globOptStep;
	ContractionCost globOptProfit;
	vector<Diagram*> replList;

	  // even if there's only one suggested next step, go through all
	  // diagrams here, and keep track of the diagrams that will need
	  // a replacement in the next step
	for (auto sIt : stepList) {
	  ContractionCost globProfit;
	  vector<Diagram*> tmpReplList;

	  for (auto ddIt = dIt; ddIt != diagList.end(); ++ddIt) {
	    if (ddIt->isDone()) continue;
	      // if the step is a subexpression, track the diagram
	    if(ddIt->getProfit(sIt.first, sIt.second, globProfit))
	      tmpReplList.push_back(&(*ddIt));
	  }

	  if (globOptProfit < globProfit) {
	    globOptProfit = globProfit;
	    globOptStep = make_tuple(sIt.first, sIt.second, maxTensId);
	    replList = tmpReplList;
	  }
	}

	  // store compStep
	compStepList.push_back(globOptStep);

	  // replace the subexpression everywhere
	for (auto ddIt = replList.begin(); ddIt != replList.end(); ++ddIt) {
	  if ((*ddIt)->isDone()) continue;
	  (*ddIt)->replaceSubexpression(std::get<0>(globOptStep),
				     std::get<1>(globOptStep),
				     std::get<2>(globOptStep));
	}
      } // diagram done
    } // diagram list done
  }
