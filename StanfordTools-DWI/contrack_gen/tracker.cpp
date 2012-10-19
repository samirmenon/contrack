/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

// General Stuff
#include "tracker.h"
#include <iostream>

// Tracing
#include <DTITrackingData.h>
#include <DTIRandomSeedGeneratorMask.h>
#include <DTIRandomSeedGenerator.h>
#include <DTIPathwayFactory.h>
#include "seq_track_gen.h"
#include <DTIPathwayDatabase.h>
#include <DTIPathwayAlgorithmHeader.h>
#include <DTIPathwayIO.h>
#include <DTIPathwayStatisticHeader.h>
#include <DTIPathwayInterface.h>
#include <DTIPathway.h>
#include <DTITractParamsFile.h>
#include <ui_utils.h>

#ifdef WINDOWS
#include <windows.h>
#endif
#include <signal.h>

using namespace std;

void Tracker::setupFromOptionsFile (const char* infoFilename,const char* pathsFilename,bool bVoxelwiseCompute,bool bPDBFormat,bool bVerbose,bool bAxialPathways,bool bAppend)
{
//    _tTotal = 0;
//    _tFileIO = 0;
//   _pointsFilename = pointsFilename;
  _pathsFilename = pathsFilename;
  _bPDBFormat = bPDBFormat;
  _bVoxelwiseCompute = bVoxelwiseCompute;
  _bAxialPathways = bAxialPathways;
  _bAppend = bAppend;
  // Load the parameters from a file
  _bVerbose = bVerbose;
  if(_bVerbose)  
    cerr << "Fetching parameters from disk ... (file: " << infoFilename << ")" << endl;
    
  _tData = new DTITrackingData();
  _tData->setupFromOptionsFile (infoFilename, _bVoxelwiseCompute, _bVerbose);
  _params = _tData->_params;
  
  // Setup generator
  setupGenerator();
}

void Tracker::setupGenerator()
{
  float ccbClosedP = 1;
  float mmScale[3];
  unsigned int dim[4];
  _pathwayFactory = new DTIPathwayFactory();
  _db = new DTIPathwayDatabase();
  _tData->_pdfVol->getVoxelSize (mmScale[0], mmScale[1], mmScale[2]);
  _tData->_pdfVol->getDimension (dim[0], dim[1], dim[2], dim[3]);  
  _db->setVoxelSize (mmScale);  
  _db->setSceneDimension (dim);
  
  DTIPathwayAlgorithmHeader*simulatedHeader = new DTIPathwayAlgorithmHeader;
  strcpy (simulatedHeader->_algorithm_name, "ConTrack");
  _db->addAlgorithmHeader(simulatedHeader);
  double *vmx = _tData->_pdfVol->getTransformMatrix();
  double mx[16] = {1,0,0,-vmx[3], 0,1,0,-vmx[7], 0,0,1,-vmx[11], 0,0,0,1};
  _db->setTransformMatrix (mx);
  
  // Setup statistics headers
  _db->clearStatisticHeaders();
  DTIPathwayStatisticHeader *header = new DTIPathwayStatisticHeader;
  header->_is_computed_per_point = false;
  header->_is_luminance_encoding = true;
  header->_is_viewable_stat = true;
  strcpy (header->_aggregate_name, "Time");
  strcpy (header->_local_name, "NA");
  header->_unique_id = 0;       
  _db->addStatisticHeader(header);
  
  header = new DTIPathwayStatisticHeader;
  header->_is_computed_per_point = false;
  header->_is_luminance_encoding = true;
  header->_is_viewable_stat = true;
  strcpy (header->_aggregate_name, "Num. paths to first accepted");
  strcpy (header->_local_name, "NA");
  header->_unique_id = 1;       
  _db->addStatisticHeader(header);
  
  _pathGenerator = new SequentialTrackGenerator(_pathwayFactory,_tData->_voiMaskVol,_tData->_wmVol,_tData->_pdfVol,_params->dStepSizeMm,_params->kGenSmooth);

  // Set Min/Max pathway length
  _pathGenerator->setMaxNumNodes(_params->nMaxChainLength);
  _pathGenerator->setMinNumNodes(_params->nMinChainLength);

  // Setting seedpoint generation
  if(!_bVoxelwiseCompute) {
    // Doing all voxels for the mask on one processor
    DTIVector test;
    _tData->_sourceGenerator->getNextSeed(test);
    if( _params->bStartIsSeedRegion && (test[0]>-1) )
      _pathGenerator->setStartSeedGenerator(_tData->_sourceGenerator);
    _tData->_endGenerator->getNextSeed(test);
    if( _params->bEndIsSeedRegion && (test[0]>-1) )
      _pathGenerator->setEndSeedGenerator(_tData->_endGenerator);
  } else {
    // Splitting up source voxel per processor, can only do this for one starting ROI
    _pathGenerator->setStartSeedGenerator(NULL);
    _pathGenerator->setEndSeedGenerator(NULL);
  }

  // Set endpoint indices for VOI volume
  _pathGenerator->setStartVOIIndex(_tData->_start_voi);
  _pathGenerator->setStopVOIIndex(_tData->_stop_voi);

  // Set way point VOI info
  _pathGenerator->setNumWayVOI(_tData->_num_way_voi);
  
  // Set prior parameter values
  _pathGenerator->setWMThresh( _params->wmThresh );
  _pathGenerator->setSTDSmoothness( _params->stdSmoothness );
  _pathGenerator->setAngleCutoff( _params->angleCutoff );

  // Special for only generating 2D (axial) pathways for debugging
  _pathGenerator->setAxialPathways( _bAxialPathways );

  // Setup cached PDF for faster computation
  // _pathGenerator->setupPDFCachedGrid(_pointsFilename);
  _pathGenerator->setupPDFCache();

  #ifdef USE_MPI_FILEIO
    char* filename = new char[strlen(_pathsFilename)];
    strcpy(filename,_pathsFilename);
    writeData = NULL;
    MPI_File_open (MPI_COMM_WORLD, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &out);
    delete[] filename;
  #endif
}

