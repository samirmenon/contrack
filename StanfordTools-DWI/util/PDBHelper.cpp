#include "PDBHelper.h"
#include "DTIPathwayIO.h"
#include "DTIPathwayDatabase.h"
#include <fstream>
#include "DTIPathDistanceMatrix.h"
#include "DTIPathwayStatisticHeader.h"
#include "DTIPathway.h"
using namespace std;

int  DTIPathwayAssignment::NumAssigned(int id)
{
	int num = 0;
	for(int i = 0; i < (int)size(); i++)
		if( (*this)[i] == id)
			num++;
	return num;
}

void DTIPathwayAssignment :: Deserialize(std::istream &s, int version)
{
	int n;
	clear();
	s>>"--- Pathway Assignment ---">>endl;
	s>>"Locked: ">>_locked>>endl;
	s>>"Selected Group: ">>_selected_group>>endl;
	s>>"Num Assigned: ">>n>>endl;
	for(int i = 0; i < n; i++)
	{
		DTIPathwayGroupID val;
		s>> val >>" ";
		push_back(val);
	}
	s>>endl;
}
void DTIPathwayAssignment :: Serialize  (std::ostream &s, int version)
{
	s<<"--- Pathway Assignment ---"<<endl;
	s<<"Locked: "<<_locked<<endl;
	s<<"Selected Group: "<<_selected_group<<endl;
	s<<"Num Assigned: "<<size()<<endl;
	for(int i = 0; i < (int)size(); i++)
		s<< (*this)[i] <<" ";
	s<<endl;
}

/*! This function actually calls the other Add function to do the processing
It simply loads the file from disk into a string stream and passes it 2nd 
Add function
*/
DTIPathwayGroupID PDBHelper::Add(std::string strFilename, bool bAppend)
{
	//Load the stream into memory
	ifstream inputStream(strFilename.c_str(), ios::binary);
	if (inputStream.bad() || inputStream.fail() || inputStream.is_open()) false;

	string strName = string(strFilename)+string(".dis");
	ifstream distanceStream(strName.c_str(), ios::binary);
  	istringstream *sinputStream=FileToString(inputStream), *sdistanceStream=FileToString(distanceStream);

	//Assign the name
	//string fiberName(strFilename);
	//fiberName = fiberName.substr(fiberName.find_last_of('\\')+1);

	DTIPathwayGroupID  bRet = Add(*sinputStream, *sdistanceStream,bAppend,false);
	if(bRet != -1)
		_pdbPaths.push_back(strFilename);

	
	//Release handles
	delete sinputStream;
	delete sdistanceStream;
	if (distanceStream)
		distanceStream.close();
	//else
	//	cerr << "Warning: No distance matrix file found - grow/shrink selection disabled." << endl;
	inputStream.close();
	
	return bRet;
}

/*! Does the actual loading of pathways. 
	This function can either append the fibers or overwrite them.
	In the latter case, it deletes any temporary fibers that might have been 
	saved on disk. Temporary fibers are created when they are pushed from Matlab
	to quench. These are saved to disk for later use 

	It also updates the pathway assignment by assigning the newly passed fiber group
	to the first available empty slot in the list of assignments.

	\param bCreatePDBFile A boolean variable which is true if temporary fibers need 
	to be saved on to disk
*/
DTIPathwayGroupID PDBHelper::Add(std::istringstream &fiberStream, std::istringstream &distanceStream, bool bAppend, bool bCreatePDBFile)
{
	// save the number of pathways, 0 if we are not appending
	int oldNumPathways = bAppend ?(_pdb ? _pdb->getNumFibers() : 0) : 0;
	
	//Delete all temporary files if not appending
	if(!bAppend)
		DeleteTempPDBFiles();

	int oldNumFibers = _pdb?_pdb->getNumFibers():0;
	LoadAndReplaceDistanceMatrix (distanceStream);

	// load and append the new pdb 
	DTIPathwayDatabase *db = DTIPathwayIO::loadAndAppendDatabasePDB(fiberStream, bAppend?_pdb.get():0);
	if(_pdb.get() != db)
		_pdb.reset(db);

	// resize assignment to make space for the newly loaded pdb
	_assn.resize(_pdb->getNumFibers(), DTI_COLOR_UNASSIGNED);
	int assignedGroup=1;
	
	// find out the the first unsed fiber group slot
	if(oldNumPathways && bAppend)
	{
		bool bIsUsed[1000];
		memset(bIsUsed,0,1000*sizeof(bool));
		for(int i = 0; i < oldNumPathways; i++)
			bIsUsed[_assn[i]]=true;

		for(int i = 1; i < 1000; i++)
			if(! bIsUsed[i])
			{
				assignedGroup = i;
				break;
			}
	}

	// assign the newly loaded fibers to this group
	for(int i = oldNumPathways; i < (int)_assn.size(); i++)
		_assn[i] = assignedGroup;

	//Create the temporary pdb files. These fibers sent from matlab
	if(bCreatePDBFile)
	{
		//Find a unique file name
		char filename[100]; 
		char currDir[2048];
		_getcwd(currDir,2048);
		for(int j=0; ; j++)
		{
			sprintf(filename,"%s/%s%2d.pdb",currDir,TEMP_PDB_FILE_PREFIX,j);
			if(_access(filename,00)==-1)
				break;
		}

		DTIPathwaySelection *selection = new DTIPathwaySelection (_pdb->getNumFibers(), false);
		for(int j = oldNumPathways; j < (int)_assn.size(); j++)
		(*selection)[j]=true;;
		
		double ACPC[3] = {0.,0.,0.};
		DTIPathwayIO::saveDatabaseAny (_pdb.get(), true, filename, ACPC, selection);
		_pdbPaths.push_back(filename);
		delete selection;
	}

	// save the assignment
	_assn.SetSelectedGroup(assignedGroup);
	_oldAssn = _assn;
	return assignedGroup;
}
void PDBHelper::MoveToTrash(DTIPathwayGroupID id)
{
	for(int i = 0; i < (int)_assn.size(); i++)
		if(_assn[i] == id)
			_assn[i] = DTI_COLOR_UNASSIGNED;
}
void PDBHelper::Save(std::string path, DTIPathwaySelection &selection)
{
	double ACPC[]={0,0,0};
	DTIPathwayIO::saveDatabaseAny(_pdb.get(), true, path, ACPC, &selection);
}

