/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef FIBER_TRACT_IO
#define FIBER_TRACT_IO

class DTIPathwayDatabase;
class DTIPathway;
class ROIManager;
class DTIPathwaySelection;

#include "typedefs.h"
#include <fstream>
#include <vector>

class DTIPathwayIO 
{
 public:
  static void saveDatabaseAny(DTIPathwayDatabase *db, bool bOverwrite, std::string filename, double ACPC[3]=NULL, DTIPathwaySelection *selection = NULL, bool bVerbose = false);
  // PDB
  static DTIPathwayDatabase *loadAndAppendDatabasePDB(std::istream &pdbStream, DTIPathwayDatabase * oldPdb=0, bool bComputeRAPID = true, double ACPC[3] = NULL, double vox_size[3] = NULL);
  static void saveDatabasePDB (DTIPathwayDatabase *pathways, std::ostream &theStream, double ACPC[3]=NULL, DTIPathwaySelection *selection = NULL);
  static void appendDatabaseFile (DTIPathwayDatabase *pathways, const char* filename, DTIPathwaySelection *selection = NULL);  
  static void savePathwayPDB (DTIPathway *pathway, std::ostream &theStream, DTIPathwayDatabase *db, bool debug);
  // BFLOAT
  static DTIPathwayDatabase *loadDatabaseBFLOAT (std::ifstream &theStream, int numPathwaysToLoad = -1, bool bSplineCompress = false, bool bCaminoNoStats = false);
  static void loadAndAppendDatabaseBFLOAT (DTIPathwayDatabase *pdb, std::ifstream &theStream, int numPathwaysToLoad = -1, bool bSplineCompress = false, bool bCaminoNoStats = false);
  static int loadPathwayBFLOAT(DTIPathway* pathway, std::ifstream &theStream, bool bSplineCompress = false, bool bCaminoNoStats = true);
  static void savePathwayBFLOAT(DTIPathway* pathway, std::ostream &theStream, bool bSplineCompress = false, bool bCaminoNoStats = true);
  static void saveDatabaseBFLOAT(DTIPathwayDatabase *db,std::ostream &pdbStream, DTIPathwaySelection *selection = NULL, bool bSplineComress = false, bool bVerbose = false); 
  static int loadPathwaySegmentBFLOAT(DTIPathway* pathway, std::ifstream &pdbStream, short p1, short p2, bool bSplineCompress = false);

  // DTIQuery
  static void openDatabase (DTIPathwayDatabase *db, const char *filename);
  static void appendFileOffsetsToDatabaseFile (int numPathways, const uint64_t *fileOffsets, const char *filename);    

protected:
  DTIPathwayIO() {}
  virtual ~DTIPathwayIO() {}

  //! load version 2 pdb
  static void loadDatabasePDB2Ver2(std::istream &theStream, DTIPathwayDatabase *db, bool multiplyMatrix, unsigned int headerSize);
  //! load version 3 pdb
  static void loadDatabasePDB2Ver3(std::istream &theStream, DTIPathwayDatabase *db, bool multiplyMatrix);

  // Spline BFLOAT
  const static int SPLINE_MIN_CTRL_POINTS = 5;
  static DTIPathway* CompressPathway (const DTIPathway* pathIn, float fReduceTarget, float fCompressThresh=-1);
  static void DecompressPathway (DTIPathway* pathOut, const DTIPathway* pathIn, uint iSampleQuantity, uint iDegree=3);
  static void CalculateError(const DTIPathway* pathCtrl, const DTIPathway* path, float &fAvrError, float &fRMSError, float &fMaxError);
};

#endif