void Tracker::cleanupGenerator()
{
  delete _db;
  delete _pathGenerator;
  delete _pathwayFactory;
    #ifdef USE_MPI_FILEIO
    if( writeData != NULL ) 
      MPI_Wait (&req, &Status);    // Wait for last write to complete
    delete[] writeData;
    MPI_File_close (&out);
  #endif
    _tData->cleanup();
    delete _tData;
}

void Tracker::setNumDesiredPaths(int n)
{
  if(_params!=NULL) 
    _params->nDesiredSamples=n;
}

int Tracker::getNumDesiredPaths()
{
  int n=-1; 
  if(_params!=NULL) 
    n=_params->nDesiredSamples; 
  return n;
}

void Tracker::generatePathways(float max_time,int startID)
{
  //    Timer tGen; tGen.start();
  if( startID!=-1 && startID < getNumStartLocs() ) {
    DTIVector v = _tData->_sourceGenerator->getLoc(startID);
    DTIVector s; 
    v[0]+=0.5; v[1]+=0.5; v[2]+=0.5; // Position center of voxel
    s[0]=1; s[1]=1; s[2]=1; // Size of one voxel
    DTIRandomSeedGenerator* sourceGenerator = new DTIRandomSeedGenerator(v,s);
    _pathGenerator->setStartSeedGenerator(sourceGenerator);
    bool bMakePair = true;
    if(bMakePair) {
      bool bForceAllPair = false;
      if(bForceAllPair) {
	// Make pair from this voxel and every other pair of voxels in the mask
	int endID;
	for(endID=0; endID<getNumStartLocs(); endID++) {
	  if(endID!=startID) {
	    v = _tData->_sourceGenerator->getLoc(endID); 
	    v[0]+=0.5; v[1]+=0.5; v[2]+=0.5; // Position center of voxel
	    //DTIRandomSeedGenerator* endGenerator = new DTIRandomSeedGenerator(v,s);
	    //_pathGenerator->setEndSeedGenerator(endGenerator);
	    assert(_tData->_voiMaskVol->getScalar(v[0],v[1],v[2])==_tData->_start_voi);
	    _tData->_voiMaskVol->setScalar(_tData->_stop_voi,v[0],v[1],v[2]);
	    innerGenerateLoop(max_time);
	    _tData->_voiMaskVol->setScalar(_tData->_start_voi,v[0],v[1],v[2]);
	  }
	}
      } else {
	// Make pair from this voxel and any other voxel in the mask
	assert(_tData->_voiMaskVol->getScalar(v[0],v[1],v[2])==_tData->_stop_voi);
	_tData->_voiMaskVol->setScalar(_tData->_start_voi,v[0],v[1],v[2]);
	innerGenerateLoop(max_time);
	_tData->_voiMaskVol->setScalar(_tData->_stop_voi,v[0],v[1],v[2]);
      }
    } else { 
      // Do not force all pairs
      innerGenerateLoop(max_time);
    }
    delete sourceGenerator;
    _pathGenerator->setStartSeedGenerator(_tData->_sourceGenerator);
  } else{
    innerGenerateLoop(max_time);
  }
//    _tTotal += tGen.elapsed();
//    _tSetup = _pathGenerator->_tSetup;
//    _tGrow = _pathGenerator->_tGrow;
//    _tTangent = _pathGenerator->_tTangent;
//    _nCountDrawTangent = _pathGenerator->_nCountDrawTangent;
}

