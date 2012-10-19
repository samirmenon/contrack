#ifndef Q_MOVE_TOOL
#define Q_MOVE_TOOL

#include "typedefs_quench.h"
class vtkActor;
class vtkRenderer;
class vtkFollower;
class vtkPropPicker;
class vtkPropCollection;

enum MoveScaleSelectionMode
{
	PLANE,
	ARROW,
	MAIN_PLANE
};

//! A multi axis manipulation tool
/*! This tool is inspired by 3ds Max's translate and scale tool. 
	Those tools consist of 3 axis and plane along each axis. 
	Dragging a axis performs a operation (like translation, scaling) along that axis
	Dragging a plane performs a operation along the 2 axis defining the plane.

	This tool can have arbitrary # of axis
*/
class qManipulationTool : public ISerializable
{
public:
	qManipulationTool(vtkRenderer *renderer, int numAxis);
	virtual ~qManipulationTool();
	//Result of dragging
	virtual void OnDrag(int dx, int dy, Vector3d &out) { }
	//! Mouse left button is pressed
	virtual bool OnLeftDown(int x, int y);
	//! Mouse left button is pressed
	virtual bool OnLeftUp(int x, int y);

	//! Gets the visibility of the tool
	PROPERTY_DECLARE(bool, _visible, Visible);
	//! Gets the scale of the tool
	PROPERTY_DECLARE(Vector3d, _scale, Scale);
	//! Gets the position of the tool
	PROPERTY_DECLARE(Vector3d, _position, Position);
	//! Gets the number of axis in this tool
	PROPERTY_READONLY(int, _nAxis, NumAxis);
	//! gets/sets the selected prop
	PROPERTY_READONLY(vtkFollower*, _selected_prop, SelectedProp);
	MoveScaleSelectionMode _selection;

	//--- ISerializable interface ---//
	void Deserialize(std::istream &s, int version = 0);
	void Serialize  (std::ostream &s, int version = 0);
protected:
	vtkRenderer *_renderer;						/// pointer to the global vtk renderer
	vtkFollower **_axis_arrows, **_axis_planes;	/// Array of vtk actors for the arrows and planes
	vtkPropPicker *_propPicker;					/// Helper class to pick a 3d point
	vtkPropCollection *_propCollection;			/// Helper class which stores a collection of props like planes and axes
};

//! Specialization of the manipulation tool to deal with translation
class qMoveTool : public qManipulationTool
{
public:
	qMoveTool(vtkRenderer *renderer);
	//Result of dragging
	void OnDrag(int dx, int dy, Vector3d &out);
};

//! Specialization of the manipulation tool to deal with scaling
/*! This tool has 4 axis, the 4th axis is used for uniform scaling 
*/
class qScaleTool : public qManipulationTool
{
public:
	qScaleTool(vtkRenderer *renderer);
	//Result of dragging
	void OnDrag(int dx, int dy, Vector3d &out);
};

#endif

