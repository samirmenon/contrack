/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef DTI_FILTER_APP_H
#define DTI_FILTER_APP_H

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

class ScalarVolume;
class wxVTKRenderWindowInteractor;
class vtkInteractorStyleDTI;
class VOIPanel;
class FilterPanel;
class wxNotebook;
class Scene;
class DTIController;
class MyApp;

// Define a new frame type: this is going to be our main frame
class MyFrame : public wxFrame
{
public:
    // ctor(s)
    MyFrame(MyApp *app, const wxString& title, const wxPoint& pos, const wxSize& size, Scene *scene);
    ~MyFrame();

    void ConstructVTK();
    void ConfigureVTK();
    void ConfigureData(char *path);
    void ConfigureVTK2();
    void DestroyVTK();
    
    void ToggleGUIShift();
    void ToggleActivePanel();
    // event handlers (these functions should _not_ be virtual)
    
    void OnLaunchWebSite(wxCommandEvent& event);
    void OnKeyboardShortcuts(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnSaveState(wxCommandEvent& event);
    void OnLoadState(wxCommandEvent& event);
    void OnLoadPathways(wxCommandEvent& event);
    void OnSavePathwaysImage(wxCommandEvent& event);
    void OnSavePathwaysPDB(wxCommandEvent& event);
    //    void OnSetSubjectDatapath(wxCommandEvent& event);
    void OnLoadBackgrounds(wxCommandEvent& event);

    void OnSelectCorP(wxCommandEvent& event);
    void OnSelectSagP(wxCommandEvent& event);
    void OnSelectAxialP(wxCommandEvent& event);

    void OnToggleVOI(wxCommandEvent& event);
    void OnToggleSagP(wxCommandEvent& event);
    void OnToggleAxialP(wxCommandEvent& event);
    void OnToggleCorP(wxCommandEvent& event);
    void OnTogglePathways(wxCommandEvent& event);
    void OnToggleSurface(wxCommandEvent& event);
    void OnToggleCutPlanes(wxCommandEvent& event);
    
    void OnResetView(wxCommandEvent &event);

    void OnStainPathways(wxCommandEvent& event);
    void OnInsertVOI(wxCommandEvent& event);	
    void OnDeleteVOI(wxCommandEvent& event);
    void OnCycleVOI(wxCommandEvent& event);
    void OnCycleBackground(wxCommandEvent& event);
    void OnCycleBackgroundBackwards(wxCommandEvent& event);

    void EnableTomoCommands( bool pred );
    void EnableVOICommands ( bool pred );

    void PathwaysLoaded (const char *filename);

    void RefreshVTK();
    void SetVisibilityCheck(DTISceneActorID actorID, bool vis);

    wxMenu *_menu_actions;
    wxMenu *_menu_file;
    wxMenu *_menu_scene;
    wxMenu *_menu_background;
    wxSizer *m_main_sizer;
    bool m_gui_shifted;
    wxVTKRenderWindowInteractor *m_pVTKWindow;
    vtkInteractorStyleDTI *getIStyle() { return istyle; }
    VOIPanel *getVOIPanel() { return m_roi_panel; }
    FilterPanel *getFilterPanel() { return m_filter_panel; }
    Scene *scene;
    
protected:

private:
  

    MyApp *_app;
    std::string smoothMeshDirectory;
    std::string bumpyMeshDirectory;
    wxNotebook *notebook;
    
    wxSizer *BuildMainSizer(wxWindow *vtkWindow, wxWindow *filterPanel, wxWindow *roiPanel);
    wxSizer *BuildVideoSizer(wxWindow *vtkWindow, wxWindow *filterPanel, wxWindow *roiPanel);
    
    // vtk classes
    vtkRenderer       *aRenderer;
    vtkInteractorStyleDTI *istyle;
    VOIPanel *m_roi_panel;
    FilterPanel *m_filter_panel;

    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE()
};

#endif
