/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

/*************************************************************************
 *    NAME: David Akers and Anthony Sherbondy
 *     
 *    FILE: DTIPathwayDatabase.cpp
 *    DATE: Thu Feb 26 21:28:09 PST 2004
 *************************************************************************/
#include "DTIPathwayDatabase.h"
#include "DTIPathway.h"
#include "DTIPathwayStatisticHeader.h"
#include "ComputeListener.h"
//#include "DTIVolumeInterface.h"
//#include "ScalarVolumeInterface.h"
#include "DTIStats.h"
#include <iostream>
#include "DTIPathwayAlgorithmHeader.h"

using namespace std;


/*************************************************************************
 * Function Name: DTIPathwayDatabase::DTIPathwayDatabase
 * Parameters: 
 * Effects: 
 *************************************************************************/

DTIPathwayDatabase::DTIPathwayDatabase()
{
  _recompute_min_needed = true;
  _recompute_max_needed = true;
  _mm_scale[0]=_mm_scale[1]=_mm_scale[2]=-1;
}


/*************************************************************************
 * Function Name: DTIPathwayDatabase::~DTIPathwayDatabase
 * Parameters: 
 * Effects: 
 *************************************************************************/

DTIPathwayDatabase::~DTIPathwayDatabase()
{
  free_vector<DTIPathway> (_fiber_pathways);
  free_vector<DTIPathwayStatisticHeader>( _pathway_statistic_headers);
  free_vector<DTIPathwayAlgorithmHeader>( _pathway_algorithm_headers);
  free_vector< DTIPathwayStatisticThreshold>( _stats_threshold);

}


/*************************************************************************
 * Function Name: DTIPathwayDatabase::addPathway
 * Parameters: DTIPathway *pathway
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIPathwayDatabase::addPathway(DTIPathway *pathway)
{
  _recompute_min_needed=_recompute_max_needed=true;
  _fiber_pathways.push_back (pathway);
}

/*************************************************************************
 * Function Name: DTIPathwayDatabase::removePathway
 * Parameters: DTIPathway *pathway
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIPathwayDatabase::removePathway(DTIPathway *pathway)
{
  for (std::vector<DTIPathway *>::iterator iter = _fiber_pathways.begin(); iter !=  _fiber_pathways.end(); iter++) {
    if (*iter == pathway) {
		_recompute_min_needed=_recompute_max_needed=true;
      _fiber_pathways.erase (iter);
      return;
    }
  }
}

/*************************************************************************
 * Function Name: DTIPathwayDatabase::removeAllPathways
 * Parameters: 
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIPathwayDatabase::removeAllPathways( )
{
  for (std::vector<DTIPathway *>::iterator iter = _fiber_pathways.begin(); iter !=  _fiber_pathways.end(); iter++)
    delete *iter;
  _fiber_pathways.clear();
}

void
DTIPathwayDatabase::clearAllPathways( )
{
  _fiber_pathways.clear();
}



/*************************************************************************
 * Function Name: DTIPathwayDatabase::getNumFibers
 * Parameters: 
 * Returns: int
 * Effects: 
 *************************************************************************/
int
DTIPathwayDatabase::getNumFibers()
{
  return _fiber_pathways.size();
}


/*************************************************************************
 * Function Name: DTIPathwayDatabase::copy
 * Parameters: 
 * Returns: DTIPathwayDatabase *
 * Effects: 
 *************************************************************************/
DTIPathwayDatabase *
DTIPathwayDatabase::copy()
{
  DTIPathwayDatabase *newDB = new DTIPathwayDatabase();
  for (std::vector<DTIPathway *>::iterator iter = _fiber_pathways.begin(); iter !=  _fiber_pathways.end(); iter++) {
    DTIPathway *pathway = *iter;
    newDB->addPathway (pathway);
  }

  return newDB;
}


/*************************************************************************
 * Function Name: DTIPathwayDatabase::getPathway
 * Parameters: int index
 * Returns: DTIPathway *
 * Effects: 
 *************************************************************************/
