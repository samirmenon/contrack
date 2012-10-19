//
//  ConTrack Scoring
//

#ifdef USE_MPI
#include <mpi.h>		// MPI header file
#endif
#include <vector>
#include "score.h"
#include <sstream>
#include "tclap/CmdLine.h"      // command line parsing
#include <cmath>                // round 
#include "typedefs.h"
#include "DTIMath.h"
#include "trueUtils.h"

using namespace TCLAP;
using namespace std;

void runAlg( Score& score_alg, string& infoFilename, string& outFilename, 
	     int pdbInInc, int extProc, int extProcID, 
	     string& strPathFilename, vector<string>& pathFiles,
	     bool bFindEnds, float fThreshold, float fMaxScore,
	     bool bUseMaxScore, bool bAggScore, bool bSeqSample, bool bSortSample, bool bAllPairs, bool bCaminoNoStats, bool bVerbose) {

  if(bVerbose) 
    cerr << "Loading pathway file: " << strPathFilename << endl;
  score_alg.setInputFilename(strPathFilename.c_str());
  // Is input original camino or new format
  score_alg.setInputBFloatType(bCaminoNoStats);
  if(bVerbose)
    cerr << "Writing to pathway file: " << outFilename << endl;
  score_alg.setOutputFilename(outFilename.c_str());

  score_alg.setUseAggScore(bAggScore);

  // Resampling of pathways
  if( bSeqSample )  {
    // Just save first pathways
    score_alg.saveConsecutivePathways( (int)round(fThreshold) );
  }
  else if( bAllPairs ) {
    if( bSortSample ) {
    // Load all of the pathways up front because we need to save top scoring
    score_alg.loadAllPathways(pathFiles);
    // Save top scoring pathways
    score_alg.saveAllPairsTopScoringPathways( (int)round(fThreshold) );
    } else {
      cerr << "Must use --sort switch to threshold with --all_pairs." << endl;
    }
  }
  else if( bSortSample ) {
    // Load all of the pathways up front because we need to save top scoring
    score_alg.loadAllPathways(pathFiles);
    // Save top scoring pathways
    score_alg.saveTopScoringPathways( (int)round(fThreshold) );
  }
  else if( bFindEnds ) {
    // Look at each point along pathway as a possible ending for a new pathway from these
    score_alg.saveAllSubPathways( );
  }
  else {
    // Load all of the pathways up front because we need to save top scoring
    score_alg.loadAllPathways(pathFiles);
    // Save all pathways above some scoring threshold
    score_alg.saveThresholdScorePathways( fThreshold, bUseMaxScore, fMaxScore );
  }
}

// struct achar {
// public:
//   int i1;
//   int i2;
//   int i3;
//   char a[255];
//   char b[255];
//   int i4;
// };
// #include "DTIPathwayStatisticHeader.h"

int main(int argc, char *argv[]) {

//   achar s;
//   DTIPathwayStatisticHeader h;
//   cout << "size: " << sizeof(h) << endl;
//   cout << "size: " << sizeof(s) << endl;
//   cout << " o1: " << offsetof(achar,i1) << endl;
//   cout << " o2: " << offsetof(achar,i2) << endl;
//   cout << " o3: " << offsetof(achar,i3) << endl;
//   cout << " o4: " << offsetof(achar,a) << endl;
//   cout << " o5: " << offsetof(achar,b) << endl;
//   cout << " o6: " << offsetof(achar,i4) << endl;
//   return 0;

  int myid=0;
  int nprocs=1;
#ifdef USE_MPI
  // initialize for MPI (should come before any other calls to
  //     MPI routines)
  if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
    cerr << "MPI initialization error\n"; exit(EXIT_FAILURE);
  } 
  // get number of processes
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  // get this process's number (ranges from 0 to nprocs - 1)
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  // has everyone initialized
  MPI_Barrier(MPI_COMM_WORLD);
