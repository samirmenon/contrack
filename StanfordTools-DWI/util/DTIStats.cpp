#include "DTIStats.h"
#include "DTIPathwayStatisticHeader.h"
#include "DTIPathwayDatabase.h"
#include "DTIPathway.h"
#include <tnt/tnt_array1d_utils.h>
#include <iostream>
#include <math.h>
#include "DTIMath.h"
#include "DTITrackingData.h"
#include "DTITractParamsFile.h"
#include "DTIConTrackPDF.h"
using namespace std;

// NON-SCORING functions

/***********************************************************************
 *  Method: DTIStats::computeLength
 *  Params: DTIPathwayInterface *path
 * Returns: void
 * Effects: 
 ***********************************************************************/
double
DTIStats::computeLength(DTIPathwayInterface *path, PathwayProperty id, double step_size_mm)
{
  double len = 0.0;
  int counter = 0;
  path->setPointStatistic(0, id, 0);
  for (int i = 1; i < path->getNumPoints(); i++) {
    float prev[3];
    float cur[3];
    path->getPoint (i-1, prev);
    path->getPoint (i, cur);
    DTIVector p1(prev);
    DTIVector p2(cur);
    // XXX hack because distance isn't exact and we need it to be for later analysis
    if(step_size_mm == 0)
      len += computeDistance (p1, p2);
    else
      len += step_size_mm;
    //std::cerr << "len: " << len << std::endl;
    path->setPointStatistic (i, id, len);
  }
  path->setPathStatistic(id, len);
  return len;
}

/***********************************************************************
 *  Method: DTIStats::computeEPDistance
 *  Params: DTIPathwayInterface *path, PathwayProperty id
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIStats::computeEPDistance(DTIPathwayInterface *path, PathwayProperty distID, PathwayProperty xposID, PathwayProperty yposID, PathwayProperty zposID)
{
//   double sp[3];
//   double ep[3];
  float sp[3];
  float ep[3];
  path->getPoint (0, sp);
  path->getPoint (path->getNumPoints()-1, ep);
  DTIVector p1(sp);
  DTIVector p2(ep);

  double distance = computeDistance(p1,p2);
  path->setPathStatistic(distID, distance);
  path->setPathStatistic(xposID, ep[0]);
  path->setPathStatistic(yposID, ep[1]);
  path->setPathStatistic(zposID, ep[2]);

  return distance;
}

/***********************************************************************
 *  Method: DTIStats::computeSampleOrder
 *  Params: DTIPathwayInterface *path, int order
 * Returns: void
 * Effects: 
 ***********************************************************************/
int
DTIStats::computeSampleOrder(DTIPathwayInterface *path, PathwayProperty id, int order)
{
  path->setPathStatistic(id, order);
  return order;
}

