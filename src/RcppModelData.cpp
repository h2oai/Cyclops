/*
 * RcppModelData.cpp
 *
 *  Created on: Apr 24, 2014
 *      Author: msuchard
 */

#include "Rcpp.h"
#include "RcppModelData.h"
#include "Timer.h"
#include "RcppCyclopsInterface.h"
#include "io/NewGenericInputReader.h"
#include "RcppProgressLogger.h"
#include "SqlModelData.h"

using namespace Rcpp;

struct Sum {
    inline double operator()(double x, double y) {
        return x + y;
    }
};

struct ZeroPower {
    inline double operator()(double x) {
        return x == 0.0 ? 0.0 : 1.0;
    }
};

struct FirstPower {
    inline double operator()(double x) {
        return x;
    }
};

struct SecondPower {
    inline double operator()(double x) {
        return x * x;
    }
};

struct InnerProduct {
    inline double operator()(double x, double y) {
        return x * y;
    }
};

XPtr<bsccs::ModelData> parseEnvironmentForPtr(const Environment& x) {
	if (!x.inherits("cyclopsData")) {
		stop("Input must be a cyclopsData object");
	}

	SEXP tSexp = x["cyclopsDataPtr"];
	if (TYPEOF(tSexp) != EXTPTRSXP) {
		stop("Input must contain a cyclopsDataPtr object");
	}

	XPtr<bsccs::ModelData> ptr(tSexp);
	if (!ptr) {
		stop("cyclopsData object is uninitialized");
	}
	return ptr;
}

XPtr<bsccs::RcppModelData> parseEnvironmentForRcppPtr(const Environment& x) {
	if (!x.inherits("cyclopsData")) {
		stop("Input must be a cyclopsData object");
	}

	SEXP tSexp = x["cyclopsDataPtr"];
	if (TYPEOF(tSexp) != EXTPTRSXP) {
		stop("Input must contain a cyclopsDataPtr object");
	}

	XPtr<bsccs::RcppModelData> ptr(tSexp);
	if (!ptr) {
		stop("cyclopsData object is uninitialized");
	}
	return ptr;
}

//' @title Print row identifiers
//'
//' @description
//' \code{printCcdRowIds} return the row identifiers in a Cyclops data object
//'
//' @param object    A Cyclops data object
//'
//' @keywords internal
// [[Rcpp::export("printCyclopsRowIds")]]
void cyclopsPrintRowIds(Environment object) {
	XPtr<bsccs::RcppModelData> data = parseEnvironmentForRcppPtr(object);
//	std::ostreamstring stream;
// 	std::vector<IdType>& rowsIds = data->get
}

// void testCcdCode(int position) {
//     std::vector<int> v{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
//
//     if (position > 0 && position < static_cast<int>(v.size()))  {
//         auto reversePosition = v.size() - position - 1;
//         std::rotate(
//             v.rbegin() + reversePosition,
//             v.rbegin() + reversePosition + 1, // rotate one element
//             v.rend());
//         std::cout << "simple rotate right : ";
//         for (int n: v) std::cout << n << ' ';
//         std::cout << '\n';
//     }
// }

// [[Rcpp::export(".isRcppPtrNull")]]
bool isRcppPtrNull(SEXP x) {
	if (TYPEOF(x) != EXTPTRSXP) {
		stop("Input must be an Rcpp externalptr");
	}
	XPtr<int> ptr(x);
	return !ptr;
}

//' @title Get number of strata
//'
//' @description
//' \code{getNumberOfStrata} return the number of unique strata in a Cyclops data object
//'
//' @param object    A Cyclops data object
//'
//' @export
// [[Rcpp::export("getNumberOfStrata")]]
int cyclopsGetNumberOfStrata(Environment object) {
	XPtr<bsccs::ModelData> data = parseEnvironmentForPtr(object);
	return static_cast<int>(data->getNumberOfPatients());
}

//' @title Get covariate identifiers
//'
//' @description
//' \code{getCovariateIds} returns a vector of integer covariate identifiers in a Cyclops data object
//'
//' @param object    A Cyclops data object
//'
//' @export
// [[Rcpp::export("getCovariateIds")]]
std::vector<int64_t> cyclopsGetCovariateIds(Environment object) {
    using namespace bsccs;
	XPtr<ModelData> data = parseEnvironmentForPtr(object);
	ProfileVector covariates;
	size_t i = 0;
	if (data->getHasOffsetCovariate()) i++;
// 	if (data->getHasInterceptCovariate()) i++;
	for (; i < data->getNumberOfColumns(); ++i) {
		covariates.push_back(data->getColumn(i).getNumericalLabel());
	}
	return covariates;
}

