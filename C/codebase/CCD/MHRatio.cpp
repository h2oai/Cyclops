/*
 * MHRatio.cpp
 *
 *  Created on: Aug 6, 2012
 *      Author: trevorshaddox
 */


#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <map>
#include <time.h>
#include <set>

#include <math.h>

#include "MHRatio.h"

#include <boost/random.hpp>
#include <boost/random/uniform_real.hpp>

#include <Eigen/Dense>
#include <Eigen/Cholesky>

#define Debug_TRS

namespace bsccs{

MHRatio::MHRatio(){}

void MHRatio::initialize(CyclicCoordinateDescent & ccd){
	storedFBetaCurrent = ccd.getLogLikelihood();
	storedPBetaCurrent = ccd.getLogPrior();

}

MHRatio::~MHRatio(){

}



bool MHRatio::evaluate(Model & currentModel, Parameter & Beta, Parameter & Beta_Hat,
		Parameter & SigmaSquared, CyclicCoordinateDescent & ccd,
		boost::mt19937& rng, Eigen::MatrixXf& PrecisionMatrix,
		double tuningParameter) {

	if(currentModel.getNewLogPriorAndLikelihood()){
		resetLikelihoodAndPrior(currentModel);
	}

	double logMetropolisRatio = getLogMetropolisRatio(Beta,Beta_Hat, SigmaSquared, ccd, rng, PrecisionMatrix, tuningParameter);
	double logHastingsRatio;
	if (currentModel.getUseHastingsRatio()){
		logHastingsRatio = getLogHastingsRatio(Beta,Beta_Hat, PrecisionMatrix, tuningParameter);
	} else {
		cout << "+++++++++++++++++++++++++++++++   Log Hastings 0   ----------------------" << endl;
		logHastingsRatio = 0;
	}



// Compute the ratio for the MH step
	double ratio;// = exp(logMetropolisRatio + logHastingsRatio);
	double logRatio = logMetropolisRatio + logHastingsRatio;
	//Check for numerical issues
	if (std::isfinite(logMetropolisRatio) && std::isfinite(logHastingsRatio)){// && std::isfinite(ratio)){
		ratio = exp(logMetropolisRatio + logHastingsRatio);
	} else {
		cout << "########--------------#########  Warning: Numerical Issues   ########-------#######" << endl;
		ratio = 0; // Want to reject if numerical issues at proposal
	}
// Set our alpha
	alpha = min(ratio, 1.0);
	static boost::uniform_01<boost::mt19937> zeroone(rng);


// Sample from a uniform distribution
	double uniformRandom = zeroone();
	double logUniformRandom = log(uniformRandom);

#ifdef Debug_TRS
		cout << "alpha = " << alpha << endl;
		cout << "ratio = " << ratio << endl;
		cout << "logRatio = " << logRatio << endl;
		cout << "uniformRandom = " << uniformRandom << endl;
		cout << "logUniformRandom = " << logUniformRandom << endl;
		cout << "logMetropolisRatio = " << logMetropolisRatio << endl;
		cout << "logHastingsRatio = " << logHastingsRatio << endl;

#endif

	bool returnValue;
	if (alpha > uniformRandom) {

#ifdef Debug_TRS
	//	cout << "logMetropolisRatio = " << logMetropolisRatio << endl;
	//	cout << "logHastingsRatio = " << logHastingsRatio << endl;
		cout << "\n \n \t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  Change Beta @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ \n \n" << endl;
#endif

		Beta.setChangeStatus(true);

		storedFBetaCurrent = fBetaPossible; // ccd.getLogLikelihood(); // TODO No need to recompute if cached correctly
		storedPBetaCurrent = pBetaPossible; // ccd.getLogPrior();
		currentModel.setLoglikelihood(fBetaPossible);

		returnValue = true;
	} else{

#ifdef Debug_TRS
		cout << "##############  Reject Beta ##################" << endl;
#endif
		Beta.setChangeStatus(false);
		Beta.restore();
		currentModel.setLoglikelihood(storedFBetaCurrent);
		returnValue = false;
	}

	return(returnValue);


}

double MHRatio::getTransformedTuningValue(double tuningParameter){
	// TODO Don't forward reference like this.
	return exp(-tuningParameter);
}

void MHRatio::resetLikelihoodAndPrior(Model & model) {
	storedFBetaCurrent = model.getLoglikelihood();
	storedPBetaCurrent = model.getLogPrior();

}


double MHRatio::getLogMetropolisRatio(Parameter & Beta, Parameter & Beta_Hat,
		Parameter & SigmaSquared, CyclicCoordinateDescent & ccd,
		boost::mt19937& rng, Eigen::MatrixXf& PrecisionMatrix,
		double tuningParameter){

	// Get the proposed Beta values
		vector<double> * betaPossible = Beta.returnCurrentValuesPointer();

	// Compute log Likelihood and log prior

		ccd.resetBeta();
		ccd.setBeta(*betaPossible);  //TODO use new setBeta

		fBetaPossible = ccd.getLogLikelihood();
		pBetaPossible = ccd.getLogPrior();

		double ratio = (fBetaPossible + pBetaPossible) - (storedFBetaCurrent + storedPBetaCurrent);



		#ifdef Debug_TRS
		cout << "fBetaPossible = " << fBetaPossible << endl;
		cout << "fBetaCurrent = " << storedFBetaCurrent << endl;
		cout << "pBetaPossible = " << pBetaPossible << endl;
		cout << "pBetaCurrent = " << storedPBetaCurrent << endl;
		#endif

		return(ratio);
}

double MHRatio::getLogHastingsRatio(Parameter & Beta,
		Parameter & Beta_Hat, Eigen::MatrixXf& PrecisionMatrix,
		double tuningParameter
		){


	int betaLength = Beta.getSize();
	Eigen::VectorXf betaCurrent(betaLength);
	Eigen::VectorXf betaProposal(betaLength);

	Eigen::VectorXf beta_hat(betaLength);

	Eigen::VectorXf betaHat_minus_current(betaLength);
	Eigen::VectorXf betaHat_minus_proposal(betaLength);

	Eigen::VectorXf precisionDifferenceProduct_current(betaLength);
	Eigen::VectorXf precisionDifferenceProduct_proposal(betaLength);


	for (int i = 0; i< betaLength; i++){
		betaProposal(i) = Beta.get(i);
		betaCurrent(i) = Beta.getStored(i);
		beta_hat(i) = Beta_Hat.get(i);
	}

	betaHat_minus_current = beta_hat - betaCurrent;
	betaHat_minus_proposal = beta_hat - betaProposal;

	precisionDifferenceProduct_current =  PrecisionMatrix * betaHat_minus_current;
	precisionDifferenceProduct_proposal = PrecisionMatrix * betaHat_minus_proposal;

	double numerator = betaHat_minus_current.dot(precisionDifferenceProduct_current);
	double denominator = betaHat_minus_proposal.dot(precisionDifferenceProduct_proposal);

	return(-0.5*(numerator - denominator) / getTransformedTuningValue(tuningParameter)); // log scale
	// NB: tuningParameter scales the variance
	//return(0);

	// TODO Check these numbers!
}


double MHRatio::min(double value1, double value2) {
	if (value1 > value2) {
		return value2;
	} else {
		return value1;
	}

}

}