DTIPathway *
DTIPathwayDatabase::getPathway(int index)
{
  return _fiber_pathways[index];
}




double
DTIPathwayDatabase::computeMinValue(PathwayProperty property)
{
  double minVal = (*(_fiber_pathways.begin()))->getPathStatistic (property);
  for (std::vector<DTIPathway *>::iterator iter = _fiber_pathways.begin(); iter !=  _fiber_pathways.end(); iter++) {
    DTIPathway *pathway = *iter;
    if (pathway->getPathStatistic(property) < minVal) {
      minVal = pathway->getPathStatistic(property);
    }
  }
  return minVal;
}

double 
DTIPathwayDatabase::computeMaxValue(PathwayProperty property)
{
 double maxVal = (*(_fiber_pathways.begin()))->getPathStatistic (property);
  for (std::vector<DTIPathway *>::iterator iter = _fiber_pathways.begin(); iter !=  _fiber_pathways.end(); iter++) {
    DTIPathway *pathway = *iter;
    if (pathway->getPathStatistic(property) > maxVal) {
      maxVal = pathway->getPathStatistic(property);
    }
  }
  return maxVal;
}

/***********************************************************************
 *  Method: DTIPathwayDatabase::addStatisticHeader
 *  Params: DTIPathwayStatisticHeader *header
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayDatabase::addStatisticHeader(DTIPathwayStatisticHeader *header)
{
  _pathway_statistic_headers.push_back (header);
}


/***********************************************************************
 *  Method: DTIPathwayDatabase::getNumPathStatistics
 *  Params: StatConstraint constraint
 * Returns: int
 * Effects: 
 ***********************************************************************/
int
DTIPathwayDatabase::getNumPathStatistics(StatConstraint constraint)
{
  int count = 0;
  for (std::vector<DTIPathwayStatisticHeader *>::iterator iter = _pathway_statistic_headers.begin(); iter != _pathway_statistic_headers.end(); iter++) {
    DTIPathwayStatisticHeader *header = *iter;
    if (constraint == STAT_CONSTRAINT_NONE || 
	(constraint == STAT_CONSTRAINT_STAT && header->_is_viewable_stat) ||
	(constraint == STAT_CONSTRAINT_LUMINANCE && header->_is_luminance_encoding)) {
      count++;
    }
  }
  return count;
}

int DTIPathwayDatabase::getStatisticIndex( std::string statistic_name)
{
	for (int i = 0; i < (int)_pathway_statistic_headers.size(); i++) 
		if( ! strncmp(_pathway_statistic_headers[i]->_aggregate_name, statistic_name.c_str(), statistic_name.size() ) )
			return i;

	return -1;
}

/***********************************************************************
 *  Method: DTIPathwayDatabase::computeStatIndex
 *  Params: StatConstraint constraint, bool pointStatistic
 * Returns: int
 * Effects: 
 ***********************************************************************/
int
DTIPathwayDatabase::computeStatIndex(StatConstraint constraint, bool pointStatistic, int index)
{
  int count = 0;
  int statIndex = 0;
  for (std::vector<DTIPathwayStatisticHeader *>::iterator iter = _pathway_statistic_headers.begin(); iter != _pathway_statistic_headers.end(); iter++) {
    DTIPathwayStatisticHeader *header = *iter;
    if ((constraint == STAT_CONSTRAINT_NONE ||
	(constraint == STAT_CONSTRAINT_LUMINANCE && header->_is_luminance_encoding) ||
	 (constraint == STAT_CONSTRAINT_STAT && header->_is_viewable_stat))
	&& (!pointStatistic || header->_is_computed_per_point)) {
      count++;
    }
    if (index == count-1) return statIndex;
    statIndex++;
  }
  return -1;
}


/***********************************************************************
 *  Method: DTIPathwayDatabase::getPathStatisticHeader
 *  Params: int id
 * Returns: DTIPathwayStatisticHeader *
 * Effects: 
 ***********************************************************************/