/***********************************************************************
 *  Method: DTIStats::computeCurvature
 *  Params: DTIPathwayInterface *path
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIStats::computeCurvature(DTIPathwayInterface *path, PathwayProperty id)
{
   double totalCurvature = 0.0;
  path->setPointStatistic (0, id, 0);

  for (int i = 0; i < path->getNumPoints()-2; i++) {
    //double a[3], b[3], c[3];
    float a[3], b[3], c[3];
    path->getPoint(i, a);
    path->getPoint(i+1,b);
    path->getPoint(i+2,c);
    
    DTIVector aVec(a);
    DTIVector bVec(b);
    DTIVector cVec(c);

    DTIVector v1 = bVec-aVec;
    DTIVector v2 = cVec-bVec;
    
    double curvature = computeLocalCurvature (aVec, bVec, cVec);

    path->setPointStatistic(i+1, id, curvature);

    totalCurvature += curvature;
  }
  path->setPointStatistic (path->getNumPoints()-1, id, 0);

  double avgCurvature;
  if (path->getNumPoints() > 2) {
    avgCurvature = totalCurvature / (double) (path->getNumPoints()-2);
  }
  else {
    avgCurvature = 0;
  }
  path->setPathStatistic (id, avgCurvature);
  return avgCurvature;
}


/***********************************************************************
 *  Method: DTIStats::computeDistance
 *  Params: const DTIVector &p1, const DTIVector &p2
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIStats::computeDistance(const DTIVector &p1, const DTIVector &p2)
{
  //double buf[3];
  //float buf[3];
  DTIVector diff;
  diff = p1-p2;
  double dot = dproduct(diff,diff);
  return sqrt(dot);
}


/***********************************************************************
 *  Method: DTIStats::computeLocalCurvature
 *  Params: const DTIVector &pt1, const DTIVector &pt2, const DTIVector &pt3
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIStats::computeLocalCurvature(const DTIVector &pt1, const DTIVector &pt2, const DTIVector &pt3)
{
  double a = computeDistance (pt1, pt2);
  double b = computeDistance (pt2, pt3);
  double c = computeDistance (pt1, pt3);
  double s = (a+b+c)/2.0;
  double denom = 4*sqrt(s*(s-a)*(s-b)*(s-c));
  double curvature = 0.0;
  if (denom != 0) {
    double radius = a*b*c/(4*sqrt(s*(s-a)*(s-b)*(s-c)));
    curvature = 1.0/radius;
  }
  else {
    curvature = 0.0;
  }
  return curvature;
}

/***********************************************************************
 *  Method: DTIStats::computeFA
 *  Params: DTIPathwayInterface *path, DTIScalarVolume *faVol
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIStats::computeFA(DTIPathwayInterface *path, PathwayProperty id, DTIScalarVolume *faVol)
{
  std::list <double> faList;
  double mmPerVox[3];
  faVol->getVoxelSize(mmPerVox[0], mmPerVox[1], mmPerVox[2]);
  for (int i = 0; i < path->getNumPoints(); i++) {
    double pt[3];
    path->getPoint (i, pt);
    float x = pt[0]/mmPerVox[0];
    float y = pt[1]/mmPerVox[1];
    float z = pt[2]/mmPerVox[2];
    double fa = faVol->getScalar(x,y,z);
    path->setPointStatistic (i, id, fabs(fa));
    // Do not include the endpoints for the minimum calculation
    if( i!=0 && i!=path->getNumPoints()-1 )
      faList.push_back(fabs(fa));
  }
  faList.sort();
  path->setPathStatistic (id, faList.front());
  return faList.front();
  
  // Tony changed this to just get the minimum
//  unsigned int count = 0;
//  for (std::list<double>::iterator iter = faList.begin(); iter != faList.end(); iter++) {
//    double fa = *iter;
//    if (count == faList.size()/2) {
//      if (fa > 1.0) fa = 0.0;
       //shouldn't have to do this - caused by negative eigenvalues from tensorcalc!
//      path->setPathStatistic (id, fa);
//      return fa;
//    }
//    count++;
//  }
  assert (false);
}

/***********************************************************************
 *  Method: DTIStats::computeFA
 *  Params: DTIPathwayInterface *path, DTIScalarVolume *faVol
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIStats::computeLinearity(DTIPathwayInterface *path, PathwayProperty id, DTIVectorVolume *pdfVol)
{
  std::list <double> clList;
  double mmPerVox[3];
  pdfVol->getVoxelSize(mmPerVox[0], mmPerVox[1], mmPerVox[2]);
  unsigned int xdim,ydim,zdim,ldim;
  pdfVol->getDimension(xdim,ydim,zdim,ldim);
      
  if( !DTIConTrackPDF::isValidVersion(pdfVol->getVector((unsigned int)0,(unsigned int)0,(unsigned int)0)) ) {
    std::cerr << "FATAL: PDF parameters image does not have the right dimensions." << std::endl;
    exit(0);
  }

  for (int pp = 0; pp < path->getNumPoints(); pp++) {
    double pt[3];
    path->getPoint (pp, pt);
    float x = pt[0]/mmPerVox[0];
    float y = pt[1]/mmPerVox[1];
    float z = pt[2]/mmPerVox[2];
    Array1D<float> pdfVec = (pdfVol->getVector(x,y,z)).copy();
    // PDF data format = (e1_x,e1_y,e1_z,e2_x,e2_y,e2_z,e3_x,e3_y,e3_z,k1,k2,Cl,eVal2,eVal3)
    // Note: e3 is actually e1 of the tensor and e1 is e3, but the distribution is commonly
    // written such that e3 is e1, we follow the literature of Mardia, etc.
    DTIConTrackPDF::Params p = DTIConTrackPDF::getPDFParams(pdfVec);
    path->setPointStatistic (pp, id, p.fCl);
    clList.push_back(p.fCl);
  }
  clList.sort();
  path->setPathStatistic (id, clList.front());
  return clList.front();
  
  // Tony changed this to just get the minimum
//  unsigned int count = 0;
//  for (std::list<double>::iterator iter = faList.begin(); iter != faList.end(); iter++) {
//    double fa = *iter;
//    if (count == faList.size()/2) {
//      if (fa > 1.0) fa = 0.0;
       //shouldn't have to do this - caused by negative eigenvalues from tensorcalc!
//      path->setPathStatistic (id, fa);
//      return fa;
//    }
//    count++;
//  }
  assert (false);
}

// SCORING functions

/***********************************************************************
 *  Method: DTIStats::scorePathway
 *  Params: DTIPathwayInterface *pathway, float lengthParam, float smoothParam, float tensorShapeParam
 * Returns: float
 * Effects: 
 ***********************************************************************/
