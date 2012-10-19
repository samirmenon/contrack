/** Copyright (c) 2010, The Board of Trustees of Leland Stanford Junior University. 
All rights reserved. **/

// Purpose ...

#ifndef QUENCHCONTROLLER_H
#define QUENCHCONTROLLER_H

#include "typedefs_quench.h"
#include "SubjectData.h"
#include "PDBHelper.h"
#include "ROIManager.h"

class VolumeInfo;
class DTIPathwayAssignment;
class PathwayGroupArray;
class SubjectData;
class PDBHelper;
class ROIManager;
class CommandManager;
class Mesh;
class QuenchFrame;
class vtkROI;

//! The main window frame
class QuenchController : public IEventListener
{
public:
    // ctor(s)
	// Creates a dataModel, GUI elements, connects them and also creates self
    QuenchController();
    ~QuenchController();
    
    	///---------- Data -------------///
	PROPERTY_READONLY(SubjectData, _subjectData, SubjectData_);
	PROPERTY_READONLY(PathwayGroupArray, _groupArray, PathwayGroupArray_);
	PROPERTY_READONLY(PDBHelper, _pdbHelper, PDBHelper_);
	PROPERTY_READONLY(ROIManager, _voiMgr, ROIManager_);
	PROPERTY_READONLY(CommandManager*, _commandMgr, CommandManager_);
	PROPERTY(bool, _bDestroyed, IsDestroyed);
    
    //! Connects all controllers to event sources
    void ConnectControllers(QuenchFrame* qFrame);
    
    //  Control slightly different options for the user depending on whether they are in 1 of 3 
    //  states 1) nothing loaded, 2) only volume data loaded, 3) pathways and volume loaded
	static const int NOTHING_LOADED  = 0x0;
	static const int VOLUME_LOADED   = 0x1;
	static const int PATHWAYS_LOADED = 0x2;
	int _state_data_loaded;
	void SetDataLoadedState(int state, bool bInit=false);
	
	//! handles events
	void OnEvent(PEvent e);
	
	//! Updates the vizualization GUI elements associated with the pathways
	void UpdatePathwayViz(bool bVisibility);
	void UpdateMesh();
	void UpdateBackground(){this->SetBackgroundVolume(this->_currBkImage);}
	
	void SelectAllPathways();
	void DeselectAllPathways();
	void ToggleAssignmentLock();	
	//! Sets the current background image
	void SetBackgroundVolume(int i);
	//! Returns the current background image	
	int BackgroundVolume() { return _currBkImage; }
	//! loads a volume specified in the path \param path
	void LoadVolume(std::string path);
	//! loads an ROI encoded in NIFTI format
	void LoadROINifti(std::string path, std::string filename);
	//! saves the current ROI in NIFTI format
	void SaveCurrentROINifti(std::string path, std::string filename);
	//! loads default volumes
	void LoadDefaultVolumes(std::string path);
	//! Add volume to volumeViz
	void AddVolumeToDisplay(VolumeInfo &vol_info);
	//! Add volume to overlay panel and menu 
	void AddVolumeToPanel(VolumeInfo &vol_info);
	//! loads all pathways specified in the list \param pathNames
	void LoadPathways(wxArrayString pathNames, bool bAppend=true);
	//! loads a single pathway
	void LoadPathway(std::string path, bool bAppend=true);
	//! loads a mesh 
	void LoadMesh(std::string path);
	//! Add per point statistics to the panel
	void AddStatsToPerPointPanel();
	//! Swaps the current assignment, used for history management
	void SwapAssignment(DTIPathwayAssignment &assn);
	//! Swaps the current assignment, used for history management
	void SwapGroupArray(PathwayGroupArray &pgarray);
	void PushToHistory(PCommand cmd);
	
	//! Add the specified ROI to the program.
	void AddNewROI(vtkROI *vInput);
	//! Replace the given ROI with a new one.
	void ReplaceROI(int oldID, vtkROI *vInput);

	//! Respond to edit event for a ROI
	void ROIEdited();
	void DrawNewROI();
	void ROIEditModeOn();
	void ROIEditModeOff();
	void OnROIProperties();
	void DeleteCurrentROI();

	void IncreaseBrushSize();
	void DecreaseBrushSize();
	
	void SetROIVisibility(bool vis);
	void SetROIBoundingBoxVisibility(bool vis);
	void SetROILabelVisibility(bool vis);

	void FilterPathwaysByStats();
	void UpdateSurfaceModeGeometry();
	void LoadState(std::string filename);
	void SaveState(std::string filename, bool saveROIs = true);
	
protected:
	void SaveROINifti(PDTIFilterROI voi, std::string path);
	void SaveAllROIs();
	void UpdateROIBoundingBoxDisplay();

	bool _voi_bounding_box_visibility; 
	bool _voi_label_visibility;
	bool _voi_visibility;
	Mesh* _mesh;
	//! Index of the current background image
	int _currBkImage;
	QuenchFrame* _qFrame;
};

#endif