#endif
  
  // Variables to set with command line
  string infoFilename;
  string outFilename;
  int pdbInInc;
  int extProc = 1;
  int extProcID = 0;
  vector<string> pathFiles;
  bool bFindEnds;
  float fThreshold;
  float fMaxScore;
  bool bUseMaxScore=false;
  bool bAggScore=false;
  bool bSeqSample;
  bool bSortSample;
  bool bFreeEnds;
  bool bAllPairs;
  bool bCaminoNoStats;

  // Command line parsing
  try {
    // Define the command line object.
    CmdLine cmd("ConTrack's scoring and resampling algorithm.\n Note that you must set a value for --thresh option and that value defaults to selecting all pathways in the database above the log score of that value.  Use --seq and --sort to modify what this value means.  You can also select --log_score_max to set a value for maximum log score.", ' ', "0.1");
    // PDB filenames as input to the scoring
    UnlabeledMultiArg<string> pdbInputArg("pdbInput", "Input pathway database file names.", true, "string",cmd);
    // Pathway input increment
    ValueArg<int> incArg("","inInc","Number of pathways to hold in memory at a time, default=1000.(EXPERIMENTAL--DON'T USE!!)",false,1000,"int",cmd);
    // External processors to score
    ValueArg<int> extProcArg("","extProc","Number of external processors that are going to score files, default=1.(EXPERIMENTAL--DON'T USE!!)",false,1,"int",cmd);
    // ID of this processor
    ValueArg<int> extIDArg("","extProcID","ID of external processor if using multiple processors, but not with MPI, default=0.(EXPERIMENTAL--DON'T USE!!)",false,0,"int",cmd);
    // Maximum log score of pathways to retain
    ValueArg<string> strMaxScoreArg("","log_score_max","Maximum score of pathways to retain. Only works with default thresh ofminimum log score, i.e. can't use --sort or --seq flags.",false,"none","string",cmd);
    // Find variable endpoints switch
    SwitchArg endsSwitch("","find_ends","For each input pathway, find all points along the ROI that intersect any ROI.  Take the longest sub pathway that goes between any ROIs (even the same ROI!)",false);
    cmd.add( endsSwitch );
    // Switch to ignore endpoints during scoring
    SwitchArg feSwitch("","free_ends","Ignores whether endpoints intersect VOIs.", false);
    cmd.add( feSwitch );
    // Switch to raw (non averaged) scoring
    SwitchArg AggScoreSwitch("","agg_score","The thresholding defaults to use the mean of the scores per point, however, this switch allows one to use the aggregate of scores per point.  Both are saved in the file no matter what threshold you use.",false);
    cmd.add( AggScoreSwitch );
    // Sequential sampling switch
    SwitchArg seqSwitch("","seq","Makes threshold the number of pathways to take sequentially from the start of the database.", false);
    cmd.add( seqSwitch );
    // Sorted sampling switch
    SwitchArg sortSwitch("","sort","Makes threshold the number of pathways to take from the highest scoring pathways of the database.", false);
    cmd.add( sortSwitch );
    // All pairs sampling switch
    SwitchArg allpairsSwitch("","all_pairs","Threshold the number of pathways for all pairs.", false);
    cmd.add( allpairsSwitch );
    // Switch for processing original Camino files or new format with stats (default is off, meaning new format)
    SwitchArg bfloat_no_statsSwitch("","bfloat_no_stats","Use this switch for the original camino format.", false);
    cmd.add( bfloat_no_statsSwitch );
    // Threshold value
    ValueArg<float> fThresholdArg("","thresh","Threshold value.  Can be log score minimum (default) or number of pathways. See --sort and --seq.",true,-1,"float",cmd);
    // Pathways file argument
    ValueArg<string> pdbArg("p","pdb","Pathway database output file, e.g. paths.Bfloat.",true,"","string",cmd);
    // Info file argument
    ValueArg<string> infoArg("i","info","Information file, e.g. met_params.txt.",true,"","string",cmd);

    // Parse the args.
    cmd.parse( argc, argv );
    
    // Get the value parsed by each arg. 
    infoFilename = infoArg.getValue();
    outFilename = pdbArg.getValue();
    pdbInInc = incArg.getValue();
    extProc = extProcArg.getValue();
    extProcID = extIDArg.getValue();
    pathFiles = pdbInputArg.getValue();
    bFindEnds = endsSwitch.getValue();
    bFreeEnds = feSwitch.getValue();
    fThreshold = fThresholdArg.getValue();
    bAggScore = AggScoreSwitch.getValue(); 
    bSeqSample = seqSwitch.getValue(); 
    bSortSample = sortSwitch.getValue(); 
    bAllPairs = allpairsSwitch.getValue();
    bCaminoNoStats = bfloat_no_statsSwitch.getValue();
    string temp = strMaxScoreArg.getValue();
    if (temp.compare("none")!=0) {
      bUseMaxScore = true;
      fMaxScore = atof(temp.c_str());
    }
    
  }catch (ArgException &e) { // catch command line exceptions
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl; 
#ifdef USE_MPI    
    MPI_Finalize ();
#endif
    exit (0);
  }
  
  // Initialize RNG
  uint nSeed = (uint)time((time_t *)NULL);
  DTIMath::initRNG(myid,nSeed);
  //DTIMath::initRNG(0,777);
  if(myid==0) {
    cerr << "Seed(s): ";
    for(int pp=0; pp<nprocs-1; pp++)
      cerr << nSeed+pp << ",";
    cerr << nSeed+nprocs-1 << endl;
  }

  // Non-command line arguments to be set  
  bool bVerbose = false;
  if(nprocs<2 || myid==0)
    bVerbose = true;
  if( extProc > 1 )
    myid = extProcID;
  
  string strPathFilename = pathFiles[0];

  // Setup the scoring algorithm
  Score score_alg;
  score_alg.setupFromOptionsFile (infoFilename.c_str(), "", "", bVerbose);  
  score_alg.setInputInc(pdbInInc);
  score_alg.setFreeEnds(bFreeEnds);

    if( nprocs > 1 || extProc > 1 ) {  
      // Each process will write to a separate file
      // Do the input filename
      strPathFilename = pathFiles[0];
      addProcToFilename(strPathFilename, myid);

      // Can't do multiple input files with parallel processing mode right now
      pathFiles.clear();
      pathFiles.push_back(strPathFilename);

      // Do the output filename
      addProcToFilename(outFilename, myid);

    }  

    // Setup running time check
    float diffT;
