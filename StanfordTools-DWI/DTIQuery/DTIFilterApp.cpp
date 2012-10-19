/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#undef VIDEO_MODE

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWindows headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/notebook.h>
#include <wx/log.h>

#include "SystemInfoDialog.h"
#include "wxVTKRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkConeSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkPolyDataReader.h"
#include "vtkClipPolyData.h"
#include "vtkCamera.h"

#include "scene.h"
#include "VOIPanel.h"
#include "FilterPanel.h"
#include "DTIController.h"

#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkVolume16Reader.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkOutlineFilter.h"
#include "vtkStripper.h"
#include "vtkLookupTable.h"
#include "vtkImageDataGeometryFilter.h"
#include "vtkProperty.h"
#include "vtkPolyDataNormals.h"
#include "vtkContourFilter.h"
#include "vtkImageData.h"
#include "vtkImageMapToColors.h"
#include "vtkImageActor.h"
#include "vtkPlane.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"
#include "vtkInteractorStyleDTI.h"
#include "vtkPolyDataConnectivityFilter.h"
#include "vtkImplicitBoolean.h"

#include "DTIFilterApp.h"

#include "vtkSphereSource.h"


// the application icon
#if defined(__WXGTK__) || defined(__WXMOTIF__)
#include "icon.xpm"
#endif

class MyApp;
class MyFrame;

// Define a new application type, each program should derive a class from wxApp
class MyApp : public wxApp
{
public:
    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();
  virtual int FilterEvent (wxEvent &event);

private:
  DTIController *_controller;
};


// IDs for the controls and the menu commands
enum
{
    // menu items
    Minimal_Quit = 1,
    Launch_Web,
    Keyboard_Shortcuts,
    Menu_Tomos,
    Menu_VOIs,
    Menu_Pathways,
    Load_Backgrounds,
    Load_State,
    Save_State,
    Load_Pathways,
    Save_Pathways_PDB,
    Save_Pathways_Image,
    Toggle_VOI,
    Select_SagP,
    Select_AxialP,
    Select_CorP,
    Toggle_SagP,
    Toggle_AxialP,
    Toggle_CorP,
    Toggle_Pathways,
    Toggle_Surface,
    Toggle_CutPlanes,
    Reset_View,
    Stain_Pathways,
    Insert_VOI,
    Delete_VOI,
    Cycle_VOI,
    Cycle_Background,
    Cycle_Background_Backwards,
};

#define MY_FRAME      101
#define MY_VTK_WINDOW 102

// the event tables connect the wxWindows events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
  EVT_MENU(Minimal_Quit,  MyFrame::OnQuit)
  EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
  EVT_MENU(Launch_Web, MyFrame::OnLaunchWebSite)
  EVT_MENU(Keyboard_Shortcuts, MyFrame::OnKeyboardShortcuts)
  EVT_MENU(Load_Backgrounds, MyFrame::OnLoadBackgrounds)
  EVT_MENU(Load_State, MyFrame::OnLoadState)
  EVT_MENU(Save_State, MyFrame::OnSaveState)
  EVT_MENU(Load_Pathways, MyFrame::OnLoadPathways)
  EVT_MENU(Save_Pathways_PDB, MyFrame::OnSavePathwaysPDB)
  EVT_MENU(Save_Pathways_Image, MyFrame::OnSavePathwaysImage)
  EVT_MENU(Toggle_VOI, MyFrame::OnToggleVOI)	
  EVT_MENU(Toggle_SagP, MyFrame::OnToggleSagP)
  EVT_MENU(Toggle_AxialP, MyFrame::OnToggleAxialP)
  EVT_MENU(Toggle_CorP, MyFrame::OnToggleCorP)
  EVT_MENU(Toggle_SagP, MyFrame::OnSelectSagP)
  EVT_MENU(Toggle_AxialP, MyFrame::OnSelectAxialP)
  EVT_MENU(Toggle_CorP, MyFrame::OnSelectCorP)
  EVT_MENU(Toggle_Pathways, MyFrame::OnTogglePathways)
  EVT_MENU(Toggle_Surface, MyFrame::OnToggleSurface)
  EVT_MENU(Toggle_CutPlanes, MyFrame::OnToggleCutPlanes)
  EVT_MENU(Reset_View, MyFrame::OnResetView)
  EVT_MENU(Stain_Pathways, MyFrame::OnStainPathways)
  EVT_MENU(Insert_VOI, MyFrame::OnInsertVOI)	
  EVT_MENU(Delete_VOI, MyFrame::OnDeleteVOI)
  EVT_MENU(Cycle_VOI, MyFrame::OnCycleVOI)
  EVT_MENU(Cycle_Background, MyFrame::OnCycleBackground)
  EVT_MENU(Cycle_Background_Backwards, MyFrame::OnCycleBackgroundBackwards)
