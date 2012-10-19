/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef ROI_MANAGER
#define ROI_MANAGER

#include <vector>
#include <memory>
#include <DTIFilterROI.h>
#include <PDBHelper.h>
#include <list>
#include "DTIPathwaySelection.h"

//! list of ROIs
typedef std::list<PDTIFilterROI> PDTIFilterROIList;

//! This class helps in parsing the ROI filter string
class DTIQueryProcessor;
class ROIFactory;

//! Manages the ROI's
class ROIManager : public PDTIFilterROIList
{
public:
	ROIManager();
	virtual ~ROIManager();
	//! generates a new id for a ROI. ROI id's need not be unique
	int NextID() { return _nextId++; }

	//! Erase a ROI specified in \param name, if present
	void erase(std::string name);
	//! Inserts a ROI to the end of the list
	void push_back(const PDTIFilterROI& elem);
	
	//! \return The ROI which is currently selected.
	/* Only the currently selected ROI can be moved/scaled
	 */
	PROPERTY_READONLY(PDTIFilterROI, _selected, Selected);
	//! Sets the ROI specified by \param name as selected, if present
	void SetSelected(std::string name);

	//! Gets the ROI based on the name. ROI names are assumed to be unique.
	virtual DTIFilterROI *getROIFilter (const char *filterName);

	//! Filter the pathways by the given ROI
	void Filter(DTIPathwaySelection &in_selection, PDBHelper &helper, DTIFilterROI *model);

	//! Filter the pathways based on all the voi's in the list
	void Filter(DTIPathwaySelection &in_selection, PDBHelper &helper);

	//! \returns the current query operation
	PROPERTY_READONLY(DTIQueryOperation, _queryOp, QueryOperation);
	//! Sets the query operation
	void SetQueryOperation(DTIQueryOperation op);

	//! \return the ROI with the name specified by \param name, if present. Otherwise returns null
	DTIFilterROI *Find(std::string name);

	//! \return the ROI with the ID specified by \param id, if present. Otherwise returns null
	DTIFilterROI *Find(int id);

	void Deserialize(ROIFactory *factory, std::istream &s, int version = 0);
	void Serialize  (std::ostream &s, int version = 0);

protected:
	//! \return 'end' if none found
	PDTIFilterROIList::iterator find(std::string name);
	//! \return 'end' if none found
	PDTIFilterROIList::iterator find(int id);

	//! Rebuilds the ROI filter for ROI based matching
	void rebuildFilters();
	int _nextId; /// The next id for a ROI
	DTIQueryProcessor *_queryProc; /// The class that does the query based filtering of ROIs
};

#endif