int Tracker::getNumStartLocs()
{
  return _tData->_sourceGenerator->getNumStartLocs();
}

void Tracker::innerGenerateLoop(float max_time)
{    
  DTIPathwayInterface* path = NULL;
  int nTotalSamples = _params->nDesiredSamples;
  int i;
  bool bLengthyAbort = false;
  
  for(i=0; i<nTotalSamples && !bLengthyAbort; i++ ) {  
  
    // Update message
    if( !_bVoxelwiseCompute && i%_params->nSaveOutSpacing==0 && _bVerbose)
      cerr << "Sampling pathways ... "<< 100*i/nTotalSamples << "% done." << endl;

    // Generate pathway
    path = NULL;
    // timer stuff
    clock_t startPP,endPP;
    float diffPP;
    int numPathsPP = 0;
    startPP = clock();
    while(path==NULL && !bLengthyAbort) {
      path = _pathGenerator->generatePathway();
      endPP = clock();
      diffPP = float(endPP-startPP) / CLOCKS_PER_SEC;
      if( i==0 && max_time>0 && diffPP > max_time ) {
        //cerr << "Length of time to first path (" << diffPP << ") is greater than " << max_time << " seconds." << endl;
        bLengthyAbort=true;
      }
      if( _pathGenerator->isOnlyFailedTerm() )
        numPathsPP++;
    }
    
    if(!bLengthyAbort) {
      // No longer first path so lets extend the time cutoff
      max_time = -1;
      path->initializePathStatistics(2, _db->getPathStatisticHeaders(), false);
      path->setPathStatistic ( 0, diffPP);
      path->setPathStatistic ( 1, numPathsPP);
      _db->addPathway((DTIPathway*)path);
      // Save out periodically
      if ((i+1)%_params->nSaveOutSpacing==0 && (i>0 || _params->nSaveOutSpacing==1)) {
        //bool bFirstTime = i == _params->nSaveOutSpacing;
	savePathways( );
      }
    }
  }
  if(_db->getNumFibers()>0 && !bLengthyAbort) {
    // Save any pathways that did not get periodically saved
    //bool bFirstTime = i <= _params->nSaveOutSpacing;
    savePathways( );
  }
}

void Tracker::savePathways( )
{
  //  Timer t; t.start();
  if( _bPDBFormat ) {
    // Save in PDB format usually not on cluster
    if( !_bAppend ) {
      double *vmx = _tData->_pdfVol->getTransformMatrix();
      double ACPC[3] = {-vmx[3],-vmx[7],-vmx[11]};
      ofstream myOut(_pathsFilename, ios::out | ios::binary);
      DTIPathwayIO::saveDatabasePDB (_db, myOut, ACPC);
      myOut.close();
      _bAppend=true;
    } else {
      DTIPathwayIO::appendDatabaseFile (_db, _pathsFilename);
    }
    _db->removeAllPathways();
  } else {
    // Save in compact nonPDB format
    ofstream myOut(_pathsFilename, ios::app | ios::binary);
    if( !myOut ) {
      cerr << "Cannot open output file " << _pathsFilename << endl;
      exit(1);
    }
    string strFilename = _pathsFilename;
    DTIPathwayIO::saveDatabaseBFLOAT (_db, myOut,NULL,strFilename.find(".SBfloat")!=string::npos);
    myOut.close();
    _db->removeAllPathways();
  }
  //  _tFileIO += t.elapsed();
}

