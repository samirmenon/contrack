/***********************************************************************
 * AUTHOR: David Akers <dakers>
 *   FILE: .//DTIPathDistanceMatrix.cpp
 *   DATE: Sun Jan 29 16:00:14 2006
 *  DESCR: 
 ***********************************************************************/
#include <typedefs.h>
#include "DTIPathDistanceMatrix.h"
#include <iostream>
#include <math.h>

/***********************************************************************
 *  Method: DTIPathDistanceMatrix::DTIPathDistanceMatrix
 *  Params: int numPathways
 * Effects: 
 ***********************************************************************/
DTIPathDistanceMatrix::DTIPathDistanceMatrix(int numPathways)
{
  int totalNumber = numPathways*(numPathways-1)/2;

  _distances = new float[totalNumber];

  _num_pathways = numPathways;
}


/***********************************************************************
 *  Method: DTIPathDistanceMatrix::~DTIPathDistanceMatrix
 *  Params: 
 * Effects: 
 ***********************************************************************/
DTIPathDistanceMatrix::~DTIPathDistanceMatrix()
{
  delete[] _distances;
}


/***********************************************************************
 *  Method: DTIPathDistanceMatrix::GetDistance
 *  Params: int path1Index, int path2Index
 * Returns: float
 * Effects: 
 ***********************************************************************/
float
DTIPathDistanceMatrix::GetDistance(int i, int j)
{
 // assert (i >= j);
  int index = j+(i*(i-1)/2);
 // assert (index < _num_pathways*(_num_pathways+1)/2);
  return _distances[index];
}


/***********************************************************************
 *  Method: DTIPathDistanceMatrix::SetDistance
 *  Params: int path1Index, int path2Index, float value
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathDistanceMatrix::SetDistance(int i, int j, float value)
{
 // assert (i >= j);
  int index = j+(i*(i-1)/2);

  //assert (index < _num_pathways*(_num_pathways+1)/2);
  _distances[index] = value;
}


/***********************************************************************
 *  Method: DTIPathwayIO::loadDistanceMatrix
 *  Params: std::ifstream &theStream
 * Returns: DTIPathDistanceMatrix *
 * Effects: 
 ***********************************************************************/
DTIPathDistanceMatrix *
DTIPathDistanceMatrix::loadDistanceMatrix(std::istream &theStream)
{
  int n = readScalar<int> (theStream);
  DTIPathDistanceMatrix *matrix = new DTIPathDistanceMatrix(n);
  int count = 0;
  float totalF = 0;
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < i; j++) {
      float f = readScalar<float> (theStream);
      matrix->SetDistance (i,j,f);
      count++;
      totalF += matrix->GetDistance (i,j);
    }
  }
  std::cerr << "Distance matrix contains " << count << " distances. Average distance = " << totalF / (count) << std::endl;
  return matrix;
}
