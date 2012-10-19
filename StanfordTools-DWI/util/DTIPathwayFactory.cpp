/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

/*************************************************************************
 *    NAME: David Akers and Anthony Sherbondy
 *     
 *    FILE: DTIPathwayFactory.cpp
 *    DATE: Mon Jun 21 23:03:31 PDT 2004
 *************************************************************************/
#include "DTIPathwayFactory.h"
#include "typedefs.h"
#include "DTIPathway.h"

/*************************************************************************
 * Function Name: DTIPathwayFactory::createPathway
 * Parameters: DTIPathwayAlgorithm algo
 * Returns: DTIPathwayInterface *
 * Effects: 
 *************************************************************************/
DTIPathwayInterface *
DTIPathwayFactory::createPathway(DTIPathwayAlgorithm algo)
{
  return new DTIPathway (algo);
}


