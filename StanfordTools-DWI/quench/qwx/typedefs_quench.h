#ifndef TYPEDEFS_QUENCH_H
#define TYPEDEFS_QUENCH_H

#include <memory>
#include <list>
#include "typedefs.h"

// Defines vtk data types based on 32/64 bit flag.
#include "vtkConfigure.h"

#include <RAPID.H>
#include <wx/wx.h>
class OverlayItem;

const int QUENCH_QST_VERSION_NUMBER = 2;
const int QUENCH_DEFAULT_WIDTH  = 1030;
const int QUENCH_DEFAULT_HEIGHT = 407;
extern wxFont DEFAULT_FONT;
extern wxSize DEFAULT_TEXTBOX_SIZE;
extern wxSize DEFAULT_BUTTON_SIZE;

//! The maximum line width of fibers
const int MAX_LINE_WIDTH = 7;
/*! Name of the temporary file which stores the current Quench state.
	This file is saved every 30 secs
*/
#define TEMP_QST_FILE_NAME "~lastState.qst"
static Colord USER_GESTURE_COLOR(1, 23/255., 23/255.);

void FillBitmapWithColorMap(wxBitmap *bmp, int index);

//! Stores the colors of a particular colormap
struct ColorMap
{
	ColorMap(std::string name):Name(name){}
	std::string Name;
	std::vector<Coloruc> Colors;
};

enum DTISceneActorID 
{
  DTI_ACTOR_SAGITTAL_TOMO = 0,
  DTI_ACTOR_CORONAL_TOMO  = 1,
  DTI_ACTOR_AXIAL_TOMO= 2,
  DTI_ACTOR_ROI = 3,
  DTI_ACTOR_TRACTS = 4,
  DTI_ACTOR_SURFACE = 5,
  DTI_ACTOR_LOCAL = 6,
  DTI_ACTOR_NONE = 7
};

enum QInteractorState 
{
  QIS_SELECT_VIS_PANEL = 994,
  QIS_PICK_SELECT ,
  QIS_PICK_GESTURE ,
  QIS_MOVE_SELECT ,
  QIS_MOVE_GESTURE ,  
  QIS_SELECT_STATE ,
  QIS_MOVE_TOOL_SELECTED,
  QIS_SCALE_TOOL_SELECTED,
  QIS_PREBEGIN_ROI_MESH,
  QIS_BEGIN_ROI_MESH
};

enum ROIMotionType 
{
  ROI_MOTION_FREE = 0,
  ROI_MOTION_SURFACE = 1,
  ROI_MOTION_SYMMETRY = 2
};

enum SelectionMode 
{
  SELECTION_MODE_NEW,
  SELECTION_MODE_ADD,
  SELECTION_MODE_REMOVE,
  SELECTION_MODE_INTERSECT,
  SELECTION_MODE_NUM_CHOICES,
  SELECTION_MODE_NONE
};

enum SelectionTool 
{
  SELECTION_TOOL_SHAPE_MATCH=-1,
  SELECTION_TOOL_TOUCH=0,
  SELECTION_TOOL_SURFACE_INTERSECT,
  SELECTION_TOOL_SCALAR_FILTER,
  SELECTION_TOOL_NUM_CHOICES,
  SELECTION_TOOL_NONE
};

enum ViewConstraint 
{
  VIEW_CONSTRAINT_NONE,
  VIEW_CONSTRAINT_FALSE_POSITIVES,
  VIEW_CONSTRAINT_FALSE_NEGATIVES,
  VIEW_CONSTRAINT_TRUE_POSITIVES,
  VIEW_CONSTRAINT_TRUE_NEGATIVES,
  NUM_VIEW_CONSTRAINTS
};


//! Forward declaration of classes to declare shared pointers
class Button;
class TextureButton;
class ColorButton;
class Overlay;

class OverlayItem;
class SliderPanel;
class wxBitmap;
class Event;
class Command;
/*	
	shared_ptr is not a part of main STL library in linux
	hence we need two different implementation.
	
	shared_ptr are extensively used to automate memory 
	management. shared_ptr of any class has a suffix P
	for pointer.
*/