#ifdef USE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
    double startT, endT;
    startT = MPI_Wtime ();
#else
    time_t startPP,endPP;
    time (&startPP);
#endif
    
    runAlg(score_alg, infoFilename, outFilename, pdbInInc, extProc, extProcID, 
	   strPathFilename, pathFiles, bFindEnds, fThreshold, fMaxScore,
	   bUseMaxScore, bAggScore, bSeqSample, bSortSample, bAllPairs, bCaminoNoStats, bVerbose);
    
    // Check running time of computation
#ifdef USE_MPI
    endT = MPI_Wtime ();
    diffT =  endT - startT;
#else
    time (&endPP);
    diffT = difftime(endPP,startPP);
#endif
    if(bVerbose)
      cerr << "contrack_score run-time: " << diffT << "s" << endl << flush;

  
    //Clean up  
#ifdef USE_MPI
  // Clean up for MPI 
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize ();
#endif

  return 0;
}

// AnyOption *parseCommandLine(int Disp, char *argv[])
// {
//   // 1. CREATE AN OBJECT
//   AnyOption *opt = new AnyOption();

//   // 2. SET PREFERENCES 
//   //opt->setVerbose(); // print warnings about unknown options

//   // 3. SET THE USAGE/HELP 
//   opt->addUsage( "" );
//   opt->addUsage( "Usage: " );
//   opt->addUsage( "" );
//   opt->addUsage( " -i track_params.txt     Input parameters file (req.)" );
//   opt->addUsage( " -o pathsScored.pdb	   Pathway database scored (req.)" );
//   opt->addUsage( " One of the following sampling methods is required ..." );
//   opt->addUsage( "    --log_score_min #      Only keep pathways above this log score" );
//   opt->addUsage( "    --log_score_max #      Only keep pathways below this log score (only in combo with --log_score_min)" );
//   opt->addUsage( "    --num_score #          Only keep specified number of top scoring" );
//   opt->addUsage( "    --num  #               Keep first number of pathways, ignoring score" );
//   opt->addUsage( "    --find_ends            Each point of pathway is considered for an end of new pathways" );
//   //opt->addUsage( " --kSmoothRange 0.0	   Range of smoothness parameter search" );
//   //opt->addUsage( " --kSmoothStep  0.0	   Step size of smoothness parameter search" );
//   //opt->addUsage( " --kLengthRange 0.0	   Range of length parameter search" );
//   //opt->addUsage( " --kLengthStep  0.0	   Step size of length parameter search" );
//   //opt->addUsage( " --kDispRange   0.0	   Range of dispersion parameter search" );
//   //opt->addUsage( " --kDispStep    0.0	   Step size of dispersion parameter search" );
//   opt->addUsage( " --inInc        1000	   Number of pathways to hold in memory at a time (1000 def.)" );
//   opt->addUsage( " --extProc      1	   Number external processors that are going to score files" );
//   opt->addUsage( " --extProcID    0	   ID of this processor if using multiple processors not in MPI mode" );
//   opt->addUsage( " pathFile1 pathFile2 ...   Remaining arguments are pathway databases to load");
//   opt->addUsage( "" );

//   // 4. SET THE OPTION STRINGS/CHARACTERS

//   // by default all  options  will be checked on the command line
//   opt->setCommandOption( "info", 'i' );	// an option (takes an argument), supporting long and short form
//   opt->setCommandOption( "output", 'o' ); 
//   opt->setCommandOption( "log_score_min" );  
//   opt->setCommandOption( "log_score_max" );  
//   opt->setCommandOption( "num_score" );
//   opt->setCommandOption( "num" );
//   opt->setCommandFlag( "find_ends" ); // a flag (takes no argument), supporting long and short form 
//   opt->setCommandOption( "kSmoothRange" );
//   opt->setCommandOption( "kSmoothStep" );
//   opt->setCommandOption( "kLengthRange" );
//   opt->setCommandOption( "kLengthStep" );
//   opt->setCommandOption( "kDispRange" );
//   opt->setCommandOption( "kDispStep" );  
//   opt->setCommandOption( "inInc" );
//   opt->setCommandOption( "extProc" );
//   opt->setCommandOption( "extProcID" );
//   // 5. PROCESS THE COMMANDLINE
//   opt->processCommandArgs( Disp, argv );

//   bool bSamplingTypeSpecified = ( opt->getValue("log_score_min") || 
// 				  opt->getValue("num") || 
// 				  opt->getValue("num_score") ||
// 				  opt->getFlag("find_ends") );

//   // 6. ASSERT ALL REQUIRED OPTIONS ARE THERE
//   if( opt->getValue( 'i' ) == NULL || 
//       opt->getValue( 'o' ) == NULL ||
//       !bSamplingTypeSpecified ||
//       opt->getArgc()<=0 ) {
//     opt->printUsage();
//     delete opt;
//     return NULL;
//   }
//   return opt;
// }
