#include "seq_track_gen.h"

#include "DTIPathwayInterface.h"
#include "DTISeedGenerator.h"
#include "DTIMath.h"
#include <math.h>
#include <tnt/tnt_array1d_utils.h>
#include <jama/jama_eig.h>
#include "DTIStats.h"
#include "DTIPathwayFactoryInterface.h"
#include <fstream>
#include <iostream>
#include "DTIPathwayIO.h"
#include "DTIPathwayDatabase.h"
#include <DTIConTrackPDF.h>
#include <ui_utils.h>

#include <numeric>
using namespace std;

/***********************************************************************
 *  Method: SequentialTrackGenerator::SequentialTrackGenerator
 *  Params: double probability, DTIPathwayFactoryInterface *pathwayFactory, DTITensorVolume *volume, DTIScalarVolume *wmVolume, double stepSizeMm
 * Effects: 
 ***********************************************************************/
SequentialTrackGenerator::SequentialTrackGenerator(DTIPathwayFactoryInterface *pathwayFactory, DTIScalarVolume *voiVolume, DTIScalarVolume *wmVolume, DTIVectorVolume *pdfVolume, double stepSizeMm, double kGenSmooth)
{
   _bCachedPDF=false;
//    _nCountDrawTangent=0;
//    _tSetup=0;
//    _tGrow=0;
//    _tTangent=0;
  _wm_volume = wmVolume;
  _voi_volume = voiVolume;
  _pdf_volume = pdfVolume;
  _bAxialPathways = false;
  _start_seed_generator = NULL;
  _end_seed_generator = NULL;
  _path_factory = pathwayFactory;

  DTIVector voxSize;
  _pdf_volume->getVoxelSize (voxSize[0], voxSize[1], voxSize[2]);
  _vox_size = voxSize.copy();

  _step_size_voxel = voxSize.copy();
  (_step_size_voxel)[0] = stepSizeMm / voxSize[0];
  (_step_size_voxel)[1] = stepSizeMm / voxSize[1];
  (_step_size_voxel)[2] = stepSizeMm / voxSize[2];
  
  _num_way_voi = 0;
  _start_voi = 0;
  _stop_voi = 0;
  _bKeep_all = false;
  _wm_thresh = 0.19;
  _angle_cutoff = M_PI / 2;
  _shapeLinearityMidCl = 0.175; 
  _shapeLinearityWidthCl = 0.15;
  _shapeUniformS = 80;
  _kGenSmooth = kGenSmooth;  // was 32
}


/***********************************************************************
 *  Method: SequentialTrackGenerator::~SequentialTrackGenerator
 *  Params: 
 * Effects: 
 ***********************************************************************/
SequentialTrackGenerator::~SequentialTrackGenerator()
{
}


/***********************************************************************
 *  Method: SequentialTrackGenerator::proposeMutation
 *  Params: DTIPathwayInterface *prevPath, DTIPathwayInterface * &proposedPath, double inv_temp
 * Returns: double
 * Effects: 
 ***********************************************************************/
DTIPathwayInterface*
SequentialTrackGenerator::generatePathway( )
{
  // Create a new pathway by starting a seed in one of the seed 
  // starting regions and tracing to the other end region
  DTIPathwayInterface* path = _path_factory->createPathway ((DTIPathwayAlgorithm)3);
  
  // Setup growing parameters
  DTIVector seedPoint;
  unsigned int startI;
  bool bWhichEnd;

  // Way for me to just grow from one VOI no termination required
  if( _start_voi==0 && _stop_voi==0 ) {
    delete path;
    return NULL;
  } else if( _start_voi==0 || _stop_voi==0 )
    _bKeep_all = true;
  else
    _bKeep_all = false;
  
  //    Timer tS; tS.start();
  // Keep track of how many pathways did not reach desired ROI, but satisfied all other criteria
  _bOnlyFailedTerm = false;
  if( setupGrow(seedPoint) ) {
    delete path;
    //    _tSetup+=tS.elapsed();
    return NULL;
  }
  //    _tSetup+=tS.elapsed();

  //    Timer tG; tG.start();
  if( goGrow(path, seedPoint) ) {
    // Abort grow
    delete path;
    //    _tGrow+=tG.elapsed();
    return NULL;
  }
  //    _tGrow+=tG.elapsed();
  
  return path;
}