END_EVENT_TABLE()

// Create a new application object: this macro will allow wxWindows to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also declares the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(MyApp)

// 'Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
  cerr << "Initializing DTI-Query..." << endl;

	if (argc > 3)
	{
		wxString msg;
		msg.Printf( _T("Usage: DTIQuery [data path]\n\nIf provided, data path must\ncontain at least one image file '[.nii.gz;.nii;.hdr]'"), wxVERSION_STRING);
		wxMessageBox(msg, _T("Usage Error"), wxOK | wxICON_INFORMATION, NULL);
		return 1;
	}

	// For testing.
// 	ScalarVolume* fa = DTIVolumeIO::loadScalarVolumeMat("D:\\Programs\\cygwin\\home\\sherbond\\mattest.mat");
// 	DTIVolumeIO::saveScalarVolumeMat(fa,"D:\\Programs\\cygwin\\home\\sherbond\\mattest.mat");
// 	return 0;

	Scene *theScene = new Scene();

	// create the main application window
	MyFrame *frame = new MyFrame(this, _T("DTIQuery 1.1"),
				     wxPoint(50, 50), wxSize(200, 200), theScene); //450, 340
	
	
	frame->ConstructVTK();

	frame->ConfigureVTK();
	
	_controller = new DTIController(frame->getIStyle(), frame->getVOIPanel(), frame->getFilterPanel(), frame->m_pVTKWindow, frame);
	
	if (argc == 2) {
	  wxString pathStr = wxString(argv[1]);
	  const char *path = pathStr.mb_str();
	  frame->ConfigureData ((char *)path);
	}

	// and show it (the frames, unlike simple controls, are not shown when
	// created initially)
	frame->Show(TRUE);
	
    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned FALSE here, the
    // application would exit immediately.
    return TRUE;
}

int
MyApp::FilterEvent (wxEvent &event)
{
  return -1;
  /*
  if (Pending() && event.GetEventType() == wxEVT_LEFT_DOWN) {
    cerr << "returning true!" << endl;
    return true;
  }
  else {
    cerr << "returning -1!" << endl;
    return -1; 
  }
  */
}

