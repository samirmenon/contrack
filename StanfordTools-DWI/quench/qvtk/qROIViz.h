#ifndef QROI_VIZ_H
#define QROI_VIZ_H

#include "DTIFilterROI.h"
#include "ROIManager.h"
#include "typedefs_quench.h"

class vtkRenderer;
class qMoveTool;
class qScaleTool;

//! This class contains the list of 3d vtk based ROI. 
/*! This class is used for selecting a ROI and translating/scaling
	the selected ROI using the move and scale tool
*/
class qROIViz : public IEventListener, public IEventSource,  public ISerializable
{
public:
	qROIViz(vtkRenderer *renderer);
	~qROIViz();
	//! Process the mouse down event
	bool OnLeftDown(int x, int y);
	//! Process a click (mouse down and up, without moving)
	bool OnClick(int x, int y);
	//! Process the mouse up event
	bool OnLeftUp(int x, int y);
	//! Process the right mouse up event
	bool OnRightButton(int x, int y);
	//! Process the mouse move event
	bool OnMouseMove(int dx, int dy, int state);
	//! Process the key down event, shows move tool when E is pressed, scale tool when R is pressed
	bool OnKeyDown(char key);
	//! Updates the scene based on the contents of ROI manager
	void Update(ROIManager &mgr);
	//! Scales the move/scale tool on zoom in/out
	void Dolly(double scale);
	void OnEvent(PEvent evt);

	//! Gets/sets the ROI manager
	PROPERTY(ROIManager*, _voiMgr, ROIManager_);
	//! Sets the bounds for the current ROI
	void SetROIBounds(Vector3d min, Vector3d max) ;

	//--- ISerializable interface ---//
	void Deserialize(std::istream &s, int version = 0);
	void Serialize  (std::ostream &s, int version = 0);
protected:
	vtkRenderer *_renderer;			/// pointer to the global vtk renderer
	qMoveTool *_move_tool;			/// pointer to the move tool to move the selected voi.
	qScaleTool *_scale_tool;		/// pointer to the scale tool to scale the selected voi
	Vector3d _pos_min, _pos_max;	/// The min and max position bounds for the currently selected voi
	Vector3d _scale_max;			/// The max scale for the currently selected voi
	Vector3d _min, _max;			/// The min and max bounds for the background image
	bool is_move_tool_selected;		/// Either the move or the scale tool is selected
	void show_move_tool(bool b);
	vtkPropPicker *_propPicker;					/// Helper class to pick a ROIs
	vtkPropCollection *_propCollection;			/// Helper class which stores a collection of voi
};

#endif