/***********************************************************************
 *  Method: SequentialTrackGenerator::setupGrow
 *  Params: DTIPathwayInterface *prevPath, DTIVector &seedPoint, unsigned int &startI
 * Returns: int
 * Effects: 
 ***********************************************************************/
int
SequentialTrackGenerator::setupGrow(DTIVector &seedPoint)
{
  // We are only going to be completely redrawing paths so not much to setup.
  if( _start_seed_generator==NULL && _end_seed_generator==NULL ) {
    std::cerr << "Tracker failing because there is no place to start!" << std::endl;
    return 1;
  }
  if( _pdf_volume==NULL ) {
    std::cerr << "Tracker failing because there is no pdf image!" << std::endl;
    return 1;
  }
    
  // Check for satisfying brain mask only with pdfVolume because if not our drawing tangents will be buggy
  bool bWithinBrain = false;
  while( !bWithinBrain ) {  
    //int count = 0;
    //while(true) {
      this->getStartpoint(seedPoint);
      //count++;
      //}

    if( _pdf_volume->inBounds(seedPoint[0],seedPoint[1],seedPoint[2],0) ) {      
      //DTIVector paramsVec = (_pdf_volume->getVector((int)floor(seedPoint[0]+0.5),(int)floor(seedPoint[1]+0.5),(int)floor(seedPoint[2]+0.5))).copy();
      DTIArray paramsVec = (_pdf_volume->getVector(seedPoint[0],seedPoint[1],seedPoint[2])).copy();
      //std::cerr << "paramsVec " <<seedPoint[0]<<","<<seedPoint[1]<<","<<seedPoint[2] <<": ";
      //for(int ii=0; ii<4; ii++) {std::cerr << paramsVec[ii] << ", " ;}
      //std::cerr << std::endl;
      //exit(0);
      if( !(paramsVec[0] == 0 && paramsVec[1] == 0 && paramsVec[2] == 0) )
	  bWithinBrain = true;
      else
	std::cerr << "Warning: Throwing out seed point because pdf at voxel ["<<seedPoint[0]<<","<<seedPoint[1]<<","<<seedPoint[2]<<"] is out of bounds." << endl;
    }
  }
  return 0;
}

/***********************************************************************
 *  Method: SequentialTrackGenerator::getStartpoint
 *  Params: DTIPathwayInterface *prevPath, DTIVector &seedPoint, unsigned int &startI
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
SequentialTrackGenerator::getStartpoint(DTIVector &seedPoint)
{
  // Lets switch the seed region randomly so that we grow from both VOIs
  if( _end_seed_generator!=NULL && _start_seed_generator!=NULL ) {
    // Randomly switch seed region
    if( DTIMath::randzeroone() > 0.5 ) {
      int nTempVOI = _stop_voi;
      _stop_voi = _start_voi;
      _start_voi = nTempVOI;
      DTISeedGenerator* sg = _start_seed_generator;
      _start_seed_generator = _end_seed_generator;
      _end_seed_generator = sg;
    }
  }else if ( _end_seed_generator!=NULL ) {
    // Just switch to always start from the end
    _stop_voi = _start_voi;
    _start_seed_generator = _end_seed_generator;
  }
  
  // Get start point
  _start_seed_generator->getNextSeed (seedPoint);

  // Debugging
  // XXX Nearest Neighbor Function
  int voi = (int)_voi_volume->getScalar (seedPoint[0],seedPoint[1],seedPoint[2]);	
  //  if(voi!=_start_voi)
  assert(voi == _start_voi);
}


/***********************************************************************
 *  Method: SequentialTrackGenerator::goGrow
 *  Params: DTIPathwayInterface *prevPath, DTIPathwayInterface * &proposedPath, DTIVector &seedPoint, double &logPNew, const unsigned int startI
 * Returns: int
 * Effects: 
 ***********************************************************************/
