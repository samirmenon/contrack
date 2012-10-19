#ifndef PDB_HELPER
#define PDB_HELPER

#include "typedefs.h"
#include "PathwayGroup.h"
#include "DTIPathwaySelection.h"

/*! Prefix for the temporary pdb files generated when fibers are 
transfered from Matlab to Quench
*/
#define TEMP_PDB_FILE_PREFIX "~tmp@#$"

class DTIPathwayDatabase;
class DTIPathDistanceMatrix;

//! Array of pathway group ID's
typedef std::vector<DTIPathwayGroupID> DTIPathwayGroupIDArray;

//! Stores the mapping of fibers to fiber groups.
class DTIPathwayAssignment : public DTIPathwayGroupIDArray, public ISerializable
{
public:
	DTIPathwayAssignment() { _locked = true; _selected_group = DTI_COLOR_UNASSIGNED; }
	int NumAssigned(int id);
	PROPERTY(bool, _locked, Locked);
	PROPERTY(int, _selected_group, SelectedGroup);
	void Copy(DTIPathwayAssignment &source){ *(DTIPathwayGroupIDArray*)this = *(DTIPathwayGroupIDArray*)&source; }

	/*! Assign the selected fibers in \param selection to the group specified by \param group 
		if \param bUnassignUnselected is true, all unselected fibers are assigned to trash group.
	*/
	void AssignSelectedToGroup(DTIPathwaySelection &selection, DTIPathwayGroupID group, bool bUnassignUnselected = true)
	{
		for(int i = 0; i < (int)selection.size(); i++)
			if(selection[i])
				(*this)[i] = group;
			else if( bUnassignUnselected && (*this)[i] == group)
				(*this)[i] = DTI_COLOR_UNASSIGNED;
	}

	//--- ISerializable interface ---//
	void Deserialize(std::istream &s, int version = 0);
	void Serialize  (std::ostream &s, int version = 0);
};

//! This class acts as a wrapper around DTIPathwayDatabase.
/*! In future this class should be merged with DTIPathwayDatabase
 */
class PDBHelper : public ISerializable
{
public:
	//! The pdb class which store the .pdb file's data
	PROPERTY_READONLY(PDTIPathwayDatabase, _pdb, PDB);
	//! Assignment of the current fibers to fiber groups
	PROPERTY_READONLY(DTIPathwayAssignment, _assn, Assignment);
	//! Assignment of the current fibers to fiber groups
	PROPERTY_READONLY(DTIPathwayAssignment, _oldAssn, InteractiveAssignment);
	//! Not used
	PROPERTY_READONLY(PDTIPathDistanceMatrix, _distanceMatrix, DistanceMatrix);

	//! Add pathways to PDB. \return GroupID to which fibers were added. -1 on failure
	DTIPathwayGroupID Add(std::string path, bool bAppend=true);

	//! Does the actual job of adding pathway. \return GroupID to which fibers were added. -1 on failure
	DTIPathwayGroupID Add(std::istringstream &fiberStream, std::istringstream &distanceStream, bool bAppend=true, bool bCreatePDBFile=true);
    
	//! Moves the fiber groups with id = \param id to trash group
	void MoveToTrash(DTIPathwayGroupID id);

	//! Save the entire pdb to disk. To save as a state file use the serialize method
	void Save(std::string path, DTIPathwaySelection &selection);

	//! Clears all data as if it was new.
	void Clear() { _pdb.reset(); DeleteTempPDBFiles(); }

	//! Filter the pathways based on statistics. 
	/*! \param in_selection acts as both input and output variable.
		Initially contains information specifying which fibers should be considered
		for selection. After the filtering operation, only the fibers
		that pass the test remain selected in \param in_selection.
	 */
	void Filter(DTIPathwaySelection &in_selection);

	void LoadAndReplaceDistanceMatrix(std::istream &matrixStream);

	//! Commit the changes made using 'Refine Selection Panel'
	void AcceptFilterOperation(bool accept);

	//! Sets the new assignment
	void SetAssignment(DTIPathwayAssignment & assn);
	//--- ISerializable interface ---//
	void Deserialize(std::istream &s, int version = 0);
	void Serialize  (std::ostream &s, int version = 0);
protected:
	void DeleteTempPDBFiles();
	std::vector<std::string> _pdbPaths; ///Array holding the paths of various pdb's loaded.
};

#endif //PDB_HELPER