float
DTIStats::scorePathway(DTIPathwayInterface *pathway, float lengthParam, float smoothParam, float tensorShapeParam, DTITrackingData *tData, bool bFreeEnds, bool bReportInf)
{
  float log_score = 0;
  vector<double> tempStats;
  
  // See if we should ignore the ROIs
  DTIScalarVolume* voiVol;
  if(bFreeEnds)
    voiVol = NULL;
  else
    voiVol = tData->_voiMaskVol;
    
  log_score += DTIStats::computeLogScoreAbsorption( pathway, 
  						    -1, 
  						    tempStats,
  						    tData->_wmVol,
  						    voiVol,  
  						    tData->_params->wmThresh,
						    tData->_start_voi,
						    tData->_stop_voi,
						    bReportInf );
  log_score += lengthParam*(pathway->getNumPoints()-2);

  if( log_score > DTIMath::float_neg_inf())
    log_score += DTIStats::computeLogScorePrior( pathway, 
						 -1, 
						 tempStats, 
						 smoothParam, 
						 tData->_params->angleCutoff,
						 bReportInf );

  if( log_score > DTIMath::float_neg_inf())
    log_score += DTIStats::computeLogScoreLikelihood( pathway, 
						      -1, 
						      tempStats,
						      tData->_pdfVol, 
						      tData->_params->shapeLinearityMidCl, 
						      tensorShapeParam, 
						      tData->_params->shapeUniformS,
						      bReportInf);
  return log_score;
}

bool
DTIStats::computeAllPathPointsInBounds(const DTIPathwayInterface *path, const DTIScalarVolume *vol)
{
  bool bIn=true;
  double voxSize[3];
  vol->getVoxelSize(voxSize[0], voxSize[1], voxSize[2]);
  for(int pp=0; pp<path->getNumPoints() && bIn; pp++) {
    DTIVector v;
    path->getPoint (pp, v);
    v[0] /= voxSize[0];
    v[1] /= voxSize[1];
    v[2] /= voxSize[2];
    bIn = bIn && vol->inBounds(v[0],v[1],v[2]); 
  }
  return bIn;
}

/***********************************************************************
 *  Method: DTIStats::scorePathway
 *  Params: DTIPathwayInterface *pathway, float lengthParam, float smoothParam, float tensorShapeParam, std::vector<double> vecLike, std::vector<double> vecPrior, std::vector<double> vecAbs
 * Returns: float
 * Effects: 
 ***********************************************************************/
