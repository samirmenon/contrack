/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
 University. All rights reserved. **/

/*************************************************************************
 *    NAME: David Akers and Anthony Sherbondy
 *     
 *    FILE: DTIPathwayIO.cpp
 *    DATE: Thu Feb 26 21:14:26 PST 2004
 *************************************************************************/
#include "DTIPathwayIO.h"
#include "DTIPathwayDatabase.h"
#include "DTIPathway.h"
#include "DTIPathwayStatisticHeader.h"
#include "DTIPathwayAlgorithmHeader.h"
#include "DTIPathwaySelection.h"
#include <time.h>
#include "io_utils.h"
#include "ui_utils.h"
#include "math.h"
#include "DTIMath.h"
 #include <tnt/tnt_array1d_utils.h>
#include <tnt/tnt_array2d_utils.h>
using namespace std;

#include "Wm4/Wm4BSplineCurveFit.h"
#include "Wm4/Wm4Utils.h"
#include "Wm4/Wm4BandedMatrix.h"
#include <limits.h>
#ifndef PATH_MAX
	#define PATH_MAX 2048
#endif

using namespace Wm4;

#ifdef _WIN32
#include <windows.h>
#endif

//#define HACK
int totfibs=0;
int tottris=0;
void writeStatHeader(DTIPathwayStatisticHeader *header,
		std::ostream &theStream) {
	theStream.write((char *) header, sizeof(*header));
}

void readStatHeader(DTIPathwayStatisticHeader &header, std::istream &theStream) {
	theStream.read((char *) &header, sizeof (header));
}

void readStatHeaderOld(DTIPathwayStatisticHeaderOld &header, std::ifstream &theStream) {
	theStream.read((char *) &header, sizeof (header));
}

void writeAlgoHeader(DTIPathwayAlgorithmHeader *header,
		std::ostream &theStream) {
	theStream.write((char *) header, sizeof(*header));
}

void readAlgoHeader(DTIPathwayAlgorithmHeader &header, std::istream &theStream) {
	theStream.read((char *) &header, sizeof (header));
}