void PDBHelper::Filter(DTIPathwaySelection &in_selection)
{
	// cache the validity of each statistics
	std::vector<bool> isValid;
	for(int i = 0 ; i < _pdb->getNumPathStatistics(); i++)
		isValid.push_back(_pdb->getPathStatisticHeader(i)->isValid());

	// for each fiber
	for(int j=0;j<_pdb->getNumFibers();j++) 
	{
		DTIPathway* pathway = _pdb->getPathway(j);
		bool bFailedThresh = false;

		// if its in the selection, see if its passes the statistics range
		if( in_selection[j] )
		{
			// for each statistic
			for(int k = 0; k < _pdb->getNumPathStatistics(); k++)
			{
				// is the statistic valid
				if(isValid[k])
				{
					int id= k;
					float dQ = (float)pathway->getPathStatistic(id);
					
					// is pathway within the range of the Kth statistic?
					if(dQ < _pdb->getPathStatistic(k)->_left || dQ > _pdb->getPathStatistic(k)->_right)// || (_vROIs.size()!=0 && !_QueryProc->doesPathwayMatch(pathway)))
					{
						// no, then remove this fiber from the selection
						bFailedThresh = true;
						break;
					}
				}
			}
		}
		if(bFailedThresh)
		{
			if(_assn[j] == _assn.SelectedGroup())
				_assn[j] = 0;
			in_selection[j] = false;
		}
	}
}

void PDBHelper::LoadAndReplaceDistanceMatrix(istream &matrixStream)
{
	if (matrixStream) 
	{
		cerr << "Loading distance matrix from file..." << endl;
		_distanceMatrix .reset(DTIPathDistanceMatrix::loadDistanceMatrix (matrixStream));
	}
}
void PDBHelper::AcceptFilterOperation(bool accept)
{
	if(accept)
		_oldAssn.Copy(_assn);
	else
		_assn.Copy(_oldAssn);
}
void PDBHelper::DeleteTempPDBFiles()
{
	// delete all temporary pdb files that might have been created
	for(int i = 0; i < (int)_pdbPaths.size(); i++)
		if(_pdbPaths[i].find(TEMP_PDB_FILE_PREFIX ) != string::npos)
			remove(_pdbPaths[i].c_str());
	_pdbPaths.clear();
}
void PDBHelper::SetAssignment(DTIPathwayAssignment & assn)
{
	_oldAssn = assn;
	_assn	 = assn;
}
void PDBHelper::Deserialize(std::istream &s, int version)
{
	_pdbPaths.clear();
	s>>"--- PDB Info ---">>endl;
	int n;
	char path[2048];
	s>>"Num PDB's: ">>n>>endl;
	for(int i = 0; i < n; i++)
	{
		s.getline(path,2048);
		string abs_path = AbsolutePath(path);
		_pdbPaths.push_back(abs_path);
		Add(abs_path,i!=0);
	}
	_assn.Deserialize(s,version);
	_oldAssn = _assn;
	s>>endl;
}
void PDBHelper::Serialize(std::ostream &s, int version)
{
	s<<"--- PDB Info ---"<<endl;
	s<<"Num PDB's: "<<_pdbPaths.size()<<endl;
	for(int i = 0; i < (int)_pdbPaths.size(); i++)
		s<<RelativePath(_pdbPaths[i])<<endl;
	_assn.Serialize(s,version);
	s<<endl;
}