// frame constructor
MyFrame::MyFrame(MyApp *app, const wxString& title, const wxPoint& pos, const wxSize& size, Scene *theScene)
       : wxFrame((wxFrame *)NULL, -1, title, pos, size)
{
  _app = app;

#ifdef __WXMAC__
    // we need this in order to allow the about menu relocation, since ABOUT is
    // not the default id of the about menu
  // wxApp::s_macAboutMenuItemId = Minimal_About;
    wxApp::s_macHelpMenuTitleName = "&Help";
#endif

#if wxUSE_STATUSBAR
    // create a status bar just for fun (by default with 1 pane only)
    //CreateStatusBar(2);
    //SetStatusText("");
#endif // wxUSE_STATUSBAR

#ifndef __WXMAC__
    // set the frame icon
    //    SetIcon(wxICON(icon));
#endif

    this->scene = theScene;
    // create a menu bar
    _menu_file = new wxMenu(_T(""), wxMENU_TEAROFF);

    _menu_file->Append(Load_State, _T("&Load State...\tCtrl-L"), _T("Load stored state"));
    _menu_file->Append(Save_State, _T("&Save State...\tCtrl-S"), _T("Save stored state"));
    _menu_file->AppendSeparator();

    _menu_file->Append(Load_Backgrounds, _T("Load Back&ground Image(s)...\tCtrl-G"), _T("Load background image(s)"));

    _menu_file->Append(Load_Pathways, _T("Load &Pathways...\tCtrl-P"), _T("Load pathways"));
    _menu_file->Append(Save_Pathways_PDB, _T("Save Visible Pathways (as PDB)...\tCtrl-E"), _T("Save visible pathways as PDB"));

    _menu_file->Append(Save_Pathways_Image, _T("Save Visible Pathways (as image)...\tCtrl-W"), _T("Save visible pathways as image"));

    //    _menu_file->Append(Load_Background, "Load Background...\tCtrl-B", "Load background image");
    _menu_file->AppendSeparator();
    
    _menu_file->Append(Minimal_Quit, _T("E&xit\tCtrl-X"), _T("Quit this program"));

    // the "About" item should be in the help menu
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(wxID_ABOUT, _T("&About...\tCtrl-A"), _T("Show about dialog"));
    helpMenu->Append(Keyboard_Shortcuts, _T("Mouse/Keyboard Shor&tcuts \tCtrl-T"), _T("Show keyboard shortcuts"));

    helpMenu->Append(Launch_Web, _T("DTI-Query Home Page"), _T("Go to the DTI-Query home page"));
    
    _menu_scene = new wxMenu;
    //wxMenuItem* miTomograms = new wxMenuItem(menuScene, Menu_Tomos, "Tomograms","",wxITEM_NORMAL, menuTomograms);
    //menuScene->Append(miTomograms);
    //wxMenuItem* miVOIs = new wxMenuItem(menuScene, Menu_VOIs, "VOIs","",wxITEM_NORMAL, menuVOIs);
    //menuScene->Append(miVOIs);
    //wxMenuItem* miPathways = new wxMenuItem(menuScene, Menu_Pathways, "Pathways","",wxITEM_NORMAL, menuPathways);
    //menuScene->Append(miPathways);

    _menu_actions = new wxMenu;
    /*
    _menu_scene->Append(Toggle_SagP, _T("Select &Sagittal Plane \tS"), _T("Select Sagittal Plane"), FALSE);

    _menu_scene->Append(Toggle_CorP, _T("Select &Coronal Plane \tC"), _T("Select Coronal Plane"), FALSE);

    _menu_scene->Append(Toggle_AxialP, _T("Select &Axial Plane \tA"), _T("Select Axial Plane"), FALSE);
    */
#ifdef __WXMSW__
    _menu_scene->Append(Toggle_SagP, _T("Sagittal Plane Visibility \tShift - S"), _T("Toggle tomogram visibility"), TRUE);
    _menu_scene->Check(Toggle_SagP, TRUE);

    _menu_scene->Append(Toggle_CorP, _T("Coronal Plane Visibility \tShift - C"), _T("Toggle tomogram visibility"), TRUE);
    _menu_scene->Check(Toggle_CorP, TRUE);

    _menu_scene->Append(Toggle_AxialP, _T("Axial Plane Visibility \tShift - A"), _T("Toggle tomogram visibility"), TRUE);
    _menu_scene->Check(Toggle_AxialP, TRUE);
    _menu_scene->AppendSeparator();
    _menu_scene->Append(Toggle_VOI, _T("VOI Visibili&ty \tShift - V"), _T("Toggle VOIs' visibility"), TRUE);
    _menu_scene->Check(Toggle_VOI, TRUE);      
    _menu_scene->Append(Toggle_Pathways, _T("Pat&hway Visibility \tShift - P"), _T("Toggle pathway visibility"), TRUE);
    _menu_scene->Check(Toggle_Pathways, TRUE);
#endif
#if defined (__WXGTK__) || defined (__WXMOTIF__)
    // mac platform...
     _menu_scene->Append(Toggle_SagP, _T("Sagittal Plane Visibility \tCtrl-Shift-S"), _T("Toggle tomogram visibility"), TRUE);
    _menu_scene->Check(Toggle_SagP, TRUE);

    _menu_scene->Append(Toggle_CorP, _T("Coronal Plane Visibility \tCtrl-Shift-C"), _T("Toggle tomogram visibility"), TRUE);
    _menu_scene->Check(Toggle_CorP, TRUE);

    _menu_scene->Append(Toggle_AxialP, _T("Axial Plane Visibility \tCtrl-Shift-A"), _T("Toggle tomogram visibility"), TRUE);
    _menu_scene->Check(Toggle_AxialP, TRUE);
    _menu_scene->AppendSeparator();
    _menu_scene->Append(Toggle_VOI, _T("VOI Visibili&ty \tCtrl-Shift-V"), _T("Toggle VOIs' visibility"), TRUE);
    _menu_scene->Check(Toggle_VOI, TRUE);      
    _menu_scene->Append(Toggle_Pathways, _T("Pat&hway Visibility \tCtrl-Shift-P"), _T("Toggle pathway visibility"), TRUE);
    _menu_scene->Check(Toggle_Pathways, TRUE); 

#endif
#ifdef __WXMAC__
    // mac platform...
     _menu_scene->Append(Toggle_SagP, _T("Sagittal Plane Visibility \tAlt-S"), _T("Toggle tomogram visibility"), TRUE);
    _menu_scene->Check(Toggle_SagP, TRUE);

    _menu_scene->Append(Toggle_CorP, _T("Coronal Plane Visibility \tAlt-C"), _T("Toggle tomogram visibility"), TRUE);
    _menu_scene->Check(Toggle_CorP, TRUE);

    _menu_scene->Append(Toggle_AxialP, _T("Axial Plane Visibility \tAlt-A"), _T("Toggle tomogram visibility"), TRUE);
    _menu_scene->Check(Toggle_AxialP, TRUE);
    _menu_scene->AppendSeparator();
    _menu_scene->Append(Toggle_VOI, _T("VOI Visibili&ty \tAlt-V"), _T("Toggle VOIs' visibility"), TRUE);
    _menu_scene->Check(Toggle_VOI, TRUE);      
    _menu_scene->Append(Toggle_Pathways, _T("Pat&hway Visibility \tAlt-P"), _T("Toggle pathway visibility"), TRUE);
    _menu_scene->Check(Toggle_Pathways, TRUE); 
#endif

    _menu_scene->AppendSeparator();
    _menu_scene->Append (Reset_View, _T("&Reset Camera Position\tCtrl-R"), _T("Reset the camera position"));

    _menu_actions->Append(Insert_VOI, _T("&Insert VOI \tCtrl-I"), _T("Insert a new VOI at the tomogram intersection."));
    _menu_actions->Append(Delete_VOI, _T("&Delete Current VOI \tCtrl-D"), _T("Delete the current VOI"));
    _menu_actions->Append(Cycle_VOI, _T("Cycle To Next &VOI \tCtrl-V"), _T("Cycle to next VOI"));
    _menu_actions->AppendSeparator();
  
    _menu_actions->Append(Stain_Pathways, _T("Stai&n Pathways \tCtrl-N"), _T("Stain Visible Pathways"));

    _menu_background = new wxMenu;
    _menu_background->Append (Cycle_Background, _T("Cycle background \tCtrl-B"), _T("Cycle the currently selected background image"));

    _menu_background->Append (Cycle_Background_Backwards, _T("Cycle background (reverse) \tCtrl-Shift-B"), _T("Cycle the currently selected background image"));

    //_menu_scene->Append(Toggle_Surface, "Surface Visibility", "Toggle surface visibility", TRUE);
    //_menu_scene->Check(Toggle_Surface, TRUE);
    //_menu_scene->Append(Toggle_CutPlanes, "Cut Planes Enabled", "Toggle cut planes", TRUE);
    //_menu_scene->Check(Toggle_CutPlanes, TRUE);

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(_menu_file, _T("&File"));
    menuBar->Append(_menu_scene, _T("&View"));
    menuBar->Append(_menu_actions, _T("&Actions"));
    menuBar->Append(_menu_background, _T("&Background"));
    menuBar->Append(helpMenu, _T("&Help"));

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);

    EnableTomoCommands (false);
    EnableVOICommands (false);

    m_pVTKWindow = new wxVTKRenderWindowInteractor(this, MY_VTK_WINDOW, wxDefaultPosition, wxSize(720,280));
    // normal window size for 720x480 view:
    //m_pVTKWindow = new wxVTKRenderWindowInteractor(this, MY_VTK_WINDOW, wxDefaultPosition, wxSize(288,405));
    m_roi_panel = new VOIPanel(this);
    m_filter_panel = new FilterPanel (this);
    m_filter_panel->SetBackgroundColour(menuBar->GetBackgroundColour());
    // xxx NULL
    
    this->SetBackgroundColour (menuBar->GetBackgroundColour());
    
    this->m_gui_shifted = false;
    
