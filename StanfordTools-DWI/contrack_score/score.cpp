/***********************************************************************
 * AUTHOR: Anthony J. Sherbondy
 *   FILE: score.cpp
 *   DATE: Wed Dec 19 13:01:52 2007
 *  DESCR: 
 ***********************************************************************/
#include "score.h"
#include <DTITrackingData.h>
#include <iostream>

// Setup
#include <DTIPathwayIO.h>

// Sampling top database
#include <DTIPathwayDatabase.h>
#include <DTIPathwayAlgorithmHeader.h>
#include <DTIPathwayStatisticHeader.h>
#include <DTIPathway.h>
#include <DTIStats.h>
#include <DTIMath.h>
#include <DTIVolume.h>
#include <DTITractParamsFile.h>
#include <math.h>
#include <map>
#include <algorithm>

using namespace std;


/***********************************************************************
 *  Method: Score::setupFromOptionsFile
 *  Params: const char *infoFilename, const char *outPathsFilename, std::vector<const char *> pathFiles, bool bPDBFormat, bool bVerbose
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
Score::setupFromOptionsFile(const char *infoFilename, const char *outPathsFilename, const char* inPathwayFile, bool bVerbose)
{
  _outPathsFilename = outPathsFilename;
  _bVerbose = bVerbose;
  _bAggScore = false;
  _bSavingConsecutive = false;

  // Load the parameters from a file
  if(_bVerbose)  
    cerr << "Fetching parameters from disk ... (file: " << infoFilename << ")" << endl;
    
  _tData = new DTITrackingData();
  _tData->setupFromOptionsFile (infoFilename, false, _bVerbose);
  _db = NULL;
  _inPathsFilename = inPathwayFile;

  // Seed the RNG
  DTIMath::initRNG();
  //srand((unsigned int)time((time_t *)NULL));
}


/***********************************************************************
 *  Method: Score::loadPathways
 *  Params: std::vector<const char *> pathFiles
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
Score::loadAllPathways(std::vector<string> pathFiles)
{
  if(_tData==NULL) {
    cerr << "Unable to load pathways before tractography data." << endl;
    return;
  }

  double *vmx = _tData->_pdfVol->getTransformMatrix();
  double ACPC[3] = {-vmx[3],-vmx[7],-vmx[11]};
  double vox_size[3];
  _tData->_pdfVol->getVoxelSize(vox_size[0],vox_size[1],vox_size[2]);

  // Load each input path file into one database
  _db = NULL;
  for(int ff=0; ff<pathFiles.size(); ff++) {
    if(_bVerbose)  
      cerr << "Loading input file: " << pathFiles[ff] << std::endl;
    ifstream inStream (pathFiles[ff].c_str(), ios::in | ios::binary);
    double foo[3];
    string strFilename = pathFiles[ff];
    if(strFilename.find(".Bfloat")!=string::npos) {
      // Camino format files
      if( _db )
	DTIPathwayIO::loadAndAppendDatabaseBFLOAT (_db,inStream,-1,false,_bCaminoNoStatsInput);
      else
	_db = DTIPathwayIO::loadDatabaseBFLOAT (inStream,-1,false,_bCaminoNoStatsInput);
	
    }
    else if(strFilename.find(".SBfloat")!=string::npos) {
      // Camino format files
      if( _db )
	DTIPathwayIO::loadAndAppendDatabaseBFLOAT (_db,inStream,-1,true);
      else
	_db = DTIPathwayIO::loadDatabaseBFLOAT (inStream,-1,true);
	
    }
    else if(strFilename.find(".pdb")!=string::npos) {
      // P_DB format files
      DTIPathwayDatabase *db = DTIPathwayIO::loadAndAppendDatabasePDB(inStream, _db, false, ACPC, vox_size);
      if(_db != db) {
	delete _db;
	_db = db;
      }

//       if( _db ) {
// 	DTIPathwayIO::loadAndAppendDatabasePDB (_db, inStream, false, ACPC);
//       } else {
// 	_db = DTIPathwayIO::loadDatabasePDB (inStream, false, ACPC, vox_size);
//       }
    }
    // else if(strFilename.find(".opdb")!=string::npos) {
//       // P_DB format files
//       if( _db )
// 	DTIPathwayIO::loadAndAppendDatabasePDB (_db, inStream, false, ACPC);
//       else
// 	_db = DTIPathwayIO::loadDatabaseOld (inStream, false, ACPC);
//     }
    
    inStream.close();    
  }
}


/***********************************************************************
 *  Method: Score::getPathway
 *  Params: 
 * Returns: void
 * Effects: 
 ***********************************************************************/
DTIPathway* 
Score::getPathway( )
{
  // The scoring algorithm treats pathway storage as a stack.  When
  // the storage on the computer is empty then more pathways will be
  // pulled from the file, when the file is finished then we return
  // NULL;

  if( _db == NULL ) 
    initInputDB(_inPathsFilename);
  else {
    // This assumes that we already popped the last path off of the stack
    //_db->removePathway(_db->getPathway(0));
  }

   
  // See if we need to refresh the database
  if(_curPathStackIter >= _db->getNumFibers()) {
    _db->removeAllPathways();
    _curPathStackIter=0;

    string strFilename = _inPathsFilename;
    if( strFilename.find(".Bfloat")!=string::npos ||
	strFilename.find(".SBfloat")!=string::npos ) {
      // Camino format files
      if( _curStreamPos >= _endStreamPos ) {
	// There are no more pathways to load from the file
	delete _db;
	_db = NULL;
	return NULL;
      }
      ifstream inStream (_inPathsFilename, ios::in | ios::binary);
      inStream.seekg(_curStreamPos,ios::beg);
      // Can assume camino format files
      bool bSBfloat = strFilename.find(".SBfloat")!=string::npos;
      if (!bSBfloat)
	DTIPathwayIO::loadAndAppendDatabaseBFLOAT (_db, inStream, _pdbInInc, false, _bCaminoNoStatsInput);
      else
	DTIPathwayIO::loadAndAppendDatabaseBFLOAT (_db, inStream, _pdbInInc, true);
      if(inStream.eof() || inStream.bad())
	_curStreamPos=_endStreamPos;
      else
	_curStreamPos=inStream.tellg();   

      inStream.close();    
    }else {
      // P_DB format files
      return NULL;
    }
  }
  
  // Lets get a pathway off of the database, we will pop it off on the
  // next call to this function
  DTIPathway* pathway = _db->getPathway(_curPathStackIter);
  _db->replacePathway(_curPathStackIter,NULL); // We are giving this file's memory allocation to the other database.
  _curPathStackIter++;
  return pathway;
}