//' @title Get covariate types
//'
//' @description
//' \code{getCovariateTypes} returns a vector covariate types in a Cyclops data object
//'
//' @param object    A Cyclops data object
//' @param covariateLabel Integer vector: covariate identifiers to return
//'
//' @export
// [[Rcpp::export("getCovariateTypes")]]
CharacterVector cyclopsGetCovariateType(Environment object, const std::vector<int64_t>& covariateLabel) {
    using namespace bsccs;
	XPtr<bsccs::RcppModelData> data = parseEnvironmentForRcppPtr(object);
	CharacterVector types(covariateLabel.size());

	for (size_t i = 0; i < covariateLabel.size(); ++i) {
		size_t index = data->getColumnIndex(covariateLabel[i]);
		types[i] = data->getColumn(index).getTypeString();
	}
	return types;
}

//' @title Get total number of covariates
//'
//' @description
//' \code{getNumberOfCovariates} returns the total number of covariates in a Cyclops data object
//'
//' @param object    A Cyclops data object
//'
//' @export
// [[Rcpp::export("getNumberOfCovariates")]]
int cyclopsGetNumberOfColumns(Environment object) {
	XPtr<bsccs::ModelData> data = parseEnvironmentForPtr(object);
	auto count = data->getNumberOfColumns();
	if (data->getHasOffsetCovariate()) {
	    --count;
	}
	return static_cast<int>(count);
}

//' @title Get total number of rows
//'
//' @description
//' \code{getNumberOfRows} returns the total number of outcome rows in a Cyclops data object
//'
//' @param object    A Cyclops data object
//'
//' @export
// [[Rcpp::export("getNumberOfRows")]]
int cyclopsGetNumberOfRows(Environment object) {
	XPtr<bsccs::ModelData> data = parseEnvironmentForPtr(object);
	return static_cast<int>(data->getNumberOfRows());
}

//' @title Get total number of outcome types
//'
//' @description
//' \code{getNumberOfTypes} returns the total number of outcome types in a Cyclops data object
//'
//' @param object    A Cyclops data object
//'
//' @keywords internal
// [[Rcpp::export("getNumberOfTypes")]]
int cyclopsGetNumberOfTypes(Environment object) {
	XPtr<bsccs::ModelData> data = parseEnvironmentForPtr(object);
	return static_cast<int>(data->getNumberOfTypes());
}

// [[Rcpp::export(.cyclopsUnivariableCorrelation)]]
std::vector<double> cyclopsUnivariableCorrelation(Environment x,
                                                  const std::vector<long>& covariateLabel) {
    XPtr<bsccs::RcppModelData> data = parseEnvironmentForRcppPtr(x);

    const double Ey1 = data->reduce(-1, FirstPower()) / data->getNumberOfRows();
    const double Ey2 = data->reduce(-1, SecondPower()) / data->getNumberOfRows();
    const double Vy = Ey2 - Ey1 * Ey1;

    std::vector<double> result;

    auto oneVariable = [&data, &result, Ey1, Vy](const size_t index) {
        const double Ex1 = data->reduce(index, FirstPower()) / data->getNumberOfRows();
        const double Ex2 = data->reduce(index, SecondPower()) / data->getNumberOfRows();
        const double Exy = data->innerProductWithOutcome(index, InnerProduct()) / data->getNumberOfRows();

        const double Vx = Ex2 - Ex1 * Ex1;
        const double cov = Exy - Ex1 * Ey1;
        const double cor = (Vx > 0.0 && Vy > 0.0) ?
                           cov / std::sqrt(Vx) / std::sqrt(Vy) : NA_REAL;

        // Rcpp::Rcout << index << " " << Ey1 << " " << Ey2 << " " << Ex1 << " " << Ex2 << std::endl;
        // Rcpp::Rcout << index << " " << ySquared << " " << xSquared <<  " " << crossProduct << std::endl;
        result.push_back(cor);
    };

    if (covariateLabel.size() == 0) {
        result.reserve(data->getNumberOfColumns());
        size_t index = (data->getHasOffsetCovariate()) ? 1 : 0;
        for (; index <  data->getNumberOfColumns(); ++index) {
            oneVariable(index);
        }
    } else {
        result.reserve(covariateLabel.size());
        for(auto it = covariateLabel.begin(); it != covariateLabel.end(); ++it) {
            oneVariable(data->getColumnIndex(*it));
        }
    }

    return std::move(result);
}