float
DTIStats::scorePathway(DTIPathwayInterface *pathway, float lengthParam, float smoothParam, float tensorShapeParam, std::vector<double> &vecLike, std::vector<double> &vecPrior, std::vector<double> &vecAbs, DTITrackingData *tData, bool bFreeEnds, bool bReportInf)
{
  float log_score = 0;
  vector<double> tempStats;

  // See if we should ignore the ROIs
  DTIScalarVolume* voiVol;
  if(bFreeEnds)
    voiVol = NULL;
  else
    voiVol = tData->_voiMaskVol;

  log_score += DTIStats::computeLogScoreAbsorption( pathway, 
  						    -1, 
  						    tempStats,
  						    tData->_wmVol,
  						    voiVol,  
  						    tData->_params->wmThresh,
						    tData->_start_voi,
						    tData->_stop_voi );
  log_score += lengthParam*(pathway->getNumPoints()-2);

  //if( log_score > DTIMath::float_neg_inf())
    log_score += DTIStats::computeLogScorePrior( pathway, 
						 -1, 
						 vecPrior, 
						 smoothParam, 
						 tData->_params->angleCutoff );
  
    //if( log_score > DTIMath::float_neg_inf())
    log_score += DTIStats::computeLogScoreLikelihood( pathway, 
						      -1, 
						      vecLike,
						      tData->_pdfVol, 
						      tData->_params->shapeLinearityMidCl, 
						      tensorShapeParam, 
						      tData->_params->shapeUniformS);
  return log_score;
}