/***********************************************************************
 *  Method: DTIPathwayIO::saveDatabaseAny
 *  Params: DTIPathwayDatabase *db, bool bOverwrite, std::string filename, double ACPC[3]
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayIO::saveDatabaseAny(DTIPathwayDatabase *db, bool bOverwrite, std::string filename, double ACPC[3], DTIPathwaySelection *selection, bool bVerbose)
{
  if(db==NULL)
    return;

  enum FORMAT{PDB,BFLOAT,SBFLOAT};
  FORMAT format;
  if (filename.find(".Bfloat")!=string::npos)
    format = BFLOAT;
  else if (filename.find(".SBfloat")!=string::npos)
    format = SBFLOAT;
  else
    format = PDB;

  ofstream myOut;
  ostringstream strStream(std::string(),ios::out|ios::binary);
  if( format==PDB || bOverwrite )
    myOut.open (filename.c_str(), ios::out | ios::binary);
  else 
    myOut.open (filename.c_str(), ios::app | ios::binary);
  if( !myOut ) {
    cerr << "Cannot open output file " << filename << endl;
    exit(1);
  }    

  if(format==PDB) {
    // PDB format files
    DTIPathwayIO::saveDatabasePDB (db, strStream, ACPC, selection);
    if(!bOverwrite)
      cerr << "Can only append with .Bfloat format, overwrote any previous .pdb file." << endl;
  }
  else{
    // BFLOAT format files
    DTIPathwayIO::saveDatabaseBFLOAT (db,strStream,selection,format==SBFLOAT);
  }
  myOut<<strStream.str();
  myOut.close();
}

DTIPathwayDatabase *
DTIPathwayIO::loadAndAppendDatabasePDB(std::istream &pdbStream, DTIPathwayDatabase * oldPdb, bool bComputeRAPID, double ACPC[3], double vox_size[3])
{
	bool bPrintOut = false;

	if(bPrintOut) {
		cout << "###################################################################" << endl;
		cout << "#####               Begin PDB Printout                        #####" << endl;
		cout << "###################################################################" << endl;
	}

	// This function assumes that we want to load pathways into ACPC space 
	// if the ACPC offset is NULL and MM space otherwise
	bool bAppend = oldPdb ? true:false;
	DTIPathwayDatabase *db = bAppend ? oldPdb : new DTIPathwayDatabase();

	// TONY 2009.08.14 
	// ADDING half voxel offset to ACPC because contrack assumes samples are at the half integers
	// and the xform in the nifti image files assume measurments are at the integers 
	if(vox_size)
	{
		db->setVoxelSize(vox_size);
		for(int ii=0; ii<3; ii++) ACPC[ii]+=0.5*vox_size[ii]; 
	}
	unsigned int headerSize;
	headerSize = readScalar<int> (pdbStream);
	//pdbStream>>headerSize;
	if(bPrintOut)
		cout << "header size: " << headerSize << "(int32)" << endl;

	// This matrix is always the transform from pathway space into ACPC space
	// XXX Currently we only support pathways either in ACPC space or mm space.  
	//     Therefore, we expect either the identity here or anything else means 
	//     we are in mm space.
	double matrixEntry;
	for (int i = 0; i < 4; i++) 
		for (int j = 0; j < 4; j++) 
		{
			matrixEntry = readScalar<double> (pdbStream);
			db->_transform_mx[i*4+j] = matrixEntry;
		}

	if(bPrintOut)
		print_matrix(db->_transform_mx, 4, 4);

	// See if we want the database in MM space rather than ACPC
	//if( ACPC!=NULL && DTIMath::sqr(M[3])<0.001 && DTIMath::sqr(M[7])<0.001 && DTIMath::sqr(M[11])<0.001 ) {
	if( ACPC!=NULL ) 
	{
		// Xform specifies ACPC space lets get these into MM 
		// space by translating by the ACPC offset
		double* M = db->_transform_mx;
		M[3]  += ACPC[0];
		M[7]  += ACPC[1];
		M[11] += ACPC[2];
	}
	// Write out the matrix xform
	if(bPrintOut) 
	{
		if(ACPC==NULL)
			std::cerr << "Pathway Transform (to AcPc):" << endl;
		else
			std::cerr << "Pathway Transform (to mm):" << endl;
		print_matrix(db->_transform_mx, 4, 4);
	}

	int numStats = readScalar<int> (pdbStream);
	if(bPrintOut)
		cout << "num stats: " << numStats << "(int32)" << endl;

	if(oldPdb && numStats != oldPdb->getNumPathStatistics())
	{
		cout<<"The number of statistics should match in the old and new fibers. Cancelling"<<endl;
		return oldPdb;
	}

	int i;
	for (i = 0; i < numStats; i++) 
	{
		DTIPathwayStatisticHeader *header = new DTIPathwayStatisticHeader;
		readStatHeader (*header, pdbStream);
		if(!oldPdb)
			db->addStatisticHeader(header);
		if(bPrintOut) 
		{
			cout << "Stat Header " << i << " {" << endl;
			//header->print();
			cout << "}" << endl;
		}
	}

	int numAlgos = readScalar<int> (pdbStream);
	if(bPrintOut)
		cout << "num algs: " << numAlgos << "(int32)" << endl;

	if (numAlgos< 1 || numAlgos > 10)
	{ 
		if(!oldPdb)
		{
			std::cerr << "Warning: Unusual number of algorithms. Assuming old path format; will create default algorithm headers." << std::endl;
			DTIPathwayAlgorithmHeader *simulatedHeader = new DTIPathwayAlgorithmHeader;
			strcpy (simulatedHeader->_algorithm_name, "STT");
			db->addAlgorithmHeader(simulatedHeader);

			simulatedHeader = new DTIPathwayAlgorithmHeader;
			strcpy (simulatedHeader->_algorithm_name, "TEND");
			db->addAlgorithmHeader(simulatedHeader);

			simulatedHeader = new DTIPathwayAlgorithmHeader;
			strcpy (simulatedHeader->_algorithm_name, "STAT");
			db->addAlgorithmHeader(simulatedHeader);
		}
	}
	else 
	{
		for (int i = 0; i < numAlgos; i++) 
		{
			DTIPathwayAlgorithmHeader *algoHeader = new DTIPathwayAlgorithmHeader ;
			readAlgoHeader (*algoHeader, pdbStream);
			if(!oldPdb)
				db->addAlgorithmHeader(algoHeader);
			if(bPrintOut) 
			{
				cout << "Alg Header " << i << " {" << endl;
				algoHeader->print();
				cout << "}" << endl;
			}
		}
	}

	int versionNumber = readScalar<int> (pdbStream);
	if(bPrintOut) cout << "version number: " << versionNumber << "(int32)" << endl;
	// End of header

	// call the appropriate version #  
	switch(versionNumber)
	{
	case 2:
		loadDatabasePDB2Ver2(pdbStream, db, true, headerSize);
		break;
	case 3:
		loadDatabasePDB2Ver3(pdbStream, db, true);
		break;
	default:
		delete db;
		cerr << "version number must be at least 2!" << endl;
		return NULL;
		break;
	}


	for(i = 0; i < db->getNumPathStatistics(); i++)
		if(!oldPdb)
			db->_stats_threshold.push_back(new DTIPathwayStatisticThreshold);

	for(i = 0; i < db->getNumPathStatistics(); i++)
	{
		db->_stats_threshold[i]->_left = db->getMinValue(i);
		db->_stats_threshold[i]->_right = db->getMaxValue(i);
	}

	if(bPrintOut) {
		cout << "###################################################################" << endl;
		cout << "#####               End PDB Printout                          #####" << endl;
		cout << "###################################################################" << endl;
	}

	std::cerr << "Finished loading pathways." << std::endl;
	return db;
}

struct vect{float a[3];};
void DTIPathwayIO::loadDatabasePDB2Ver2(std::istream &theStream, DTIPathwayDatabase *db, bool multiplyMatrix, unsigned int headerSize_)
{
	bool bPrintOut = false;
	theStream.seekg (headerSize_, std::ios::beg);
	int numPathways = readScalar<int> (theStream);
	if(bPrintOut) cout << "number of pathways: " << numPathways << "(int32)" << endl;

	int numAccepted = 0;
	int nCntPercent = 10;
	db->_fiber_pathways.reserve(numPathways+10+db->_fiber_pathways.size());

	fprintf(stderr,"Loaded     ");
	for (int k = 0; k < numPathways; k++) 
	{
		int iDonePercent = (int)(k*100.0f/(numPathways-1));
		if( iDonePercent >= nCntPercent ) 
		{
			if(!bPrintOut) 
				fprintf(stderr,"\b\b\b\b%3d%%", iDonePercent);
			nCntPercent+=10;
		}


		// load a single pathway
		int numPathStatistics = db->getNumPathStatistics();
		unsigned int headerSize = readScalar<int>(theStream);
		if(bPrintOut) cout << endl << "path header size: " << headerSize << "(int32)" << endl;

		unsigned int pos = theStream.tellg();
		int numPoints = readScalar<int> (theStream);
		if(bPrintOut) cout << "num points: " << numPoints << "(int32)" << endl;

		int algoInt = readScalar<int> (theStream);
		if(bPrintOut) cout << "alg type: " << algoInt << "(int32)" << endl;

		DTIPathway *pathway = new DTIPathway((DTIPathwayAlgorithm) algoInt);
		int seedPointIndex = readScalar<int> (theStream);
		if(bPrintOut) cout << "seed index: " << seedPointIndex << "(int32)" << endl;
		pathway->setSeedPointIndex(seedPointIndex);

		int i = 0;  
		double *pathStat = new double[numPathStatistics];

		for (i = 0; i < numPathStatistics; i++) 
		{
			pathStat[i] = readScalar<double>  (theStream);
			if(bPrintOut) cout << "path stat " << i << ": " << pathStat[i] << "(double)" << endl;
		}
		theStream.seekg(pos + headerSize, std::ios::beg);

		const double *mx = db->getTransformMatrix();
		TNT::Array2D<double> multMx(4,4);
		for(int ii=0; ii<4; ii++)
			for(int jj=0; jj<4; jj++)
				multMx[ii][jj] = *(mx+ii*4+jj);
		totfibs+=numPoints;
		pathway->_point_vector.reserve(numPoints);
		pathway->_path_grow_weight.reserve(numPoints);
		for (i = 0; i < numPoints; i++) 
		{
			double p[4];
			p[0] = readScalar<double>  (theStream);
			p[1] = readScalar<double>  (theStream);
			p[2] = readScalar<double>  (theStream);
			p[3] = 1.0;	  
			float pFloat[] = {(float)p[0], (float)p[1], (float)p[2]};

			if(bPrintOut) cout << "point " << i << ": [" << p[0] << "(double)" << "," << p[1] << "(double)" << "," << p[2] << "(double)" << "]" << endl;
			if (multiplyMatrix) 
			{
				double pOut[4];
				TNT::Array1D<double> vecIn(4,p);
				TNT::Array1D<double> vecOut(4,pOut);
				vecOut = multMx * vecIn;
				pFloat[0] = (float)vecOut[0] ; pFloat[1] = (float)vecOut[1] ; pFloat[2] = (float)vecOut[2]; 
			}
			DTIVector v(pFloat);
			pathway->append (v);
		}

		pathway->initializePathStatistics(numPathStatistics, db->getPathStatisticHeaders(), true);

		for (i = 0; i < numPathStatistics; i++) 
		{
			pathway->setPathStatistic(i, pathStat[i]);
			if (db->getPathStatisticHeader(i)->_is_computed_per_point) 
			{
				for (int j = 0; j < numPoints; j++) 
				{
					double stat;
					stat = readScalar<double>  (theStream);
					pathway->setPointStatistic(j, i, stat);
					if(bPrintOut) cout << "Point Stats " << i << "," << j << ": " << stat << "(double)" << endl;
				}
			}
		}
		delete []pathStat;
		pathway->setID(numAccepted++);
		db->addPathway (pathway);

		// build the rapid model
		pathway->getRAPIDModel();
	}
	cerr<<endl;
}
void DTIPathwayIO::loadDatabasePDB2Ver3(std::istream &theStream, DTIPathwayDatabase *db, bool multiplyMatrix)
{
	bool bPrintOut			= false;
	int numPathways			= readScalar<int> (theStream);
	int numPathStatistics	= db->getNumPathStatistics();
	int nCntPercent			= 10;

	// Read the number of points per fiber
	int *points_per_fiber = new int[numPathways];
	theStream.read((char*)points_per_fiber, sizeof(int)*numPathways);

	// Compute the total # of points across all fibers
	int total_pts=0;
	for(int i = 0; i < numPathways; i++)
		total_pts += points_per_fiber[i];

	// Read all the points
	double *fiber_points = new double[total_pts*3];
	theStream.read((char*)fiber_points, sizeof(double)*total_pts*3);

	fprintf(stderr,"Loaded     ");

	for (int k = 0, points_read=0; k < numPathways; k++) 
	{
		int iDonePercent = (float)k*100.0f/(numPathways-1);
		if( iDonePercent >= nCntPercent ) 
		{
			if(!bPrintOut) 
				fprintf(stderr,"\b\b\b\b%3d%%", iDonePercent);
			nCntPercent+=10;
		}

		// load a single pathway
		int numPoints = points_per_fiber[k];
		if(bPrintOut) 
			cout << "num points: " << numPoints << "(int32)" << endl;

		DTIPathway *pathway = new DTIPathway((DTIPathwayAlgorithm) 0);
		int seedPointIndex = 1;
		if(bPrintOut) 
			cout << "seed index: " << seedPointIndex << "(int32)" << endl;
		pathway->setSeedPointIndex(seedPointIndex);

		const double *mx = db->getTransformMatrix();
		
		pathway->_point_vector.reserve(numPoints);
		pathway->_path_grow_weight.reserve(numPoints);
		for (int i = 0; i < numPoints; i++, points_read+=3) 
		{
			double *p = fiber_points + points_read ;
			float pFloat[3];

			if(bPrintOut) cout << "point " << i << ": [" << p[0] << "(double)" << "," << p[1] << "(double)" << "," << p[2] << "(double)" << "]" << endl;
			//if (multiplyMatrix) 
			{
				for(int j = 0; j < 3; j++)
					pFloat[j] = p[0]*mx[0+j*4] + p[1]*mx[1+j*4] +p[2]*mx[2+j*4] + mx[3+j*4];
			}
			DTIVector v(pFloat);
			pathway->append (v);
		}

		pathway->initializePathStatistics(numPathStatistics, db->getPathStatisticHeaders(), true);
		pathway->setID(k);
		db->addPathway (pathway);

		// build the rapid model
		pathway->getRAPIDModel();
	}
	delete [] fiber_points;

	//Read the per fiber stats
	for(int i = 0; i < numPathStatistics; i++)
	{
		double *per_fiber_stat = new double[numPathways];
		theStream.read((char*)per_fiber_stat, sizeof(double)*numPathways);

		for(int j = 0, points_read = 0; j < numPathways; j++)
			db->getPathway(j)->setPathStatistic(i, per_fiber_stat[j]);

		delete [] per_fiber_stat;
	}

	//Read per point stat
	for(int i = 0; i < numPathStatistics; i++)
	{
		double *per_point_stat = new double[total_pts];
		if (db->getPathStatisticHeader(i)->_is_computed_per_point) 
			theStream.read((char*)per_point_stat, sizeof(double) * total_pts);

		for(int j = 0, points_read = 0; j < numPathways; j++)
			if (db->getPathStatisticHeader(i)->_is_computed_per_point) 
				for (int k = 0; k < points_per_fiber[j]; k++, points_read++) 
				{
					db->getPathway(j)->setPointStatistic(k, i, per_point_stat[points_read]);
					if(bPrintOut) cout << "Point Stats " << i << "," << j << ": " << per_point_stat[points_read] << "(double)" << endl;
				}

		delete [] per_point_stat;
	}
	delete []points_per_fiber;
	cerr<<endl;
}


void DTIPathwayIO::saveDatabasePDB(DTIPathwayDatabase *db, std::ostream &theStream, double ACPC[3], DTIPathwaySelection *selection)
{
	// Header size
	unsigned int headerSize;
	headerSize = 3 * sizeof(int)+ 16 * sizeof(double)
		+ db->getNumPathStatistics()*sizeof(DTIPathwayStatisticHeader)
		+ db->getNumAlgorithms() * sizeof(DTIPathwayAlgorithmHeader) + sizeof(int);
	writeScalar<int>  (headerSize, theStream);

	// TONY 2009.08.14 
	// ADDING half voxel offset to ACPC because contrack assumes samples are at the half integers
	// and the xform in the nifti image files assume measurments are at the integers 
	float vox_size[3];
	db->getVoxelSize(vox_size);
	for(int ii=0; ii<3; ii++)
		if(vox_size[ii]!=-1)
			ACPC[ii]+=0.5*vox_size[ii]; 

	// Xform
	for (int i = 0; i < 16; i++) 
	{
		if( i%5==0 )
			writeScalar<double> (1, theStream); 
		else if(i==3 || i==7 || i==11)
			writeScalar<double> (-ACPC[(int)floor(float(i)/3-1)], theStream); 
		else
			writeScalar<double> (0, theStream); 
	}

	// Statistics header
	writeScalar<int>  (db->getNumPathStatistics(), theStream);
	for (int i = 0; i < db->getNumPathStatistics(); i++) 
		writeStatHeader (db->getPathStatisticHeader(i), theStream);

	// Algorithm info. header
	writeScalar<int>  (db->getNumAlgorithms(), theStream);
	if (db->getNumAlgorithms() == 0) 
	{
		cerr << "ERROR: Missing algorithm header information in database." << endl;
		cerr << "Cannot open database for writing..."<< endl;
		return;
	}
	for (int i = 0; i < db->getNumAlgorithms(); i++)
		writeAlgoHeader (db->getPathAlgorithmHeader(i), theStream);

	// Version info
	writeScalar<int> (3, theStream);
	int total_pathways = db->getNumFibers();

	int numPathways;
	if (selection)
		numPathways = selection->NumSelectedPathways();
	else
		numPathways = db->getNumFibers();
	assert(numPathways<=db->getNumFibers());
	writeScalar<int>  (numPathways, theStream);


	// Save points per fiber
	for(int i = 0; i < total_pathways; i++)
		if( selection==NULL || (*selection)[i] ) 
			writeScalar<int>( db->getPathway(i)->getNumPoints(), theStream);

	//Save the fiber points to buffer
	for(int i = 0; i < total_pathways; i++)
		if( selection==NULL || (*selection)[i] ) 
			for(int j = 0; j < db->getPathway(i)->getNumPoints(); j++)
			{
				DTIGeometryVector *vec = db->getPathway(i)->_point_vector[j];
				writeScalar<double>((*vec)[0], theStream);
				writeScalar<double>((*vec)[1], theStream);
				writeScalar<double>((*vec)[2], theStream);
			}

	//Write the per fiber stats
	for(int i = 0; i < db->getNumPathStatistics(); i++)
		for(int j = 0; j < total_pathways; j++)
			if( selection==NULL || (*selection)[j] )
				writeScalar<double>(db->getPathway(j)->getPathStatistic(i), theStream);

	// Write per point stats
	for(int i = 0; i < db->getNumPathStatistics(); i++)
		if (db->getPathStatisticHeader(i)->_is_computed_per_point)
			for(int j = 0; j < total_pathways; j++)
				if( selection==NULL || (*selection)[j] )
					for(int k = 0; k < db->getPathway(j)->getNumPoints(); k++)
						writeScalar<double>(db->getPathway(j)->getPointStatistic(k,i), theStream);
}
void
DTIPathwayIO::appendDatabaseFile(DTIPathwayDatabase *pathways, const char *filename, DTIPathwaySelection *selection)
{
  // Assume that the file has already been written to by this database
  std::ifstream _theIStream(filename, std::ios::in | std::ios::binary);
  std::istream *ptr = FileToString(_theIStream);
  std::istream &theIStream = *ptr;

  if ( !ptr) {
    std::cerr << "Can't open input file \""<< filename << "\""<< std::endl;
    return;
  }
  
  // Skip stats loading
  int headerSize = readScalar<int>(theIStream);
  theIStream.seekg(headerSize, std::ios::beg);

  // Get pathways in current file database 
  int numPaths;
  numPaths = readScalar<int>(theIStream);
  
  // Append all pathways in the database in memory to the end of the PDB stored in the file
  int numAppendedPathways;
  if (selection)
    numAppendedPathways = selection->NumSelectedPathways();
  else
    numAppendedPathways = pathways->getNumFibers();
  int newNumPaths = numPaths + numAppendedPathways;

  if(numPaths < 0 || numPaths >= newNumPaths) {
    std::cerr << "Cannot append to file due to reading error or trying to append empty database." << std::endl;
    //theIStream.close();
  }
  else {
    // Setup file offset vector to handle offsets of file database and memory
    uint64_t *fileOffsets = new uint64_t[newNumPaths];

    // First load the file offsets into the vector
    theIStream.seekg(0, std::ios::end);
    int length = theIStream.tellg();
    theIStream.seekg(length-numPaths*sizeof(uint64_t));	
    for (int i = 0; i < numPaths; i++) {
       fileOffsets[i] = readScalar<uint64_t> (theIStream);
    }
    // Finished with the input file
    //theIStream.close();

    // Start appending
    std::ofstream theOStream(filename, std::ios::in | std::ios::out | std::ios::binary);
    // Write new pathway database size
    theOStream.seekp(headerSize,std::ios::beg);
    writeScalar<int> (newNumPaths,theOStream);

    // Seek to the end of the previous pathway database, the start of the file offsets footer
    theOStream.seekp(0,std::ios::end);
    length = theOStream.tellp();
    theOStream.seekp(length-(numPaths)*sizeof(uint64_t));
      
    // Append the pathways in memory
    //std::vector<DTIPathway *>::iterator iter;
    int writecount = numPaths;
    for (int ii = 0; ii < pathways->getNumFibers(); ii++, writecount++) {
      if (selection && !((*selection)[ii])) 
	continue;
      // Only write pathway if there is no selections or if there are 
      // selections and this one was selected
      DTIPathway *pathway = pathways->getPathway(ii);
      fileOffsets[writecount] = theOStream.tellp();
      savePathwayPDB (pathway, theOStream, pathways, false);
    }
    // Write entirely new file offsets to the footer
    for (int ii = 0; ii < newNumPaths; ii++) {
      writeScalar<uint64_t> (fileOffsets[ii], theOStream);
    }
    theOStream.close();
    delete[] fileOffsets;
  }
  delete ptr;
}
void
DTIPathwayIO::savePathwayPDB(DTIPathway *pathway, std::ostream &theStream, DTIPathwayDatabase *db, bool debug)
{
  unsigned int headerSize;
  headerSize = 3* sizeof(int)+ db->getNumPathStatistics()*sizeof(double);
  writeScalar<int>  (headerSize, theStream);
  
  int numPoints = pathway->_point_vector.size();
  writeScalar<int>  (numPoints, theStream);
  int algo = (int) pathway->getPathwayAlgorithm();
  writeScalar<int>  (algo, theStream);
  int seedPointIndex = pathway->getSeedPointIndex();
  writeScalar<int>  (seedPointIndex, theStream);
  for (int i = 0; i < db->getNumPathStatistics(); i++) {
    writeScalar<double> (pathway->getPathStatistic(i), theStream);
  }
  
  //std::cerr << "path: ";	
  for (std::vector<DTIGeometryVector *>::iterator
	 iter = pathway->_point_vector.begin(); iter
	 != pathway->_point_vector.end(); iter++) {
    DTIGeometryVector *vec = *iter;
    writeScalar<double>((*vec)[0], theStream);
    writeScalar<double>((*vec)[1], theStream);
    writeScalar<double>((*vec)[2], theStream);
    //std::cerr << "[" << (*vec)[0] << "," << (*vec)[1] << "," << (*vec)[2] << "];  ";	
  }
  //std::cerr << std::endl;
  
  for (int i = 0; i < db->getNumPathStatistics(); i++) {
    if (debug) {
      std::cerr << "num points of stats: "
		<< pathway->_point_stat_array[i].size() << std::endl;
    }
    if (db->getPathStatisticHeader(i)->_is_computed_per_point) {
      for (std::vector<double>::iterator
	     iter = pathway->_point_stat_array[i].begin(); iter
	     != pathway->_point_stat_array[i].end(); iter++) {
	double stat = *iter;
	writeScalar<double> (stat, theStream);
      }
    }
  }
}



/***********************************************************************
 *  Method: DTIPathwayIO::loadDatabaseBFLOAT
 *  Params: std::ifstream &pdbStream, int numPathwaysToLoad, bool bSplineCompress, bool bCaminoNoStats
 * Returns: DTIPathwayDatabase *
 * Effects: 
 ***********************************************************************/