// [[Rcpp::export(".cyclopsSumByGroup")]]
List cyclopsSumByGroup(Environment x, const std::vector<long>& covariateLabel,
		const long groupByLabel, const int power) {
	XPtr<bsccs::RcppModelData> data = parseEnvironmentForRcppPtr(x);
    List list(covariateLabel.size());
    IntegerVector names(covariateLabel.size());
    for (size_t i = 0; i < covariateLabel.size(); ++i) {
        std::vector<double> result;
        data->sumByGroup(result, covariateLabel[i], groupByLabel, power);
        list[i] = result;
        names[i] = covariateLabel[i];
    }
	list.attr("names") = names;
	return list;
}

// [[Rcpp::export(".cyclopsSumByStratum")]]
List cyclopsSumByStratum(Environment x, const std::vector<long>& covariateLabel,
		const int power) {
	XPtr<bsccs::RcppModelData> data = parseEnvironmentForRcppPtr(x);
    List list(covariateLabel.size());
    IntegerVector names(covariateLabel.size());
    for (size_t i = 0; i < covariateLabel.size(); ++i) {
        std::vector<double> result;
        data->sumByGroup(result, covariateLabel[i], power);
        list[i] = result;
        names[i] = covariateLabel[i];
    }
	list.attr("names") = names;
	return list;
}

// [[Rcpp::export(".cyclopsSum")]]
std::vector<double> cyclopsSum(Environment x, const std::vector<long>& covariateLabel,
		const int power) {
	XPtr<bsccs::RcppModelData> data = parseEnvironmentForRcppPtr(x);
	std::vector<double> result;
	for (std::vector<long>::const_iterator it = covariateLabel.begin();
	        it != covariateLabel.end(); ++it) {
	    result.push_back(data->sum(*it, power));
	}
	return result;
}



// [[Rcpp::export(".cyclopsNewSqlData")]]
List cyclopsNewSqlData(const std::string& modelTypeName, const std::string& noiseLevel) {
	using namespace bsccs;

	NoiseLevels noise = RcppCcdInterface::parseNoiseLevel(noiseLevel);
	bool silent = (noise == SILENT);

    ModelType modelType = RcppCcdInterface::parseModelType(modelTypeName);
	SqlModelData* ptr = new SqlModelData(modelType,
        bsccs::make_shared<loggers::RcppProgressLogger>(silent),
        bsccs::make_shared<loggers::RcppErrorHandler>());

	XPtr<SqlModelData> sqlModelData(ptr);

    List list = List::create(
            Rcpp::Named("cyclopsDataPtr") = sqlModelData
        );
    return list;
}

// [[Rcpp::export(".cyclopsMedian")]]
double cyclopsMedian(const NumericVector& vector) {
    // Make copy
    std::vector<double> data(vector.begin(), vector.end());
    return bsccs::median(data.begin(), data.end());
}

// [[Rcpp::export(".cyclopsQuantile")]]
double cyclopsQuantile(const NumericVector& vector, double q) {
    if (q < 0.0 || q > 1.0) Rcpp::stop("Invalid quantile");
    // Make copy
    std::vector<double> data(vector.begin(), vector.end());
    return bsccs::quantile(data.begin(), data.end(), q);
}

// [[Rcpp::export(".cyclopsNormalizeCovariates")]]
std::vector<double> cyclopsNormalizeCovariates(Environment x, const std::string& normalizationName) {
    using namespace bsccs;
    XPtr<ModelData> data = parseEnvironmentForPtr(x);
    NormalizationType type = RcppCcdInterface::parseNormalizationType(normalizationName);
    return data->normalizeCovariates(type);
}

// [[Rcpp::export(".cyclopsSetHasIntercept")]]
void cyclopsSetHasIntercept(Environment x, bool hasIntercept) {
    using namespace bsccs;
    XPtr<ModelData> data = parseEnvironmentForPtr(x);
    data->setHasInterceptCovariate(hasIntercept);
}

