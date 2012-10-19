/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

/*************************************************************************
 *    NAME: David Akers and Anthony Sherbondy
 *     
 *    FILE: DTIFilterAlgorithm.cpp
 *    DATE: Thu Apr  1  2:33:07 PST 2004
 *************************************************************************/
#include "DTIFilterAlgorithm.h"
#include "DTIPathway.h"

/*************************************************************************
 * Function Name: DTIFilterAlgorithm::DTIFilterAlgorithm
 * Parameters: 
 * Effects: 
 *************************************************************************/

DTIFilterAlgorithm::DTIFilterAlgorithm()
{
  _algo = 0;
}


/*************************************************************************
 * Function Name: DTIFilterAlgorithm::~DTIFilterAlgorithm
 * Parameters: 
 * Effects: 
 *************************************************************************/

DTIFilterAlgorithm::~DTIFilterAlgorithm()
{
}


/*************************************************************************
 * Function Name: DTIFilterAlgorithm::matches
 * Parameters: DTIPathway *pathway
 * Returns: bool
 * Effects: 
 *************************************************************************/
bool
DTIFilterAlgorithm::matches(DTIPathway *pathway)
{
  if (pathway->getPathwayAlgorithm() == _algo || _algo == DTI_PATHWAY_ALGORITHM_ANY) {
    return true;
  }
  else {
    return false;
  }
}


