/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef Q_PATHWAYVIZ_H
#define Q_PATHWAYVIZ_H

#include "typedefs_quench.h"
#include "SubjectData.h"
#include "PDBHelper.h"
#include "PathwayGroup.h"
#include <map>

class PathwayGroupArray;
class ModeSelector;
class vtkRenderer;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkPropCollection;
class vtkActor;
class DTIPathwayDatabase ;
class DTIPathway;
class VisibilityPanel;
class vtkPropPicker;
class vtkPropCollection;
class ColorMapPanel;

//! This class deals in storing and coloring fibers in 3d vtk format
class qPathwayViz : public IEventListener, public IEventSource, public ISerializable
{
public:
	qPathwayViz(vtkRenderer* renderer);
	~qPathwayViz();

	//!  Generate the 3d pathways in vtk based format from the input pdb
	void GeneratePathways(PDBHelper& helper, PathwayGroupArray &groupArray);
	
	//!  Update color of pathways
	void UpdatePathwaysColor(PDBHelper& helper, PathwayGroupArray &groupArray);
	
	//!  Update visibility of pathways
	void UpdatePathwaysVisibility(PDBHelper& helper, PathwayGroupArray &groupArray);

	//! Start per point coloring mode
	void BeginPerPointColoringMode(PDBHelper& helper, PathwayGroupArray &groupArray, EventShowPerPointColorPanel &event_info);
	
	//! End per point coloring mode
	void EndPerPointColoringMode();

	//! Passes control to the mode selector panel to see if they were clicked
	bool OnLeftDown(int x, int y);

	//! Brings up the stats panel if a fiber group is clicked.
	bool OnRightButtonUp(int x, int y);

	//! Tries to move the mode selector panel
	bool OnMouseMove(int x, int y);

	//!	 Sets the image bound used for surface selection mode
	void SetActiveImageExtents(double pts[4][3], double normal[4]);
	
	//!  Set the geometry for surface selection mode
	void SetIntersectionGeometry(PDBHelper& helper, PathwayGroupArray &groupArray);

	//! Changes the line width from 1-8 by the specified amount.
	void IncreaseLineWidth(int amount);

	//!  resets the color cache forcing color update
	void InvalidateColorCache() { _is_color_cached = false; }

	//! Returns the pointer to the visibility panel
	PROPERTY_READONLY(VisibilityPanel* , _vis_panel, VisibilityPanel_);
	//! \return the current filtering operation
	PROPERTY_READONLY(BOOLEAN_OPERATION, _filterOperation, FilterOperation);
	//! \return the current mode: Touch or surface
	int  Mode();
	//! get/set pathway actor's visibility
	PROPERTY_DECLARE(bool, _pathwayVisibility, PathwayVisibility);
	//! get/set point actor's visibility
	PROPERTY_DECLARE(bool, _pointsVisibility, PointsVisibility);
	PROPERTY(ColorMapPanel*, _cmapPanel, ColorMapPanel_);

	static const int TOUCH_MODE   = 0;
	static const int SURFACE_MODE = 1;

	//--- ISerializable interface ---//
	void Deserialize(std::istream &s, int version = 0);
	void Serialize  (std::ostream &s, int version = 0);

	//! Take a screenshot
	void TakeScreenshot(void **pixels, int &width, int &height, int quality);

protected:

	PathwayGroupArray _cached_group_array;
	DTIPathwayAssignment _cached_assn;
	EventShowPerPointColorPanel _per_point_info;
	bool _do_per_point_color;

	bool _is_color_cached;
	bool  _panel_selected;
	int prev_x, prev_y;
	vtkRenderer* _renderer;
	void LoadAndReplaceDistanceMatrix(std::istream &matrixStream);
	void ComputePerPointColor(DTIPathway *pathway, int idx, int stat_idx, float stat_min, float inv_range, ColorMap &c, unsigned char rgb[3]);
	void JitteredPathwayColor(DTIPathway* t, Colord &col, unsigned char rgb[3]);
	void OnEvent(PEvent e);

	static inline double normalizeValue (double value, double minValue, double maxValue)
	{
		if (value < minValue) return 0;
		if (value > maxValue) return 1;
		return (value-minValue)/(maxValue-minValue);
	}

	// UI
	ModeSelector *_msGesture;
	ModeSelector *_msSelection;

	// PDB
	int _lineWidthSetting;
	vtkPolyData* _pdPathways;
	vtkPolyDataMapper* _mPathways;
	vtkActor* _aPathways;
	DTILuminanceEncoding _LumEncoding;
	std::map<DTIPathway*,double> _mapPathwayLuminance;

	vtkPolyData* _pdIntersections;
	vtkPolyDataMapper* _mIntersections;
	vtkActor* _aIntersections;
	double _planePts[4][3], _planeNormal[4];
	vtkPropPicker *_propPicker;					/// Helper class to pick a ROIs
	vtkPropCollection *_propCollection;			/// Helper class which stores a collection of voi
};

#endif
