/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

/*************************************************************************
 *    NAME: David Akers and Anthony Sherbondy
 *     
 *    FILE: DTIPathway.cpp
 *    DATE: Mon Feb 23  6:00:16 PST 2004
 *************************************************************************/
#include "DTIPathway.h"
#include <exception>
#include <stdexcept>
#include <tnt/tnt_array1d_utils.h>
#include <RAPID.H>
#include <math.h>
#include <list>

#ifndef M_PI
#define M_PI 3.141592
#endif

/*************************************************************************
 * Function Name: DTIPathway::DTIPathway
 * Parameters: 
 * Effects: 
 *************************************************************************/

DTIPathway::DTIPathway(DTIPathwayAlgorithm algo) : DTIPathwayInterface(algo)
{
  _seed_point_index = 0;
  _rapid_model = NULL;
}


/*************************************************************************
 * Function Name: DTIPathway::~DTIPathway
 * Parameters: 
 * Effects: 
 *************************************************************************/

DTIPathway::~DTIPathway()
{
  delete _rapid_model;
}



/*************************************************************************
 * Function Name: DTIPathway::getRAPIDModel
 * Parameters: 
 * Returns: RAPID_model *
 * Effects: 
 *************************************************************************/
extern int tottris;
RAPID_model *
DTIPathway::getRAPIDModel()
{
  double EPS = 0.001;
  if (_rapid_model) {
    return _rapid_model;
  }
  else {
    _rapid_model = new RAPID_model;
    _rapid_model->BeginModel();
    int counter = 0;
    DTIGeometryVector *previous = NULL;
    for (std::vector<DTIGeometryVector *>::iterator iter = _point_vector.begin();
	 iter != _point_vector.end();
	 iter++) {
      DTIGeometryVector *current = *iter;
      //std::cerr << "current: " << current << std::endl;
      if (counter != 0) {
	tottris+=3;
	double p1[3] = {(*previous)[0]+EPS, (*previous)[1], (*previous)[2]};
	
	double p2[3] = {(*previous)[0]+EPS, (*previous)[1] + EPS, (*previous)[2]};
	double p3[3] = {(*current)[0], (*current)[1], (*current)[2]};
	//std::cerr << "pt1: " << p1[0] << ", " << p1[1] << ", " << p3[2] << std::endl;
	_rapid_model->AddTri (p1, p2, p3, counter);
      }
      previous = current;
      counter++;
    }
    _rapid_model->EndModel();
    return _rapid_model;
  }
}