int Tracker::getLastSavedID( )
{
  int nID=0;
  if( !_bPDBFormat ) {
    // This is a Bfloat or SBFloat...we have the code for this and are not afraid
    
    cerr << "Opening " << _pathsFilename << " to find last voxel processed..." << endl;
    // Get last pathway in database file
    ifstream ins(_pathsFilename, ios::in | ios::binary);
    if( !ins ) {
      cerr << "Cannot open pathway file " << _pathsFilename << endl;
      exit(1);
    }
    DTIPathway* pathway=NULL;
    bool bStillLoading=true;
    string strFilename = _pathsFilename;
    bool bSplineCompress = strFilename.find(".SBfloat")!=string::npos;
    int nCount=0;
    while(bStillLoading) {
      DTIPathway *next = new DTIPathway((DTIPathwayAlgorithm) 2);
      if(DTIPathwayIO::loadPathwayBFLOAT(next,ins,bSplineCompress)<0) {
	bStillLoading=false;
	delete next;
      }else {
	delete pathway;
	pathway = next;
      } 
      nCount++;
    }
    ins.close();
    assert(pathway!=NULL);
    cerr << "Found last pathway of " << nCount << "pathways." << endl;
    // Get first point of the last pathway in the file
    DTIVector pt = pathway->getPointV(0);
    delete pathway;

    // Get this point into image coordinates
    float mmScale[3];
    _tData->_pdfVol->getVoxelSize (mmScale[0], mmScale[1], mmScale[2]);
    for(int ii=0; ii<3; ii++) pt[ii]= DTIMath::interpNN(pt[ii]/mmScale[ii]);

    cerr << "First point: " << pt[0] << "," << pt[1] << "," << pt[2] << endl;

    //Test all of the starting points until we find a match
    for(; nID < getNumStartLocs(); nID++) {
      DTIVector v = _tData->_sourceGenerator->getLoc(nID);
      //      cerr << "Test point: " << v[0] << "," << v[1] << "," << v[2] << endl;
      //v[0]+=0.5; v[1]+=0.5; v[2]+=0.5; // Position center of voxel
      bool bIn=true;
      for(int ii=0; ii<3; ii++) 
	bIn = bIn & fabs(v[ii]-pt[ii])<0.001;
      if(bIn) 
	break;
    }
  } else {
    // No code
    cerr << "Warning: Cannot start where we left off with .pdb format!" << endl;
  }
  if(nID>=getNumStartLocs())
    nID=0;

  cerr << "Returning startID " << nID << endl;
  return nID;
}

// void Tracker::time_test(double &d)
// {
//   //   double kWatson = _params->kGenSmooth;
//   //   DTIVector v = DTIMath::randUPSphereVector(curDir,kWatson);
  
//   DTIVector curPos(3); curPos[0]=34; curPos[1]=39; curPos[2]=39;
//   DTIVector newDir(3);
//   DTIVector curDir(3); curDir[0]=1; curDir[1]=0; curDir[2]=0;
//   double logProbInc;
//   double _shapeLinearityMidCl = 0.175; 
//   double _shapeLinearityWidthCl = 0.15;
//   double _shapeUniformS = 80;
//   SequentialTrackGenerator::drawTangent(_tData->_pdfVol, curPos, curDir, newDir, logProbInc, _nCountDrawTangent,_pathGenerator->_vCachedPDFSmooth, _params->stdSmoothness, _shapeLinearityMidCl, _shapeLinearityWidthCl, _shapeUniformS, _params->kGenSmooth);
  
//   d += newDir[0];
  
//   //d += DTIMath::randzeroone ();
// }