#ifdef VIDEO_MODE
    
    m_main_sizer = BuildVideoSizer(m_pVTKWindow,
				   m_filter_panel,
				   m_roi_panel);
#else
    
    m_main_sizer = BuildMainSizer(m_pVTKWindow,
				  m_filter_panel,
				  m_roi_panel);
#endif
    
    m_main_sizer->SetSizeHints (this);
    SetSizerAndFit (m_main_sizer, FALSE);
    //SetSizer(vSizer);
    Layout();
    
    //turn on mouse grabbing if possible
    m_pVTKWindow->UseCaptureMouseOn();
    SendSizeEvent ();
}

void shiftWindow (wxWindow *window, int amount, int size) {
	wxRect rect = window->GetRect();
	rect.x+=amount;
	rect.width+= size;
	window->SetSize (rect);
}
/*
void
MyFrame::OnSetSubjectDatapath (wxCommandEvent &event)
{
  bool success = istyle->SetSubjectDatapath ();
  if (success) {
    EnableTomoCommands(true);
  }
}
*/

void
MyFrame::OnLoadState (wxCommandEvent &event)
{
  this->istyle->RestoreState();
}

void
MyFrame::OnSaveState (wxCommandEvent &event)
{
  this->istyle->SaveState();
}

void 
MyFrame::OnLoadPathways(wxCommandEvent& event)
{
  this->istyle->LoadPathways();
}