// [[Rcpp::export(".cyclopsGetHasIntercept")]]
bool cyclopsGetHasIntercept(Environment x) {
    using namespace bsccs;
    XPtr<ModelData> data = parseEnvironmentForPtr(x);
    return data->getHasInterceptCovariate();
}

// [[Rcpp::export(".cyclopsGetHasOffset")]]
bool cyclopsGetHasOffset(Environment x) {
    using namespace bsccs;
    XPtr<ModelData> data = parseEnvironmentForPtr(x);
    return data->getHasOffsetCovariate();
}

// [[Rcpp::export(".cyclopsGetMeanOffset")]]
double cyclopsGetMeanOffset(Environment x) {
    using namespace bsccs;
    XPtr<RcppModelData> data = parseEnvironmentForRcppPtr(x);
    return (data->getHasOffsetCovariate()) ?
        data->sum(-1, 1) / data->getNumberOfRows() :
        0.0;
}

// [[Rcpp::export(".cyclopsFinalizeData")]]
void cyclopsFinalizeData(
        Environment x,
        bool addIntercept,
        SEXP sexpOffsetCovariate,
        bool offsetAlreadyOnLogScale,
        bool sortCovariates,
        SEXP sexpCovariatesDense,
        bool magicFlag = false) {
    using namespace bsccs;
    XPtr<ModelData> data = parseEnvironmentForPtr(x);

    if (data->getIsFinalized()) {
        ::Rf_error("OHDSI data object is already finalized");
    }

    if (addIntercept) {
        if (data->getHasInterceptCovariate()) {
            ::Rf_error("OHDSI data object already has an intercept");
        }

        if (magicFlag) {
            data->insert(0, INTERCEPT);
        } else {
            data->insert(0, DENSE); // add to front, TODO fix if offset
            const size_t numRows = data->getNumberOfRows();
            for (size_t i = 0; i < numRows; ++i) {
                data->getColumn(0).add_data(i, static_cast<real>(1.0));
            }
        }
        data->setHasInterceptCovariate(true);
    }

    if (!Rf_isNull(sexpOffsetCovariate)) {
        // TODO handle offset
        IdType covariate = as<IdType>(sexpOffsetCovariate);
        int index;
        if (covariate == -1) { // TODO  Bad, magic number
            //std::cout << "Trying to convert time to offset" << std::endl;
            //data->push_back(NULL, NULL, offs.begin(), offs.end(), DENSE); // TODO Do not make copy
            //data->push_back(NULL, &(data->getTimeVectorRef()), DENSE);
            data->moveTimeToCovariate(true);
            index = data->getNumberOfColumns() - 1;
        } else {
            index = data->getColumnIndexByName(covariate);
 	    	if (index == -1) {
                std::ostringstream stream;
     			stream << "Variable " << covariate << " not found.";
                stop(stream.str().c_str());
     			//error->throwError(stream);
            }
        }
        data->moveToFront(index);
        data->getColumn(0).add_label(-1); // TODO Generic label for offset?
        data->setHasOffsetCovariate(true);
    }

    if (data->getHasOffsetCovariate() && !offsetAlreadyOnLogScale) {
        //stop("Transforming the offset is not yet implemented");
        data->getColumn(0).transform([](real x) {
            return std::log(x);
        });
    }

    if (!Rf_isNull(sexpCovariatesDense)) {
        // TODO handle dense conversion
        ProfileVector covariates = as<ProfileVector>(sexpCovariatesDense);
        for (auto it = covariates.begin(); it != covariates.end(); ++it) {
        	IdType index = data->getColumnIndex(*it);
        	data->getColumn(index).convertColumnToDense(data->getNumberOfRows());
        }
    }

    data->setIsFinalized(true);
}


// [[Rcpp::export(".loadCyclopsDataY")]]
void cyclopsLoadDataY(Environment x,
        const std::vector<int64_t>& stratumId,
        const std::vector<int64_t>& rowId,
        const std::vector<double>& y,
        const std::vector<double>& time) {

    using namespace bsccs;
    XPtr<ModelData> data = parseEnvironmentForPtr(x);
    data->loadY(stratumId, rowId, y, time);
}