DTIPathwayDatabase *
DTIPathwayIO::loadDatabaseBFLOAT(std::ifstream &pdbStream, int numPathwaysToLoad, bool bSplineCompress, bool bCaminoNoStats)
{
  DTIPathwayDatabase *db = new DTIPathwayDatabase();

  DTIPathwayAlgorithmHeader *simulatedHeader = new DTIPathwayAlgorithmHeader;
  strcpy (simulatedHeader->_algorithm_name, "STT");
  db->addAlgorithmHeader(simulatedHeader);
  simulatedHeader = new DTIPathwayAlgorithmHeader;
  strcpy (simulatedHeader->_algorithm_name, "TEND");
  db->addAlgorithmHeader(simulatedHeader);
  simulatedHeader = new DTIPathwayAlgorithmHeader;
  strcpy (simulatedHeader->_algorithm_name, "STAT");
  db->addAlgorithmHeader(simulatedHeader);


  int numAccepted = 0;
  int count = 0;
  while ( numPathwaysToLoad<0 || numAccepted<numPathwaysToLoad ){
    DTIPathway *pathway = new DTIPathway((DTIPathwayAlgorithm) 2);
    pathway->initializePathStatistics(0, db->getPathStatisticHeaders(), true);
    if(loadPathwayBFLOAT(pathway,pdbStream,bSplineCompress,bCaminoNoStats)<0) {
      delete pathway;
      break;
    }
    pathway->setID(numAccepted++);
    db->addPathway(pathway);	
  }
  return db;
}