void
MyFrame::PathwaysLoaded (const char *filename) 
{
  char title[255];
  sprintf (title, "DTIQuery - %s", filename);
#if wxUSE_UNICODE
  wxString str (title, wxConvUTF8);
#else
  wxString str (title);
#endif
  SetTitle (str);
}

void 
MyFrame::OnSavePathwaysPDB(wxCommandEvent& event)
{
  this->istyle->SavePathwaysPDB();
}

void 
MyFrame::OnSavePathwaysImage(wxCommandEvent& event)
{
  this->istyle->SavePathwaysImage();
}

void 
MyFrame::ToggleGUIShift ()
{
	int amount;
	if (this->m_gui_shifted) {
		amount = 18;
	}
	else {
		amount = -18;
	}
	for (int i = 0; i < 71/3; i++) {
	shiftWindow (this->notebook, amount, 0);
	shiftWindow (this->m_roi_panel, amount, 0);
	shiftWindow (this->m_filter_panel, amount, 0);

	shiftWindow (this->m_pVTKWindow, amount, -amount);

	Refresh(FALSE);
	Update();
	//this->m_main_sizer->Layout();
    }
	shiftWindow (this->notebook, amount*2/3, 0);
	shiftWindow (this->m_roi_panel, amount*2/3, 0);
	shiftWindow (this->m_filter_panel, amount*2/3, 0);
	shiftWindow (this->m_pVTKWindow, amount*2/3, -amount*2/3);
	Refresh(FALSE);
	Update();
	this->m_gui_shifted = !this->m_gui_shifted;
}

