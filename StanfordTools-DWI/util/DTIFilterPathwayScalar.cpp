/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

/*************************************************************************
 *    NAME: David Akers and Anthony Sherbondy
 *     
 *    FILE: DTIFilterPathwayScalar.cpp
 *    DATE: Mon Mar  1  5:51:32 PST 2004
 *************************************************************************/
#include "DTIFilterPathwayScalar.h"
#include "DTIPathway.h"
#include <iostream>
using namespace std;

/*************************************************************************
 * Function Name: DTIFilterLength::DTIFilterLength
 * Parameters: 
 * Effects: 
 *************************************************************************/

DTIFilterPathwayScalar::DTIFilterPathwayScalar(PathwayProperty property)
{
  _property = property;
  _min_value = 0;
  _max_value = 100;
}


/*************************************************************************
 * Function Name: DTIFilterPathwayScalar::~DTIFilterPathwayScalar
 * Parameters: 
 * Effects: 
 *************************************************************************/

DTIFilterPathwayScalar::~DTIFilterPathwayScalar()
{
}


/*************************************************************************
 * Function Name: DTIFilterPathwayScalar::setMinValue
 * Parameters: double minimum
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIFilterPathwayScalar::setMinValue(double minimum)
{
  _min_value = minimum;
  setDirty(true);
}


/*************************************************************************
 * Function Name: DTIFilterPathwayScalar::setMaxValue
 * Parameters: double maximum
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIFilterPathwayScalar::setMaxValue(double maximum)
{
  _max_value = maximum;
  setDirty(true);
}


/*************************************************************************
 * Function Name: DTIFilterPathwayScalar::matches
 * Parameters: DTIPathway *pathway
 * Returns: bool
 * Effects: 
 *************************************************************************/
bool
DTIFilterPathwayScalar::matches(DTIPathway *pathway)
{
  // Default to allow pathways that don't have this statistic
  double valueToCompare = _min_value;
  if( _property < pathway->getNumPathStatistics() )
    valueToCompare = pathway->getPathStatistic (_property);
  if (valueToCompare >= _min_value && valueToCompare <= _max_value) {
    return true;
  }
  else {
    //    cerr << valueToCompare << " : " << _min_value << " : " << _max_value << endl;
    return false;
  }
}