DTIPathwayStatisticHeader *
DTIPathwayDatabase::getPathStatisticHeader(int id)
{
  return _pathway_statistic_headers[id];
}

DTIPathwayStatisticThreshold *
DTIPathwayDatabase::getPathStatistic(int id)
{
  return _stats_threshold[id];
}
/***********************************************************************
 *  Method: DTIPathwayDatabase::getNumPointStatistics
 *  Params: 
 * Returns: int
 * Effects: 
 ***********************************************************************/
int
DTIPathwayDatabase::getNumPointStatistics(StatConstraint constraint)
{
  int count = 0;
  for (std::vector<DTIPathwayStatisticHeader *>::iterator iter = _pathway_statistic_headers.begin(); iter != _pathway_statistic_headers.end(); iter++) {
    DTIPathwayStatisticHeader *header = *iter;
    if (header->_is_computed_per_point && 
	(constraint == STAT_CONSTRAINT_NONE) ||
	(constraint == STAT_CONSTRAINT_LUMINANCE && header->_is_luminance_encoding) ||
	(constraint == STAT_CONSTRAINT_STAT && header->_is_viewable_stat)) {
      count++;
    }
  }
  return count;
}


/***********************************************************************
 *  Method: DTIPathwayDatabase::getMinValues
 *  Params: 
 * Returns: const double *
 * Effects: 
 ***********************************************************************/
const double *
DTIPathwayDatabase::getMinValues()
{
 	assert(0);//This functionality is disabled, please use getPathStatistic
 return 0;//_cached_min_values;
}


/***********************************************************************
 *  Method: DTIPathwayDatabase::getMaxValues
 *  Params: 
 * Returns: const double *
 * Effects: 
 ***********************************************************************/
const double *
DTIPathwayDatabase::getMaxValues()
{
	assert(0);//This functionality is disabled, please use getPathStatistic
  return 0;//_cached_max_values;
}


/***********************************************************************
 *  Method: DTIPathwayDatabase::clearStatisticHeaders
 *  Params: 
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayDatabase::clearStatisticHeaders()
{
  _pathway_statistic_headers.clear();
}

/***********************************************************************
 *  Method: DTIPathwayDatabase::getNumAlgorithms
 *  Params: 
 * Returns: int
 * Effects: 
 ***********************************************************************/
int
DTIPathwayDatabase::getNumAlgorithms()
{
  return _pathway_algorithm_headers.size();
}


/***********************************************************************
 *  Method: DTIPathwayDatabase::replacePathway
 *  Params: int index, DTIPathway *newPathway
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayDatabase::replacePathway(int index, DTIPathway *newPathway)
{
  _fiber_pathways[index] = newPathway;
}

 /***********************************************************************
  *  Method: DTIPathwayDatabase::getPathAlgorithmHeader
  *  Params: int id
  * Returns: DTIPathwayAlgorithmHeader *
  * Effects: 
  ***********************************************************************/
 DTIPathwayAlgorithmHeader *
 DTIPathwayDatabase::getPathAlgorithmHeader(int id)
 {
   return _pathway_algorithm_headers[id];
 }

 /***********************************************************************
  *  Method: DTIPathwayDatabase::addAlgorithmHeader
  *  Params: DTIPathwayAlgorithmHeader *header
  * Returns: void
  * Effects: 
  ***********************************************************************/
 void
 DTIPathwayDatabase::addAlgorithmHeader(DTIPathwayAlgorithmHeader *header)
 {
   _pathway_algorithm_headers.push_back (header);
 }