// [[Rcpp::export(".loadCyclopsDataMultipleX")]]
int cyclopsLoadDataMultipleX(Environment x,
		const std::vector<int64_t>& covariateId,
		const std::vector<int64_t>& rowId,
		const std::vector<double>& covariateValue,
		const bool checkCovariateIds,
		const bool checkCovariateBounds,
		const bool append,
		const bool forceSparse) {

	using namespace bsccs;
	XPtr<ModelData> data = parseEnvironmentForPtr(x);

	return data->loadMultipleX(covariateId, rowId, covariateValue, checkCovariateIds,
                            checkCovariateBounds, append, forceSparse);
}

// [[Rcpp::export(".loadCyclopsDataX")]]
int cyclopsLoadDataX(Environment x,
        const int64_t covariateId,
        const std::vector<int64_t>& rowId,
        const std::vector<double>& covariateValue,
        const bool replace,
        const bool append,
        const bool forceSparse) {

    using namespace bsccs;
    XPtr<ModelData> data = parseEnvironmentForPtr(x);

    // rowId.size() == 0 -> dense
    // covariateValue.size() == 0 -> indicator

    return data->loadX(covariateId, rowId, covariateValue, replace, append, forceSparse);
}

// NOTE:  IdType does not get exported into RcppExports, so hard-coded here
// TODO Could use SEXP signature and cast in function

// [[Rcpp::export(".appendSqlCyclopsData")]]
int cyclopsAppendSqlData(Environment x,
        const std::vector<int64_t>& oStratumId,
        const std::vector<int64_t>& oRowId,
        const std::vector<double>& oY,
        const std::vector<double>& oTime,
        const std::vector<int64_t>& cRowId,
        const std::vector<int64_t>& cCovariateId,
        const std::vector<double>& cCovariateValue) {
        // o -> outcome, c -> covariates

    using namespace bsccs;
    XPtr<ModelData> data = parseEnvironmentForPtr(x);
    size_t count = data->append(oStratumId, oRowId, oY, oTime, cRowId, cCovariateId, cCovariateValue);
    return static_cast<int>(count);
}


// [[Rcpp::export(".cyclopsGetInterceptLabel")]]
SEXP cyclopsGetInterceptLabel(Environment x) {
    using namespace bsccs;
    XPtr<ModelData> data = parseEnvironmentForPtr(x);
    if (data->getHasInterceptCovariate()) {
        size_t index = data->getHasOffsetCovariate() ? 1 : 0;
        return Rcpp::wrap(data->getColumn(index).getNumericalLabel());
    } else {
        return R_NilValue;
    }
}

// [[Rcpp::export(".cyclopsReadData")]]
List cyclopsReadFileData(const std::string& fileName, const std::string& modelTypeName) {

		using namespace bsccs;
		Timer timer;
    ModelType modelType = RcppCcdInterface::parseModelType(modelTypeName);
    InputReader* reader = new NewGenericInputReader(modelType,
    	bsccs::make_shared<loggers::RcppProgressLogger>(true), // make silent
    	bsccs::make_shared<loggers::RcppErrorHandler>());
		reader->readFile(fileName.c_str()); // TODO Check for error

    XPtr<ModelData> ptr(reader->getModelData());


    const auto& y = ptr->getYVectorRef();
    double total = std::accumulate(y.begin(), y.end(), 0.0);
    // delete reader; // TODO Test

    double time = timer();
    List list = List::create(
            Rcpp::Named("cyclopsDataPtr") = ptr,
            Rcpp::Named("timeLoad") = time,
            Rcpp::Named("debug") = List::create(
            		Rcpp::Named("totalY") = total
            )
    );
    return list;
}

