/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
    University. All rights reserved. **/

#ifndef SCORE_H
#define SCORE_H

#include <vector>
#include <fstream>

class DTIPathwayInterface; 
class DTIPathwayDatabase;
class DTIPathway;
class DTITrackingData;

using namespace std;

struct PathSample {
  float score;
  unsigned int index;
};
inline bool operator< (PathSample a, PathSample b)
{
  return a.score < b. score;
}

struct AllPairsSample {
  unsigned int index_low;
  unsigned int index_high;
  PathSample sample;
};
inline bool operator< (AllPairsSample a, AllPairsSample b)
{
  if( a.index_low == b.index_low )
    return a.index_high < b.index_high;
  else
    return a.index_low < b.index_low;
}
inline bool operator== (AllPairsSample a, AllPairsSample b)
{
  return a.index_low == b.index_low && a.index_high == b.index_high;
}
/* inline AllPairsSample& operator= (const AllPairsSample b) */
/* { */
/*   a.index_low = b.index_low; */
/*   a.index_high = b.index_high; */
/* } */

class Score {
public:
  void setupFromOptionsFile (const char* infoFilename, const char* outPathsFilename, const char* inPathwayFile, bool bVerbose=true);
  void scoreAllPathways(float lengthParam, float smoothParam, float tensorShapeParam, bool bAllPairs=false);
  void saveConsecutivePathways(int nSave);
  void saveTopScoringPathways(int nSave);
  void saveAllPairsTopScoringPathways(int nSave);
  void saveThresholdScorePathways(float tMinScore, bool bUseMaxScore, float tMaxScore);
  void saveAllSubPathways( );
  void loadAllPathways(std::vector<string> pathFiles);
  void cleanup();
  void setUseAggScore(bool b){_bAggScore=b;}
  void setFreeEnds(bool b){_bFreeEnds=b;}
  void setInputInc(int n){_pdbInInc = n;}
  void setInputFilename(const char* in){_inPathsFilename=in;}
  void setInputBFloatType(bool b){_bCaminoNoStatsInput=b;}
  void setOutputFilename(const char* out){_outPathsFilename=out;}
  bool isFilenamePDB(const char* filename);

private:
  // Functions
  DTIPathwayDatabase* getOutputDB();
  DTIPathwayDatabase* getSubPathwaysOutputDB();
  void addPathwayToOutDB(DTIPathway* pathway, DTIPathwayDatabase* dbOut, float score);
  void addSubPathwayToOutDB(int nPathID, int nBegID, int nEndID, float log_score, DTIPathwayDatabase *dbOut);
  void findAndAddAllSubPathwaysToOutDB(DTIPathway* pathway, int nPathID, DTIPathwayDatabase* dbOut);
  void findAndAddAllNonOutSubPathwaysToOutDB(DTIPathway* pathway, int nPathID, DTIPathwayDatabase* dbOut);
  void findAndAddLongestSubPathwaysToOutDB(DTIPathway* pathway, int nPathID, DTIPathwayDatabase* dbOut);
  void writeOutputDB(DTIPathwayDatabase* dbOut, bool bOverwrite=true);
  void initInputDB(const char* filename);
  DTIPathway* getPathway();

  // Members
  const char* _outPathsFilename;
  DTIPathwayDatabase* _db;
  DTITrackingData* _tData;
  bool _bVerbose;
  bool _bFreeEnds;
  bool _bAggScore;
  int _pdbInInc;
  int _curStreamPos;
  int _endStreamPos;
  int _curPathStackIter;
  const char* _inPathsFilename;
  bool _bCaminoNoStatsInput;
  bool _bSavingConsecutive;
  
  // Data structures for holding the per pathway values, these need to support random access and searches
  std::vector<PathSample> _auxSamples; 
  std::vector<AllPairsSample> _auxPairSamples; 
};

#endif