void 
MyFrame::ToggleActivePanel() {
	this->notebook->SetSelection (1-this->notebook->GetSelection());
}

wxSizer *MyFrame::BuildMainSizer(wxWindow *vtkWindow, wxWindow *filterPanel, wxWindow *roiPanel)
{

	wxBoxSizer *vSizer = new wxBoxSizer(wxVERTICAL);
	vSizer->Add (vtkWindow, 2, wxGROW, 3); //wxTOP | wxBOTTOM

	wxBoxSizer *filterSizer = new wxBoxSizer(wxHORIZONTAL);
    
	filterSizer->Add (filterPanel, 1, wxGROW | wxADJUST_MINSIZE);
	filterSizer->Add (15,0);	
	filterSizer->Add (roiPanel, 1, wxGROW | wxADJUST_MINSIZE);
	filterSizer->Add (10,0);
	vSizer->Add (filterSizer, 0, wxGROW | wxADJUST_MINSIZE);

	return vSizer;
}

wxSizer *MyFrame::BuildVideoSizer(wxWindow *vtkWindow, wxWindow *filterPanel, wxWindow *roiPanel)
{
	this->notebook = new wxNotebook (this, -1, wxDefaultPosition, wxDefaultSize);

	wxBoxSizer *hSizer = new wxBoxSizer(wxHORIZONTAL);
	
	notebook->AddPage(filterPanel, _T("Queries"));
	notebook->AddPage(roiPanel, _T("VOIs"));
	
	hSizer->Add (notebook, 0, wxGROW | wxADJUST_MINSIZE);
	hSizer->Add (vtkWindow, 2, wxGROW, 3); //wxTOP | wxBOTTOM

	return hSizer;
}

MyFrame::~MyFrame()
{
  if(m_pVTKWindow) m_pVTKWindow->Delete();
  DestroyVTK();
}

void MyFrame::ConstructVTK()
{
  
}

// extern void SetBackground(double,double,double,double);

void MyFrame::ConfigureVTK()
{
  // stream junk
  // ScalarVolume *scalarVolume = DTIVolumeIO::loadScalarVolume (faStream);
  
  istyle = vtkInteractorStyleDTI::New();
  istyle->SetFrame(this);
  istyle->SetScene(scene);
  m_pVTKWindow->SetInteractorStyle(istyle);
  
  // connect the render window and wxVTK window
  vtkRenderWindow *pRenderWindow = m_pVTKWindow->GetRenderWindow();
  aRenderer = vtkRenderer::New();
  pRenderWindow->AddRenderer(aRenderer);
  pRenderWindow->LineSmoothingOff();
  pRenderWindow->PolygonSmoothingOff();
  pRenderWindow->SetAAFrames(0);

  //  pRenderWindow->SetSize(200,200);  

  // Setup cursor text
  //this->scene->cursorInfo->ScaledTextOn();
  this->scene->cursorInfo->SetDisplayPosition(2, 10);
  //textActor.GetPosition2Coordinate().SetCoordinateSystemToNormalizedViewport()
  //textActor.GetPosition2Coordinate().SetValue(0.6, 0.1)
  this->scene->cursorInfo->GetTextProperty()->SetFontSize(18);
  this->scene->cursorInfo->GetTextProperty()->SetFontFamilyToArial();
  this->scene->cursorInfo->GetTextProperty()->SetJustificationToLeft();
  this->scene->cursorInfo->GetTextProperty()->BoldOn();
  this->scene->cursorInfo->GetTextProperty()->ItalicOn();
  this->scene->cursorInfo->GetTextProperty()->ShadowOn();
  this->scene->cursorInfo->GetTextProperty()->SetColor(1,1,1);
  
  this->scene->ReportPositionFromTomos(NULL);
  aRenderer->AddActor((vtkProp*)this->scene->cursorInfo);

  // Set a background color for the renderer and set the size of the
  // render window (expressed in pixels).

  aRenderer->SetBackground(scene->backgrounds[scene->curBG].r,scene->backgrounds[scene->curBG].g,scene->backgrounds[scene->curBG].b);
	 
  vtkCamera *camera = vtkCamera::New();
  aRenderer->SetActiveCamera(camera);
  camera->Delete();
  
  //this->Maximize();
  //this->SendSizeEvent();
  //  istyle->Initialize();
}

