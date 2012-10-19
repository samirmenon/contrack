#ifndef VISIBILITY_PANEL_H
#define VISIBILITY_PANEL_H

class vtkActor2D;
class vtkTextActor;
class vtkRenderer;
class PathwayGroupArray;
class vtkTextActor;
class DTIPathwayAssignment;
class vtkTexturedActor2D;
class qPathwayViz;
class SwatchEventListener;
class IconWidget;
class vtkActor;
class vtkTransform;
class qROIEditor;

#include "typedefs_quench.h"
#include "typedefs.h"
#include "vtkActor2D.h"
#include "vtkTextActor.h"
#include "Button.h"

extern int SWATCHES_PER_ROW;

//! Gets the swatch under the current mouse(x,y) position. \return Swatch index if found otherwise -1
int SwatchUnderCursor(int x, int y, int xmin, int ymin, int numSwatches);

//! Get the extent of swatches 
void GetExtents(int xmin, int ymin, int &xmax, int &ymax, int numSwatches);

//! Array of fibergroups which are displayed on the screen
/*! The user can select and toggle the visibility of fiber groups by clicking on the
	buttons in the fiber group
*/
class VisibilityPanel : public IEventSource
{
public:
	VisibilityPanel(vtkRenderer *renderer);
	virtual ~VisibilityPanel();

	//! \return the fiber group that was under the mouse position. -1 if none
	DTIPathwayGroupID OnLeftDown(int x, int y);
	//! Brings up the stats panel if a fiber group is clicked.
	bool OnRightButtonUp(int x, int y);

	//! Update the panel based on the fiber group array 
	void Update(PathwayGroupArray &array, DTIPathwayAssignment &assignment);
	//! Sets the visibility of the text panel which shows information about the currently selected fiber group
	void SetTextInfoVisibility(bool b);
	
	//! Sets the visibility of the panel
	PROPERTY_DECLARE(bool, _visible, Visible);

	//! Makes the visibility panel transparent/opaque for per point coloring mode
	PROPERTY_DECLARE(bool, _transparent, Transparent);

private:
	//! Creates a text label at a specified 2d point
	vtkTextActor *CreateTextActor(int x , int y);

	std::vector<PButton> _swatches;								/// Array of buttons corresponding to fiber groups
	TextureButton *_delete_group_button, *_add_group_button;	/// The add and remove button for the fiber groups
	int _xmax, _ymax, _xmin, _ymin;								/// The extent of the visibility panel

	// Text Info Stuff
	vtkTextActor* _taSelectedGroup;								/// vtk actor displaying the info about the selected fiber group
	vtkTextActor* _taLockedGroup;								/// specifies the label whether assignments are locked
	vtkTextActor* _taNameGroup;									/// text label displaying the name of the selected fiber group
	vtkTextActor* _voiEditingGroup;

	ColorButton*  _swCurrentAssign;								/// button showing the color of the selected fiber group
	vtkRenderer* _renderer;										/// pointer to the global vtk renderer
};


#endif
