/** Copyright (c) 2010, The Board of Trustees of Leland Stanford Junior University. 
All rights reserved. **/

// Purpose ...

#ifndef QUENCHFRAME_H
#define QUENCHFRAME_H

#include "typedefs_quench.h"

class QuenchController;
class vtkInteractorStyleQuench;
class SceneWindow;
class ColorMapPanel;
class OverlayPanel;
class StatsPanel;
class ImagePreview;
class PerPointColor;

//! The main window frame
class QuenchFrame : public wxFrame
{
	//friend class QuenchController;
public:
    // ctor(s)
	//Creates a dataModel, GUI elements, connects them and also creates self
    QuenchFrame(QuenchController* qCtrlr, const wxString& title, const wxPoint& pos, const wxSize& size);
    ~QuenchFrame();
    
    //! Need to show menus depending on what data has been loaded
    void SetDataLoadedMenuState(int state);
    void SetRefineSelectionMenuState(bool bIsVisible) {_menu_panels->Check(Refine_Selection_Panel, bIsVisible);}
    void SetUndoRedoMenuState(bool bUndo, bool bRedo) {_menu_edit->Enable(Undo_Command, bUndo); _menu_edit->Enable(Redo_Command, bRedo);}
    void SetFreezeAssignmentMenuState(bool bIsLocked) {_menu_edit->Check(Toggle_Freeze_Assignment, bIsLocked);}
    void SetROISelectedMenuState(bool roiSelected);
    void SetROIEditingMenuState(bool roiEditing);

    void AddBackgroundToMenu(const char* str_bg);
    void StartTimer();
    void ClearForLoadState();
    
    //! Redraws the screen 
    void RefreshViz();
	//! Toggles the slicer visibility
	void ToggleImagePlane(DTISceneActorID id, bool bVisibility);
    //! Run the singleton file dialog to select file
    bool DoFileDialog(const char *caption, const char *file_type, int flags);
    //! We need to save the state often so this is function is available here
    bool DoSaveStateDialog();
	//! Check visibility of planes and update menu state
	void UpdateImageVisibilityMenuState();
	void UpdatePathwayVisibilityMenuState();

	///------ Panels -----///
	PROPERTY_READONLY(vtkInteractorStyleQuench*, _istyle, Interactor);
	PROPERTY_READONLY(SceneWindow*, _pSceneWindow, MainWindow);
	PROPERTY_READONLY(ColorMapPanel*, _color_map_panel, ColorMapPanel_);
	PROPERTY_READONLY(OverlayPanel*, _overlay_panel, OverlayPanel_);
	PROPERTY_READONLY(StatsPanel*, _stats_panel, StatsPanel_);
	PROPERTY_READONLY(ImagePreview*, _imagePreview, ImagePreview_);
	PROPERTY_READONLY(PerPointColor*, _perPointColor, PerPointColor_);
	
	QuenchController* _qCtrlr;
	bool _bRendererDeleted;

	void OnOverlayPanelToggle		(wxCommandEvent &event);
	void OnRefineSelectionPanelToggle(wxCommandEvent& event);
protected:
	//Menu events
    void OnTimerEvent(wxTimerEvent &event);    
    void OnMouseMoved(wxMouseEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnLoadState(wxCommandEvent& event);
	void OnSaveState(wxCommandEvent& event);
	void OnLoadVolume(wxCommandEvent& event);
    void OnLoadStandardVolumes(wxCommandEvent& event);
    void OnLoadPathways(wxCommandEvent& event);
    void OnSavePathwaysPDB(wxCommandEvent& event);    
    void OnLoadROINifti(wxCommandEvent &event);
    void OnSaveROINifti(wxCommandEvent &event);
	void OnQuit(wxCommandEvent& event);
	void OnCloseWindow(wxCloseEvent& event);
	void OnKeyboardShortcuts(wxCommandEvent& event);
	void OnUndoCommand(wxCommandEvent &event);
    void OnRedoCommand(wxCommandEvent &event);
	void OnToggleSagP  (wxCommandEvent& event);
    void OnToggleAxialP(wxCommandEvent& event);
    void OnToggleCorP  (wxCommandEvent& event);
    void OnTogglePathways(wxCommandEvent& event);
    void OnToggleFreezeAssignment(wxCommandEvent &event);
	void OnShowColorChooser(wxCommandEvent &event);
	void OnShowColorMapChooser(wxCommandEvent &event);
	void OnResetView(wxCommandEvent& event);
	void OnResetPathwayColors(wxCommandEvent &event);
    void OnSelectAll(wxCommandEvent &event);
    void OnDeselectAll(wxCommandEvent &event);
    void OnCycleBackground(wxCommandEvent& event);
	void OnImagePreviewToggle		(wxCommandEvent& event);
	void OnPerPointColorPanelToggle	(wxCommandEvent& event);
	
	void OnROIProperties(wxCommandEvent &event);
	void OnDrawNewROI(wxCommandEvent &event);
	void OnToggleROIEditingMode(wxCommandEvent &event);
	void OnIncreaseBrushSize(wxCommandEvent &event);
	void OnDecreaseBrushSize(wxCommandEvent &event);

	void OnToggleROIs(wxCommandEvent &event);
	void OnToggleROIBoundingBoxes(wxCommandEvent &event);
	void OnToggleROILabels(wxCommandEvent &event);

	void OnDeleteROI(wxCommandEvent &event);

	// Creation and destruction functions
	void CreateMenu();
	void DestroyMenu();
	void CreateSceneWindowAndPanels();
	void DestroySceneWindowAndPanels();

private:
	
	wxFileDialog *_file_dialog;
	wxMenu *_menu_actions;
    wxMenu *_menu_file;
    wxMenu *_menu_background;
    wxMenu *_menu_panels;
    wxMenu *_menu_view;
    wxMenu *_menu_edit;
    wxMenu *_menu_select;
    wxMenu *_menu_prototype;
    wxMenu *_menu_roi;
    wxSizer *m_main_sizer;
    wxTimer *_saveStateTimer;
	wxString _lastDirDlgPath;
	
	// any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE();
};


#endif

	