/***********************************************************************
 *  Method: Score::initInputDB
 *  Params: const char* filename
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
Score::initInputDB(const char* filename)
{
  // The scoring algorithm treats pathway storage as a stack. This
  // function initializes the pathway database from the pdb file.

  _inPathsFilename=filename;
  _db = NULL;    
  ifstream inStream (filename, ios::in | ios::binary);
  if( !inStream ) {
    cerr << "Cannot open input file " << filename << endl;
    exit(1);
  }
  inStream.seekg(0,ios::end);
  _endStreamPos = inStream.tellg();
  inStream.seekg(0,ios::beg);
  string strFilename = filename;
  if(strFilename.find(".Bfloat")!=string::npos) {
    // Camino format files
    _db = DTIPathwayIO::loadDatabaseBFLOAT (inStream, _pdbInInc, false, _bCaminoNoStatsInput);
  }else if(strFilename.find(".SBfloat")!=string::npos) {
    // Camino format files
    _db = DTIPathwayIO::loadDatabaseBFLOAT (inStream, _pdbInInc, true);
  }else {
    // P_DB format files
    if(_tData==NULL) {
      cerr << "Unable to load pathways before tractography data." << endl;
      return;
    }
    double *vmx = _tData->_pdfVol->getTransformMatrix();
    double ACPC[3] = {-vmx[3],-vmx[7],-vmx[11]};
    double vox_size[3];
    _tData->_pdfVol->getVoxelSize(vox_size[0],vox_size[1],vox_size[2]);
    cerr << "Warning: Can only load database incrementally for .Bfloat format." << endl;
    DTIPathwayDatabase *db = DTIPathwayIO::loadAndAppendDatabasePDB(inStream, _db, false, ACPC, vox_size);
    if(_db != db) {
      delete _db;
      _db = db;
    }
      //_db = DTIPathwayIO::loadDatabasePDB (inStream, false, ACPC, vox_size);
    //exit(-1);
  } 
  bool bE = inStream.eof();
  bool bB = inStream.bad();
  if(inStream.eof() || inStream.bad())
    _curStreamPos=_endStreamPos;
  else
    _curStreamPos=inStream.tellg();   

  inStream.close();
  _curPathStackIter=0;
}


/***********************************************************************
 *  Method: Score::cleanup
 *  Params: 
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
Score::cleanup()
{
}


/***********************************************************************
 *  Method: Score::saveConsecutivePathways
 *  Params: int nSave
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
Score::saveConsecutivePathways(int nSave)
{
  bool bWriteDBFirstTime = true;
  _bSavingConsecutive = true;

  // Initialize the input database
  initInputDB(_inPathsFilename);

  string strFilename = _outPathsFilename;
  bool bPDBFormatOut=false;
  if(strFilename.find(".pdb")!=string::npos)
    bPDBFormatOut=true;

  // Get output database to store pathways for writing
  DTIPathwayDatabase* dbOut = getOutputDB();

  // Select the first pathways regardless of score
  for( int pp=0; pp<nSave; pp++ ) {
    DTIPathway* add_path = getPathway();
    //cout << add_path->getNumPoints() << endl;
    if(add_path==NULL)
      break;
    float log_score=0;
    addPathwayToOutDB(add_path, dbOut, log_score);
    if( !bPDBFormatOut && _curPathStackIter >= _db->getNumFibers() ){
      if(_bVerbose)  
	cerr << "Writing " << pp+1 << " pathways." << endl;
      // We have reached the end of a pathway increment lets store the
      // current pathways
      // Write the database to a file
      writeOutputDB(dbOut,bWriteDBFirstTime);
      bWriteDBFirstTime=false;
      // Don't erase the pathway data itself becuase the erasing of
      // the input data will do this, but do erase the database and
      // then get a new one
      delete dbOut;
      dbOut = getOutputDB();
    }
  }
  //cerr << "Output contains " << dbOut->getNumFibers() << " pathways." << endl;
  // Write the database to a file
  writeOutputDB(dbOut,bWriteDBFirstTime);
}


/***********************************************************************
 *  Method: Score::saveTopScoringPathways
 *  Params: int nSave
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
Score::saveTopScoringPathways(int nSave)
{
  // Get output database to store pathways for writing
  if(_bVerbose)  
    cerr << "Initializing output database..." << endl;
  DTIPathwayDatabase* dbOut = getOutputDB();

  // Score pathways
  float smoothParam = _tData->_params->stdSmoothness;
  float lengthParam = _tData->_params->absRateNormal;
  float tensorShapeParam = _tData->_params->shapeLinearityMidCl;
  scoreAllPathways( lengthParam, smoothParam, tensorShapeParam );

  // Reorder pathways according to descending score 
  sort   ( _auxSamples.begin(), _auxSamples.end() );
  reverse( _auxSamples.begin(), _auxSamples.end() );

  // Sample the desired number of samples just storing path index and score
  //map<unsigned int, double> sampled_ids;
  // map<unsigned int, double>::iterator map_iter;
  if(_bVerbose)  
    cerr << "Returning highest " << nSave << " pathways from database." << endl;
  for( int pp=0; pp < _auxSamples.size() && pp < nSave; pp++ ) {
    //sampled_ids.insert( make_pair(_auxSamples[pp].index, _auxSamples[pp].score) );
    DTIPathway* add_path = _db->getPathway( _auxSamples[pp].index );
    float log_score = _auxSamples[pp].score;
    addPathwayToOutDB(add_path, dbOut, log_score);
  }
  if(_bVerbose)  
    cerr << "Output contains " << dbOut->getNumFibers() << " pathways." << endl;

  // Add selected samples to the output DB
//   for( map_iter = sampled_ids.begin(); map_iter != sampled_ids.end(); ++map_iter ) {
//     DTIPathway* add_path = _db->getPathway( map_iter->first );
//     float log_score = map_iter->second;
//     addPathwayToOutDB(add_path, dbOut, log_score);
//   }

  // Write the database to a file
  writeOutputDB(dbOut);
}

/***********************************************************************
 *  Method: Score::saveAllPairsTopScoringPathways
 *  Params: int nSave
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
Score::saveAllPairsTopScoringPathways(int nSave)
{
  // Get output database to store pathways for writing
  if(_bVerbose)  
    cerr << "Initializing output database..." << endl;
  DTIPathwayDatabase* dbOut = getOutputDB();

  // Score pathways
  float smoothParam = _tData->_params->stdSmoothness;
  float lengthParam = _tData->_params->absRateNormal;
  float tensorShapeParam = _tData->_params->shapeLinearityMidCl;
  scoreAllPathways( lengthParam, smoothParam, tensorShapeParam, true );

  // Reorder pathways according to endpoint indices 
  sort   ( _auxPairSamples.begin(), _auxPairSamples.end() );

  // Descend the sorted list only retaining the top pathways for all pairs of voxels
  vector<AllPairsSample>::iterator it = _auxPairSamples.begin();
  vector<AllPairsSample>::iterator next_it;
  int nTotalPairs=0; // Keep track of all pairs saved
  while( it != _auxPairSamples.end() ) {
    nTotalPairs++;
    next_it = find_end(_auxPairSamples.begin(), _auxPairSamples.end(), it, it+1);
    assert(next_it!=_auxPairSamples.end());
    // Go to the position just after the last instance of the same pair
    next_it++;
    // Create a vector for this endpoint pair so that we can find the top scoring pathways
    unsigned int nPairs = (unsigned int)(next_it-it);
    std::vector<PathSample> auxSamples(nPairs); 
    for(int ii=0; ii<nPairs; ii++) {
      auxSamples[ii] = (it+ii)->sample;
    }
    it+=nPairs;
    assert(it==next_it);
    // Pick the top scoring of the pair
    sort   ( auxSamples.begin(), auxSamples.end() );
    reverse   ( auxSamples.begin(), auxSamples.end() );
    for( int pp=0; pp < auxSamples.size() && pp < nSave; pp++ ) {
      DTIPathway* add_path = _db->getPathway( auxSamples[pp].index );
      float log_score = auxSamples[pp].score;
      addPathwayToOutDB(add_path, dbOut, log_score);
    }
  }
  if(_bVerbose)  {
    cerr << "Output contains " << dbOut->getNumFibers() << " pathways over a total of " << nTotalPairs << " ROI voxel pairs." << endl;
  }
  // Write the database to a file
  writeOutputDB(dbOut);
}


/***********************************************************************
 *  Method: Score::saveThresholdScorePathways
 *  Params: float tScore
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
Score::saveThresholdScorePathways(float tMinScore, bool bUseMaxScore, float tMaxScore)
{

 // Get output database to store pathways for writing
  if(_bVerbose)  
    cerr << "Initializing output database..." << endl;
  DTIPathwayDatabase* dbOut = getOutputDB();

  // Score pathways
  float smoothParam = _tData->_params->stdSmoothness;
  float lengthParam = _tData->_params->absRateNormal;
  float tensorShapeParam = _tData->_params->shapeLinearityMidCl;
  scoreAllPathways( lengthParam, smoothParam, tensorShapeParam );

  // Reorder pathways according to descending score 
  sort   ( _auxSamples.begin(), _auxSamples.end() );
  reverse( _auxSamples.begin(), _auxSamples.end() );

  // Sample the desired number of samples just storing path index and score
  //map<unsigned int, double> sampled_ids;
  // map<unsigned int, double>::iterator map_iter;
  if(_bVerbose)  {
    if(bUseMaxScore)
      cerr << "Returning all pathways with log2 scores between: " << tMinScore << " -> " << tMaxScore << endl; 
    else
      cerr << "Returning all pathways with log2 scores between: " << tMinScore << " -> " << _auxSamples[0].score << endl; 
  }
  for( int pp=0; pp < _auxSamples.size() && _auxSamples[pp].score >= tMinScore; pp++ ) {
    //sampled_ids.insert( make_pair(_auxSamples[pp].index, _auxSamples[pp].score) );
    if( !bUseMaxScore || _auxSamples[pp].score <= tMaxScore ) { 
      DTIPathway* add_path = _db->getPathway( _auxSamples[pp].index );
      float log_score = _auxSamples[pp].score;
      addPathwayToOutDB(add_path, dbOut, log_score);
    }
  }
  if(_bVerbose)  
    cerr << "Output contains " << dbOut->getNumFibers() << " pathways." << endl;

  // Add selected samples to the output DB
//   for( map_iter = sampled_ids.begin(); map_iter != sampled_ids.end(); ++map_iter ) {
//     DTIPathway* add_path = _db->getPathway( map_iter->first );
//     float log_score = map_iter->second;
//     addPathwayToOutDB(add_path, dbOut, log_score);
//   }

  // Write the database to a file
  writeOutputDB(dbOut);
}


/***********************************************************************
 *  Method: Score::saveAllSubPathways
 *  Params: 
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
Score::saveAllSubPathways()
{
  bool bWriteDBFirstTime = true;
  // Initialize the input database
  initInputDB(_inPathsFilename);
  
  // Get output database to store pathways for writing
  //DTIPathwayDatabase* dbOut = getSubPathwaysOutputDB();
  DTIPathwayDatabase* dbOut = getOutputDB();
  DTIPathway* pathway = getPathway();
  int pp=0;
  while( pathway!=NULL) {
    bool bInBounds = DTIStats::computeAllPathPointsInBounds(pathway, _tData->_pdfVol);
    if (!bInBounds) {
      cerr << "Throwing out " << pp << " pathway because it is outside image boundary." << endl;
    } else {
      // Look for subpathways in each pathway of the input DB
      findAndAddAllNonOutSubPathwaysToOutDB(pathway, pp, dbOut);    
      //findAndAddAllSubPathwaysToOutDB(pathway, pp, dbOut);    
      //findAndAddLongestSubPathwaysToOutDB(pathway, pp, dbOut);    
      // Default to write the output DB everytime we read a new input DB
      if( _curPathStackIter >= _db->getNumFibers() ){
	if(_bVerbose)  
	  cerr << "Examined " << pp+1 << " pathways, writing " << dbOut->getNumFibers() << " pathways to: " << _outPathsFilename << "." << endl;
	// We have reached the end of a pathway increment lets store the
	// current pathways
	writeOutputDB(dbOut,bWriteDBFirstTime);
	bWriteDBFirstTime=false;
	// Clean-up the new sub pathway memory
	dbOut->removeAllPathways();
	delete dbOut;
	//dbOut = getSubPathwaysOutputDB();
	dbOut = getOutputDB();
      }
    }
    pp++;
    pathway = getPathway();
  }

  // Write the database to a file  
  if(dbOut->getNumFibers()>0) {
    writeOutputDB(dbOut);
    // Clean-up the new sub pathway memory
    dbOut->removeAllPathways();
    delete dbOut;
    if(_db!=NULL)
      _db->removeAllPathways();
  }
}


/***********************************************************************
 *  Method: Score::getOutputDB
 *  Params: 
 * Returns: DTIPathwayDatabase *
 * Effects: 
 ***********************************************************************/
