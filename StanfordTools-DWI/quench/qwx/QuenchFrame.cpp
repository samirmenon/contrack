#include "QuenchFrame.h"
#include "QuenchController.h"
#include "NButtonChoiceDialog.h"
#include "SceneWindow.h"
#include "vtkInteractorStyleQuench.h"
#include "ColorMapPanel.h"
#include "OverlayPanel.h"
#include "StatsPanel.h"
#include "StatsCheckboxArray.h"
#include "ImagePreview.h"
#include "PerPointColor.h"
#include "qVolumeViz.h"
#include "qPathwayViz.h"
#include "VisibilityPanel.h"
#include "qROIViz.h"
#include "AboutDlg.h"
#include "History.h"
#include "wx/colordlg.h"
#include "vtkRenderer.h"

// XXX REMOVE
//QuenchFrame *the_global_frame=NULL;

wxFont DEFAULT_FONT(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
wxSize DEFAULT_TEXTBOX_SIZE(100,21); //(100,21);
wxSize DEFAULT_BUTTON_SIZE(-1,-1);

BEGIN_EVENT_TABLE(NButtonChoiceDialog, wxDialog)
	EVT_BUTTON ( BUTTON_CHOICE_1, NButtonChoiceDialog::OnChoice1)
	EVT_BUTTON ( BUTTON_CHOICE_2, NButtonChoiceDialog::OnChoice2)
	EVT_BUTTON ( BUTTON_CHOICE_3, NButtonChoiceDialog::OnChoice3)
END_EVENT_TABLE()

	// the event tables connect the wxWindows events with the functions (event
	// handlers) which process them. It can be also done at run-time, but for the
	// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(QuenchFrame, wxFrame)
	EVT_TIMER(-1, QuenchFrame::OnTimerEvent)
	EVT_CLOSE(QuenchFrame::OnCloseWindow)
	EVT_MOTION(QuenchFrame::OnMouseMoved)

	EVT_MENU(Load_State, QuenchFrame::OnLoadState)
	EVT_MENU(Save_State, QuenchFrame::OnSaveState)
	EVT_MENU(Load_Volume, QuenchFrame::OnLoadVolume)
	EVT_MENU(Load_StandardVolumes, QuenchFrame::OnLoadStandardVolumes)
	EVT_MENU(Load_Pathways, QuenchFrame::OnLoadPathways)
	EVT_MENU(Save_Pathways_PDB, QuenchFrame::OnSavePathwaysPDB)
        EVT_MENU(Load_ROI_Nifti, QuenchFrame::OnLoadROINifti)
        EVT_MENU(Save_ROI_Nifti, QuenchFrame::OnSaveROINifti)
	EVT_MENU(Minimal_Quit,  QuenchFrame::OnQuit)
	EVT_MENU(Minimal_About, QuenchFrame::OnAbout)
	EVT_MENU(Keyboard_Shortcuts, QuenchFrame::OnKeyboardShortcuts)
	EVT_MENU(Undo_Command, QuenchFrame::OnUndoCommand)
	EVT_MENU(Redo_Command, QuenchFrame::OnRedoCommand)
	EVT_MENU(Toggle_SagP, QuenchFrame::OnToggleSagP)
	EVT_MENU(Toggle_AxialP, QuenchFrame::OnToggleAxialP)
	EVT_MENU(Toggle_CorP, QuenchFrame::OnToggleCorP)
	EVT_MENU(Toggle_Pathways, QuenchFrame::OnTogglePathways)
        EVT_MENU(Toggle_ROIs, QuenchFrame::OnToggleROIs)
        EVT_MENU(Toggle_ROI_Bounding_Boxes, QuenchFrame::OnToggleROIBoundingBoxes)
        EVT_MENU(Toggle_ROI_Labels, QuenchFrame::OnToggleROILabels)
	EVT_MENU(Show_ColorChooser, QuenchFrame::OnShowColorChooser)
	EVT_MENU(Show_ColorMapChooser, QuenchFrame::OnShowColorMapChooser)
	EVT_MENU(Reset_View, QuenchFrame::OnResetView)
	EVT_MENU(Reset_PathwayColors, QuenchFrame::OnResetPathwayColors)
	EVT_MENU(Select_All, QuenchFrame::OnSelectAll)
	EVT_MENU(Deselect_All, QuenchFrame::OnDeselectAll)
	EVT_MENU(Toggle_Freeze_Assignment, QuenchFrame::OnToggleFreezeAssignment)

	EVT_MENU(Cycle_Background0, QuenchFrame::OnCycleBackground)
	EVT_MENU(Cycle_Background1, QuenchFrame::OnCycleBackground)
	EVT_MENU(Cycle_Background2, QuenchFrame::OnCycleBackground)
	EVT_MENU(Cycle_Background3, QuenchFrame::OnCycleBackground)
	EVT_MENU(Cycle_Background4, QuenchFrame::OnCycleBackground)
	EVT_MENU(Cycle_Background5, QuenchFrame::OnCycleBackground)
	EVT_MENU(Cycle_Background6, QuenchFrame::OnCycleBackground)
	EVT_MENU(Cycle_Background7, QuenchFrame::OnCycleBackground)
	EVT_MENU(Cycle_Background8, QuenchFrame::OnCycleBackground)
	EVT_MENU(Cycle_Background9, QuenchFrame::OnCycleBackground)
	EVT_MENU(Refine_Selection_Panel, QuenchFrame::OnRefineSelectionPanelToggle)
	EVT_MENU(Overlay_Panel, QuenchFrame::OnOverlayPanelToggle)
	EVT_MENU(Show_ImagePreview, QuenchFrame::OnImagePreviewToggle)
	EVT_MENU(Show_PerPointColor, QuenchFrame::OnPerPointColorPanelToggle)

        EVT_MENU(ROI_Properties, QuenchFrame::OnROIProperties)
        EVT_MENU(Draw_New_ROI, QuenchFrame::OnDrawNewROI)
        EVT_MENU(Delete_ROI, QuenchFrame::OnDeleteROI)
        EVT_MENU(Toggle_ROI_Editing_Mode, QuenchFrame::OnToggleROIEditingMode)
        EVT_MENU(Increase_Brush_Size, QuenchFrame::OnIncreaseBrushSize)
        EVT_MENU(Decrease_Brush_Size, QuenchFrame::OnDecreaseBrushSize)

END_EVENT_TABLE()
	
// IDs for the controls and the menu commands
#define MY_VTK_WINDOW 102

///////////////////////////////////////////////////////////////////////////////////////////////////
// Setup and destruction section
///////////////////////////////////////////////////////////////////////////////////////////////////

QuenchFrame::QuenchFrame(QuenchController* qCtrlr, const wxString &title, const wxPoint &pos, const wxSize &size)
	: wxFrame((wxFrame *)NULL, -1, title, pos, size)
{
	_qCtrlr = qCtrlr;
	//the_global_frame = this; // XXX
	
	// The GUI shares one file dialog for finding files, here we initialize to NULL
	_file_dialog=0;
	
	// Time to create the top level menu
	this->CreateMenu();

	// Time to create the main window with the VTK scene.  We will also create all the GUI panels
	// that monitor scene data
	this->CreateSceneWindowAndPanels();

	// Now that the GUI elements have been created tell QuenchController to start listening to
	// events that could be generated from them
	_qCtrlr->ConnectControllers(this);
	
	// Create timer for state file auto-save, but don't start it running.  This is done here because
	// QuenchFrame is the main wxEvent handler, whereas QuenchController is the main Quench Event 
	// handler
	_saveStateTimer = new wxTimer(this, 1);
	
	// Tell the user where the controller will save the temporary state of the system if a state 
	// file is not given
	cout<<"In case of crash your last selection is saved to "<<TEMP_QST_FILE_NAME<<" file"<<endl;
}

QuenchFrame::~QuenchFrame()
{
	bool b = true;
	this->_qCtrlr->SetIsDestroyed(b);
	this->DestroySceneWindowAndPanels();
	this->DestroyMenu();
	delete _saveStateTimer;
}

void QuenchFrame::CreateMenu()
{
#ifdef __WXMAC__
	// we need this in order to allow the about menu relocation, since ABOUT is
	// not the default id of the about menu
	wxApp::s_macAboutMenuItemId = Minimal_About;
#endif

	// create a menu bar
	_menu_file = new wxMenu("", wxMENU_TEAROFF);
	_menu_file->Append(Load_State, "&Load State...\tCtrl-L", "Load stored state");
	_menu_file->Append(Save_State, "&Save State...\tCtrl-S", "Save stored state");
	_menu_file->AppendSeparator();

	_menu_file->Append(Load_Volume, "Load &Volume...\tCtrl-G", "Load subject volume data");
	_menu_file->Append(Load_StandardVolumes, "Load Standard Volumes...", "Load standard subject volume data");
	_menu_file->Append(Load_Pathways, "Load &Pathways...\tCtrl-P", "Load pathways");

	_menu_file->Append(Save_Pathways_PDB, "Save Visible Pathwa&ys (as PDB)...\tCtrl-E", "Save visible pathways as PDB");

	_menu_file->AppendSeparator();
	_menu_file->Append(Load_ROI_Nifti, "Load &NIFTI ROI(s)...\tCtrl-N", "Load ROI as a NIFTI mask file...");
	_menu_file->Append(Save_ROI_Nifti, "Save selected ROI as NIFTI...\tCtrl-F", "Save Current ROI as a NIFTI mask file...");
	_menu_file->Enable(Save_ROI_Nifti, FALSE);

	_menu_file->AppendSeparator();

	_menu_file->Append(Minimal_Quit, "E&xit\tAlt-F4", "Quit this program");

	// the "About" item should be in the help menu
	wxMenu *helpMenu = new wxMenu;
	helpMenu->Append(Minimal_About, "&About...\tCtrl-H", "Show about dialog");
	helpMenu->AppendSeparator();
	helpMenu->Append(Keyboard_Shortcuts, _T("Mouse/Keyboard Shor&tcuts \tCtrl-T"), _T("Show mouse and keyboard shortcuts"));


	_menu_actions = new wxMenu;
	_menu_actions->AppendSeparator();

	_menu_background = new wxMenu;
	//_menu_background->Append (Cycle_Background0, "Cycle background \tCtrl-B", "Cycle the currently selected background image");

	_menu_panels = new wxMenu;
	_menu_panels->Append (Refine_Selection_Panel, "Refine Selection", "Display score panel", TRUE);
	_menu_panels->Append(Overlay_Panel, "Backgrounds && Overlays", "Show the background and overlays");
	_menu_panels->Append(Show_ImagePreview, "Screenshot", "Show the screenshot panel");
	_menu_panels->Append(Show_PerPointColor, "Per point color", "Show the per point coloring panel");

	_menu_view = new wxMenu;

#ifdef __WXMSW__
#	define MODIFIER_KEY  wxString("Shift-")
#elif defined (__WXGTK__) || defined (__WXMOTIF__)
#	define MODIFIER_KEY  wxString("Ctrl-Shift-")
#elif defined(__WXMAC__)
#	define MODIFIER_KEY  wxString("Alt-")
#endif

	_menu_view->Append(Toggle_SagP, _T(wxString("Show Sagittal Plane \t")+MODIFIER_KEY+wxString("S")), _T("Toggle tomogram visibility"), TRUE);
	_menu_view->Check(Toggle_SagP, TRUE);

	_menu_view->Append(Toggle_CorP, _T("Show Coronal Plane \t")+MODIFIER_KEY+wxString("C"), _T("Toggle tomogram visibility"), TRUE);
	_menu_view->Check(Toggle_CorP, TRUE);

	_menu_view->Append(Toggle_AxialP, _T("Show Axial Plane \t")+MODIFIER_KEY+wxString("A"), _T("Toggle tomogram visibility"), TRUE);
	_menu_view->Check(Toggle_AxialP, TRUE);
#undef MODIFIER_KEY

	_menu_view->Append(Toggle_Pathways, _T("Show Pathways"), _T("Toggle Pathways visibility"), TRUE);
	_menu_view->Check(Toggle_Pathways, FALSE);

	_menu_view->AppendSeparator();

	_menu_view->Append(Toggle_ROIs, _T("Show ROIs"), _T("Toggle ROI visibility"), TRUE);
	_menu_view->Check(Toggle_ROIs, TRUE);

	_menu_view->Append(Toggle_ROI_Bounding_Boxes, _T("Show ROI Bounds"), _T("Toggle ROI bounding box visibility"), TRUE);
	_menu_view->Check(Toggle_ROI_Bounding_Boxes, TRUE);

	_menu_view->Append(Toggle_ROI_Labels, _T("Show ROI Labels"), _T("Toggle ROI label visibility"), TRUE);
	_menu_view->Check(Toggle_ROI_Labels, TRUE);

	_menu_view->AppendSeparator();
	_menu_view->Append (Reset_View, _T("Snap &Camera to Axis\tCtrl-C"), _T("Snap camera to closest axis"));
	_menu_view->Append (Reset_PathwayColors, _T("Reset Pathway Colors"), _T("Reset the pathway groups color"));
	_menu_view->AppendSeparator();
	_menu_view->Append (Show_ColorChooser, _T("Choose color for current group"), _T("Choose a color for the currently selected fiber group"));
	_menu_view->Append (Show_ColorMapChooser, _T("Choose color map for fiber groups"), _T("Choose a color map for the fiber groups"));

	_menu_edit = new wxMenu;
	_menu_edit->Append (Undo_Command, "Undo \tCtrl-Z", "Undo last command");
	_menu_edit->Append (Redo_Command, "Redo \tCtrl-Y", "Redo last command");

	_menu_edit->Enable (Undo_Command, false);
	_menu_edit->Enable (Redo_Command, false);
	
	
	_menu_edit->AppendSeparator();
	_menu_edit->Append (Select_All, "Select &All Paths\tCtrl-A", "Select all pathways");
	_menu_edit->Append (Deselect_All, "Deselect &All Paths\tShift-Ctrl-A", "Deselect all pathways");
	_menu_edit->AppendSeparator();
	_menu_edit->Append (Toggle_Freeze_Assignment, "Loc&k Assigned Groups\tCtrl-K", "Lock assigned groups", TRUE);
	_menu_edit->Check(Toggle_Freeze_Assignment, TRUE);

	_menu_select = new wxMenu;

	_menu_roi = new wxMenu;
	_menu_roi->Append(ROI_Properties, "ROI Properties", "Edit properties of current ROI");
	_menu_roi->Enable(ROI_Properties, FALSE);
	_menu_roi->Append(Delete_ROI, "Delete current ROI", "Delete the currently selected ROI");
	_menu_roi->Enable(Delete_ROI, FALSE);
	_menu_roi->Append(Draw_New_ROI, "&Draw new ROI\tCtrl-D", "Draw a new ROI on the current tomogram");
	_menu_roi->Append(Toggle_ROI_Editing_Mode, "ROI Editing mode\tCtrl-R", "Edit an existing ROI", TRUE);
	_menu_roi->Check(Toggle_ROI_Editing_Mode, FALSE);
	_menu_roi->Enable(Toggle_ROI_Editing_Mode, FALSE);
	
	_menu_roi->AppendSeparator();
	_menu_roi->Append(Increase_Brush_Size, "Increase brush size\t=", "Increase the brush size");
	_menu_roi->Append(Decrease_Brush_Size, "Decrease brush size\t-", "Decrease the brush size");
	_menu_roi->Enable(Increase_Brush_Size, FALSE);
	_menu_roi->Enable(Decrease_Brush_Size, FALSE);

	// now append the freshly created menu to the menu bar...
	wxMenuBar *menuBar = new wxMenuBar();
	menuBar->Append(_menu_file, "&File"); menuBar->Append(_menu_edit, "&Edit"); menuBar->Append(_menu_view, "&View");
	menuBar->Append(_menu_roi, "&ROI");
	//menuBar->Append(_menu_select, "&Select"); //menuBar->Append(_menu_background, "&Background");
	menuBar->Append(_menu_panels, "&Panels"); menuBar->Append(helpMenu, "&Help");
	menuBar->SetFont( DEFAULT_FONT );

	// ... and attach this menu bar to the frame
	SetMenuBar(menuBar);
	
	// XXX is this color necessary?
	wxColour bkgnd_col = menuBar->GetBackgroundColour();
	this->SetBackgroundColour ( bkgnd_col );
}

void QuenchFrame::DestroyMenu()
{
	// XXX why isn't there more?
	delete _menu_actions;	
}

void QuenchFrame::CreateSceneWindowAndPanels()
{
	_bRendererDeleted = false;	// XXX
	
	_pSceneWindow = new SceneWindow(this, MY_VTK_WINDOW, wxDefaultPosition, wxSize(QUENCH_DEFAULT_WIDTH, QUENCH_DEFAULT_HEIGHT));
	_istyle = vtkInteractorStyleQuench::New();

	m_main_sizer = new wxBoxSizer (wxHORIZONTAL);
	m_main_sizer->Add (_pSceneWindow, 2, wxGROW, 3); //wxTOP | wxBOTTOM
	m_main_sizer ->Add (5,0);

	m_main_sizer->SetSizeHints (this);
	SetSizerAndFit (m_main_sizer, FALSE);
	Layout();

	_istyle->Init(_pSceneWindow);
	
	//turn on mouse grabbing if possible
	_pSceneWindow->UseCaptureMouseOn();

	//Create Panels
	_color_map_panel = new ColorMapPanel(this);
	_color_map_panel->_qFrame = this;
	_istyle->PathwayViz()->SetColorMapPanel_(_color_map_panel);
	_overlay_panel = new OverlayPanel(this);
	_overlay_panel->SetVolViz(_istyle->VolumeViz());
	_stats_panel = new StatsPanel(this);
	_imagePreview = new ImagePreview(this);
	_imagePreview->SetPathwayViz(_istyle->PathwayViz());
	_perPointColor = new PerPointColor(this,_color_map_panel);
}

void QuenchFrame::DestroySceneWindowAndPanels()
{
	// Destroy panels
	_color_map_panel->Destroy();
	_overlay_panel ->Destroy();
	_stats_panel->Destroy(); // tony new
	_imagePreview->Destroy();
	_perPointColor->Destroy(); // tony new
	
	// Destroy scene stuff
	VTK_SAFE_DELETE(_pSceneWindow);
	VTK_SAFE_DELETE(_istyle);
	_bRendererDeleted = true;
}

void QuenchFrame::ClearForLoadState()
{
	_stats_panel->Clear();
	_perPointColor->Clear();
	_overlay_panel->Clear();
	_istyle->VolumeViz()->Clear();

	//Remove the background menu items from menu
	wxMenuItem * separator = _menu_background->AppendSeparator(); //Add this to end radio group
	_menu_background->Delete(separator);
	int numMenuItems = (int)_menu_background->GetMenuItemCount();
	for(int i = 0; i < numMenuItems; i++)
		_menu_background->Delete(Cycle_Background0+i);
}

void QuenchFrame::StartTimer()
{
	// start the timer to save state every 30 secs
	if(!_saveStateTimer->IsRunning())
		_saveStateTimer->Start(30000);
}

void QuenchFrame::AddBackgroundToMenu(const char* str_bg)
{
	unsigned i;
	for(i=0; i<_menu_background->GetMenuItemCount(); i++) 
		_menu_background->Check(Cycle_Background0+i,false);
	_menu_background->AppendRadioItem(Cycle_Background0+i, str_bg);
	_menu_background->Check(Cycle_Background0+i,true);
}

void QuenchFrame::SetDataLoadedMenuState(int state)
{

	bool pred1 = state > QuenchController::NOTHING_LOADED;
	bool pred2 = state > QuenchController::VOLUME_LOADED;
	
	_menu_file->Enable (Load_Pathways, pred1);

	_menu_file->Enable(Load_ROI_Nifti, pred1);

	_menu_view->Enable (Toggle_SagP, pred1);
	_menu_view->Enable (Toggle_CorP, pred1);
	_menu_view->Enable (Toggle_AxialP, pred1);
	_menu_view->Enable (Reset_View, pred1);
	_menu_view->Enable (Toggle_ROIs, pred1);
	_menu_view->Enable (Toggle_ROI_Bounding_Boxes, pred1);
	_menu_view->Enable (Toggle_ROI_Labels, pred1);

	_menu_panels->Enable (Overlay_Panel, pred1);
	_menu_panels->Enable (Refine_Selection_Panel, pred2);

	_menu_view->Enable (Toggle_Pathways, pred2);
	_menu_file->Enable(Save_Pathways_PDB, pred2);
	_menu_panels->Enable (Refine_Selection_Panel, pred2);
	_menu_panels->Enable (Show_ImagePreview, pred2);
	_menu_panels->Enable (Show_PerPointColor, pred2);
	_menu_view->Enable(Show_ColorChooser, pred2);
	_menu_view->Enable(Show_ColorMapChooser, pred2);

	_menu_edit->Enable(Select_All, pred2);
	_menu_edit->Enable(Deselect_All, pred2);
	_menu_edit->Enable(Toggle_Freeze_Assignment, pred2);
	
	_menu_roi->Enable(Draw_New_ROI, pred1);
}

void QuenchFrame::SetROISelectedMenuState(bool roiSelected)
{
  _menu_roi->Enable(Toggle_ROI_Editing_Mode, roiSelected);
  _menu_file->Enable(Save_ROI_Nifti, roiSelected);
  _menu_roi->Enable(ROI_Properties, roiSelected);
  _menu_roi->Enable(Delete_ROI, roiSelected && !_menu_roi->IsChecked(Toggle_ROI_Editing_Mode));
}

void QuenchFrame::SetROIEditingMenuState(bool roiEditing)
{
  _menu_roi->Enable(Delete_ROI, !roiEditing);
  _menu_roi->Enable(Increase_Brush_Size, roiEditing);
  _menu_roi->Enable(Decrease_Brush_Size, roiEditing);
  _menu_roi->Enable(Draw_New_ROI, !roiEditing);
  _menu_file->Enable(Load_ROI_Nifti, !roiEditing);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// VTK Window Management
///////////////////////////////////////////////////////////////////////////////////////////////////

void QuenchFrame::RefreshViz()
{
	_pSceneWindow->GetRenderWindow()->SetSwapBuffers (true);
	_pSceneWindow->Refresh();

	// Need this -- otherwise the near and far clipping planes may be
	// incorrect.
	Interactor()->Renderer()->ResetCameraClippingRange();

	// Update() causes an immediate redraw:
	// If I don't do this on Windows, the screen never gets redrawn during ROI
	// dragging.
	// If I do this on Linux/X11, the event handler gets behind processing mouse
	// drag events, and there is horrible latency.
	// Yield is supposed to let other events execute, but seems to do nothing.
	// What happens on Mac/Carbon?
	// What happens on GTK/Linux?

#ifdef __WXMSW__
	_pSceneWindow->Update();
#endif
	//this->_app->Yield();
}

void QuenchFrame::ToggleImagePlane(DTISceneActorID id, bool bVisibility) 
{ 
	// Tell interactor to handle this toggle image state request
	this->Interactor()->ToggleImages(id, bVisibility);
	
	// Set the menu item checekd state with the current visibility state
	this->UpdateImageVisibilityMenuState();
	
	// Refresh visualization
	this->RefreshViz(); 
}

void QuenchFrame::UpdateImageVisibilityMenuState()
{
	// Check each image plane to update the menus
	_menu_view->Check(Toggle_SagP,   this->Interactor()->VolumeViz()->Visibility(DTI_ACTOR_SAGITTAL_TOMO));
	_menu_view->Check(Toggle_CorP,   this->Interactor()->VolumeViz()->Visibility(DTI_ACTOR_CORONAL_TOMO));
	_menu_view->Check(Toggle_AxialP, this->Interactor()->VolumeViz()->Visibility(DTI_ACTOR_AXIAL_TOMO));
}

void QuenchFrame::UpdatePathwayVisibilityMenuState()
{
	_menu_view->Check(Toggle_Pathways, this->Interactor()->PathwayViz()->PathwayVisibility());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// wxEvent handling
///////////////////////////////////////////////////////////////////////////////////////////////////

void QuenchFrame::OnTimerEvent(wxTimerEvent &event)
{
	// save the state of the system
	char currDir[2048]; 
	char *unused = _getcwd(currDir,2048);
	sprintf(currDir,"%s/%s",currDir,TEMP_QST_FILE_NAME);
	bool saveROIs = false;
	_qCtrlr->SaveState(currDir, saveROIs);
}

void QuenchFrame::OnAbout(wxCommandEvent& event)
{
	AboutDlg aboutdlg(this);
	aboutdlg.ShowModal();
}

void QuenchFrame::OnMouseMoved(wxMouseEvent& event)
{
	if(this != FindFocus() && event.LeftIsDown())
		RefreshViz();
}

void QuenchFrame::OnLoadState(wxCommandEvent &event)
{
	// Load the state of the controller, basically the entire Quench system
	if(!DoFileDialog(_T("Choose a program-state file to load:"),_T("Quench State (*.qst)|*.qst"),0)) 
		return;
	_qCtrlr->LoadState(_file_dialog->GetPath().c_str());
}

void QuenchFrame::OnSaveState(wxCommandEvent& event) 
{ 
	// Save the state of the controller, basically the entire Quench system	
	DoSaveStateDialog(); 
}

void QuenchFrame::OnLoadVolume(wxCommandEvent &event)
{
	if(!DoFileDialog(_T("Choose a background image to load:"),_T("NIFTI (*.nii;*.nii.gz;*.hdr)|*.nii;*.nii.gz;*.hdr"),wxFD_OPEN))
		return;
	_qCtrlr->LoadVolume(_file_dialog->GetPath().c_str());
}

void QuenchFrame::OnLoadStandardVolumes(wxCommandEvent &event)
{
	wxDirDialog ddlg(this,wxDirSelectorPromptStr,_lastDirDlgPath);
	if(ddlg.ShowModal() == wxID_OK)
		_qCtrlr->LoadDefaultVolumes(ddlg.GetPath().c_str());
}

void QuenchFrame::OnLoadPathways(wxCommandEvent &event)
{
	if(!DoFileDialog(_T("Choose a pathway file to load:"),_T("Pathways (*.pdb)|*.pdb"),wxOPEN|wxMULTIPLE))
		return;

	int answer=wxNO;
	NButtonChoiceDialog afd(this, "Append or overwrite ?","\n\nDo you wish to append or overwrite fibers?\n","Append","Overwrite","Cancel");
	if(_qCtrlr->PDBHelper_().PDB())
		answer = afd.ShowModal();
	if(answer != BUTTON_CHOICE_3)
	{
		wxArrayString pathNames;
		_file_dialog->GetPaths(pathNames);
		_qCtrlr->LoadPathways(pathNames, answer==BUTTON_CHOICE_1);
	}
}

void QuenchFrame::OnSavePathwaysPDB(wxCommandEvent &event)
{
	if(!DoFileDialog(_T("Choose pathway database filename."),_T("Pathways (*.pdb)|*.pdb"),wxSAVE | wxOVERWRITE_PROMPT))
		return;
	//	cerr << _file_dialog->GetWildcard() << endl;

	DTIPathwaySelection selection = DTIPathwaySelection ::PruneHiddenFibers(&_qCtrlr->PDBHelper_().Assignment(), _qCtrlr->PathwayGroupArray_());
	
	_qCtrlr->PDBHelper_().Save(_file_dialog->GetPath().c_str(), selection);
}

void QuenchFrame::OnLoadROINifti(wxCommandEvent &event)
{
  // Later change this to handle multiple images.
  if(!DoFileDialog(_T("Choose NIFTI mask image to load:"),_T("NIFTI (*.nii;*.nii.gz;*.hdr)|*.nii;*.nii.gz;*.hdr"),wxMULTIPLE))
    return;
  wxArrayString pathNames;
  wxArrayString fileNames;
  _file_dialog->GetPaths(pathNames);
  _file_dialog->GetFilenames(fileNames);
  for (int i = 0; i < (int)pathNames.Count(); i++) {
    _qCtrlr->LoadROINifti(pathNames[i].c_str(), fileNames[i].c_str());
    
  }
  _menu_roi->Enable(Toggle_ROI_Editing_Mode, TRUE);
  _menu_roi->Enable(ROI_Properties, TRUE);
  _menu_roi->Enable(Delete_ROI, TRUE);
}

void QuenchFrame::OnSaveROINifti(wxCommandEvent &event)
{
  int result = _chdir("ROIs");
  if(!DoFileDialog(_T("Choose ROI filename."),_T("NIFTI (*.nii.gz)|*.nii.gz"),wxSAVE | wxOVERWRITE_PROMPT))
    return;
  if (result == 0) {
    // If we succeeded in changing the directory, change it back...
    _chdir("..");
  }
  std::string path = _file_dialog->GetPath().c_str();
  size_t foundExtensionNii = path.find(".nii");
  size_t foundExtensionHdr = path.find(".hdr");
  std::string fName = _file_dialog->GetFilename().c_str();
  if (foundExtensionNii == std::string::npos && foundExtensionHdr == std::string::npos)
    {
      path = path + ".nii.gz";
      fName = fName + ".nii.gz";
    }
  _qCtrlr->SaveCurrentROINifti(path, fName);

}

void QuenchFrame::OnCloseWindow(wxCloseEvent& event)
{
	NButtonChoiceDialog ed(this, "Exiting Quench...","\n\nDo you want to save state information?\n","Save","Don't Save","Cancel");
	bool bSaveDecisionMade = false;
	while (!bSaveDecisionMade)
	{
		int result = ed.ShowModal();
		switch(result)
		{
		case BUTTON_CHOICE_1: // Save
			if(!DoSaveStateDialog())
				continue;
			bSaveDecisionMade = true;
			remove(TEMP_QST_FILE_NAME);
			Destroy();
			break;
		case BUTTON_CHOICE_2: // Don't Save
			bSaveDecisionMade = true;
			Destroy();
			break;
		case BUTTON_CHOICE_3: // Cancel
			bSaveDecisionMade = true;
			event.Veto();
			break;
		}
	}
}

void QuenchFrame::OnQuit(wxCommandEvent& event) 
{ 
	Close(); 
}

void QuenchFrame::OnKeyboardShortcuts(wxCommandEvent& event)
{	
	wxMessageBox( _T("Mouse Shortcuts:\n\n  Left mouse - rotate\n  Right mouse - zoom\n  Shift + Left mouse - pan\n Ctrl + Left mouse - use selection tool\n\nKeyboard Shortcuts:\n\n  's', 'c', and 'a': select sagittal, coronal, or axial tomogram\n  ',' and '.': slide the current tomogram back and forth \n  'b': cycle background color \n  'e' Show/Hide move ROI tool\n  'r' Show/Hide scale ROI tool\n  'k': toggle anti-aliasing \n  '[':  cycle between camera positions \n  'w': toggle line widths \n '1'-'8': select a pathway group \n Shift + '1'-'8': toggle visibility of a pathway group\n '+' and '-': grow/shrink current selection (requires distance matrix)"),_T("Quench Keyboard/Mouse Shortcuts"),wxICON_INFORMATION);
}

void QuenchFrame::OnUndoCommand(wxCommandEvent &event)
{	
	this->SetUndoRedoMenuState(_qCtrlr->CommandManager_()->Undo(), _qCtrlr->CommandManager_()->size() > 0);	
}

void QuenchFrame::OnRedoCommand(wxCommandEvent &event)
{
	this->SetUndoRedoMenuState(_qCtrlr->CommandManager_()->size() > 0, _qCtrlr->CommandManager_()->Redo());	
}

void QuenchFrame::OnToggleFreezeAssignment(wxCommandEvent &event)
{
	_qCtrlr->ToggleAssignmentLock();
	this->RefreshViz();
}

void QuenchFrame::OnTogglePathways(wxCommandEvent &event)
{
	bool bNewVisibility = !this->Interactor()->PathwayViz()->PathwayVisibility();
	this->Interactor()->PathwayViz()->SetPathwayVisibility( bNewVisibility );
	this->UpdatePathwayVisibilityMenuState();
	this->RefreshViz();
}

void QuenchFrame::OnToggleSagP  (wxCommandEvent& event) 
{ 
	this->ToggleImagePlane(DTI_ACTOR_SAGITTAL_TOMO, TRUE); 
}

void QuenchFrame::OnToggleAxialP(wxCommandEvent& event) 
{ 
	this->ToggleImagePlane(DTI_ACTOR_AXIAL_TOMO, TRUE); 
}

void QuenchFrame::OnToggleCorP  (wxCommandEvent& event) 
{ 
	this->ToggleImagePlane(DTI_ACTOR_CORONAL_TOMO, TRUE); 
}

void QuenchFrame::OnShowColorChooser(wxCommandEvent &event)
{
	wxColourDialog colordlg(this,0);

	if(colordlg.ShowModal() != wxID_OK)
		return; // color wasn't changed

	wxColour color = colordlg.GetColourData().GetColour();
	Colord col( color.Red()/255., color.Green()/255., color.Blue()/255. );

	// set the new color
	_qCtrlr->PathwayGroupArray_()[ _qCtrlr->PDBHelper_().Assignment().SelectedGroup() ].SetColor( col );

	// update 3d data and menus
	_istyle->PathwayViz()->InvalidateColorCache();
	_qCtrlr->UpdatePathwayViz(false);
	this->RefreshViz();

	// add event to history?
}

void QuenchFrame::OnShowColorMapChooser(wxCommandEvent &event)
{
	_color_map_panel->ShowWithListener(_qCtrlr);	
	// This will trigger EVENT_COLORMAP_CHANGED handled in QuenchController::OnEvent 
}

void QuenchFrame::OnResetView(wxCommandEvent& event) 
{ 	
	_istyle->ResetView(); 
}

void QuenchFrame::OnResetPathwayColors(wxCommandEvent &event)
{
	_qCtrlr->PathwayGroupArray_().ResetColors(false);
	_qCtrlr->UpdatePathwayViz(false);
	this->RefreshViz();
}

void QuenchFrame::OnSelectAll(wxCommandEvent &event)
{
	_qCtrlr->SelectAllPathways();
}

void QuenchFrame::OnDeselectAll(wxCommandEvent &event)
{
	_qCtrlr->DeselectAllPathways();
}

void QuenchFrame::OnCycleBackground(wxCommandEvent &event)
{
	int nItems = (int)_menu_background->GetMenuItemCount();
	//assert(nItems==_subjectData.size());
	for(int i=0; i<nItems; i++) {
		if(_menu_background->IsChecked(Cycle_Background0+i)) 
		{
			_qCtrlr->SetBackgroundVolume(i);
			(*dynamic_cast<VPOverlayItem*>(_overlay_panel))[i]->OnSelected();
			break;
		}
	}
}

void QuenchFrame::OnRefineSelectionPanelToggle(wxCommandEvent &event)
{
	if(!_stats_panel->IsVisible())//About to be shown hence recompute the dynamic assigment
		_qCtrlr->FilterPathwaysByStats();
	else //Save the result
		_qCtrlr->PDBHelper_().AcceptFilterOperation(true);

	_stats_panel->Refresh();
	_stats_panel->Show( !_stats_panel->IsVisible() );

	// xxx dakers
	// Why were we toggling off the text info visibility here? The scientists
	// thought this was a bug, when in fact it appears to have been a purposeful
	// "feature"?

	//_istyle->PathwayViz()->VisibilityPanel_()->SetTextInfoVisibility( !_stats_panel->IsVisible() );
	this->RefreshViz();
}

void QuenchFrame::OnOverlayPanelToggle(wxCommandEvent &event)
{ 
	_overlay_panel->Show(!_overlay_panel->IsVisible()); 
}

void QuenchFrame::OnImagePreviewToggle(wxCommandEvent& event)
{ 
	_imagePreview->Show(!_imagePreview->IsVisible()); 
}

void QuenchFrame::OnPerPointColorPanelToggle(wxCommandEvent& event)
{ 
	_perPointColor->Show(!_perPointColor->IsVisible()); 
}
    
///////////////////////////////////////////////////////////////////////////////////////////////////
// Reusable quick GUI components
///////////////////////////////////////////////////////////////////////////////////////////////////

bool QuenchFrame::DoFileDialog(const char *caption, const char *file_type, int flags)
{
	if(_file_dialog)delete _file_dialog;

	// create a file dialog
	_file_dialog = new wxFileDialog(this,caption,"",_T(""),file_type,flags,wxDefaultPosition);
	bool ret = _file_dialog->ShowModal()==wxID_OK;
	if(!ret)
	{
		delete _file_dialog;
		_file_dialog=0;
	}
	return ret;
}

bool QuenchFrame::DoSaveStateDialog()
{
	if(!DoFileDialog(_T("Choose state filename."),_T("Quench State (*.qst)|*.qst"),wxSAVE | wxOVERWRITE_PROMPT))
		return false;
	std::string path = _file_dialog->GetPath().c_str();
	size_t foundExtensionQst = path.find(".qst");

	// If they forgot to include '.qst' in the filename, add it for them:
	if (foundExtensionQst == std::string::npos)
	  {
	    path = path + ".qst";
	  }
	_qCtrlr->SaveState(path);
	return true;
}

void QuenchFrame::OnROIProperties(wxCommandEvent &event)
{
  _qCtrlr->OnROIProperties();
}

void QuenchFrame::OnDrawNewROI(wxCommandEvent &event)
{
  _qCtrlr->DrawNewROI();
  _menu_roi->Check(Toggle_ROI_Editing_Mode, TRUE);
  
  bool roiEditing = true;
  SetROIEditingMenuState(roiEditing);

  bool roiSelected = true;
  this->SetROISelectedMenuState(roiSelected);

}

void QuenchFrame::OnToggleROIEditingMode(wxCommandEvent &event)
{
  if (_menu_roi->IsChecked(Toggle_ROI_Editing_Mode)) {
    SetROIEditingMenuState(true);
    _qCtrlr->ROIEditModeOn();
  }
  else {
    SetROIEditingMenuState(false);
    _qCtrlr->ROIEditModeOff();
  }
}

void QuenchFrame::OnIncreaseBrushSize(wxCommandEvent &event)
{
  _qCtrlr->IncreaseBrushSize();
}

void QuenchFrame::OnDecreaseBrushSize(wxCommandEvent &event)
{
  _qCtrlr->DecreaseBrushSize();
}

void QuenchFrame::OnToggleROIs(wxCommandEvent &event)
{
  bool isChecked = _menu_view->IsChecked(Toggle_ROIs);
  _qCtrlr->SetROIVisibility(isChecked);
  _menu_view->Enable(Toggle_ROI_Bounding_Boxes, isChecked);
  _menu_view->Enable(Toggle_ROI_Labels, isChecked);
}

void QuenchFrame::OnToggleROIBoundingBoxes(wxCommandEvent &event)
{
  _qCtrlr->SetROIBoundingBoxVisibility(_menu_view->IsChecked(Toggle_ROI_Bounding_Boxes));
}

void QuenchFrame::OnToggleROILabels(wxCommandEvent &event)
{
  _qCtrlr->SetROILabelVisibility(_menu_view->IsChecked(Toggle_ROI_Labels));
}

void QuenchFrame::OnDeleteROI(wxCommandEvent &event)
{
  _qCtrlr->DeleteCurrentROI();
}
