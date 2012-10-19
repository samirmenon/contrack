/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef TRACKER_H
#define TRACKER_H

#ifdef USE_MPI_FILEIO
#include <mpi.h>		// MPI header file
#endif

class SequentialTrackGenerator;
class DTIPathwayDatabase;
class DTIPathwayFactory;
class DTIRandomSeedGeneratorMask;
class DTITrackingData;
class DTITractParamsFile;


class Tracker {
 public:
  void setupFromOptionsFile (const char* infoFilename, const char* pathsFilename, bool bTimePathways, bool bPDBFormat=false,bool bVerbose=true,bool bAxialPathways=false,bool bAppend=false);
  void generatePathways(float max_time, int StartID=-1);
  void cleanupGenerator();
  void savePathways();
  void setNumDesiredPaths(int n);
  int getNumDesiredPaths();
  int getNumStartLocs();
  int getLastSavedID();
  //  void time_test(double &d);

 private:
  void setupGenerator();
  void innerGenerateLoop(float max_time);
  
 private:
  DTITrackingData* _tData;
  DTITractParamsFile* _params;
  const char* _pathsFilename;
  bool _bAppend;
  bool _bVoxelwiseCompute;
  bool _bPDBFormat;
  bool _bAxialPathways;
  SequentialTrackGenerator* _pathGenerator;
  DTIPathwayDatabase* _db;
  DTIPathwayFactory* _pathwayFactory;
  bool _bVerbose;

public:
//   const char* _pointsFilename;
//    unsigned int _nCountDrawTangent;
//    float _tTotal;
//    float _tFileIO;
//    float _tSetup;
//    float _tGrow;
//    float _tTangent;
  
#ifdef USE_MPI_FILEIO
  // Asynchronous buffered MPI I/O
  MPI_File out;    
  MPI_Request req; 
  MPI_Status Status;
  float *writeData;
#endif
  
};

#endif