int
SequentialTrackGenerator::goGrow(DTIPathwayInterface* path, DTIVector &seedPoint)
{
  DTIVector curPos = seedPoint.copy();
  DTIVector prevPos;
  DTIVector newDir;

  // Place seedpoint as start to path
  path->append (seedPoint*_vox_size);
  
  // Initialize first direction
  DTIVector curDir;
  curDir = SequentialTrackGenerator::initFirstDir(0, path, 0, -1, curPos, _vox_size);    
  if(_bAxialPathways) {
    // Zero out the z direction and normalize
    // Handle special starting case
    if( curDir[0] != 0 || curDir[1] != 0 ) {
      double N = sqrt(curDir[0]*curDir[0]+curDir[1]*curDir[1]);
      curDir[0] = curDir[0]/N;
      curDir[1] = curDir[1]/N;
    }
    curDir[2]=0;
  }

  // Lets trace the path until we are absorbed
  bool bAbsorbed = false;
  bool bOutOfBounds = false;
  bool bReachedWayMask = false;
  bool bBadCurvature = false;
  bool bFailedMaxLength = false;
  int nCurWayVOI = 3;
  int nNumPoints = path->getNumPoints();
  while (!bAbsorbed) {      
      // Get next tangent direction according to a closed form
      // probability distribution based on STT
      double logProbInc;

      //Timer tT; tT.start();
      SequentialTrackGenerator::drawTangent(_pdf_volume, curPos, curDir, newDir, logProbInc, _vCachedPDFSmooth, _std_smooth, _shapeLinearityMidCl, _shapeLinearityWidthCl, _shapeUniformS, _kGenSmooth);
      //_tTangent+=tT.elapsed();

      if(_bAxialPathways) {
	// Zero out the z direction and normalize
	double N = sqrt(newDir[0]*newDir[0]+newDir[1]*newDir[1]);
	newDir[0] = newDir[0]/N;
	newDir[1] = newDir[1]/N;
	newDir[2] = 0;
      }

      double quotient = dproduct(curDir,newDir);
      double angle = acos(quotient);
      if( angle > _angle_cutoff ) {
        bAbsorbed = true;
        bBadCurvature = true;
      }	
      
      // trace along current direction, add point
      curDir = newDir.copy();
      prevPos = curPos.copy();
      curPos  = prevPos.copy() + _step_size_voxel.copy()*curDir.copy();	
      path->append (_vox_size*curPos);
      nNumPoints++;
      
      // Have we gone horribly awry and somehow made it outside of the data?
      if( !_pdf_volume->inBounds(curPos[0],curPos[1],curPos[2],0) ) {
        // Outside of entire data volume should NEVER happen
        //std::cerr << "Reached bounds of PDF volume.  This shouldn't happen." << std::endl;
	//std::cerr << "Pos: " << curPos[0] << "," << curPos[1] << "," << curPos[2] << std::endl;
	//exit(0);
	return 1;
      }
      
      // Check for satisfying brain mask only with pdfVolume because if not our drawing tangents will be buggy
      DTIArray paramsVec = (_pdf_volume->getVector(curPos[0],curPos[1],curPos[2])).copy();
      if( paramsVec[0]==0 && paramsVec[1]==0 && paramsVec[2]==0 ) {
	bOutOfBounds = true;
	bAbsorbed = true;
      }

      // Check to see if we reached the next way point mask
      if( _num_way_voi>0 && !bReachedWayMask ) {
	int n = SequentialTrackGenerator::reachedWayMaskFlag(_wm_volume, _voi_volume, curPos, _wm_thresh, nCurWayVOI, _start_voi);
	// If we reached a way mask out of order then absorb
	if(n<0)
	  bAbsorbed = true;
	// We reached the way VOI
	if(n>0) {
	  // Finished all way VOIs or update current searched way
	  if(nCurWayVOI==(_num_way_voi-1))
	    bReachedWayMask = true;
	  else
	    nCurWayVOI++;
	}
      }

      // See if we should stop the path tracing because of reaching endpoints or exclusion
      if(!bAbsorbed) {
	if(SequentialTrackGenerator::absorptionRate(_wm_volume, _voi_volume, curPos, _wm_thresh, _stop_voi)>0)
	  bAbsorbed = true;
      }

      if( nNumPoints >= _max_num_nodes ) {
	// This path is outside length bounds we must throw it out
	// unless we don't have an end region and are keeping all
	// paths
	bAbsorbed = true;
	if( !_bKeep_all )
	  bFailedMaxLength = true;
      }
  }

  // Check to see if we hit the end region
  bool bSatisfiedTerm = SequentialTrackGenerator::satisfiedTerm( _voi_volume, curPos, _stop_voi );
  if( _num_way_voi==0 )
    bReachedWayMask = true;    
  bool bMinLength = nNumPoints >= _min_num_nodes;
  bool bFailedTerm = !_bKeep_all && ( bOutOfBounds || !bReachedWayMask || !bSatisfiedTerm );
  
  // Check to see if we should throw out path
  if( !bBadCurvature && bMinLength && !bFailedMaxLength ) {
    _bOnlyFailedTerm = true;
    if( bFailedTerm )
      return 1;
  } else {
    return 1;
  }

  return 0;
}