// [[Rcpp::export(".cyclopsModelData")]]
List cyclopsModelData(SEXP pid, SEXP y, SEXP z, SEXP offs, SEXP dx, SEXP sx, SEXP ix,
    const std::string& modelTypeName,
    bool useTimeAsOffset = false,
    int numTypes = 1) {

    using namespace bsccs;
    ModelType modelType = RcppCcdInterface::parseModelType(modelTypeName);

	Timer timer;

	IntegerVector ipid;
	if (!Rf_isNull(pid)) {
		ipid = pid; // This is not a copy
	} // else pid.size() == 0

	NumericVector iy(y);

	NumericVector iz;
	if (!Rf_isNull(z)) {
		iz = z;
	} // else z.size() == 0

	NumericVector ioffs;
	if (!Rf_isNull(offs)) {
		ioffs = offs;
	}

	// dense
	NumericVector dxv;
	if (!Rf_isNull(dx)) {
		S4 dxx(dx);
		dxv = dxx.slot("x");
	}

	// sparse
	IntegerVector siv, spv; NumericVector sxv;
	if (!Rf_isNull(sx)) {
		S4 sxx(sx);
		siv = sxx.slot("i");
		spv = sxx.slot("p");
		sxv = sxx.slot("x");
	}

	// indicator
	IntegerVector iiv, ipv;
	if (!Rf_isNull(ix)) {
		S4 ixx(ix);
		iiv = ixx.slot("i"); // TODO Check that no copy is made
		ipv = ixx.slot("p");
	}

    XPtr<RcppModelData> ptr(new RcppModelData(modelType, ipid, iy, iz, ioffs, dxv, siv,
    	spv, sxv, iiv, ipv, useTimeAsOffset, numTypes));

	double duration = timer();

 	List list = List::create(
 			Rcpp::Named("data") = ptr,
 			Rcpp::Named("timeLoad") = duration
 		);
  return list;
}

