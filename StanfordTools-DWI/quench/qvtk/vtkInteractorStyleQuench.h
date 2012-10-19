/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef __vtkInteractorStyleQuench_h
#define __vtkInteractorStyleQuench_h

#include "vtkInteractorStyle.h"
#include "typedefs_quench.h"
#include "typedefs.h"
#include <string>
#include <wx/timer.h>

class vtkCellLocator;
class ModeSelector;
class VisibilityPanel;
class qGestureInteractor;
class qROIEditor;
class vtkPropPicker;
class SceneWindow;
class qVolumeViz;
class qPathwayViz;
class qROIViz;
class MyFrame;

enum QuenchInteractionMode
  {
    INTERACTION_IDLE,
    INTERACTION_DRAW_GESTURE,
    INTERACTION_ROI_MANIPULATE,
    INTERACTION_PATHWAYS,
    INTERACTION_ROI_EDIT,
  };

//! Main class to handle interaction with the vtk renderer
class vtkInteractorStyleQuench : public vtkInteractorStyle, public IEventSource, public wxTimer
{
public:
	friend class MyFrame;
	static vtkInteractorStyleQuench *New();
	vtkTypeRevisionMacro(vtkInteractorStyleQuench,vtkInteractorStyle);
	void PrintSelf(ostream& os, vtkIndent indent);
	void SetFrame (MyFrame *frameParam) {this->frame = frameParam; }

	void Init(SceneWindow *_pSceneWindow);
	// Description:
	// Event bindings controlling the effects of pressing mouse buttons
	// or moving the mouse.
	virtual void OnKeyPress();
	virtual void OnMouseMove();
	virtual void OnLeftButtonDown();
	virtual void OnLeftButtonUp();
	virtual void OnMiddleButtonDown();
	virtual void OnMiddleButtonUp() { this->OnLeftButtonUp(); }
	virtual void OnRightButtonDown();
	virtual void OnRightButtonUp()  ;
	virtual void OnMouseWheelForward();
	virtual void OnMouseWheelBackward();
	// These methods for the different interactions in different modes
	// are overridden in subclasses to perform the correct motion. Since
	// they are called by OnTimer, they do not have mouse coord parameters
	// (use interactor's GetEventPosition and GetLastEventPosition)
	virtual void Rotate();
	virtual void Pan();
	virtual void Dolly();

	//! This function is called to handle the timer fired after a right down.
	void Notify();
	
	void ResetView();
	void SaveScreenImage (const char *filename, bool pristine = false);

	void ToggleImages(DTISceneActorID imageId, int ShiftKeyDown);
	// Gestures
	void StartSelect();
	void EndSelect();
 
	//--- ISerializable interface ---//
	void Deserialize(std::istream &s, int version = 0);
	void Serialize  (std::ostream &s, int version = 0);


	PROPERTY_READONLY(vtkRenderer*, _renderer, Renderer);
	PROPERTY_READONLY(qVolumeViz*, _volViz, VolumeViz);
	PROPERTY_READONLY(qPathwayViz*, _pathwayViz, PathwayViz);
	PROPERTY_READONLY(qROIViz*, _voiViz, ROIViz);
	PROPERTY_READONLY(qGestureInteractor*, _GestureInteractor, GestureInteractor);
	PROPERTY_READONLY(qROIEditor*, _voiEditor, voiEditor);
	
protected:
	vtkInteractorStyleQuench();
	~vtkInteractorStyleQuench();
	//int PickCell (vtkActor *actor, vtkCellLocator *locator, int mouseX, int mouseY, double pickedPoint[3]){return 0;}
	MyFrame *frame;
	virtual void Dolly(double factor);
	vtkSetMacro(State,int);

	void ToggleROIEditingMode();
	void HandDrawROI();

private:
	vtkPropPicker* _propPicker ;
	vtkInteractorStyleQuench(const vtkInteractorStyleQuench&);  // Not implemented.
	long _old_cursor_x;
	long _old_cursor_y;
	int _mousedown_x;
	int _mousedown_y;
	int _saved_mouse_x;
	int _saved_mouse_y;
	bool _mouse_moved;
	int _left_button_down;
	int _right_button_down;
	double MotionFactor;

	QuenchInteractionMode _interaction_mode;

	// UI Viz
	std::string _strIconsPath;
};

#endif