/***********************************************************************
 *  Method: DTIPathwayDatabase::setTransformMatrix
 *  Params: const double mx[16]
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayDatabase::setTransformMatrix(const double mx[16])
{
  for (int i = 0; i < 16; i++) {
    _transform_mx[i] = mx[i];
  }
}


/***********************************************************************
 *  Method: DTIPathwayDatabase::getBoundingBox
 *  Params: double xRange[2], double yRange[2], double zRange[2]
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayDatabase::getBoundingBox(double xRange[2], double yRange[2], double zRange[2])
{
  xRange[0] = 10000;
  xRange[1] = -10000;
  yRange[0] = 10000;
  yRange[1] = -10000;
  zRange[0] = 10000;
  zRange[1] = -10000;

  for (int i = 0; i < getNumFibers(); i++) {
    DTIPathway *p = getPathway (i);
    for (int j = 0; j < p->getNumPoints(); j++) {
      double pt[3];
      p->getPoint (j, pt);
      if (pt[0] < xRange[0]) {
	xRange[0] = pt[0];
      }
      if (pt[0] > xRange[1]) {
	xRange[1] = pt[0];
      }
      if (pt[1] < yRange[0]) {
	yRange[0] = pt[1];
      }
      if (pt[1] > yRange[1]) {
	yRange[1] = pt[1];
      }
      if (pt[2] < zRange[0]) {
	zRange[0] = pt[2];
      }
      if (pt[2] > zRange[1]) {
	zRange[1] = pt[2];
      }
    }
  } 
}


/***********************************************************************
 *  Method: DTIPathwayDatabase::getMinValue
 *  Params: PathwayProperty property
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIPathwayDatabase::getMinValue(PathwayProperty property)
{
	if (_recompute_min_needed) {
		//    std::cerr << "getMinValues: recomputing!" << std::endl;
		for (int i = 0; i < (int)_pathway_statistic_headers.size(); i++) {
			_stats_threshold[i]->_min = computeMinValue(i);
			//      std::cerr << "min value for " << i << " is: " << _cached_min_values[i] << std::endl;
		}
	}
	_recompute_min_needed = false;

	if (property < (int)_pathway_statistic_headers.size() ) 
		return _stats_threshold[property]->_min ;
	return 0;
}

/*************************************************************************
 * Function Name: DTIPathwayDatabase::setVoxelSize
 * Parameters: const double mmScale[3]
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIPathwayDatabase::setVoxelSize(const double mmScale[3])
{
	_mm_scale[0] = mmScale[0];
	_mm_scale[1] = mmScale[1];
	_mm_scale[2] = mmScale[2];
}


/*************************************************************************
 * Function Name: DTIPathwayDatabase::getVoxelSize
 * Parameters: double mmScale[3]
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIPathwayDatabase::getVoxelSize(double mmScale[3])
{
	mmScale[0] = _mm_scale[0];
	mmScale[1] = _mm_scale[1];
	mmScale[2] = _mm_scale[2];
}

/*************************************************************************
 * Function Name: DTIPathwayDatabase::setVoxelSize
 * Parameters: const float mmScale[3]
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIPathwayDatabase::setVoxelSize(const float mmScale[3])
{
	_mm_scale[0] = mmScale[0];
	_mm_scale[1] = mmScale[1];
	_mm_scale[2] = mmScale[2];
}


/*************************************************************************
 * Function Name: DTIPathwayDatabase::getVoxelSize
 * Parameters: float mmScale[3]
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIPathwayDatabase::getVoxelSize(float mmScale[3])
{
	mmScale[0] = _mm_scale[0];
	mmScale[1] = _mm_scale[1];
	mmScale[2] = _mm_scale[2];
}


/*************************************************************************
 * Function Name: DTIPathwayDatabase::setSceneDimension
 * Parameters: const double mmScale[3]
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIPathwayDatabase::setSceneDimension(const unsigned int sceneDim[3])
{
  _scene_dim[0] = sceneDim[0];
  _scene_dim[1] = sceneDim[1];
  _scene_dim[2] = sceneDim[2];
}


/*************************************************************************
 * Function Name: DTIPathwayDatabase::getSceneDimension
 * Parameters: double mmScale[3]
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIPathwayDatabase::getSceneDimension(unsigned int sceneDim[3])
{
  sceneDim[0] = _scene_dim[0];
  sceneDim[1] = _scene_dim[1];
  sceneDim[2] = _scene_dim[2];
}


/***********************************************************************
 *  Method: DTIPathwayDatabase::getMaxValue
 *  Params: PathwayProperty property
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIPathwayDatabase::getMaxValue(PathwayProperty property)
{
	if (_recompute_max_needed) 
		for (int i = 0; i < (int)_pathway_statistic_headers.size(); i++)
			_stats_threshold[i]->_max = computeMaxValue (i);

	_recompute_max_needed = false;

	if (property < (int)_pathway_statistic_headers.size() )
		return _stats_threshold[property]->_max;
	return 0;
}



/***********************************************************************
 *  Method: DTIPathwayDatabase::computeAllStats
 *  Params: 
 * Returns: void
 * Effects: 
 ***********************************************************************/