/***********************************************************************
 *  Method: SequentialTrackGenerator::initFirstDir
 *  Params: bool bWhichEnd, const DTIPathwayInterface *prevPath, unsigned int startI, int endI, const DTIVector &curPos, const DTIVector &vox_size
 * Returns: DTIVector
 * Effects: 
 ***********************************************************************/
DTIVector
SequentialTrackGenerator::initFirstDir(bool bWhichEnd, const DTIPathwayInterface *prevPath, unsigned int startI, int endI, const DTIVector &curPos, const DTIVector &vox_size)
{
  DTIVector dir;
  if( (!bWhichEnd && startI == 0) || (bWhichEnd && endI < 0) || (bWhichEnd && endI == prevPath->getNumPoints()-1) ) {
    // Initialize bogus previous direction for prior
    DTIVector originalDir;
    originalDir[0] = 0;
    originalDir[1] = 0;
    originalDir[2] = 0;
    dir = originalDir.copy();
  } 
  else if (!bWhichEnd){
    DTIVector temp = prevPath->getPointV(startI-1);
    dir = norm(curPos*vox_size-temp).copy();
  }
  else {
    DTIVector temp = prevPath->getPointV(endI+1);
    dir = norm(curPos*vox_size-temp).copy();
  }
  return dir.copy();
}


/***********************************************************************
 *  Method: SequentialTrackGenerator::absorptionRate
 *  Params: DTIScalarVolume *wm_volume, const DTIVector &curPos, double wmThresh
 * Returns: double
 * Effects: 
 ***********************************************************************/
double
SequentialTrackGenerator::absorptionRate(DTIScalarVolume *wm_volume, DTIScalarVolume *voi_volume, const DTIVector &curPos, double wmThresh, int stop_voi)
{
  return DTIStats::computeLocalScoreAbsorptionRate(wm_volume,voi_volume,curPos,wmThresh,stop_voi);
}

/***********************************************************************
 *  Method: SequentialTrackGenerator::satisfiedTerm
 *  Params: DTIScalarVolume *wm_volume, DTIScalarVolume *voi_volume, const DTIVector &curPos, int stop_voi
 * Returns: bool
 * Effects: 
 ***********************************************************************/