/***********************************************************************
 *  Method: DTIPathwayIO::loadAndAppendDatabaseBFLOAT
 *  Params: DTIPathwayDatabase *pdb, std::ifstream &pdbStream, int numPathwaysToLoad, bool bSplineCompress, bool bCaminoNoStats
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayIO::loadAndAppendDatabaseBFLOAT(DTIPathwayDatabase *pdb, std::ifstream &pdbStream, int numPathwaysToLoad, bool bSplineCompress, bool bCaminoNoStats)
{
  if(pdb==NULL)
    return;

  int numAccepted = 0;
  while ( numPathwaysToLoad<0 || numAccepted<numPathwaysToLoad ){
    // Read a pathway
    DTIPathway *pathway = new DTIPathway((DTIPathwayAlgorithm) 2);
    pathway->initializePathStatistics(0, pdb->getPathStatisticHeaders(), true);
    if(loadPathwayBFLOAT(pathway,pdbStream,bSplineCompress,bCaminoNoStats)<0) {
      delete pathway;
      break;
    }
    pathway->setID(numAccepted++);
    pdb->addPathway(pathway);	
  }
  return;
}

/***********************************************************************
 *  Method: DTIPathwayIO::loadPathwayBFLOAT
 *  Params: DTIPathway *pathway, std::ifstream &pdbStream, bool bSplineCompress, bool bCaminoNoStats
 * Returns: int
 * Effects: 
 ***********************************************************************/