void 
MyFrame::ConfigureData (char *path)
{
  istyle->SetSubjectDatapathInternal(path);
  // disabled for now...
  EnableTomoCommands (true);
  //  _menu_file->Enable (Load_Pathways, true);
}

void MyFrame::DestroyVTK()
{
  aRenderer->Delete();
}

// event handlers

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // TRUE is to force the frame to close
    Close(TRUE);
}

void MyFrame::OnToggleVOI(wxCommandEvent& WXUNUSED(event))
{
  // Call the VTK stuff eventually
  istyle->ToggleVOIVisibility();
}

void MyFrame::OnToggleSagP(wxCommandEvent& WXUNUSED(event))
{
  // Call the VTK stuff eventually
  istyle->ToggleVisibility(DTI_ACTOR_SAGITTAL_TOMO);
}

void MyFrame::OnToggleAxialP(wxCommandEvent& WXUNUSED(event))
{
  // Call the VTK stuff eventually
  istyle->ToggleVisibility(DTI_ACTOR_AXIAL_TOMO);
}

void MyFrame::OnToggleCorP(wxCommandEvent& WXUNUSED(event))
{
  // Call the VTK stuff eventually
  istyle->ToggleVisibility(DTI_ACTOR_CORONAL_TOMO);
}

void MyFrame::OnTogglePathways(wxCommandEvent& WXUNUSED(event))
{
	// Call the VTK stuff eventually
	istyle->ToggleVisibility(DTI_ACTOR_TRACTS);
}

void MyFrame::OnToggleSurface(wxCommandEvent& WXUNUSED(event))
{
	// Call the VTK stuff eventually
  //	istyle->ToggleVisibility(VTKIS_SURFACE);
}

void MyFrame::OnToggleCutPlanes(wxCommandEvent& WXUNUSED(event))
{
	// Call the VTK stuff eventually
  //	istyle->ToggleVisibility(VTKIS_CUTPLANES);
}

void MyFrame::OnStainPathways(wxCommandEvent& event)
{
	this->istyle->StainPathways();
}

void MyFrame::OnInsertVOI(wxCommandEvent& event)
{
	this->istyle->InsertVOI();
}

void MyFrame::OnDeleteVOI(wxCommandEvent& event)
{
  this->istyle->DeleteVOI();
}

void MyFrame::OnCycleVOI(wxCommandEvent& event)
{
  this->istyle->CycleVOI();
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxString msg;
    wxString caption(_T("About DTIQuery"));
    wxString message(_T("DTIQuery 1.1 - November 4, 2007\n\nDavid Akers (dakers@stanford.edu) \n       and \nAnthony Sherbondy (sherbond@stanford.edu) \n\nhttp://graphics.stanford.edu/projects/dti/"));

    SystemInfoDialog* info_dialog = new SystemInfoDialog(this, caption, message);
    info_dialog->Show();
}

void MyFrame::EnableTomoCommands (bool pred)
{
  _menu_scene->Enable (Toggle_SagP, pred);
  _menu_scene->Enable (Toggle_CorP, pred);
  _menu_scene->Enable (Toggle_AxialP, pred);
  _menu_background->Enable (Cycle_Background, pred);
  _menu_background->Enable (Cycle_Background_Backwards, pred);
  _menu_file->Enable (Load_Pathways, pred);
  _menu_scene->Enable (Reset_View, pred);
}