bool
SequentialTrackGenerator::satisfiedTerm(DTIScalarVolume *voi_volume, const DTIVector &curPos, int stop_voi)
{
  // Make sure we satisfy the appropriate termination criteria
  int voi = (int)voi_volume->getScalar( curPos[0],curPos[1],curPos[2] );
  return voi == stop_voi;
}

/***********************************************************************
 *  Method: SequentialTrackGenerator::reachedWayMaskFlag
 *  Params: DTIScalarVolume *wm_volume, DTIScalarVolume *voi_volume, const DTIVector &curPos, double wmThresh, int way_voi, int start_voi
 * Returns: double
 * Effects: 
 ***********************************************************************/
int
SequentialTrackGenerator::reachedWayMaskFlag(DTIScalarVolume *wm_volume, DTIScalarVolume *voi_volume, const DTIVector &curPos, double wmThresh, int way_voi, int start_voi)
{
  // Return value based on current VOI value
  // 1: This value satisfies the given VOI index and is within WM
  // -1: Have reached a different VOI value other than given or start
  // 0: Otherwise
  double wm = wm_volume->getScalar(curPos[0],curPos[1],curPos[2]);
  int voi = (int)voi_volume->getScalar(curPos[0],curPos[1],curPos[2]);

  bool bThresh = wm > wmThresh;
  bool bVOI = voi == way_voi;
  bool bOtherVOI = voi > 0 && voi!=start_voi;
  int nRet = 0;
  if( bVOI && bThresh)
    nRet = 1;
  else if(bOtherVOI)
    nRet = -1;

  return nRet;
}