int
DTIPathwayIO::loadPathwayBFLOAT(DTIPathway *pathway, std::ifstream &pdbStream, bool bSplineCompress, bool bCaminoNoStats)
{
  // Read first float as a test, must do this to use the eofbit test
  float firstFloatRead = readScalarBE<float>(pdbStream);
  if(pdbStream.rdstate() & ifstream::eofbit) //pdbStream.fail()
    return -1; // Must check after we read

  // Two types of Bfloat files depending on whether the format supports stats
  //
  // Type 1 (No Stats or original Camino): num_points, p0_x, p0_y, p0_z, p1_x, p1_y, p1_z, ...
  //
  // Type 2 (With Stats): num_stats, num_points, s0, s1, ..., p0_x, p0_y, p0_z, p1_x, p1_y, p1_z, ...
  float numStats = 0;
  float numPoints = 0; 

  if (!bCaminoNoStats) {
    numStats = firstFloatRead;
    numPoints = readScalarBE<float>(pdbStream);
  } else {
    numPoints = firstFloatRead;
    numStats  = 1; // for seed id, maybe should just ignore
    // Don't support bspline compression and original camino format
    if (bSplineCompress)
      return -1;
  }

  DTIPathway* pathTemp = pathway;
  float numSamplePoints;
  if(bSplineCompress) {
    float numCtrlPoints = readScalarBE<float>(pdbStream);
    numSamplePoints = numPoints;
    numPoints = numCtrlPoints;
    pathTemp = new DTIPathway(DTI_PATHWAY_ALGORITHM_ANY);
  }

  // Just ignore these numbers for now
  unsigned int ss;
  for( ss=0; ss<numStats; ss++) {
    float stat;
    stat = readScalarBE<float>(pdbStream);
  }

  for(uint pp=0; pp<numPoints; pp++ ) {
    float p[3];
      p[0] = readScalarBE<float> (pdbStream);
      p[1] = readScalarBE<float> (pdbStream);
      p[2] = readScalarBE<float> (pdbStream);

    DTIVector v(p);
    pathTemp->append(v);
  }

  if(bSplineCompress) {
    DTIPathwayIO::DecompressPathway(pathway,pathTemp,(uint)numSamplePoints);
    delete pathTemp;
  }
  return 0;
}