#ifdef _WIN32
	#include <memory>
	typedef std::shared_ptr<Event>				PEvent;
	typedef std::shared_ptr<TextureButton>		PTextureButton;
	typedef std::shared_ptr<Button>				PButton;
	typedef std::shared_ptr<ColorButton>		PColorButton;
	typedef std::shared_ptr<Overlay>			POverlay;
	typedef std::shared_ptr<OverlayItem>		POverlayItem;
	typedef std::shared_ptr<SliderPanel>		PSliderPanel;
	typedef std::shared_ptr<wxBitmap>			PwxBitmap;
	typedef std::shared_ptr<Command>			PCommand;
#else
	#include <tr1/memory>
	typedef std::tr1::shared_ptr<Event>			PEvent;
	typedef std::tr1::shared_ptr<TextureButton> PTextureButton;
	typedef std::tr1::shared_ptr<Button>		PButton;
	typedef std::tr1::shared_ptr<ColorButton>	PColorButton;
	typedef std::tr1::shared_ptr<Overlay>		POverlay;
	typedef std::tr1::shared_ptr<OverlayItem>	POverlayItem;
	typedef std::tr1::shared_ptr<SliderPanel>	PSliderPanel;
	typedef std::tr1::shared_ptr<wxBitmap>		PwxBitmap;
	typedef std::tr1::shared_ptr<Command>		PCommand;
#endif

#include "typedefs_event.h"

#define VTK_SAFE_DELETE(a) if(a){ (a)->Delete(); a = NULL;} 

//! Quench menu enums
enum
{
	// menu items
	Minimal_Quit = 1,
	Minimal_About,
	Keyboard_Shortcuts,
	Launch_Web,
	Load_State,
	Save_State,
	Load_Pathways,
	Load_Volume,
	Load_StandardVolumes,
	Save_Pathways_PDB,
	Toggle_SagP,
	Toggle_AxialP,
	Toggle_CorP,
	Toggle_Pathways,
	Toggle_ROIs,
	Toggle_ROI_Bounding_Boxes,
	Toggle_ROI_Labels,
	Reset_View,
	Reset_PathwayColors,
	Show_ColorChooser,
	Show_ColorMapChooser,
	Overlay_Panel,
	Show_PerPointColor,
	Cycle_Background0,
	Cycle_Background1,
	Cycle_Background2,
	Cycle_Background3,
	Cycle_Background4,
	Cycle_Background5,
	Cycle_Background6,
	Cycle_Background7,
	Cycle_Background8,
	Cycle_Background9,  // Can have 10 backgrounds no more
	Refine_Selection_Panel,
	Show_ImagePreview,
	Undo_Command,
	Redo_Command,
	Select_All,
	Deselect_All,
	Toggle_Freeze_Assignment,

	Cycle_ROI0,
	Cycle_ROI1,
	Cycle_ROI2,
	Cycle_ROI3,
	Cycle_ROI4,
	Cycle_ROI5,
	Cycle_ROI6,
	Cycle_ROI7,
	Cycle_ROI8,
	Cycle_ROI9,

	Load_ROI_Nifti,
	Save_ROI_Nifti,
	Draw_New_ROI,
	Toggle_ROI_Editing_Mode,
	Increase_Brush_Size,
	Decrease_Brush_Size,
	ROI_Properties,
	Delete_ROI,
};

// Transform points between 3d world space and nifti image space.
void WorldToLocal(double *mat, double *wPos, uint *dim, DTIVoxel &lPos);
void WorldToLocal(double *mat, double *wPos, uint *dim, double *flPos);
void LocalToWorld(double *mat, DTIVoxel &lPos, double *wPos);
void LocalToWorld(double *mat, double *lPos, double *wPos);

// Convert relative path to absolute path, and vice versa.
//std::string RelativePath(char *currDir, std::string path);
//std::string RelativePath(std::string path);
//std::string AbsolutePath(std::string path);
//std::string AbsolutePath(char *currDir, std::string path);
//std::string FileNameToPath(std::string filename);

extern class QuenchFrame *the_global_frame;

wxBitmap CreateBitmapFromMemory(void *data, size_t size, int type);

#endif