namespace bsccs {

RcppModelData::RcppModelData(
        ModelType _modelType,
		const IntegerVector& _pid,
		const NumericVector& _y,
		const NumericVector& _type,
		const NumericVector& _time,
		const NumericVector& dxv, // dense
		const IntegerVector& siv, // sparse
		const IntegerVector& spv,
		const NumericVector& sxv,
		const IntegerVector& iiv, // indicator
		const IntegerVector& ipv,
		bool useTimeAsOffset,
		int numTypes
		) : ModelData(
                _modelType,
				_pid,
				_y,
				_type,
				_time,
				bsccs::make_shared<loggers::RcppProgressLogger>(),
				bsccs::make_shared<loggers::RcppErrorHandler>()
				) {
	if (useTimeAsOffset) {
	    // offset
        RealVectorPtr r = make_shared<RealVector>();
        push_back(NULL, r, DENSE);
        r->assign(offs.begin(), offs.end()); // TODO Should not be necessary with shared_ptr
        setHasOffsetCovariate(true);
	    getColumn(0).add_label(-1);
	}

    nTypes = numTypes; // TODO move into constructor

	// Convert dense
	int nCovariates = static_cast<int>(dxv.size() / y.size());
	for (int i = 0; i < nCovariates; ++i) {
		if (numTypes == 1) {
			push_back(
					static_cast<IntegerVector::iterator>(NULL), static_cast<IntegerVector::iterator>(NULL),
					dxv.begin() + i * y.size(), dxv.begin() + (i + 1) * y.size(),
					DENSE);
			getColumn(getNumberOfColumns() - 1).add_label(getNumberOfColumns() - (getHasOffsetCovariate() ? 1 : 0));
		} else {
			std::vector<RealVectorPtr> covariates;
			for (int c = 0; c < numTypes; ++c) {
				covariates.push_back(make_shared<RealVector>(y.size(), 0));
			}
			size_t offset = i * y.size();
			for (size_t k = 0; k < y.size(); ++k) {
				covariates[static_cast<int>(_type[k])]->at(k) = dxv[offset + k];
			}
			for (int c = 0; c < numTypes; ++c) {
				push_back(
// 						static_cast<IntegerVector::iterator>(NULL),static_cast<IntegerVector::iterator>(NULL),
//						covariates[c].begin(), covariates[c].end(),
                        NULL,
                        covariates[c],
						DENSE);
				getColumn(getNumberOfColumns() - 1).add_label(getNumberOfColumns() - (getHasOffsetCovariate() ? 1 : 0));
			}
		}
	}

	// Convert sparse
	nCovariates = spv.size() - 1;
	for (int i = 0; i < nCovariates; ++i) {

		int begin = spv[i];
		int end = spv[i + 1];

		if (numTypes == 1) {
		    push_back(
			    	siv.begin() + begin, siv.begin() + end,
				    sxv.begin() + begin, sxv.begin() + end,
    				SPARSE);
            getColumn(getNumberOfColumns() - 1).add_label(getNumberOfColumns() - (getHasOffsetCovariate() ? 1 : 0));
        } else {
			std::vector<IntVectorPtr> covariatesI;
			std::vector<RealVectorPtr> covariatesX;
			for (int c = 0; c < numTypes; ++c) {
				covariatesI.push_back(make_shared<IntVector>());
				covariatesX.push_back(make_shared<RealVector>());
				push_back(covariatesI[c], covariatesX[c], SPARSE);
				getColumn(getNumberOfColumns() - 1).add_label(getNumberOfColumns() - (getHasOffsetCovariate() ? 1 : 0));
			}

            auto itI = siv.begin() + begin;
            auto itX = sxv.begin() + begin;
            for (; itI != siv.begin() + end; ++itI, ++itX) {
                int type = _type[*itI];
                covariatesI[type]->push_back(*itI);
                covariatesX[type]->push_back(*itX);
            }
        }
	}

	// Convert indicator
	nCovariates = ipv.size() - 1;
	for (int i = 0; i < nCovariates; ++i) {

		int begin = ipv[i];
		int end = ipv[i + 1];

        if (numTypes == 1) {
    		push_back(
	    			iiv.begin() + begin, iiv.begin() + end,
		    		static_cast<NumericVector::iterator>(NULL), static_cast<NumericVector::iterator>(NULL),
			    	INDICATOR);
            getColumn(getNumberOfColumns() - 1).add_label(getNumberOfColumns() - (getHasOffsetCovariate() ? 1 : 0));
        } else {
			std::vector<IntVectorPtr> covariates;
			for (int c = 0; c < numTypes; ++c) {
				covariates.push_back(make_shared<IntVector>());
				push_back(covariates[c], NULL, INDICATOR);
				getColumn(getNumberOfColumns() - 1).add_label(getNumberOfColumns() - (getHasOffsetCovariate() ? 1 : 0));
			}

            for (auto it = iiv.begin() + begin; it != iiv.begin() + end; ++it) {
                int type = _type[*it];
                covariates[type]->push_back(*it);
            }
        }
	}

	this->nRows = y.size();

	// Clean out PIDs
	std::vector<int>& cpid = getPidVectorRef();

	if (cpid.size() == 0) {
	    for (size_t i = 0; i < nRows; ++i) {
	        cpid.push_back(i); // TODO These are not necessary; remove.
	    }
	    nPatients = nRows;
	} else {
    	int currentCase = 0;
    	int currentPID = cpid[0];
    	cpid[0] = currentCase;
    	for (size_t i = 1; i < pid.size(); ++i) {
    	    int nextPID = cpid[i];
    	    if (nextPID != currentPID) {
	            currentCase++;
	            currentPID = nextPID;
    	    }
	        cpid[i] = currentCase;
    	}
        nPatients = currentCase + 1;
    }
}

double RcppModelData::sum(const IdType covariate, int power) {

    size_t index = getColumnIndex(covariate);
    if (power == 0) {
		return reduce(index, ZeroPower());
	} else if (power == 1) {
		return reduce(index, FirstPower());
	} else {
		return reduce(index, SecondPower());
	}
}

void RcppModelData::sumByGroup(std::vector<double>& out, const IdType covariate, const IdType groupBy, int power) {
    size_t covariateIndex = getColumnIndex(covariate);
    size_t groupByIndex = getColumnIndex(groupBy);
    out.resize(2);
    if (power == 0) {
    	reduceByGroup(out, covariateIndex, groupByIndex, ZeroPower());
    } else if (power == 1) {
  		reduceByGroup(out, covariateIndex, groupByIndex, FirstPower());
    } else {
    	reduceByGroup(out, covariateIndex, groupByIndex, SecondPower());
    }
}

void RcppModelData::sumByGroup(std::vector<double>& out, const IdType covariate, int power) {
    size_t covariateIndex = getColumnIndex(covariate);
    out.resize(nPatients);
    if (power == 0) {
    	reduceByGroup(out, covariateIndex, pid, ZeroPower());
    } else if (power == 1) {
  		reduceByGroup(out, covariateIndex, pid, FirstPower());
    } else {
    	reduceByGroup(out, covariateIndex, pid, SecondPower());
    }
}

RcppModelData::~RcppModelData() {
//	std::cout << "~RcppModelData() called." << std::endl;
}

} /* namespace bsccs */