/***********************************************************************
 *  Method: DTIPathwayIO::loadPathwaySegmentBFLOAT
 *  Params: DTIPathway *pathway, std::ifstream &pdbStream, short p1, short p2, bool bSplineCompress
 * Returns: int
 * Effects: 
 ***********************************************************************/
int
DTIPathwayIO::loadPathwaySegmentBFLOAT(DTIPathway *pathway, std::ifstream &pdbStream, short p1, short p2, bool bSplineCompress)
{
  if(loadPathwayBFLOAT(pathway, pdbStream, bSplineCompress)!=0) {
    cerr << "Warning: We have reached the end of the pathway file...perhaps prematurely." << endl;
    return -1;
  }
  int numPoints = pathway->getNumPoints();
  if(p1<0)
    p1=0;
  if(p2<0)
    p2=(short)numPoints-1;

  if(numPoints<p1 || numPoints<p2) {
    cerr << "isLE: " << isLittleEndian() << endl;
    cerr << "numPoints: " << numPoints << endl;
    cerr << "p1: " << p1 << " p2: " << p2 << endl;
    cerr << "Unable to load pathway segment." << endl;
    return -1;
  }

  // Remove points  
  if(p2<numPoints-1)
    pathway->remove(p2+1,numPoints);
  if(p1>0)
    pathway->remove(0,p1);

//   pathway->_point_vector.erase(pathway->_point_vector.begin()+p2+1,pathway->_point_vector.end());
//   pathway->_point_vector.erase(pathway->_point_vector.begin(),pathway->_point_vector.begin()+p1);
//   // Remove any stats for this point
//   if(!pathway->_point_stat_array.empty()) {
//     pathway->_point_stat_array.erase(pathway->_point_stat_array.begin()+p2+1,pathway->_point_stat_array.end());
//     pathway->_point_stat_array.erase(pathway->_point_stat_array.begin(),pathway->_point_stat_array.begin()+p1);
//   }
//   if(!pathway->_path_stat_vector.empty()) {
//     pathway->_path_stat_vector.erase(pathway->_path_stat_vector.begin()+p2+1,pathway->_path_stat_vector.end());
//     pathway->_path_stat_vector.erase(pathway->_path_stat_vector.begin(),pathway->_path_stat_vector.begin()+p1);
//   }
  return 0;
}