/***********************************************************************
 *  Method: SequentialTrackGenerator::drawTangent
 *  Params: DTIVectorVolume *pdfVolume, const DTIVector &curP, const DTIVector &curDir, DTIVector &nextDir, double &log_prob_chose, double prior_k
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
SequentialTrackGenerator::drawTangent(DTIVectorVolume *pdfVolume, const DTIVector &curP, const DTIVector &curDir, DTIVector &nextDir, double &log_prob_chose, std::vector<DTIVector> &vVecs, double prior_k, double linearityMidCl, double linearityWidthCl, double uniform_s, double kGenSmooth)
{
  //  nCountDrawTangent++;
  // Purpose is to draw a tangent from a closed form distribution that
  // is a lot like the local likelihood or prior

  //  std::cerr << "Drawing from spherical distribution." << std::endl;

  // Get parameters from the pdfVolume
  DTIArray paramsVec = (pdfVolume->getVector(curP[0],curP[1],curP[2])).copy();

  if( !DTIConTrackPDF::isValidVersion(paramsVec) ) {
    std::cerr << "FATAL: PDF parameters image does not have the right dimensions." << std::endl;
    exit(0);
  }
  DTIConTrackPDF::Params p = DTIConTrackPDF::getPDFParams(paramsVec);
  // Fixing lowest uncertainty for thermal noise and adding variation for shape.
  double thermalnoise_thresh = 205;
  //  double kWatson = 18;
  if( p.fScannerK < -thermalnoise_thresh || p.fScannerK == 0)
    p.fScannerK = thermalnoise_thresh;
  double linshape_ds = uniform_s / (1+exp(-(linearityMidCl-p.fCl)*10/linearityWidthCl));
  double ratio_l3 = p.fE3 / (p.fE2+p.fE3);
  double ds1 = linshape_ds*ratio_l3;
  double ds2 = linshape_ds*(1-ratio_l3);
  double dk1 = 1 / (sin(ds1*M_PI/180)*sin(ds1*M_PI/180));
  double dk2 = 1 / (sin(ds2*M_PI/180)*sin(ds2*M_PI/180));
  double comp_k1 = p.fScannerK*dk1 / (dk1 + p.fScannerK + 2*sqrt(p.fScannerK*dk1));
  double comp_k2 = p.fScannerK*dk2 / (dk2 + p.fScannerK + 2*sqrt(p.fScannerK*dk2));	
  double k1 = -comp_k1;
  double k2 = -comp_k2;
  if(k1 > -1 ) k1 = -1;
  if(k2 > -1 ) k2 = -1;
  if( k2 < -205 || k1 < -205 )  {k2 = -205; k1 = -205;}
  if( k1 >= k2 ) k1 = k2 - 0.01; // This has only happened because of round off error
  //double kWatson = -k2;

  // Get spherical coordinates of new direction
  if( curDir[0]==0 && curDir[1]==0 && curDir[2]==0) {
    // Do double sided spherical distribution based on likelihood
    nextDir = DTIMath::randFB5Vector(p.vE1,p.vE2,p.vE3,k1,k2);
    // Get probability of what I just did
    //log_prob_chose = DTIMath::logFB5PDF(nextDir,p.vE1,p.vE2,k1,k2) - 2.53;
  }else {
    // Do single sided spherical distribution based on likelihood
    DTIVector tangent;
    // Too low certainty on voxel than follow previous direction
    if( k1 > -kGenSmooth )  {// 10 degree dispersion 
      //    if(true) {
      //Brain
      double kWatson = kGenSmooth;
      //Muscle
      //kWatson = 68;
      tangent = DTIMath::randUPSphereVector(curDir,kWatson);
      //tangent = SequentialTrackGenerator::randTangentCache(curDir,vVecs);
      // Need to divide prior calculation by 2*PI for hemisphere
      //log_prob_chose = DTIMath::logUPSpherePDF( dproduct(tangent,curDir), kWatson) - 2.53;
    }
    else {
      tangent = DTIMath::randFB5Vector(p.vE1,p.vE2,p.vE3,k1,k2);
      // Get probability of what I just did (add log(2) because we are only doing hemisphere)
      // Divide pdf value by 4*pi so that it integrates to 1 
      //log_prob_chose =  DTIMath::logFB5PDF(tangent,p.vE1,p.vE2,k1,k2) - 2.53;
    }

    //  // Do not allow 
    if( dproduct (p.vE3,curDir) < 0 ) {
      if( dproduct (p.vE3,tangent) > 0 ) {
 	tangent = -tangent;
      }
    }
    else {
      if( dproduct (p.vE3,tangent) < 0 ) {
 	tangent = -tangent;
      }
    }

    nextDir = tangent;// - curDir;
  }
  // Return the next dir
}

void SequentialTrackGenerator::setupPDFCache()
{
  _bCachedPDF=true;
  // Simply make a large number of random draws that will just be saved
  int nNumRand = 10000;
  _vCachedPDFSmooth.clear();
  _vCachedPDFSmooth.reserve(nNumRand);
  // Default previous direction on z-axis
  DTIVector curDir(3,0.0f); curDir[2]=1; 
  for(int ii=0; ii<nNumRand; ii++) {
    DTIVector v = DTIMath::randUPSphereVector(curDir,_kGenSmooth);
    _vCachedPDFSmooth.push_back(v);
  }
}

DTIVector SequentialTrackGenerator::randTangentCache(const DTIVector &curDir, vector<DTIVector> &vVecs)
{
assert(vVecs.size()>0);

  // Make random draw with replacement from cached samples
  uint nIndexR = vVecs.size();
  while(nIndexR==vVecs.size())
    nIndexR = (uint)floor(vVecs.size()*DTIMath::randzeroone());
  DTIVector vR = vVecs[nIndexR].copy();
  
  // Rotate from x-axis into current frame depending on curDir
  double thetaC,phiC,thetaR,phiR,r;
  DTIVector vM(3); vM[2]=1;
  DTIMath::convertEucToSphCamino(curDir, thetaC, phiC, r);
  //DTIMath::convertEucToSphCamino(vR, thetaR, phiR, r);
  //DTIMath::convertSphToEucCamino(vR, thetaR+thetaC-thetaM, phiR+phiC-phiM, r); 

  DTIMatrix mTheta(3,3,0.0f);
  mTheta[0][0] = cos(thetaC);  mTheta[0][2] = sin(thetaC);
  mTheta[1][1] = 1;
  mTheta[2][0] = -sin(thetaC);  mTheta[2][2] =  cos(thetaC);
  DTIMatrix mPhi  (3,3,0.0f);
  mPhi[0][0] = cos(phiC);  mPhi[0][1] = -sin(phiC);
  mPhi[1][0] = sin(phiC);  mPhi[1][1] =  cos(phiC);
  mPhi[2][2] = 1;   
 
  vR = mTheta * vR;
  vR = mPhi * vR;
  return vR;
}


// void SequentialTrackGenerator::setupPDFCachedGrid(const char* pointsFilename)
// {
//   ifstream in(pointsFilename, std::ios::in);
//   if(in) {
//     cerr << "Caching smooth PDF ..." << endl;
//     _bCachedPDF=true;
//     int nVecs;
//     in >> nVecs;
//     _vCachedPDFVecs.clear();
//     _vCachedPDFVecs.resize(nVecs);
//     for(int vv=0; vv<nVecs; vv++) {
//       DTIVector v(3);
//       for(int ii=0; ii<3; ii++) {
// 	double f;
// 	in >> f;
// 	v[ii]=f;
//       }
//       _vCachedPDFVecs[vv] = v;
//     }
//     in.close();
//   }
//   DTIVector vMu(3); vMu[0]=1; vMu[1]=0; vMu[2]=0;
//   _vCachedCDF.clear();
//   _vCachedCDF.resize(_vCachedPDFVecs.size());
//   for(int vv=0; vv<_vCachedPDFVecs.size(); vv++){
//     double d = dproduct(vMu,_vCachedPDFVecs[vv]);
//     _vCachedCDF[vv] = exp(DTIMath::logUPSpherePDF(d,_kGenSmooth));
//   }
//   double fSum = accumulate(_vCachedCDF.begin(),_vCachedCDF.end(),0.0);
//   for(int vv=0; vv<_vCachedCDF.size(); vv++) {
//     _vCachedCDF[vv] /= fSum;
//     if(vv!=0)
//       _vCachedCDF[vv]+=_vCachedCDF[vv-1];
//   }
// }

// DTIArray SequentialTrackGenerator::randCachedTangent(const DTIVector &curDir, vector<DTIVector> &vVecs, vector<double> &vCDF)
// {
//   assert(vVecs.size()==vCDF.size());
//   assert(vCDF.size()>0);

//   double theta,phi,r;
//   DTIMath::convertEucToSph(curDir, theta, phi, r);
//   DTIMatrix mTheta(3,3,0.0f);
//   mTheta[0][0] = cos(theta);  mTheta[0][2] = -sin(theta);
//   mTheta[1][1] = 1;
//   mTheta[2][0] = sin(theta);  mTheta[2][2] =  cos(theta);
//   DTIMatrix mPhi  (3,3,0.0f);
//   mPhi[0][0] = cos(phi);  mPhi[0][1] = -sin(phi);
//   mPhi[1][0] = sin(phi);  mPhi[1][1] =  cos(phi);
//   mPhi[2][2] = 1;   
//   //cout << "Applied theta: " << theta*180/M_PI << " phi: " << phi*180/M_PI << endl;
//   //cout << "vec: " << x[0] << "," << x[1] << "," << x[2] << endl;
//   //cout << "Returnd theta: " << theta*180/M_PI << " phi: " << phi*180/M_PI << endl;

//   // Select rand U[0,1] number
//   double fR = DTIMath::randzeroone();
//   // Use the CDF to invert this as a random vector selection
//   vector<double>::iterator iter = lower_bound( vCDF.begin(), vCDF.end(), fR );
//   int nIndex = iter - vCDF.begin();

//   // Apply rotation
//   DTIVector v = vVecs[nIndex].copy();
//   v = mTheta * v;
//   v = mPhi * v;

//   return v;
// }
