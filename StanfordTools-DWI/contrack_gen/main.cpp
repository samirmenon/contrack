//
//  ConTrack
//

#ifdef USE_MPI
#include <mpi.h>		// MPI header file
#endif
#include <iostream>
#include <cstdlib>		// has exit(), etc.
#include <fstream>		// file I/O
#include <string>		// strings
#include <sstream>
#include "tracker.h"
#include <math.h>
#include "tclap/CmdLine.h"      // command line parsing
#include <unistd.h>             // sleep on linux
#include <ui_utils.h>

using namespace TCLAP;
using namespace std;

// testing
#include "typedefs.h"
#include <DTIMath.h>

int main(int argc, char *argv[]) {
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
  string pathsFilename;
  bool bDoAllVoxelsROI1;
  float max_time;
  bool bAxialPathways;
  uint nSeed = (uint)time((time_t *)NULL);

  // Command line parsing
  try {
    // Define the command line object.
    CmdLine cmd("ConTrack's pathway generation algorithm.", ' ', "0.1");
    // Seed
    // Info file argument
    ValueArg<uint> rseedArg("","seed","RNG Seed Number",false,nSeed,"uint");
    cmd.add( rseedArg );
    // Voxelwise generation
    SwitchArg voxelSwitch("v","vox","Voxelwise pathway generation.(EXPERIMENTAL--DON'T USE!!)", false);
    cmd.add( voxelSwitch );
    // Axial pathway generation only
    SwitchArg axialSwitch("A","Axial","Axial pathway generation.(EXPERIMENTAL--DON'T USE!!)", false);
    cmd.add( axialSwitch );
    // Generation time limit
    ValueArg<float> timeArg("t","time","Pathway generation time limit.(EXPERIMENTAL--DON'T USE!!)",false,-1,"float");
    cmd.add( timeArg );
    // Pathways file argument
    ValueArg<string> pdbArg("p","pdb","Pathway database output file, e.g. paths.Bfloat.",true,"","string");
    cmd.add( pdbArg );
    // Info file argument
    ValueArg<string> infoArg("i","info","Information file, e.g. met_params.txt.",true,"","string");
    cmd.add( infoArg );
    
    // Parse the args.
    cmd.parse( argc, argv );
    
    // Get the value parsed by each arg. 
    infoFilename = infoArg.getValue();
    pathsFilename = pdbArg.getValue();
    bDoAllVoxelsROI1 = voxelSwitch.getValue();
    bAxialPathways = axialSwitch.getValue();
    max_time = timeArg.getValue();
    nSeed = rseedArg.getValue();
    
  }catch (ArgException &e) { // catch command line exceptions
    cerr << "error: " << e.error() << " for arg " << e.argId() << endl; 
#ifdef USE_MPI    
    MPI_Finalize ();
#endif
    exit (0);
  }
  // Initialize RNG
  DTIMath::initRNG(myid,nSeed);
  //DTIMath::initRNG(0,777);
  if(myid==0) {
    cerr << "Seed(s): ";
    for(int pp=0; pp<nprocs-1; pp++)
      cerr << nSeed+pp << ",";
    cerr << nSeed+nprocs-1 << endl;
  }

  // Non-command line arguments to be set  
  bool bPDBFormat = false;
  size_t pos = pathsFilename.find_last_of(".");    // get position of extension
  string strExt = pathsFilename.substr(pos);                // get extension
  if (strExt.compare(".pdb")==0)
    bPDBFormat=true;

  bool bVerbose = myid==0;
  
  
  if( nprocs > 1 ) {  
    // Each process will write to a separate file
    // Find the .Bfloat in the pathsFilename
    ostringstream ost;
    string::size_type iext = pathsFilename.find_last_of(".");
    int ext_length = pathsFilename.size()-iext;
    string strExt(pathsFilename,iext,ext_length);
    // New filename ending
    //ost << "_" << myid << ".SBfloat";
    ost << "_" << myid << strExt;
    // Replace extension with new ending
    pathsFilename.replace(iext,ext_length,ost.str().c_str());
  }
  
  // Warn that we are about to append to an existing file
  ifstream input_test(pathsFilename.c_str());
  bool bAppend = false;
  if(input_test) {
    if (myid==0) {
      cerr << "#########################################################################" << endl;
      cerr << "## WARNING!!!" << endl;
      cerr << "## We are about to append to " << pathsFilename << ", which already exists." << endl;
      cerr << "#########################################################################" << endl;
    }
    bAppend = true;
  }
  input_test.close();
  
  // Setup the pathway tracker
  Tracker tracker;
  tracker.setupFromOptionsFile (infoFilename.c_str(),pathsFilename.c_str(),bDoAllVoxelsROI1,bPDBFormat,bVerbose,bAxialPathways,bAppend);
  

  Timer tAll;
  tAll.start();
  
  if( bDoAllVoxelsROI1 ) {  
    // Perform processor based tracking
    int startID;
    int numIDs = tracker.getNumStartLocs();
    int skipID;
    int ticPercent = 1;
    int lastTic = 0;
    
    if(myid==0) {
      cerr << numIDs << " cortical voxels." << endl << flush;
    }
      
    if( tracker.getNumDesiredPaths() < nprocs ) {
      skipID=nprocs;
      startID=myid;
      if(myid==0) {
        cerr << nprocs << " processors assigned to voxels." << endl << flush;
      }
    }else {
      // If a file already exists then try to find where we last left off
      if(!bAppend)
	startID=0;
      else {
	startID = tracker.getLastSavedID();
      }
      skipID=1;
      int new_num = (int)floor( (float)tracker.getNumDesiredPaths()/(float)nprocs );
      tracker.setNumDesiredPaths(new_num);
      if(myid==0) {
        cerr << nprocs << " processors assigned to " << new_num << " pathways per voxel." << endl << flush;
      }
    }
    
    for(; startID < numIDs; startID+=skipID) {
      if ( myid==0 && float(startID)/numIDs * 100 >= lastTic) {
        cerr << "Finished "<< float(startID)/numIDs * 100 << "% cortical voxels." << endl << flush;
        lastTic = int(floor((float(startID)/numIDs)*100/ticPercent)*ticPercent)+ticPercent;
      }
      tracker.generatePathways(max_time,startID);
    }
  } else {
    tracker.generatePathways(max_time,-1);
  }

   if(myid==0) {
//      cerr << "Tracker FileIO took " << tracker._tFileIO*1000 << " ms." << endl << flush;
//      cerr << "Tracker Gen took " << (tracker._tTotal-tracker._tFileIO)*1000 << " ms." << endl << flush;
//      cerr << "Tracker Gen Setup took " << tracker._tSetup*1000 << " ms." << endl << flush;
//      cerr << "Tracker Gen Grow took " << tracker._tGrow*1000 << " ms." << endl << flush;
//      cerr << "Tracker Gen Grow Tangent took " << tracker._tGrow*1000/tracker._nCountDrawTangent << " ms/i." << endl << flush;
//      cerr << "Count Grow Tangent " << tracker._nCountDrawTangent << " times." << endl << flush;
     cerr << "Generator took " << tAll.elapsed() << "s." << endl << flush;
   }

//    cerr << "Performing time test..." << endl;
//    double dTest;
//    Timer tTest; tTest.start();
//    for(int ii=0; ii<288000; ii++)
//      tracker.time_test(dTest);
//    cerr << "Foo: " << dTest << endl;
//    cerr << "Took: " << tTest.elapsed()*1000 << " ms." << endl;
//    cerr << "Iteration cost: " << tTest.elapsed()/288 << " ms/iter." << endl;

  
  // Cleanup
  tracker.cleanupGenerator();    
  

#ifdef USE_MPI
  // Clean up for MPI 
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize ();
#endif

  return EXIT_SUCCESS;
}