/***********************************************************************
 *  Method: DTIPathwayIO::savePathwayBFLOAT
 *  Params: DTIPathway *pathway, std::ostream &theStream
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayIO::savePathwayBFLOAT(DTIPathway* pathway, std::ostream &pdbStream, bool bSplineCompress, bool bCaminoNoStats)
{
	// Write a pathway header

	// Two types of Bfloat files depending on whether the format supports stats
	//
	// Type 1 (No Stats or original Camino): num_points, seed_id, p0_x, p0_y, p0_z, p1_x, p1_y, p1_z, ...
	//
	// Type 2 (With Stats): num_stats, num_points, s0, s1, ..., p0_x, p0_y, p0_z, p1_x, p1_y, p1_z, ...
	float numStats = 0;
	float numPoints = (float)pathway->getNumPoints();

	// Write stats if our format has them
	if (!bCaminoNoStats) {
		
		numStats = (float)pathway->getNumPathStatistics();
		if(bSplineCompress)	
			numStats=0;
		writeScalarBE<float> (numStats,pdbStream);
		
	} 
	
	// Write points
	writeScalarBE<float> (numPoints,pdbStream); 
	
	// Write seed id if our format has it
	if (bCaminoNoStats) {
		
		float seed_id = (float)pathway->getID();
		writeScalarBE<float> (seed_id,pdbStream);
		
	} 
	

  //float numStats = (float)pathway->getNumPathStatistics();
//  if(bSplineCompress)
//    numStats=0;
//  writeScalarBE<float> (numStats,pdbStream);
//  float numPoints = (float)pathway->getNumPoints();
//  writeScalarBE<float> (numPoints,pdbStream); 

  if(bSplineCompress) {
    pathway = DTIPathwayIO::CompressPathway(pathway,0.1);
    //pathway = DTIPathwayIO::CompressPathway(pathway,0.1,1.5);
    float numCtrlPoints = (float)pathway->getNumPoints();
    writeScalarBE<float> (numCtrlPoints,pdbStream);
  }    

  // Write pathway stats if our format had them
  for(uint ss=0; ss<(uint)numStats; ss++) {
    float stat = pathway->getPathStatistic(ss);
    writeScalarBE<float> (stat,pdbStream);
  }

  // Write pathway points
   int pp;
  for( pp=0; pp<pathway->getNumPoints(); pp++ ) {
    float p[3];
    pathway->getPoint(pp,p);
    writeScalarBE<float> (p[0],pdbStream);
    writeScalarBE<float> (p[1],pdbStream);
    writeScalarBE<float> (p[2],pdbStream);
  }

  if(bSplineCompress)
    delete pathway;
}

/***********************************************************************
 *  Method: DTIPathwayIO::saveDatabaseBFLOAT
 *  Params: DTIPathwayDatabase *db, std::ostream &pdbStream
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayIO::saveDatabaseBFLOAT(DTIPathwayDatabase *db, std::ostream &pdbStream, DTIPathwaySelection *selection, bool bSplineCompress, bool bVerbose)
{
  if(bSplineCompress && bVerbose)
    cout << "Saving pathways in lossy BSpline compressed format..." << endl;
  ProgressBar pb;
  int nFibers = db->_fiber_pathways.size();
  for (int pp=0; pp<nFibers; pp++) {
    DTIPathway *pathway = db->_fiber_pathways[pp];
    savePathwayBFLOAT(pathway, pdbStream, bSplineCompress);
    if(bVerbose)
      pb.update((float)pp/nFibers*100);
  }
  if(bVerbose) {
    pb.update(100.); cout << endl;
  }
}


/***********************************************************************
 *  Method: DTIPathwayIO::openDatabase
 *  Params: DTIPathwayDatabase *db, const char *filename
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayIO::openDatabase(DTIPathwayDatabase *db, const char *filename)
{
  ofstream theStream (filename, std::ios::out | std::ios::binary);
  unsigned int headerSize;
  headerSize = 4 * sizeof (int) + 16 * sizeof(double) + db->getNumPathStatistics()*sizeof (DTIPathwayStatisticHeader) + db->getNumAlgorithms() * sizeof (DTIPathwayAlgorithmHeader);

//  cerr << "Header size: " << headerSize << endl;

  writeScalar<int>  (headerSize, theStream);

  // write matrix transform:
  for (int i = 0; i < 16; i++) {
    writeScalar<double> (db->_transform_mx[i], theStream);
  }

  writeScalar<int>  (db->getNumPathStatistics(), theStream);
  
  for (int i = 0; i < db->getNumPathStatistics(); i++) {
    writeStatHeader (db->getPathStatisticHeader(i), theStream);
  }
  
  writeScalar<int>  (db->getNumAlgorithms(), theStream);

  //  for (int i = 0; i < pathways->getNumAlgorithms(); i++) {
  if (db->getNumAlgorithms() == 0) {
    cerr << "ERROR: Missing algorithm header information in database." << endl;
    cerr << "Cannot open database for writing..." << endl;
    return;
  }
  for (int i = 0; i < db->getNumAlgorithms(); i++) {
    writeAlgoHeader (db->getPathAlgorithmHeader(i), theStream);
  }
  int versionNum = 2; // DTIQuery 1.2
  writeScalar<int>  (versionNum, theStream);

  // zero pathways...
  writeScalar<int>  (0, theStream);
  theStream.close();
}


/***********************************************************************
 *  Method: DTIPathwayIO::appendFileOffsetsToDatabaseFile
 *  Params: int numPathways, const uint64_t *fileOffsets, const char *filename
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
DTIPathwayIO::appendFileOffsetsToDatabaseFile(int numOffsets, const uint64_t *fileOffsets, const char *filename)
{
  ofstream outStream;
  outStream.open (filename, std::ios::in | std::ios::out | std::ios::binary);
  outStream.seekp (0, std::ios::end);
  for (int i = 0; i < numOffsets; i++) {
    writeScalar<uint64_t> (fileOffsets[i], outStream);
  }
  outStream.close();
}


 //----------------------------------------------------------------------------
DTIPathway* DTIPathwayIO::CompressPathway (const DTIPathway* pathIn, float fReduceTarget, float fCompressThresh)
 {
   DTIPathway* pathOut = NULL;
   // Set initial control point parameters
   uint iSampleQuantity = pathIn->getNumPoints();
   uint iDegree = 3;
   uint iCtrlQuantity = (uint)DTIMath::round((float)iSampleQuantity*fReduceTarget);
   if(iCtrlQuantity<SPLINE_MIN_CTRL_POINTS)
     iCtrlQuantity=SPLINE_MIN_CTRL_POINTS;

   // Only do spline compression if there are the minimum points necessary
   if( pathIn->getNumPoints() < iCtrlQuantity ) {
     // Just copy the input points out without compression
     pathOut = new DTIPathway(DTI_PATHWAY_ALGORITHM_ANY);
     for(int pp=0; pp<pathIn->getNumPoints(); pp++)
       pathOut->append(pathIn->getPointV(pp));
   } else {
     // Flatten sample points into a single dimensional vector
     float* v1D = new float[iSampleQuantity*3];
     uint i1D = 0;
     for(uint pp=0; pp<iSampleQuantity; pp++) 
       pathIn->getPoint(pp, (float*)(v1D+(pp*3)) );
     
     //cout << "Compressing pathway with " << iSampleQuantity << " points..." << endl;
     bool bCompressed = false;
     float fMaxError;
     while(!bCompressed) {
       // Create the curve from the current parameters.
       BSplineCurveFitf* spline = new BSplineCurveFitf(3,iSampleQuantity,(const float*)v1D,iDegree,iCtrlQuantity);
       
       // Store control points in path
       DTIPathway* pathTemp = new DTIPathway(DTI_PATHWAY_ALGORITHM_ANY);
       const float* ctrls = spline->GetControlData();
       for(int pp=0; pp<spline->GetControlQuantity(); pp++) {
	 DTIVector v;
	 for(int ii=0; ii<3; ii++) v[ii]=ctrls[pp*3+ii];
	 pathTemp->append(v);
       }
       
       if(fCompressThresh>=0) {
	 float fAvrErrorTemp,fRMSErrorTemp,fMaxErrorTemp;
	 //CalculateError(spline,pathIn,fAvrErrorTemp,fRMSErrorTemp,fMaxErrorTemp);
	 CalculateError(pathTemp,pathIn,fAvrErrorTemp,fRMSErrorTemp,fMaxErrorTemp);
	 //     cout << "Ctrl Num: " << iCtrlQuantity << " Max Error: " << fMaxErrorTemp << " Avr Error: " << fAvrErrorTemp << ", RMS Error: " << fRMSErrorTemp << endl;
	 if(fMaxErrorTemp>fCompressThresh || iCtrlQuantity==5) {
	   bCompressed = true;
	   delete pathTemp;
	 } else {
	   iCtrlQuantity--;   
	   fMaxError = fMaxErrorTemp;   
	   delete pathOut;
	   pathOut = pathTemp;
	 }
       } else {
	 bCompressed = true;
	 pathOut = pathTemp;
       }
       delete spline;
     }
     //iCtrlQuantity++;
     //cout << "Compression: " << (float)iCtrlQuantity/iSampleQuantity*100 << "% Max Error: " << fMaxError << endl;
     //  cout << "Compressed ctrl num: " << iCtrlQuantity << endl;
     delete[] v1D;
   }
   return pathOut;
 }
 //----------------------------------------------------------------------------
void DTIPathwayIO::DecompressPathway (DTIPathway* pathOut, const DTIPathway* pathIn, uint iSampleQuantity, uint iDegree)
 {
   if( pathIn->getNumPoints()<SPLINE_MIN_CTRL_POINTS ) {
     for(int pp=0; pp<pathIn->getNumPoints(); pp++)
       pathOut->append(pathIn->getPointV(pp));
   } else {
     // Set variables
     int iCtrlQuantity = pathIn->getNumPoints();
     
     // Construct spline from control points   
     float* fCtrls = new float[iCtrlQuantity*3];
     for(int pp=0; pp<iCtrlQuantity; pp++) 
       pathIn->getPoint(pp,fCtrls+pp*3);
     BSplineCurveFitf* spline = new BSplineCurveFitf(3,iDegree,iCtrlQuantity,fCtrls);
     delete[] fCtrls;
     
     // Reconstruct polyline with number of samples
     float fMult = 1.0f/(iSampleQuantity - 1);
     for (uint i = 0; i < iSampleQuantity; i++)
       {
	 // Get spline point reconstruction
	 float fT = fMult*i;
	 DTIVector v;
	 spline->GetPosition(fT,v);
	 pathOut->append(v);
       }
     delete spline;
   }
 }
 
//----------------------------------------------------------------------------
void DTIPathwayIO::CalculateError(const DTIPathway* pathCtrl, const DTIPathway* path, float &fAvrError, float &fRMSError, float &fMaxError)
{
  DTIPathway* pathD = new DTIPathway(DTI_PATHWAY_ALGORITHM_ANY);
  DTIPathwayIO::DecompressPathway(pathD,pathCtrl,path->getNumPoints());
  assert(pathD->getNumPoints() == path->getNumPoints());
  
  uint iSampleQuantity = path->getNumPoints();
  // Compute error measurements.
  fAvrError = 0.0f;
  fRMSError = 0.0f;
  fMaxError = 0.0f;
  for (uint i = 0; i < iSampleQuantity; i++)
    {
      DTIVector vDiff = pathD->getPointV(i) - path->getPointV(i);
      float fSqrLength = dproduct(vDiff,vDiff);
      fRMSError += fSqrLength;
      float fLength = sqrt(fSqrLength);
      fAvrError += fLength;
      if(fLength>fMaxError)
	fMaxError = fLength;
    }
  fAvrError /= (float)iSampleQuantity;
  fRMSError /= (float)iSampleQuantity;
  fRMSError = sqrt(fRMSError);
  delete pathD;
}
//----------------------------------------------------------------------------

std::istringstream* FileToString(std::ifstream &file)
{
	  file.seekg (0, std::ios::end);long length = file.tellg();file.seekg (0, std::ios::beg);
	  std::istringstream *str= 0;
//printf("\n\n\n File length is %d\n\n\n",(int)length);
	  if(length > 0)
	  {
		  char *buffer = new char [length];
		  file.read (buffer,length);
		  str = new istringstream( string( buffer,length ), ios::binary);
		  delete [] buffer;
	  }
	  else
	  {
		  str = new istringstream();
		  str->setstate(ios_base::badbit);
	  }
	  return str;
}

void StringToFile(std::ofstream &file, std::ostringstream &localStream)
{
}

std::string RelativePath(char *currDir, std::string path)
{
	int pathlen = path.size();
	int dirlen = strlen(currDir);
	int i, diff=0,pos;
	char relpath[2048]; relpath[0]='\0';

	//Convert to linux paths
	for(i = 0; i < dirlen; i++)
		if(currDir[i]=='\\')
			currDir[i]='/';

	for(i = 0; i < pathlen; i++)
		if(path[i]=='\\')
			path[i]='/';

	//Find common subpath
	int common_len = min(pathlen,dirlen);
	for(i=0; i < common_len; i++)
		if(tolower(path[i]) != tolower(currDir[i]))
			break;
	pos=i;

	for(;i < dirlen; i++)
		if(currDir[i]=='/')diff++;

	for(i=0; i < diff; i++)
		sprintf(relpath+i*3,"../");

	sprintf(relpath+i*3,"%s",path.c_str()+pos);
//	cerr<<"ABSOLUTE PATH:"<<path<<": RELATIVE PATH:"<<relpath<<":"<<endl;
	return relpath;
}
std::string RelativePath(std::string path)
{
	char currDir[2048];
	char* unused = _getcwd(currDir,2048);

	//Add a '/' in the end if not already there
	int l = strlen(currDir)-1;
	if(currDir[l] != '/' && currDir[l] != '\\')
	{
		currDir[++l]='/';
		currDir[++l]='\0';
	}

	return RelativePath(currDir,path);
}
std::string AbsolutePath(std::string path)
{
	//Convert to absolute path incase a relative path is given
	char fullpath[PATH_MAX+1];
#ifdef _WIN32
	GetFullPathName(path.c_str(), 2048, fullpath, 0);
#else
	char *unused = realpath(path.c_str(), fullpath);
#endif
	path = fullpath;
	return path;

/*	char currDir[2048];
	char* unused = _getcwd(currDir,2048);
	return AbsolutePath(currDir,path);
*/
}
std::string AbsolutePath(char *currDir, std::string path)
{
	//If already an absolute path, return that
	//if(path[0]!='.')
		//return path;

	int ups=0,pos,i,pathpos=0,currDirLen=strlen(currDir);
	//Count the number of ups
	for(i = 0; i < (int)path.size(); i+=3)
		if(path[i]!='.' || path[i+1] != '.' || path[i+2] != '/')
			{
				pathpos=i;
				break;
			}
		else
			ups++;

	//Convert to linux paths
	for(i = 0; i < currDirLen; i++)
		if(currDir[i]=='\\')
			currDir[i]='/';


	//Adjust the currDir path accordingly
	for(i = currDirLen-1; i>=0 && ups; i--)
	{
		if(currDir[i]=='/')
			ups--;
	}
	pos=i+1;

	char abspath[2048];
	sprintf(abspath,"%s",currDir);
	abspath[pos]='\0';
	sprintf(abspath,"%s/%s", abspath, path.c_str()+pathpos);
//	cerr<<"RELATIVE PATH:"<<path<<": ABSOLUTE PATH:"<<abspath<<":"<<endl;
	return abspath;
}
std::string FileNameToPath(std::string filename)
{
	int i;
	for(i = (int)filename.size()-1; i >=0; i--)
		if(filename[i]=='/' || filename[i]=='\\')
			break;
	filename[i+1]='\0';
	return filename;
}
std::string PathToFileName(std::string path)
{
	int i;
	for(i = (int)path.size()-1; i >=0; i--)
		if(path[i]=='/' || path[i]=='\\')
			break;
	return path.substr(i+1);
}