void MyFrame::EnableVOICommands(bool pred) 
{
  _menu_scene->Enable (Toggle_VOI, pred);
  _menu_scene->Enable (Toggle_Pathways, pred);
  _menu_actions->Enable (Insert_VOI, pred);
  _menu_actions->Enable (Delete_VOI, pred);
  _menu_actions->Enable (Cycle_VOI, pred);
  _menu_actions->Enable (Stain_Pathways, pred);
  _menu_file->Enable(Load_State, pred);
  _menu_file->Enable(Save_State, pred);
  _menu_file->Enable(Save_Pathways_PDB, pred);
  _menu_file->Enable(Save_Pathways_Image, pred);
}

void MyFrame::OnCycleBackground (wxCommandEvent &event) 
{
  this->istyle->CycleBackground(true);
}

void MyFrame::OnCycleBackgroundBackwards (wxCommandEvent &event)
{
  this->istyle->CycleBackground(false);
}

void MyFrame::RefreshVTK () 
{
  this->m_pVTKWindow->Refresh();

  // Update() causes an immediate redraw:
  // If I don't do this on Windows, the screen never gets redrawn during VOI
  // dragging.
  // If I do this on Linux/X11, the event handler gets behind processing mouse
  // drag events, and there is horrible latency.
  // Yield is supposed to let other events execute, but seems to do nothing.
  // What happens on Mac/Carbon?
  // What happens on GTK/Linux?

#ifdef __WXMSW__
  this->m_pVTKWindow->Update();
#endif
  //this->_app->Yield();
}

void MyFrame::OnSelectAxialP (wxCommandEvent &event)
{
  istyle->SelectPlane (DTI_ACTOR_AXIAL_TOMO);
}

void MyFrame::OnSelectCorP (wxCommandEvent &event)
{
  istyle->SelectPlane(DTI_ACTOR_CORONAL_TOMO);
}
void MyFrame::OnSelectSagP (wxCommandEvent &event)
{
  istyle->SelectPlane(DTI_ACTOR_SAGITTAL_TOMO);
}

void
MyFrame::OnResetView (wxCommandEvent &event)
{
  this->istyle->ResetView();
}

void
MyFrame::OnLaunchWebSite (wxCommandEvent &event)
{
#ifdef __WXMSW__
  ShellExecute( NULL, "open", "http://graphics.stanford.edu/projects/dti/dti-query", NULL, "C:\\", SW_SHOWNORMAL );
#endif

#ifdef __WXMAC__
  system("open http://graphics.stanford.edu/projects/dti/dti-query");
#endif

  // xxx how to do this on Linux?
}

void
MyFrame::SetVisibilityCheck(DTISceneActorID actorID, bool vis) {
  switch (actorID) {
  case DTI_ACTOR_AXIAL_TOMO:
    _menu_scene->Check (Toggle_AxialP, vis);
    break;
  case DTI_ACTOR_SAGITTAL_TOMO:
    _menu_scene->Check (Toggle_SagP, vis);
    break;
  case DTI_ACTOR_CORONAL_TOMO:
    _menu_scene->Check (Toggle_CorP, vis);
    break;
  default:
    break;
  };
}

void
MyFrame::OnLoadBackgrounds(wxCommandEvent &event)
{
  bool success = istyle->LoadBackgrounds ();
  if (success) {
    EnableTomoCommands(true);
  }
}

void
MyFrame::OnKeyboardShortcuts(wxCommandEvent &event)
{
   wxString msg;
   wxString caption(_T("DTIQuery Keyboard/Mouse Shortcuts"));
   wxString message(_T("Mouse Shortcuts:\n\n  Left mouse - rotate\n  Right mouse - zoom\n  Shift + Left mouse - pan\n  Ctrl + Left mouse - drag current VOI on current tomogram\n\nKeyboard Shortcuts:\n\n  's', 'c', and 'a': select sagittal, coronal, or axial tomogram\n  ',' and '.': slide the current tomogram back and forth \n  'b': cycle background color \n  'k': toggle anti-aliasing \n  '[':  cycle between camera positions \n  'w': toggle line widths \n  't': toggle all VOI labels on/off"));

    SystemInfoDialog* info_dialog = new SystemInfoDialog(this, caption, message);
    info_dialog->Show();
}