/***********************************************************************
 *  Method: DTIStats::computeLogScoreLikelihood
 *  Params: DTIPathwayInterface *path, PathwayProperty likelihoodID, std::vector<double> vecLikelihood, DTIVectorVolume *pdfParamsImage, double linearityMidCl, double linearityWidthCl, double uniform_s
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIStats::computeLogScoreLikelihood(DTIPathwayInterface *path, PathwayProperty likelihoodID, std::vector<double> &vecLikelihood, DTIVectorVolume *pdfParamsImage, double linearityMidCl, double linearityWidthCl, double uniform_s, bool bReportInf)
{
  if( pdfParamsImage == NULL ){
    std::cerr << "Must specify a pdf image for score calculation." << std::endl;
    return 0;
  }
  // Figure out what pdf we are using by the number of parameters
  unsigned int xdim,ydim,zdim,ldim;
  pdfParamsImage->getDimension(xdim,ydim,zdim,ldim);
  if( !DTIConTrackPDF::isValidVersion(pdfParamsImage->getVector((unsigned int)0,(unsigned int)0,(unsigned int)0)) ) {
    std::cerr << "FATAL: PDF parameters image does not have the right dimensions." << std::endl;
    exit(0);
  }	
  
  double likelihood = 0;
  int numPoints = path->getNumPoints();
  double voxSize[3];
  pdfParamsImage->getVoxelSize(voxSize[0], voxSize[1], voxSize[2]);
  bool bSymmetric = true;

  // Only forward differencing for tangent
  DTIVector p0;
  DTIVector p1;
  DTIVector p2;

  // Get the first point
  path->getPoint (0, p1);
  p1[0] /= voxSize[0];
  p1[1] /= voxSize[1];
  p1[2] /= voxSize[2];

  // Only look at all tangents, therefore do not look at the last point
  for (int ii = 0; ii < numPoints-1; ii++) {
    DTIVector tangent(3);
    if( ii == 0 ) {
      // start point or previous non-symmetric method
      path->getPoint (ii+1, p2);
      p2[0] /= voxSize[0];
      p2[1] /= voxSize[1];
      p2[2] /= voxSize[2];
      computePathTangent (tangent, p1, p2);
    }
    else if ( ii == numPoints-2 ){
      // end point
      computePathTangent (tangent, p0, p1);
    }
    else {
      // interior point
      path->getPoint (ii+1, p2);
      p2[0] /= voxSize[0];
      p2[1] /= voxSize[1];
      p2[2] /= voxSize[2];
      computePathTangent (tangent, p0, p1, p2);
    }

    // Doing nearest neighbor interpolation now
    // If we have the pdf parameters lets use that for scoring
    double angle, deviation;
    double localLogLikelihood;
    Array1D<float> paramsVec = (pdfParamsImage->getVector(p1[0],p1[1],p1[2])).copy();

    // Check to see if we have fallen out of the valid data
    if( DTIConTrackPDF::isValidData(paramsVec) ){
      localLogLikelihood = DTIMath::double_neg_inf();
    }
    else {
      // FB5 data format = (e1_x,e1_y,e1_z,e2_x,e2_y,e2_z,e3_x,e3_y,e3_z,k1,k2,Cl,eVal2,eVal3)
      // Note: e3 is actually e1 of the tensor and e1 is e3, but the distribution is commonly
      // written such that e3 is e1, we follow the literature of Mardia, etc.
      DTIConTrackPDF::Params p = DTIConTrackPDF::getPDFParams(paramsVec);
      double thermalnoise_thresh = 205;
      // Handle p.fScannerK = 0 which comes from too certain of a fit probably number underflow??
      if( p.fScannerK > thermalnoise_thresh || p.fScannerK == 0)
	p.fScannerK = thermalnoise_thresh;

      //double uniform_s = 100; // degrees
      double linshape_ds = uniform_s / (1+exp(-(linearityMidCl-p.fCl)*10/linearityWidthCl));
      double ratio_l3 = p.fE3 / (p.fE2+p.fE3);
      double ds1 = linshape_ds*ratio_l3;
      double ds2 = linshape_ds*(1-ratio_l3);
      //double dk1 = 1 / ((ds1*M_PI/180)*(ds1*M_PI/180));
      //double dk2 = 1 / ((ds2*M_PI/180)*(ds2*M_PI/180));
      double dk1 = 1 / ((sin(ds1*M_PI/180))*(sin(ds1*M_PI/180)));
      double dk2 = 1 / ((sin(ds2*M_PI/180))*(sin(ds2*M_PI/180)));
      double comp_k1 = p.fScannerK*dk1 / (dk1 + p.fScannerK + 2*sqrt(p.fScannerK*dk1));
      double comp_k2 = p.fScannerK*dk2 / (dk2 + p.fScannerK + 2*sqrt(p.fScannerK*dk2));	
      double k1 = -comp_k1;
      double k2 = -comp_k2;
      if(k1 > -1 ) k1 = -1;
      if(k2 > -1 ) k2 = -1;
      if( k2 < -205 || k1 < -205 ) {k2 = -205; k1 = -205;}
      if( k1 > k2 ) k1 = k2 - 0.01; // This has only happened because of round off error
      //std::cerr << " k1: " << k1 << " k2: " << k2 << std::endl;
      localLogLikelihood = computeLocalLogLikelihoodFB5(tangent,p.vE1,p.vE2,k1,k2,angle,deviation);
    }
  
    likelihood = likelihood + localLogLikelihood;
    path->setPointStatistic (ii, likelihoodID, localLogLikelihood);
    vecLikelihood.push_back( localLogLikelihood );
    p0 = p1.copy();// next point, circular buffer
    p1 = p2.copy();// next point, circular buffer
  }
  path->setPointStatistic (numPoints-1, likelihoodID, 0);
  vecLikelihood.push_back( 0 );
  path->setPathStatistic (likelihoodID, likelihood);  

  string strReasonInf;
  if (!(likelihood > DTIMath::float_neg_inf()))
    strReasonInf = "Pathway received very low likelihood score.";
  if (bReportInf  && !strReasonInf.empty()) 
    cerr << "Warning: " << strReasonInf << endl;

  return likelihood; 
}


/***********************************************************************
 *  Method: DTIStats::computeLogScorePrior
 *  Params: DTIPathwayInterface *path, PathwayProperty priorID, std::vector<double> vecPrior, double stdSmoothness, double angleCutoff
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIStats::computeLogScorePrior(DTIPathwayInterface *path, PathwayProperty priorID, std::vector<double> &vecPrior, double stdSmoothness, double angleCutoff, bool bReportInf)
{
  path->setPointStatistic (0, priorID, 0);
  vecPrior.push_back( 0 );
  double log_prior = 0;
  string strReasonInf;
  for (int ii = 1; ii < path->getNumPoints()-1; ii++) {
    // Get 3 points that define instantaneous curvature at a point
    DTIVector p1;
    DTIVector p2;
    DTIVector p3;
    path->getPoint (ii-1, p1);
    path->getPoint (ii,   p2);
    path->getPoint (ii+1, p3);
    DTIVector v1 = p2-p1;
    DTIVector v2 = p3-p2;
    double angle;
    double np = computeLocalLogPriorWatson(v1,v2,angle,stdSmoothness,angleCutoff);
    
    if (!(np > DTIMath::float_neg_inf())) {
      char buffer[256];
      sprintf(buffer, "Pathway at position %d has v1 = [%g,%g,%g] and v2 = [%g,%g,%g].", ii, v1[0],v1[1],v1[2],v2[0],v2[1],v2[2]);
      strReasonInf = buffer;
    }

    log_prior = log_prior + np;
    path->setPointStatistic (ii, priorID, np);
    vecPrior.push_back( log_prior );
  }
  path->setPointStatistic (path->getNumPoints()-1, priorID, 0);
  vecPrior.push_back( 0 );
  path->setPathStatistic (priorID, log_prior);

  
 

  if (bReportInf  && !strReasonInf.empty()) 
    cerr << "Warning: " << strReasonInf << endl;

  return log_prior;
}


/***********************************************************************
 *  Method: DTIStats::computeLogScoreAbsorption
 *  Params: DTIPathwayInterface *path, PathwayProperty id, std::vector<double> &vecAbs, DTIScalarVolume *wmImage, DTIScalarVolume *voiImage, double wmThresh, int start_voi, int stop_voi
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIStats::computeLogScoreAbsorption(DTIPathwayInterface *path, PathwayProperty id, std::vector<double> &vecAbs, DTIScalarVolume *wmImage, DTIScalarVolume *voiImage, double wmThresh, int start_voi, int stop_voi, bool bReportInf)
{
  double log_absorb=0;
  double local_absorb;
  double pAbsorb;
  int numPoints = path->getNumPoints();
  unsigned int dim[4];
  double voxSize[3];
  wmImage->getDimension(dim[0], dim[1], dim[2], dim[3]);
  wmImage->getVoxelSize(voxSize[0], voxSize[1], voxSize[2]);

  // Setup the condition that we must hit both end ROIs but in any order
  int cur_start_voi = start_voi;
  int cur_stop_voi = stop_voi;
  DTIVector p;
  if(voiImage!=NULL) {
    path->getPoint (0,p);
    int voi = (int)voiImage->getScalar(p[0]/(float)voxSize[0],p[1]/(float)voxSize[1],p[2]/(float)voxSize[2]);
    if(voi==stop_voi) {
      cur_start_voi = stop_voi;
      cur_stop_voi  = start_voi; 
    }
  }

  string strReasonInf;
  for (int ii = 0; ii < numPoints; ii++) {
    path->getPoint (ii, p);
    p[0] /= voxSize[0];
    p[1] /= voxSize[1];
    p[2] /= voxSize[2];
    
    double pAbsorb;
    if( ii==0 ) {
      if(voiImage==NULL)
	pAbsorb=1;
      else
	pAbsorb = computeLocalScoreAbsorptionRate( wmImage, voiImage, p, wmThresh, cur_start_voi );
    } else {
      if(ii==numPoints-1 && voiImage==NULL)
	pAbsorb=1;
      else
	pAbsorb = computeLocalScoreAbsorptionRate( wmImage, voiImage, p, wmThresh, cur_stop_voi );
    }
    if( ii>0 && ii<numPoints-1) {
      pAbsorb = 1-pAbsorb;      
    }
    if(pAbsorb==0) {
      local_absorb = DTIMath::double_neg_inf();
      if (strReasonInf.empty()) {
	if (ii==0)
	  strReasonInf = "Min score given due to starting point.";
	else {
	  double wm = wmImage->getScalar(p[0],p[1],p[2]);
	  if(wm<wmThresh)
	    strReasonInf = "Min score given due to white matter threshold.";
	  else if(ii==numPoints-1)
	    strReasonInf = "Min score given due to ending point.";
	  else
	    strReasonInf = "Min score given due to unknown.";
	}
      }
    } else if(pAbsorb==1) {
      local_absorb = 0;
    } else {
      local_absorb = log(pAbsorb);
    }
    
    path->setPointStatistic(ii, id, local_absorb);
    log_absorb += local_absorb;
    vecAbs.push_back( local_absorb );
  }
  if (bReportInf  && !strReasonInf.empty()) 
    cerr << "Warning: " << strReasonInf << endl;

  path->setPathStatistic(id, log_absorb);
  return log_absorb;
}

// LOCAL CALCULATIONS

/***********************************************************************
 *  Method: DTIStats::computeLocalLogLikelihoodFB5
 *  Params: const DTIVector &x, const DTIVector &e1, const DTIVector &e2, double k1, double bk2, double log_A, double &angle, double &deviation
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIStats::computeLocalLogLikelihoodFB5(const DTIVector &x, const DTIVector &e1, const DTIVector &e2, double k1, double k2, double &angle, double &deviation)
{
  double cosTheta = dproduct (x, e1);
  deviation = k1;
  
  if(cosTheta > 1)
    angle = 0;
  else if( cosTheta < -1)
    angle = M_PI;
  else
    angle = acos(cosTheta);
  
  return DTIMath::logFB5PDF(x,e1,e2,k1,k2) - 2.53; // log(1/(4*PI))
}

/***********************************************************************
 *  Method: DTIStats::computeLocalLogPriorWatson
 *  Params: const DTIVector &v1, const DTIVector &v2, double &angle, double k
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIStats::computeLocalLogPriorWatson(const DTIVector &v1, const DTIVector &v2, double &angle, double k, double angleCutoff)
{
  double xDotMean = dproduct(v1,v2)/(abs(v1)*abs(v2));
  angle = acos( xDotMean );
  double p;
//   // Have a cut-off
//   if( angle > angleCutoff ) {
//     p = DTIMath::double_neg_inf();//log((double)0.0);
//   } 
//   else if (xDotMean < 0) {   
//     //p = DTIMath::logWatsonPDF(0, k) - 1.84;
//   }  
//   else {
//     p = DTIMath::logUPSpherePDF(xDotMean, k);
//     //p = DTIMath::logWatsonPDF(xDotMean, k) - 1.84; // log(1/(2*PI))
//   }

   // Have a cut-off
  // XXX The calculation of the PDF ignores the effect of the cutoff, 
  // which should be negligible for cutoffs > 90 and k larrge enough 
  // (we use k=32 often)
   if( angle > angleCutoff )
     p = DTIMath::double_neg_inf();//log((double)0.0);
   else
     p = DTIMath::logUPSpherePDF(xDotMean, k) - 2.53;  

  return p;
}

/***********************************************************************
 *  Method: DTIStats::computeLocalScoreAbsorptionRate
 *  Params: DTIScalarVolume *fa_volume, DTIScalarVolume *voi_volume, const DTIVector &curPos, const DTIVector &termPos, const DTIVector &termSize, int nAbsType, double faThresh, int start_voi
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
DTIStats::computeLocalScoreAbsorptionRate(DTIScalarVolume *wm_volume, DTIScalarVolume *voi_volume, const DTIVector &curPos, double wmThresh, int stop_voi)
{
  // Absorption rate is either 1 or 0 right now
  // 1: Hit target ROI or exited WM
  // 0: Otherwise
  if(wm_volume==NULL)
    return 0;
  
  double wm = wm_volume->getScalar(curPos[0],curPos[1],curPos[2]);

  int voi;
  if(voi_volume!=NULL) {
    voi = (int)voi_volume->getScalar(curPos[0],curPos[1],curPos[2]);
  }

  double rate=0;
  if( voi_volume!=NULL && (voi == stop_voi || voi < 0) ) {
    // We are within the desired end region or we have reached the exclusion mask
    rate = 1; 
  }
  else {
    // We use negative values to use the end region
    if( wm > wmThresh )
      rate = 0;  // WM value is fine
    else
      rate = 1;   // WM value is too low, stop tracing
  }
  return rate;
}

/***********************************************************************
 *  Method: DTIStats::computePathTangent
 *  Params: DTIVector &tangent, const DTIVector &p1, const DTIVector &p2, const DTIVector &p3
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIStats::computePathTangent(DTIVector &tangent, const DTIVector &p1, const DTIVector &p2, const DTIVector &p3)
{
  tangent = norm(norm(p2-p1)*0.5 + norm((p3-p2))*0.5);
}


/***********************************************************************
 *  Method: DTIStats::computePathTangent
 *  Params: DTIVector &tangent, const DTIVector &p1, const DTIVector &p2
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIStats::computePathTangent(DTIVector &tangent, const DTIVector &p1, const DTIVector &p2)
{
  tangent = norm(p2-p1);
}