DTIPathwayDatabase *
Score::getOutputDB()
{
  // Create new database with unique pathways and store weight
  DTIPathwayDatabase *dbOut = new DTIPathwayDatabase();
  double mmScale[3];
  _db->getVoxelSize(mmScale);
  dbOut->setVoxelSize(mmScale);
  unsigned int dim[3];
  _db->getSceneDimension(dim);
  dbOut->setSceneDimension(dim);

  DTIPathwayAlgorithmHeader *simulatedHeader = new DTIPathwayAlgorithmHeader;
  strcpy (simulatedHeader->_algorithm_name, "ConTrack");
  dbOut->addAlgorithmHeader(simulatedHeader);
  // Lets store in ACPC space
  double mx[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
  dbOut->setTransformMatrix (mx);
      
  // Setup statistics headers
  dbOut->clearStatisticHeaders();
  DTIPathwayStatisticHeader *header = new DTIPathwayStatisticHeader;
  header->_is_computed_per_point = true;
  header->_is_luminance_encoding = true;
  header->_is_viewable_stat = true;
  strcpy (header->_aggregate_name, "Length");
  strcpy (header->_local_name, "Local Length");
  header->_unique_id = 0;       
  dbOut->addStatisticHeader(header);
      
  header = new DTIPathwayStatisticHeader;
  header->_is_computed_per_point = false;
  header->_is_luminance_encoding = true;
  header->_is_viewable_stat = true;
  strcpy (header->_aggregate_name, "Score");
  strcpy (header->_local_name, "Local Score");
  header->_unique_id = 1;       
  dbOut->addStatisticHeader(header);

  header = new DTIPathwayStatisticHeader;
  header->_is_computed_per_point = false;
  header->_is_luminance_encoding = true;
  header->_is_viewable_stat = true;
  strcpy (header->_aggregate_name, "Avg. Score");
  strcpy (header->_local_name, "NA");
  header->_unique_id = 2;
  dbOut->addStatisticHeader(header);

  header = new DTIPathwayStatisticHeader;
  header->_is_computed_per_point = true;
  header->_is_luminance_encoding = true;
  header->_is_viewable_stat = true;
  strcpy (header->_aggregate_name, "Linearity");
  strcpy (header->_local_name, "Local Linearity");
  header->_unique_id = 3;       
  dbOut->addStatisticHeader(header);

//   header = new DTIPathwayStatisticHeader;
//   header->_is_computed_per_point = true;
//   header->_is_luminance_encoding = true;
//   header->_is_viewable_stat = true;
//   strcpy (header->_aggregate_name, "Planarity");
//   strcpy (header->_local_name, "Local Planarity");
//   header->_unique_id = 2;       
//   dbOut->addStatisticHeader(header);

  return dbOut;
}

/***********************************************************************
 *  Method: Score::getSubPathwaysOutputDB
 *  Params: 
 * Returns: DTIPathwayDatabase *
 * Effects: 
 ***********************************************************************/
DTIPathwayDatabase *
Score::getSubPathwaysOutputDB()
{
  // Create new database with unique pathways and store weight
  DTIPathwayDatabase *dbOut = new DTIPathwayDatabase();
  double mmScale[3];
  _db->getVoxelSize(mmScale);
  dbOut->setVoxelSize(mmScale);
  unsigned int dim[3];
  _db->getSceneDimension(dim);
  dbOut->setSceneDimension(dim);

  DTIPathwayAlgorithmHeader *simulatedHeader = new DTIPathwayAlgorithmHeader;
  strcpy (simulatedHeader->_algorithm_name, "ConTrack");
  dbOut->addAlgorithmHeader(simulatedHeader);
  // Lets store in ACPC space
  double mx[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
  dbOut->setTransformMatrix (mx);

  // Add statistic for pathID to each pathway
  DTIPathwayStatisticHeader* header = new DTIPathwayStatisticHeader;
  header->_is_computed_per_point = false;
  header->_is_luminance_encoding = true;
  header->_is_viewable_stat = true;
  strcpy (header->_aggregate_name, "PathID");
  strcpy (header->_local_name, "NA");
  header->_unique_id = 0;       
  dbOut->addStatisticHeader(header);

  // Add statistic for begID to each pathway
  header = new DTIPathwayStatisticHeader;
  header->_is_computed_per_point = false;
  header->_is_luminance_encoding = true;
  header->_is_viewable_stat = true;
  strcpy (header->_aggregate_name, "EndID1");
  strcpy (header->_local_name, "NA");
  header->_unique_id = 1;       
  dbOut->addStatisticHeader(header);

  // Add statistic for begID to each pathway
  header = new DTIPathwayStatisticHeader;
  header->_is_computed_per_point = false;
  header->_is_luminance_encoding = true;
  header->_is_viewable_stat = true;
  strcpy (header->_aggregate_name, "EndID2");
  strcpy (header->_local_name, "NA");
  header->_unique_id = 2;       
  dbOut->addStatisticHeader(header);

// Add statistic for begID to each pathway
  header = new DTIPathwayStatisticHeader;
  header->_is_computed_per_point = false;
  header->_is_luminance_encoding = true;
  header->_is_viewable_stat = true;
  strcpy (header->_aggregate_name, "Score");
  strcpy (header->_local_name, "NA");
  header->_unique_id = 3;       
  dbOut->addStatisticHeader(header);

  /*
  // XXX backup this function and make a new one with minimal info 
// Get output database to store pathways for writing
  DTIPathwayDatabase* dbOut = getOutputDB();

  // Add statistic for pathID to each pathway
  DTIPathwayStatisticHeader* header = new DTIPathwayStatisticHeader;
  header->_is_computed_per_point = false;
  header->_is_luminance_encoding = true;
  header->_is_viewable_stat = true;
  strcpy (header->_aggregate_name, "PathID");
  strcpy (header->_local_name, "NA");
  header->_unique_id = 3;       
  dbOut->addStatisticHeader(header);
  */

  return dbOut;
}


/***********************************************************************
 *  Method: Score::writeOutputDB
 *  Params: DTIPathwayDatabase *dbOut
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
Score::writeOutputDB(DTIPathwayDatabase *dbOut, bool bOverwrite)
{
  if(_tData==NULL) {
    cerr << "Unable to load pathways before tractography data." << endl;
    return;
  }
  double *vmx = _tData->_pdfVol->getTransformMatrix();
  double ACPC[3] = {-vmx[3],-vmx[7],-vmx[11]};
  double vox_size[3];
  _tData->_pdfVol->getVoxelSize(vox_size[0], vox_size[1], vox_size[2]);
  dbOut->setVoxelSize(vox_size);
  // Write the database to a file
  //cerr << "Writing out database with " << dbOut->getNumFibers() << " pathways." << endl;
  string strFilename = _outPathsFilename;
  DTIPathwayIO::saveDatabaseAny(dbOut, bOverwrite, strFilename, ACPC);
}


/***********************************************************************
 *  Method: Score::addPathwayToOutDB
 *  Params: DTIPathway *pathway, DTIPathwayDatabase *dbOut, float score
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
Score::addPathwayToOutDB(DTIPathway *pathway, DTIPathwayDatabase *dbOut, float score)
{
  pathway->initializePathStatistics(4, dbOut->getPathStatisticHeaders(), true);
  DTIStats::computeLength( pathway, 0, pathway->getStepSize() );
  //DTIStats::computeFA( pathway, 1, (DTIScalarVolume *) _tData->_wmVol );
  if(_bAggScore) {
    pathway->setPathStatistic ( 1, score );
    pathway->setPathStatistic( 2, score / pathway->getNumPoints() );
  } else {
    pathway->setPathStatistic( 1, score * pathway->getNumPoints() );
    pathway->setPathStatistic ( 2, score );
  }
  // Let's use as little of parameters file as possible if we are doing consecutive loading
  if (_bSavingConsecutive)
    pathway->setPathStatistic ( 3, 0);
  else
    DTIStats::computeLinearity( pathway, 3, _tData->_pdfVol );


  dbOut->addPathway ( pathway  );
}

/***********************************************************************
 *  Method: Score::scoreAllPathways
 *  Params: float lengthParam, float smoothParam, float tensorShapeParam
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
Score::scoreAllPathways(float lengthParam, float smoothParam, float tensorShapeParam, bool bAllPairs)
{
  if(_bVerbose) {
    cerr << "Scoring database of " << _db->getNumFibers() << " pathways..." << endl;
    cerr << "Length param: "<< lengthParam << " Smooth param:  "<< smoothParam << " Tensor Shape param:  " << tensorShapeParam << endl;
  }

  // Prepare auxillary data structures for resampling
  if(bAllPairs) {
    _auxPairSamples.reserve(_db->getNumFibers());  
    _auxPairSamples.erase( _auxPairSamples.begin(), _auxPairSamples.end() );
  } else {
    _auxSamples.reserve(_db->getNumFibers());  
    _auxSamples.erase( _auxSamples.begin(), _auxSamples.end() );
  }

  for ( int pp = 0; pp < _db->getNumFibers(); pp++) {

    // Update progress
    if(_bVerbose)  {
    if ( (_db->getNumFibers() >= 100) && (pp % (_db->getNumFibers()/10) == 0) )
      cerr << (double) pp*100 / _db->getNumFibers() << "% done"<< endl;
    }

    // Get pathway and score it	
    DTIPathway *pathway = _db->getPathway(pp);
    bool bInBounds = DTIStats::computeAllPathPointsInBounds(pathway, _tData->_pdfVol);
    if (!bInBounds) {
      cerr << "Throwing out " << pp << " pathway because it is outside image boundary." << endl;
    } else {
      bool bReportInf = true;
      float log_score=0;
      if(bAllPairs)
	log_score = DTIStats::scorePathway( pathway, lengthParam, smoothParam, tensorShapeParam, _tData, true, bReportInf);    
      else
	log_score = DTIStats::scorePathway( pathway, lengthParam, smoothParam, tensorShapeParam, _tData, _bFreeEnds, bReportInf);    
      
      // Store score and index
      float min_acceptal_log_prob = DTIMath::float_neg_inf();
      if( log_score > min_acceptal_log_prob ) {
	if(!_bAggScore) log_score /= pathway->getNumPoints();
	PathSample newsample = {log_score, pp};
	if(bAllPairs) {
	  double voxSize[3];
	  _tData->_pdfVol->getVoxelSize(voxSize[0], voxSize[1], voxSize[2]);
	  // Get indices of endpoints and order them by which index is smaller
	  DTIVector pS(3);
	  pathway->getPoint (0,pS);
	  for(int pp=0; pp<3; pp++) pS[pp]/=voxSize[pp];
	  DTIVector pE(3);
	  pathway->getPoint (pathway->getNumPoints()-1,pE);
	  for(int pp=0; pp<3; pp++) pE[pp]/=voxSize[pp];
	  unsigned int low;
	  unsigned int high;
	  _tData->_pdfVol->sub2index(pS[0], pS[1], pS[2], low);
	  _tData->_pdfVol->sub2index(pE[0], pE[1], pE[2], high);
	  if( low>high ) {
	    unsigned int temp = low;
	    low = high;
	    high=temp;
	  }
	  AllPairsSample apSample = {low, high, newsample};
	  _auxPairSamples.push_back(apSample);
	} else {
	  _auxSamples.push_back(newsample);
	}
      } else {
	cerr << "Throwing out " << pp << " pathway due to low score." << endl;
      }
    }
  }
}






/***********************************************************************
 *  Method: Score::addSubPathwayToOutDB
 *  Params: DTIPathway *pathway, int endID, DTIPathwayDatabase *dbOut, float score
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
Score::addSubPathwayToOutDB(int nPathID, int nBegID, int nEndID, float log_score, DTIPathwayDatabase *dbOut)
{
  DTIPathway* add_path = new DTIPathway(DTI_PATHWAY_ALGORITHM_ANY);
  add_path->initializePathStatistics(4, dbOut->getPathStatisticHeaders(), true);
  add_path->setPathStatistic ( 0, nPathID );
  add_path->setPathStatistic ( 1, nBegID );
  add_path->setPathStatistic ( 2, nEndID );
  add_path->setPathStatistic ( 3, log_score );
  dbOut->addPathway ( add_path  );

 //  // Check to see if this point is actually within an ROI vs. stopped because of GM
//   bool bWithinROI = true;
//   if( _tData->_voiMaskVol!=NULL && _tData->_pdfVol!=NULL ) {
//     double voxSize[3];
//     _tData->_voiMaskVol->getVoxelSize(voxSize[0], voxSize[1], voxSize[2]);
//     DTIVector p(3);
//     pathway->getPoint(endID, p); 
//     p[0] /= voxSize[0]; p[1] /= voxSize[1]; p[2] /= voxSize[2];
//     int voiValue = (int)_tData->_voiMaskVol->getScalar ((int) floor (p[0]+0.5),
// 							(int) floor (p[1]+0.5),
// 							(int) floor (p[2]+0.5));
//     if(voiValue==0)
//       bWithinROI=false;
//   }
//   if( bWithinROI ) {
//     // Add the endpoints of the sub-pathway to the output DB
//     DTIPathway* add_path = new DTIPathway(pathway->getPathwayAlgorithm());
//     add_path->append( pathway->getPointV(0) );
//     add_path->append( pathway->getPointV(endID) );
//     add_path->initializePathStatistics(4, dbOut->getPathStatisticHeaders(), true);
//     add_path->setPathStatistic ( 0, pathway->getStepSize()*endID );
//     DTIStats::computeFA( add_path, 1, (DTIScalarVolume *) _tData->_wmVol );
//     add_path->setPathStatistic ( 2, score );
//     dbOut->addPathway ( add_path  );
//   }

}



void
Score::findAndAddLongestSubPathwaysToOutDB(DTIPathway* pathway, int nPathID, DTIPathwayDatabase* dbOut)
{ 
  vector<double> vecLike;
  vector<double> vecPrior;
  vector<double> vecAbs;
  vector<int> vecPointID;
  float smoothParam = _tData->_params->stdSmoothness;
  float lengthParam = _tData->_params->absRateNormal;
  float tensorShapeParam = _tData->_params->shapeLinearityMidCl;
  double voxSize[3];
  _tData->_wmVol->getVoxelSize(voxSize[0], voxSize[1], voxSize[2]);

  // Find all posssible sub segments, i.e. GM points that have only GM and WM between them
  vector<int> vEndID;
  vector<int> vSegID;
  float pt[3];
  int nSeg=0;
  for(int ii=0; ii<pathway->getNumPoints(); ii++) {
    // Go through all pathway points marking possible segment endpoints
    pathway->getPoint(ii,pt);
    for(int pp=0; pp<3; pp++) pt[pp] = pt[pp]/voxSize[pp];
    double val = _tData->_wmVol->getScalar(pt[0],pt[1],pt[2]);
    short roiVal = _tData->_voiMaskVol->getScalar(pt[0],pt[1],pt[2]);
    if (fabs(val) <= _tData->_params->wmThresh) {
      // We have exited WM increase the seg count if needed
      if(!vEndID.empty()) {
	if(vEndID.size()==1 || 
	   (nSeg==0 && vEndID[vEndID.size()-1]-vEndID[0] <= _tData->_params->nMinChainLength-1) ||
	   (nSeg==0 && vEndID[vEndID.size()-1]-vEndID[vSegID.size()-1]-1 <= _tData->_params->nMinChainLength-1) ||
	   (nSeg!=0 && vEndID.size()-2==vSegID[vSegID.size()-1]) ) {
	  // Remove one endpoint segments
	  vEndID.pop_back();
	} else if(nSeg==0 || vEndID.size()-1!=vSegID[vSegID.size()-1]) {
	  // Point to the last endpoint in a segment
	  vSegID.push_back(vEndID.size()-1);
	  nSeg++;
	}
      }
    } else if (roiVal > 0){
      // This is GM voxel
      vEndID.push_back(ii);
    }
  }
  if(!vEndID.empty()) {
    // Score pathway, also finding possible endpoints
    DTIStats::scorePathway( pathway, 
			    lengthParam, 
			    smoothParam, 
			    tensorShapeParam, 
			    vecLike, 
			    vecPrior, 
			    vecAbs, 
			    _tData, 
			    _bFreeEnds);    

    // Find largest segment to add
    int nBegID = vEndID[0];
    int nEndID = vEndID[vEndID.size()-1];
    double log_score = 0;
    DTIPathway* add_path = new DTIPathway(DTI_PATHWAY_ALGORITHM_ANY);
    if(nEndID-nBegID+1 >= _tData->_params->nMinChainLength) {
      for(int ss=nBegID; ss<=nEndID; ss++) {
	if( log_score > DTIMath::float_neg_inf() ) {
	  log_score+=vecLike[ss];
	  log_score+=vecPrior[ss];
	}
	// add the point to the path
	add_path->append( pathway->getPointV(ss) );
      }
      if(!_bAggScore) log_score /= pathway->getNumPoints();
      addPathwayToOutDB(add_path, dbOut, log_score);
      //addSubPathwayToOutDB(nPathID, nBegID, nEndID, log_score, dbOut);
    }
  }
}

/***********************************************************************
 *  Method: Score::addSubPathwayToOutDB
 *  Params: DTIPathway *pathway, DTIPathwayDatabase *dbOut
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
Score::findAndAddAllSubPathwaysToOutDB(DTIPathway* pathway, int nPathID, DTIPathwayDatabase* dbOut)
{ 
  vector<double> vecLike;
  vector<double> vecPrior;
  vector<double> vecAbs;
  vector<int> vecPointID;
  float smoothParam = _tData->_params->stdSmoothness;
  float lengthParam = _tData->_params->absRateNormal;
  float tensorShapeParam = _tData->_params->shapeLinearityMidCl;
  double voxSize[3];
  _tData->_wmVol->getVoxelSize(voxSize[0], voxSize[1], voxSize[2]);

  // Find all posssible sub segments, i.e. GM points that have only GM and WM between them
  float pt[3];
  vector<short> vType(pathway->getNumPoints(),0); //0: out, 1: WM, 2: GM
  for(int ii=0; ii<pathway->getNumPoints(); ii++) {
    // Go through all pathway points marking possible segment endpoints
    pathway->getPoint(ii,pt);
    for(int pp=0; pp<3; pp++) pt[pp] = pt[pp]/voxSize[pp];
    double val = _tData->_wmVol->getScalar(pt[0],pt[1],pt[2]);
    short roiVal = _tData->_voiMaskVol->getScalar(pt[0],pt[1],pt[2]);
    bool bW = !(fabs(val) <= _tData->_params->wmThresh);
    bool bG = roiVal > 0;
    //bool bO = !(bW || bG);
    if(bW) vType[ii]++;
    if(bG) vType[ii]++;
  }
  vector<int> vStartID;
  vector<int> vEndID;
  for(int ii=0; ii<vType.size(); ii++) {
    short nPrevT=2;
    short nNextT=2;
    short nType = vType[ii];
    if(ii!=0) nPrevT=vType[ii-1];
    if(ii!=vType.size()-1) nNextT=vType[ii+1];
    if(nType==2) { 
      //GM, only set as an endpoint if we have a neighboring WM
      if(nPrevT==1) vEndID.push_back(ii);
      if(nNextT==1) vStartID.push_back(ii);
      }
    if(nType==0) {
      // Outside tissue, remove any unmatched ending points to assure segments will not traverse the outside voxels
      if(vEndID.size()<vStartID.size())
	vStartID.pop_back();
      else if(vStartID.size()<vEndID.size())
	vEndID.pop_back();
      assert(vEndID.size()==vStartID.size());
    }
  }
  // Remove any trailing endpoints
  if(vEndID.size()<vStartID.size())
    vStartID.pop_back();
  else if(vStartID.size()<vEndID.size())
    vEndID.pop_back();
  assert(vEndID.size()==vStartID.size());

  if(!vEndID.empty()) {
    // Score pathway, also finding possible endpoints
    DTIStats::scorePathway( pathway, 
			    lengthParam, 
			    smoothParam, 
			    tensorShapeParam, 
			    vecLike, 
			    vecPrior, 
			    vecAbs, 
			    _tData, 
			    _bFreeEnds);    
    
    // For each possible endpoint pair within a segment create a new path sample with a new
    // pathID Possible endpoints can only occur after the minimum
    // pathway distance has been reached.
    for(int ss=0; ss<vEndID.size(); ss++) {
      // Find largest segment to add
      int nStartID = vStartID[ss];
      int nEndID = vEndID[ss];
      double log_score = 0;
      DTIPathway* add_path = new DTIPathway(DTI_PATHWAY_ALGORITHM_ANY);
      if(nEndID-nStartID+1 >= _tData->_params->nMinChainLength) {
	for(int pp=nStartID; pp<=nEndID; pp++) {
	  if( log_score > DTIMath::float_neg_inf() ) {
	    log_score+=vecLike[pp];
	    log_score+=vecPrior[pp];
	  }
	  // add the point to the path
	  add_path->append( pathway->getPointV(pp) );
	}
	if(!_bAggScore) log_score /= pathway->getNumPoints();
	addPathwayToOutDB(add_path, dbOut, log_score);
      }
    }  
  }
}

/***********************************************************************
 *  Method: Score::addSubPathwayToOutDB
 *  Params: DTIPathway *pathway, DTIPathwayDatabase *dbOut
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
Score::findAndAddAllNonOutSubPathwaysToOutDB(DTIPathway* pathway, int nPathID, DTIPathwayDatabase* dbOut)
{ 
  vector<double> vecLike;
  vector<double> vecPrior;
  vector<double> vecAbs;
  vector<int> vecPointID;
  float smoothParam = _tData->_params->stdSmoothness;
  float lengthParam = _tData->_params->absRateNormal;
  float tensorShapeParam = _tData->_params->shapeLinearityMidCl;
  double voxSize[3];
  _tData->_wmVol->getVoxelSize(voxSize[0], voxSize[1], voxSize[2]);

  // Find all posssible sub segments, i.e. GM points that have only GM and WM between them
  float pt[3];
  vector<short> vType(pathway->getNumPoints(),0); //0: out, 1: WM, 2: GM
  for(int ii=0; ii<pathway->getNumPoints(); ii++) {
    // Go through all pathway points marking possible segment endpoints
    pathway->getPoint(ii,pt);
    for(int pp=0; pp<3; pp++) pt[pp] = pt[pp]/voxSize[pp];
    double val = _tData->_wmVol->getScalar(pt[0],pt[1],pt[2]);
    short roiVal = _tData->_voiMaskVol->getScalar(pt[0],pt[1],pt[2]);
    bool bW = !(fabs(val) <= _tData->_params->wmThresh);
    bool bG = roiVal > 0;
    //bool bO = !(bW || bG);
    if(bW) vType[ii]++;
    if(bG) vType[ii]++;
  }
  vector<int> vStartID;
  vector<int> vEndID;
  short nSeg=0;
  for(int ii=0; ii<vType.size(); ii++) {
    short nType = vType[ii];
    if(nType==2) { 
      // Do we already have a start for this segment?
      if( vStartID.size()==nSeg )
	vStartID.push_back(ii);
      // Do we already have a possible end node?
      else if( vEndID.size()==nSeg )
	vEndID.push_back(ii); 
      // Update the possible end node
      else
	vEndID[nSeg] = ii;
    }
    if(nType==0) {
      // Outside tissue, stop the previous segment and remove any dangling starting points
      if(vEndID.size()<vStartID.size())
	vStartID.pop_back(); // Remove dangling starting point
      else if(vStartID.size()>nSeg)
	nSeg++; // We got a starting point and an ending point so lets accept a segment
      assert(vEndID.size()==vStartID.size());
      assert(nSeg==vStartID.size());
    }
  }
  // Remove any trailing endpoints
  if(vEndID.size()<vStartID.size())
    vStartID.pop_back();
  assert(vEndID.size()==vStartID.size());

  if(!vEndID.empty()) {
    // Score pathway, also finding possible endpoints
    DTIStats::scorePathway( pathway, 
			    lengthParam, 
			    smoothParam, 
			    tensorShapeParam, 
			    vecLike, 
			    vecPrior, 
			    vecAbs, 
			    _tData, 
			    _bFreeEnds);    
    
    // For each possible endpoint pair within a segment create a new path sample with a new
    // pathID Possible endpoints can only occur after the minimum
    // pathway distance has been reached.
    for(int ss=0; ss<vEndID.size(); ss++) {
      // Find largest segment to add
      int nStartID = vStartID[ss];
      int nEndID = vEndID[ss];
      double log_score = 0;
      DTIPathway* add_path = new DTIPathway(DTI_PATHWAY_ALGORITHM_ANY);
      if(nEndID-nStartID+1 >= _tData->_params->nMinChainLength) {
	for(int pp=nStartID; pp<=nEndID; pp++) {
	  if( log_score > DTIMath::float_neg_inf() ) {
	    log_score+=vecLike[pp];
	    log_score+=vecPrior[pp];
	  }
	  // add the point to the path
	  add_path->append( pathway->getPointV(pp) );
	}
	if(!_bAggScore) log_score /= pathway->getNumPoints();
	addPathwayToOutDB(add_path, dbOut, log_score);
      }
    }  
  }
}