// void
// DTIPathwayDatabase::computeAllStats(DTIVolumeInterface *tensors, ScalarVolumeInterface *faVol, ComputeListener *listener)
// {
//   int i = 0;
//   int statCount = 0;

//   setupStandardStatisticHeaders();

//   // compute all statistics (pointwise and aggregate)
//   for (i = 0; i < this->getNumFibers(); i++) {
//     //cerr << "Initializing path statistics for: "<< i << endl;
//     DTIPathway *path = this->getPathway(i);
//     path->initializePathStatistics((int) NUM_STAT_IDS, getPathStatisticHeaders(), true);
//   }
//   //cerr << "Done initializing.\n";

//   for (i = 0; i < this->getNumFibers(); i++) {
//     if ( (this->getNumFibers() >= 100) && (i % (this->getNumFibers()/100) == 0) ) {
//       if (listener) listener->progressUpdate ((double) i / this->getNumFibers(), "Computing path stats...", false);
//     }
//     DTIPathway *path = this->getPathway(i);
//     //cerr << "Length Calc\n";
//     DTIStats::computeLength(path, STAT_ID_LENGTH);
//     //cerr << "FA Calc\n";
//     DTIStats::computeFA (path, STAT_ID_FA, (ScalarVolume *) faVol);
//     //cerr << "Curvature Calc\n";
//     DTIStats::computeCurvature(path, STAT_ID_CURVATURE);
//   }
// }

/***********************************************************************
 *  Method: DTIPathwayDatabase::setupStandardStatisticHeaders
 *  Params: 
 * Returns: void
 * Effects: 
 ***********************************************************************/
// void
// DTIPathwayDatabase::setupStandardStatisticHeaders()
// {
//   clearStatisticHeaders();

//   DTIPathwayStatisticHeader *header;

//   header = new DTIPathwayStatisticHeader;
//   header->_is_computed_per_point = false;
//   header->_is_luminance_encoding = true;
//   header->_is_viewable_stat = true;
//   strcpy (header->_aggregate_name, "Length");
//   strcpy (header->_local_name, "Distance along path");
//   header->_unique_id = (int) STAT_ID_LENGTH;

//   this->addStatisticHeader(header);

  
//   header = new DTIPathwayStatisticHeader;
//   header->_is_computed_per_point = false;
//   header->_is_luminance_encoding = true;
//   header->_is_viewable_stat = true;
//   strcpy (header->_aggregate_name, "Median FA");
//   strcpy (header->_local_name, "Local FA");
//   header->_unique_id = (int) STAT_ID_FA;

  
//   this->addStatisticHeader(header);

//   header = new DTIPathwayStatisticHeader;
//   header->_is_computed_per_point = false;
//   header->_is_luminance_encoding = true;
//   header->_is_viewable_stat = true;
//   strcpy (header->_aggregate_name, "Mean Curvature");
//   strcpy (header->_local_name, "Local Curvature");
//   header->_unique_id = (int) STAT_ID_CURVATURE;

//   this->addStatisticHeader(header);

// }
