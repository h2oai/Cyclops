/*
 * HierarchyAutoSearchCrossValidationDriver.cpp
 *
 *  Created on: April 10, 2014
 *      Author: Trevor Shaddox
 */

#include <iostream>
#include <iomanip>
#include <numeric>
#include <math.h>
#include <cstdlib>

#include "HierarchyAutoSearchCrossValidationDriver.h"
#include "AutoSearchCrossValidationDriver.h"
#include "CyclicCoordinateDescent.h"
#include "CrossValidationSelector.h"
#include "AbstractSelector.h"
//#include "ccd.h"
#include "../utils/HParSearch.h"

namespace bsccs {

//const static int MAX_STEPS = 50;

HierarchyAutoSearchCrossValidationDriver::HierarchyAutoSearchCrossValidationDriver(const ModelData& _modelData,
		int iGridSize,
		double iLowerLimit,
		double iUpperLimit,
		loggers::ProgressLoggerPtr _logger,
		loggers::ErrorHandlerPtr _error,		
		vector<real>* wtsExclude) : AutoSearchCrossValidationDriver(
				_modelData,
				iGridSize,
				iLowerLimit,
				iUpperLimit,
				_logger,
				_error,
				wtsExclude)
			 {}

HierarchyAutoSearchCrossValidationDriver::~HierarchyAutoSearchCrossValidationDriver() {
	// Do nothing
}

void HierarchyAutoSearchCrossValidationDriver::resetForOptimal(
		CyclicCoordinateDescent& ccd,
		CrossValidationSelector& selector,
		const CCDArguments& arguments) {

	ccd.setWeights(NULL);
	ccd.setHyperprior(maxPoint);
	ccd.setClassHyperprior(maxPointClass);
	ccd.resetBeta(); // Cold-start
}


void HierarchyAutoSearchCrossValidationDriver::drive(
		CyclicCoordinateDescent& ccd,
		AbstractSelector& selector,
		const CCDArguments& arguments) {

	// TODO Check that selector is type of CrossValidationSelector
	std::vector<real> weights;


	double tryvalue = modelData.getNormalBasedDefaultVar();
	double tryvalueClass = tryvalue; // start with same variance at the class and element level; // for hierarchy class variance
	UniModalSearch searcher(10, 0.01, log(1.5));
	UniModalSearch searcherClass(10, 0.01, log(1.5)); // Need a better way to do this.

//	const double eps = 0.05; //search stopper
    std::ostringstream stream;
	stream << "Default var = " << tryvalue;
	logger->writeLine(stream);


	bool finished = false;
	bool drugLevelFinished = false;
	bool classLevelFinished = false;

	int step = 0;
	while (!finished) {

		// More hierarchy logic
		ccd.setHyperprior(tryvalue);
		ccd.setClassHyperprior(tryvalueClass);

		std::vector<double> predLogLikelihood;

		// Newly re-located code
		double pointEstimate = doCrossValidation(ccd, selector, arguments, step, predLogLikelihood);

		double stdDevEstimate = computeStDev(predLogLikelihood, pointEstimate);

        std::ostringstream stream;
		stream << "AvgPred = " << pointEstimate << " with stdev = " << stdDevEstimate;
		logger->writeLine(stream);


        // alternate adapting the class and element level, unless one is finished
        if ((step % 2 == 0 && !drugLevelFinished) || classLevelFinished){
        	searcher.tried(tryvalue, pointEstimate, stdDevEstimate);
        	pair<bool,double> next = searcher.step();
        	tryvalue = next.second;
        	std::ostringstream stream;        	
            stream << "Next point at " << next.second << " and " << next.first;
            logger->writeLine(stream);
            if (!next.first) {
               	drugLevelFinished = true;
            }
       	} else {
       		searcherClass.tried(tryvalueClass, pointEstimate, stdDevEstimate);
       		pair<bool,double> next = searcherClass.step();
       		tryvalueClass = next.second;
       		std::ostringstream stream;
       	    stream << "Next Class point at " << next.second << " and " << next.first;
       	    logger->writeLine(stream);
            if (!next.first) {
               	classLevelFinished = true;
            }
        }
        // if everything is finished, end.
        if (drugLevelFinished && classLevelFinished){
        	finished = true;
        }

        std::ostringstream stream2;
        stream2 << searcher;
        logger->writeLine(stream2);
        step++;
        if (step >= maxSteps) {
            std::ostringstream stream;
        	stream << "Max steps reached!";
        	logger->writeLine(stream);
        	finished = true;
        }
	}

	maxPoint = tryvalue;
	maxPointClass = tryvalueClass;

	// Report results
	std::ostringstream stream2;
	stream2 << std::endl;
	stream2 << "Maximum predicted log likelihood estimated at:" << std::endl;
	stream2 << "\t" << maxPoint << " (variance)" << std::endl;
	stream2 << "class level = " << maxPointClass;
	logger->writeLine(stream2);


	if (!arguments.useNormalPrior) {
		double lambda = convertVarianceToHyperparameter(maxPoint);
		std::ostringstream stream;
		stream << "\t" << lambda << " (lambda)";
		logger->writeLine(stream);
	}
	
    std::ostringstream stream3;
	logger->writeLine(stream3);	
}


} // namespace
