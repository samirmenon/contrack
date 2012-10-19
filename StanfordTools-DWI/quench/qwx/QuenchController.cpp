#include "QuenchController.h"
#include "QuenchFrame.h"
#include "vtkInteractorStyleQuench.h"
#include "qVolumeViz.h"
#include "qPathwayViz.h"
#include "VisibilityPanel.h"
#include "qROIViz.h"
#include "StatsPanel.h"
#include "PerPointColor.h"
#include "OverlayPanel.h"
#include "SceneWindow.h"
#include "StatsCheckboxArray.h"
#include "History.h"
#include "DTIPathwayDatabase.h"
#include "RangeSlider.h"
#include "vtkROI.h"
#include "ColorMapPanel.h"
#include "Mesh.h"
#include "DTIVolumeIO.h"
#include "MeshBuilder.h"
#include "qROIEditor.h"
#include "ROIPropertiesDialog.h"
#include "vtkROIFactory.h"
#include "SystemInfoDialog.h"
#include "NButtonChoiceDialog.h"

using namespace std;

QuenchController::QuenchController()
{
  _voi_bounding_box_visibility = true;
  _voi_label_visibility = true;
  _voi_visibility = true;

	_mesh = 0;
	_currBkImage = 0;
	
	// Once QuenchFrame has created all the GUI elements, the frame object will call ConnectControllers
	// to pass so that we can listen for all GUI element data manipulation events
	_qFrame = 0;
	
	_commandMgr = new CommandManager();
}

QuenchController::~QuenchController()
{
	delete _commandMgr;
	//Delete DataModel
	//_voiMgr.clear();
}

void QuenchController::ConnectControllers(QuenchFrame* qFrame)
{
	_qFrame = qFrame;
	
	//IEventListener* thectrlr = (IEventListener*)_qCtrlr;
	_qFrame->Interactor()->push_back(this);
	_qFrame->Interactor()->VolumeViz ()->push_back(this);
	_qFrame->Interactor()->PathwayViz()->push_back(this);
	_qFrame->Interactor()->PathwayViz()->push_back(_qFrame->MainWindow());
	_qFrame->Interactor()->PathwayViz()->VisibilityPanel_()->push_back(this);
	_qFrame->Interactor()->ROIViz()->push_back(this);
	_qFrame->StatsPanel_()->push_back(this);
	_qFrame->StatsPanel_()->ROIPanel_()->push_back(this);
	_qFrame->PerPointColor_()->push_back( this );
	((IEventSource*)_qFrame->OverlayPanel_())->push_back( this );

	_qFrame->Interactor()->voiEditor()->push_back(this);
// XXX Thinking about these	
// qwx/OverlayItem.cpp:    cmp->push_back(this);
// qwx/OverlayPanel.cpp:   oi->push_back(this);
// qwx/OverlayPanel.cpp:   oi->push_back(o);
// qwx/PerPointColor.cpp:  cmp->push_back(this);
// qwx/StatsPanel.cpp:     _cbArray->push_back(this);

	// Now that the GUI is hooked up, lets initialize data loaded state to no data loaded
	this->SetDataLoadedState(NOTHING_LOADED, true);
}

void QuenchController::SetDataLoadedState(int state, bool bInit)
{
	// Don't do anything if this is not initialization and the state hasn't changed
	if(!bInit && _state_data_loaded >= state) return;

	switch(state) 
	{
	case NOTHING_LOADED:
		_qFrame->OverlayPanel_()->Hide();
		_qFrame->StatsPanel_()->Hide();
		break;
	case VOLUME_LOADED:
		Vector3d min, max;
		_qFrame->Interactor()->VolumeViz()->ComputeCurrentVolumeBounds(min, max);
		_qFrame->StatsPanel_()->ROIPanel_()->SetROIBounds(min,max);
		_qFrame->Interactor()->ROIViz()->SetROIBounds(min,max);
		break;
	}
	
	if (_qFrame)
		_qFrame->SetDataLoadedMenuState(state);

	_state_data_loaded=state;
}

void QuenchController::OnEvent(PEvent evt)
{
	switch(evt->EventType())
	{
	case SELECTION_MODE_CHANGED:
	  {
	    // Added by dakers, 8/17/11 to fix a bug when moving away from
	    // surface intersection mode to touch mode. (pathway colors
	    // were failing to update).
	    _qFrame->Interactor()->PathwayViz()->UpdatePathwaysColor(_pdbHelper, _groupArray);
	  }
	  break;
	case PATHWAY_GROUP_VISIBILITY_CHANGED:
		{
			EventPathwayGroupVisibilityChanged* e = dynamic_cast<EventPathwayGroupVisibilityChanged*>(evt.get());
			
			// is the pathway id within bounds?
			if(e->Value() >= (int)_groupArray.size())
				return;
				
			// toggle the visibility of the group
			bool vis = !_groupArray[e->Value()].Visible();
			if(e->Value() != _pdbHelper.Assignment().SelectedGroup())
				_groupArray[e->Value()].SetVisible( vis );

			// update the 3d pathway data
			this->UpdatePathwayViz(true);
		}
		break;

	case PATHWAY_GROUP_SELECTED:
		{
			EventPathwayGroupSelected* e = dynamic_cast<EventPathwayGroupSelected*>(evt.get());

			// is the pathway id within bounds?
			if(e->Value() >= (int)_groupArray.size())
				return;

			// if same group is selected, then toggle visibility	
			bool vis = true;
			if(e->Value() == _pdbHelper.Assignment().SelectedGroup()) 
				vis = !_groupArray[e->Value()].Visible();
			_groupArray[e->Value()].SetVisible( vis );
			_pdbHelper.Assignment().SetSelectedGroup( e->Value() );
			
			this->UpdatePathwayViz(true);
		}
		break;

	case PATHWAY_GROUP_ADDED:
		{
			// add this event to history
			this->PushToHistory( PCommand(new CommandFiberGroupsChanged(this) ));

			// chose a default name for the newly added fibergroup
			char name[100]; sprintf(name, "FG %d",(int)_groupArray.size());
			_groupArray.Add( name );
			
			// update 3d data and menus
			this->UpdatePathwayViz(false);
			//_istyle->PathwayViz()->VisibilityPanel_()->Update(_groupArray, _pdbHelper.Assignment());
			//_stats_panel->StatsCheckboxArray_()->Update(&_pdbHelper.Assignment(), &_groupArray);
		}
		break;
	case PATHWAY_GROUP_DELETED:
		{
			// add this event to history
			this->PushToHistory( PCommand(new CommandFiberGroupsChanged(this) ));
			
			// move the fibers in the selected fiber group to trash
			_pdbHelper.MoveToTrash(_groupArray.size()-1);

			// if this is the very last group, select a new group to remain in bounds
			int newGroup = 0;
			if(_pdbHelper.Assignment().SelectedGroup() == _groupArray.size() - 1)
				_pdbHelper.Assignment().SetSelectedGroup(newGroup);

			// remove the group
			_groupArray.pop_back();

			// update 3d data and menus
			this->UpdatePathwayViz(false);
		}
		break;
	case TOGGLE_ASSIGNMENTS_LOCKED:
		{
			this->ToggleAssignmentLock();
		}
		break;

	case FILTER_PATHWAYS_BY_STATS:
		{
			this->FilterPathwaysByStats();
		}
		break;

	case FILTER_PATHWAYS_BY_GESTURE:
		{
		// add this event to history
		PushToHistory( PCommand(new CommandAssignmentChangedByGesture(this) )); 	

		EventFilterPathwaysByGesture* e = dynamic_cast<EventFilterPathwaysByGesture*>(evt.get());
		// create a voi from the RAPID model
		DTIFilterROI voi(ROI_TYPE_GESTURE, e->Value());

		BOOLEAN_OPERATION op = _qFrame->Interactor()->PathwayViz()->FilterOperation();
		DTIPathwaySelection selection = DTIPathwaySelection :: PruneForGesture (op, &_pdbHelper.Assignment(), _groupArray);
		
		// filter the pathways using this gesture
		_voiMgr.Filter(selection,_pdbHelper, &voi);
		
		// assign the fibers which have passed the test to the selected group or trash depending on the boolean operation 
		_pdbHelper.Assignment().AssignSelectedToGroup(selection, op != SUBTRACT? _pdbHelper.Assignment().SelectedGroup() : DTI_COLOR_UNASSIGNED, op != UNION);

		// update 3d data
		UpdatePathwayViz(false);

		//Save the assignment
		_pdbHelper.AcceptFilterOperation(true);
		}
		break;

	case ACCEPT_FILTER_PATHWAYS_BY_STATS:
		// add this event to history
		PushToHistory( PCommand(new CommandAssignmentChangedByStats(this) )); 
		
		// make the assignment
		_pdbHelper.AcceptFilterOperation(true);
		
		// update 3d data and menus
		this->UpdatePathwayViz(false);
		_qFrame->SetRefineSelectionMenuState(_qFrame->StatsPanel_()->IsVisible());
		
		break;

	case UPDATE_SURFACE_MODE_GEOMETRY:
		{
			this->UpdateSurfaceModeGeometry();
		}
		break;
	case SHOW_PERPOINTCOLOR_PANEL:
		{
			EventShowPerPointColorPanel &event_info = *dynamic_cast<EventShowPerPointColorPanel*>(evt.get());
			_qFrame->Interactor()->PathwayViz()->BeginPerPointColoringMode(_pdbHelper, _groupArray, event_info);
		}
		break;
	case HIDE_PERPOINTCOLOR_PANEL:
		{
			_qFrame->Interactor()->PathwayViz()->EndPerPointColoringMode();
			this->UpdatePathwayViz(false);
		}
		break;

	case ROI_CREATE_CUBE:
		{
			PDTIFilterROI voi(vtkROI::CreateCube(_qFrame->Interactor()->Renderer(), _voiMgr.NextID(), _qFrame->Interactor()->VolumeViz()->Position()));
			_voiMgr.push_back(voi);
			_qFrame->StatsPanel_()->ROIPanel_()->Update(_voiMgr);
			_qFrame->Interactor()->ROIViz()->Update(_voiMgr);
		}
		break;
	case ROI_CREATE_SPHERE:
		{
			PDTIFilterROI voi(vtkROI::CreateSphere(_qFrame->Interactor()->Renderer(), _voiMgr.NextID(), _qFrame->Interactor()->VolumeViz()->Position()));
			_voiMgr.push_back(voi);
			_qFrame->StatsPanel_()->ROIPanel_()->Update(_voiMgr);
			_qFrame->Interactor()->ROIViz()->Update(_voiMgr);
		}
		break;
	case ROI_CREATE_MESH:
		break;
	case ROI_DELETE:
	  {
	    DeleteCurrentROI();
	  }
	  break;
	case ROI_SELECT_BY_NAME:
		{
			EventROISelectByName* e = dynamic_cast<EventROISelectByName*>(evt.get());
			_voiMgr.SetSelected(e->Value());

			//			_qFrame->Interactor()->HighlightProp( ((vtkROI*)_voiMgr.Selected().get())->Actor());
			UpdateROIBoundingBoxDisplay();
			_qFrame->StatsPanel_()->ROIPanel_()->Update(_voiMgr);
			_qFrame->Interactor()->ROIViz()->Update(_voiMgr);
			_qFrame->SetROISelectedMenuState(_voiMgr.Selected() != NULL);
		}
		break;
	case ROI_SELECT_BY_ACTOR:
		{
			// event fired when we select a voi by clicking on it.
			EventROISelectByActor* e = dynamic_cast<EventROISelectByActor*>(evt.get());
			vtkActor *actor = e->Value();

			// see if a new voi was selected
			if( ((vtkROI*) (_voiMgr.Selected().get()) )->Actor() != actor)
			{
				// foreach voi
				for(PDTIFilterROIList::iterator iter = _voiMgr.begin(); iter != _voiMgr.end(); iter++)
					// is this the one selected?
					if(actor== (  ((vtkROI*) (*iter).get())->Actor() ) )
					{
						// update 3d data and panels
						_voiMgr.SetSelected( (*iter)->Name());
						//						_qFrame->Interactor()->HighlightProp( actor );
						UpdateROIBoundingBoxDisplay();
						_qFrame->StatsPanel_()->ROIPanel_()->Update(_voiMgr);
						_qFrame->Interactor()->ROIViz()->Update(_voiMgr);
						break;
					}
			}
			_qFrame->SetROISelectedMenuState(_voiMgr.Selected() != NULL);
		}
		break;
	case ROI_NAME_CHANGED:
		{
			EventROINameChanged* e = dynamic_cast<EventROINameChanged*>(evt.get());
			( (vtkROI*)(_voiMgr.Selected().get()) )->SetName( e->Value() );
		}
		break;
	case ROI_TRANSLATE:
		{
			EventROITranslate* e = dynamic_cast<EventROITranslate*>(evt.get());
			//Check to see if there is a ROI
			if(_voiMgr.Selected())
			{
				// update 3d data and menus
				_voiMgr.Selected()->SetPosition(e->Value());
				_qFrame->StatsPanel_()->ROIPanel_()->Update(_voiMgr);
				_qFrame->Interactor()->ROIViz()->Update(_voiMgr);
				//				_qFrame->Interactor()->HighlightProp( ((vtkROI*)_voiMgr.Selected().get())->Actor());
				UpdateROIBoundingBoxDisplay();
			
				//Recompute fiber stats only if the stats panel is visible, other wise just play with the ROI
				if(_qFrame->StatsPanel_()->IsVisible())
					this->FilterPathwaysByStats();
			}
		}
		break;
	case ROI_SCALE:
		{
			//Check to see if there is a ROI
			if(_voiMgr.Selected())
			{
				EventROIScale* e = dynamic_cast<EventROIScale*>(evt.get());

				// update 3d data and menus
				_voiMgr.Selected()->SetScale(e->Value());
				_qFrame->StatsPanel_()->ROIPanel_()->Update(_voiMgr);
				_qFrame->Interactor()->ROIViz()->Update(_voiMgr);
				//				_qFrame->Interactor()->HighlightProp( ((vtkROI*)_voiMgr.Selected().get())->Actor());
				UpdateROIBoundingBoxDisplay();

				//Recompute fiber stats only if the stats panel is visible, other wise just play with the ROI
				if(_qFrame->StatsPanel_()->IsVisible())
					this->FilterPathwaysByStats();
			}
		}
		break;
	case ROI_COLOR:
			((vtkROI*)(_voiMgr.Selected().get()))  ->  SetColor( ((EventROIColor*)(evt.get()))->Value() );
		break;
		/*
	case ROI_SHOW:
			((vtkROI*)(_voiMgr.Selected().get()))  ->  SetVisible( ((EventROIShow*)(evt.get()))->Value() );
		break;

	case ROI_SHOW_ALL:
			for(PDTIFilterROIList::iterator iter = _voiMgr.begin(); iter != _voiMgr.end(); iter++)
				((vtkROI*)((*iter).get()))  ->  SetShowAll( ((EventROIShowAll*)(evt.get()))->Value() );
		break;

	case ROI_SHOW_LABEL:
			for(PDTIFilterROIList::iterator iter = _voiMgr.begin(); iter != _voiMgr.end(); iter++)
				((vtkROI*)((*iter).get()))  ->  SetShowLabel( ((EventROIShowLabel*)(evt.get()))->Value() );
				break;*/
	case ROI_QUERY_OP_CHANGED:
		{
			EventROIQueryOpChanged* e = dynamic_cast<EventROIQueryOpChanged*>(evt.get());
			DTIQueryOperation op = (DTIQueryOperation)e->Value();
			_voiMgr.SetQueryOperation(op);
			this->FilterPathwaysByStats();
		}
		break;

	case SHOW_OVERLAY_PANEL:
		{
			wxCommandEvent e;
			_qFrame->OnOverlayPanelToggle(e);
		}
		break;
	case SHOW_REFINE_SELECTION_PANEL:
		{
			_qFrame->StatsPanel_()->ShowPage(SHOW_REFINE_SELECTION_PANEL);
			wxCommandEvent e;
			_qFrame->OnRefineSelectionPanelToggle(e);
		}
		break;
	case SHOW_ROI_PANEL:
		{
			_qFrame->StatsPanel_()->ShowPage(SHOW_ROI_PANEL);
			wxCommandEvent e;
			_qFrame->OnRefineSelectionPanelToggle(e);
		}
		break;
	case COLOR_MAP_CHANGED:
		{
			EventColorMapChanged *e = dynamic_cast<EventColorMapChanged*>(evt.get());
			// get the new color map index
			int index = e->Value();
			int numGroups = 0;

			//Find the max groupId which has an assignment to it
			DTIPathwayAssignment &assn = _pdbHelper.Assignment();
			for(int i = 0; i < (int)assn.size(); i++)
				if( assn[i]>numGroups )
					numGroups= assn[i];

			for(int i = 1; i <= numGroups; i++)
			{
				size_t num_colors = _qFrame->ColorMapPanel_()->ColorMaps() [index].Colors.size()-1;
				Coloruc &col = _qFrame->ColorMapPanel_()->ColorMaps() [index].Colors[ i*num_colors / numGroups ];
				Colord c(col.r/255., col.g/255., col.b/255.);
				_groupArray[i].SetColor(c);
			}

			// update 3d data and menus
			_qFrame->Interactor()->PathwayViz()->InvalidateColorCache();
			UpdatePathwayViz(false);
		}
		break;
			
	case TOGGLE_IMAGE_PLANE:
		{
			EventToggleImagePlane *e = dynamic_cast<EventToggleImagePlane*>(evt.get());
			_qFrame->ToggleImagePlane(e->IDPlane(), e->Visibility());
			_qFrame->Interactor()->voiEditor()->UpdateStatusDisplay();
		}
		break;


	case TOGGLE_MESH_VISIBILITY:
		if(_mesh) _mesh->SetVisible( !_mesh->Visible());
		break;
	case UPDATE_MESH:
		this->UpdateMesh();
		break;
	case SET_BACKGROUND:
		{
			EventSetBackground *e = dynamic_cast<EventSetBackground*>(evt.get());
			// get the new color map index
			int index = e->Value();
			this->SetBackgroundVolume(index);
		}
		break;	
	case UPDATE_BACKGROUND:
		this->UpdateBackground();
		break;

	case ROI_DRAW_NEW_ROI:
	  this->DrawNewROI();
	  break;
	case ROI_EDIT_MODE_ON:
	  this->ROIEditModeOn();
	  break;
	case ROI_EDIT_MODE_OFF:
	  this->ROIEditModeOff();
	  break;
	case ROI_EDITED:
	  this->ROIEdited();
	  break;
	case UPDATE_VISIBILITY_PANEL:
	  //	  cerr << "updating vis panel" << endl;
	  _qFrame->Interactor()->PathwayViz()->VisibilityPanel_()->Update(_groupArray, _pdbHelper.Assignment());
	  break;
	case REFRESH_VIZ:
	  // do nothing.
	  break;
	}
	_qFrame->RefreshViz();
}

void QuenchController::UpdateSurfaceModeGeometry()
{
	//Set the bounds rectangle
	double pts[4][3], normal[4];
	_qFrame->Interactor()->VolumeViz()->ActiveImageExtents(pts, normal);
	_qFrame->Interactor()->PathwayViz()->SetActiveImageExtents(pts, normal);

	if(_qFrame->Interactor()->PathwayViz()->Mode() == qPathwayViz::SURFACE_MODE)
	{
		//Hide other surfaces
		for(int i = 0; i < 3; i++)
			_qFrame->Interactor()->VolumeViz()->SetVisibility((DTISceneActorID)i, i==_qFrame->Interactor()->VolumeViz()->ActiveImage());
		_qFrame->Interactor()->PathwayViz()->SetIntersectionGeometry(_pdbHelper, _groupArray);
	}
	
	// Update menu states
	_qFrame->UpdateImageVisibilityMenuState();
}

void QuenchController::FilterPathwaysByStats()
{
	//Sync values from the Refine Selection panel back to 
	for(int i = 0; i < _pdbHelper.PDB()->getNumPathStatistics(); i++)
	{
		_pdbHelper.PDB()->getPathStatistic(i)->_left  = _qFrame->StatsPanel_()->RangeSliders()[i]->Left ();
		_pdbHelper.PDB()->getPathStatistic(i)->_right = _qFrame->StatsPanel_()->RangeSliders()[i]->Right();
	}
	
	//Restore to the original state
	_pdbHelper.AcceptFilterOperation(false);

	// Create the selection
	DTIPathwaySelection selection = DTIPathwaySelection::PruneForStatistics(&_pdbHelper.Assignment(), _groupArray);

	// Filter based on statistics
	_pdbHelper.Filter(selection);

	//Filter based on the ROI's
	_voiMgr.Filter(selection,_pdbHelper);

	// Apply the results
	_pdbHelper.Assignment().AssignSelectedToGroup(selection, _pdbHelper.Assignment().SelectedGroup() );
	this->UpdatePathwayViz(false);
	_qFrame->StatsPanel_()->Update(_pdbHelper, _groupArray);
}

void QuenchController::ToggleAssignmentLock()
{
	bool locked = !_pdbHelper.Assignment().Locked();
	_pdbHelper.Assignment().SetLocked(locked);

	// update the menu
	_qFrame->SetFreezeAssignmentMenuState(locked);

	// if the stats panel is visible, do the filtering again, otherwise just update the text
	if(_qFrame->StatsPanel_()->IsVisible())
		this->FilterPathwaysByStats();
	else  
		this->UpdatePathwayViz(false); //_qFrame->Interactor()->PathwayViz()->VisibilityPanel_()->Update(_groupArray, _pdbHelper.Assignment());
}

void QuenchController::SelectAllPathways()
{
	DTIPathwaySelection selection = DTIPathwaySelection :: PruneForGesture (SELECT_ALL, &_pdbHelper.Assignment(), _groupArray);
	_pdbHelper.Assignment().AssignSelectedToGroup(selection, _pdbHelper.Assignment().SelectedGroup(), false);
	this->UpdatePathwayViz(false);
	_qFrame->RefreshViz();
}

void QuenchController::DeselectAllPathways()
{
	DTIPathwaySelection selection = DTIPathwaySelection :: PruneForGesture (DESELECT_ALL, &_pdbHelper.Assignment(), _groupArray);
	_pdbHelper.Assignment().AssignSelectedToGroup(selection, 0);
	this->UpdatePathwayViz(false);
	_qFrame->RefreshViz();
}

void QuenchController::SetBackgroundVolume(int i)
{
	_currBkImage = i;
	if(_qFrame->Interactor()->VolumeViz()->NumOverlays() > 0)
	{
		float left=0,right=1,min,max;
		_qFrame->Interactor()->VolumeViz()->GetOverlay(i)->Range(left,right,min,max);
		_qFrame->Interactor()->VolumeViz()->SetVolume(_subjectData[i].Volume().get(), left, right); 
		_qFrame->OverlayPanel_()->SetBackgroundVolume(i);
	}
	_qFrame->Interactor()->Renderer()->ResetCameraClippingRange();
	_qFrame->RefreshViz();
	_qFrame->Interactor()->voiEditor()->UpdateStatusDisplay();
}

void QuenchController::LoadVolume(string path)
{
	// convert to absolute path
	path = AbsolutePath(path);

	// try loading volume
	if(_subjectData.LoadVolume(path))
	{
		this->AddVolumeToDisplay(_subjectData[ _subjectData.size()-1 ]);
		this->AddVolumeToPanel(_subjectData[ _subjectData.size()-1 ]);
	}
}

void QuenchController::LoadROINifti(string path, string filename)
{
  // convert to absolute path
  path = AbsolutePath(path);
  DTIScalarVolume *v = DTIVolumeIO::loadScalarVolumeNifti(path.c_str());
  vtkPolyData *pData = MeshBuilder::ImageToMesh(v);
  vtkROI *newMeshROI = vtkROI::CreateMesh(_qFrame->Interactor()->Renderer(), ROIManager_().NextID(), Vector3d(0,0,0), pData, filename);
  newMeshROI->SetShowROI(_voi_visibility);
  newMeshROI->SetShowLabel(_voi_label_visibility);
  newMeshROI->SetImageRepresentation(v);
  newMeshROI->SetFileLocation(RelativePath(path));
  AddNewROI(newMeshROI);
  UpdateROIBoundingBoxDisplay();
}

void QuenchController::LoadDefaultVolumes(string path)
{
	//Remove an extra '/' if it exists
	int pathlen = (int)path.length();
	if(path[pathlen-1] == '/' || path[pathlen-1] == '\\')
		path.resize(pathlen-1);

	//load as many as possible
	for(int i = 0; i < 4; i++)
		if(_access((path+SubjectData::_vStdFiles[i]).c_str(),00) != -1)
			this->LoadVolume(path+SubjectData::_vStdFiles[i]);
}

void QuenchController::AddVolumeToDisplay(VolumeInfo &vol_info)
{
	//Add to volume viz interactor style
	_qFrame->Interactor()->VolumeViz()->AddVolume(vol_info.Volume().get(), _qFrame->ColorMapPanel_());
	this->UpdateSurfaceModeGeometry();
	_qFrame->RefreshViz();
}

void QuenchController::AddVolumeToPanel(VolumeInfo &vol_info)
{
	//Add to the menu and select it
	_qFrame->AddBackgroundToMenu(vol_info.Name().c_str());
	//Add to overlay panel
	_qFrame->OverlayPanel_()->AddVolume(vol_info,_qFrame->ColorMapPanel_());

	this->SetDataLoadedState(VOLUME_LOADED);
}

void QuenchController::LoadPathways(wxArrayString pathNames, bool _bAppend)
{
	// clear the history
	_commandMgr->clear();

	// for each pathway name in the list
	for(int i = 0; i < (int)pathNames.Count(); i++)
	{
		bool bAppend = i==0?_bAppend:true;
		// if we are not appending remove old data
		if(bAppend == false)
		{
			_qFrame->StatsPanel_()->Clear();
			_qFrame->PerPointColor_()->Clear();
		}
		
		DTIPathwayGroupID groupID;
		// check if this a path or is this filename
		if(pathNames[i].size() > 10000 || _access(pathNames[i].c_str(),00)==-1 ) // assuming that fiber groups are generally this big or file doesnt exist
		{
			// create a string stream from the params
			istringstream str(string( pathNames[i].c_str(), pathNames[i].size()),ios::binary);
			istringstream distanceStream; 
			distanceStream.setstate(ios_base::badbit);

			// load the pathway and save to disk
			groupID = _pdbHelper.Add(str, distanceStream, bAppend, true);
		}
		else
		{
			// load the path
			string fullpath = AbsolutePath(pathNames[i].mb_str());
			groupID = _pdbHelper.Add(fullpath, bAppend);

			//Add the groups to the group array
			string groupName = PathToFileName(pathNames[i].mb_str());

			// did we allocate a new group for this pathway
			if( groupID <= (int)_groupArray.size() )
				_groupArray[groupID].SetName(groupName);
			else
				_groupArray.Add(groupName);
		}
		
		// set the screen title
		string strTitle = (string("Quench - ")+ _groupArray[groupID].Name());
		_qFrame->SetTitle (strTitle.c_str());
		this->SetDataLoadedState(PATHWAYS_LOADED);
		_qFrame->StatsPanel_()->Update(_pdbHelper, _groupArray);
		if(bAppend == false)
			this->AddStatsToPerPointPanel();

		if(_qFrame->StatsPanel_()->IsVisible() == false)
			_qFrame->Interactor()->PathwayViz()->VisibilityPanel_()->SetTextInfoVisibility(true);
	}

	//Add them to the Pathway Viz
	_qFrame->Interactor()->PathwayViz()->GeneratePathways(_pdbHelper,_groupArray);
	_qFrame->UpdatePathwayVisibilityMenuState();
	_qFrame->StartTimer();
	_qFrame->Interactor()->Renderer()->ResetCameraClippingRange();
	_qFrame->Refresh();
}

void QuenchController::LoadPathway(std::string path, bool bAppend) 
{ 
	wxArrayString a; 
	wxString str(path.size(), 'a');
	memcpy((void*)str.mb_str(), path.c_str(), sizeof(char)*path.size());
	a.Add(str);
	str.size();
	this->LoadPathways(a,bAppend); 
}

void QuenchController::LoadMesh(std::string path)
{
	_mesh = new Mesh(path.c_str(), _qFrame->Interactor()->Renderer());
}

void QuenchController::AddStatsToPerPointPanel()
{
	DTIPathwayDatabase *pdb = _pdbHelper.PDB().get();
	for(int i = 0; i < pdb->getNumPathStatistics(); i++)
	{
		DTIPathwayStatisticHeader* stat_header = pdb->getPathStatisticHeader(i);
		if(stat_header->_is_computed_per_point)
		{
			DTIPathwayStatisticThreshold *stat_threshold = pdb->getPathStatistic(i);
			_qFrame->PerPointColor_()->AddStatistic(*stat_header, *stat_threshold);
		}
	}
}

void QuenchController::SwapAssignment(DTIPathwayAssignment &assn)
{
	// used by history
	DTIPathwayAssignment temp_assn = _pdbHelper.Assignment();
	_pdbHelper.SetAssignment(assn);
	assn = temp_assn;
	this->UpdatePathwayViz(false);
	_qFrame->RefreshViz();
}

void QuenchController::SwapGroupArray(PathwayGroupArray &pgarray)
{
	// used by history
	std::swap(_groupArray, pgarray);
	//_qFrame->Interactor()->PathwayViz()->VisibilityPanel_()->Update(_groupArray, _pdbHelper.Assignment());
	//_qFrame->StatsPanel_()->StatsCheckboxArray_()->Update(&_pdbHelper.Assignment(), &_groupArray);
	this->UpdatePathwayViz(false);
	_qFrame->RefreshViz();
}

void QuenchController::PushToHistory(PCommand cmd)
{
	// used by history
	_commandMgr->push_back(cmd);
	// Set undo <- true and redo <- false
	_qFrame->SetUndoRedoMenuState(true,false); 
}

void QuenchController::UpdateMesh()
{
	if(_mesh && _mesh->Visible())
	{
		int selected = _qFrame->OverlayPanel_()->Selected();
		_mesh->UpdateColors( _qFrame->Interactor()->VolumeViz()->GetOverlay( selected ).get(), _subjectData[selected].Volume().get(), _qFrame->ColorMapPanel_() );			
	}
}

void QuenchController::UpdatePathwayViz(bool bVisibility)
{
	if(_qFrame->Interactor()->PathwayViz()->Mode() == qPathwayViz::SURFACE_MODE)
		_qFrame->Interactor()->PathwayViz()->SetIntersectionGeometry(_pdbHelper, _groupArray);
	else
	{
		if(bVisibility)
			_qFrame->Interactor()->PathwayViz()->UpdatePathwaysVisibility(_pdbHelper, _groupArray);
		else
			_qFrame->Interactor()->PathwayViz()->UpdatePathwaysColor(_pdbHelper, _groupArray);
	}
	
	// XXX I don't know why this is only for visibility, just capturing logic right now
	if(!bVisibility)
	{
	  _qFrame->Interactor()->PathwayViz()->VisibilityPanel_()->Update(_groupArray, _pdbHelper.Assignment());
	  _qFrame->StatsPanel_()->StatsCheckboxArray_()->Update(&_pdbHelper.Assignment(), &_groupArray);
	}
}

void QuenchController::LoadState(std::string filename)
{
	filename = AbsolutePath(filename);

	// clear data
	_commandMgr->clear();
	_qFrame->ClearForLoadState();
	// Set data loaded state to nothing
	this->SetDataLoadedState(NOTHING_LOADED, true);
	
	//Set the directory of the state file
	char oldDir[2048], *unused; unused = _getcwd(oldDir,2048);
	int iunused = _chdir(FileNameToPath(filename).c_str());
	std::ifstream stream(filename.c_str());
	if(stream.is_open())
	{
		int idx = -1, x,y,w,h,version;

		stream>>"Version ">>version>>endl;

		if (version > QUENCH_QST_VERSION_NUMBER)
		  {
		    wxString caption(_T("Error"));
		    char msg[2048];
		    sprintf (msg, "The version number of this .qst file is %d. This version of Quench only reads .qst files\nwith version numbers up to %d. Upgrade Quench to read this .qst", version, QUENCH_QST_VERSION_NUMBER);
		    SystemInfoDialog infoDlg(this->_qFrame, caption, msg);
		    infoDlg.ShowModal();
		    return;
		  }
		stream>>"--- Main Window ---">>endl;
		stream>>"Position ">>x>>y>>endl;
		stream>>"Size ">>w>>h>>endl;
		_qFrame->SetSize(x,y,w,h);
		stream>>"Is Maximized ">>x>>endl; _qFrame->Maximize(x?true:false);
		stream>>"Current background image index ">>_currBkImage>>endl;
		stream>>endl;

		//Deserialize data
		_subjectData	.Deserialize(stream, version);
		_pdbHelper		.Deserialize(stream, version);
		_groupArray		.Deserialize(stream, version);

		this->SetBackgroundVolume(_currBkImage);

		//Update GUI elements
		if(_subjectData.size() > 0)
			for(int i = 0; i < (int)_subjectData.size(); i++)
				this->AddVolumeToDisplay(_subjectData[i]);

		_qFrame->Interactor()->Deserialize(stream, version);

		//Update the overlay panel with the overlays
		if(_subjectData.size() > 0)
			for(int i = 0; i < (int)_subjectData.size(); i++)
				this->AddVolumeToPanel(_subjectData[i]);

		if(_pdbHelper.PDB().get() != 0)
		{
			_qFrame->Interactor()->PathwayViz()->GeneratePathways(_pdbHelper,_groupArray);
			this->SetDataLoadedState(PATHWAYS_LOADED);
		}

		if(_state_data_loaded == PATHWAYS_LOADED)
		{
			if(_qFrame->StatsPanel_()->IsVisible() == false)
				_qFrame->Interactor()->PathwayViz()->VisibilityPanel_()->SetTextInfoVisibility(true);
			_qFrame->StatsPanel_()->Update(_pdbHelper, _groupArray);
			this->AddStatsToPerPointPanel();
			_qFrame->Interactor()->PathwayViz()->VisibilityPanel_()->Update(_groupArray, _pdbHelper.Assignment());
		}

		//Deserialize gui panels
		_qFrame->StatsPanel_()  ->Deserialize(stream, version);
		_qFrame->OverlayPanel_()->Deserialize(stream, version);

		//Deserialize the ROI information
		vtkROIFactory *factory = new vtkROIFactory(_qFrame->Interactor()->Renderer(), &_voiMgr);
		_voiMgr.Deserialize(factory, stream, version);

		_qFrame->Interactor()->ROIViz()->Update(_voiMgr);

		UpdateROIBoundingBoxDisplay();
		_qFrame->SetROISelectedMenuState(_voiMgr.Selected() != NULL);

		this->SetBackgroundVolume(_currBkImage);
		// send an event to overlay item to select the correct background
	       
		wxCommandEvent dummyEvent;
		(*_qFrame->OverlayPanel_())[_currBkImage]->OnBackgroundVolumeChanged(dummyEvent);
	}
	stream.close();

	//RefreshViz();
	//iunused = _chdir(oldDir);
	_qFrame->Interactor()->Renderer()->ResetCameraClippingRange();
	_qFrame->RefreshViz();
}

// Save all ROIs to disk. ROIs are stored in a subdirectory called
// "ROIs" underneath the current working directory:
void QuenchController::SaveAllROIs()
{
  for(PDTIFilterROIList::iterator iter = _voiMgr.begin(); iter != _voiMgr.end(); iter++) {
    std::string fileLocation = (*iter)->GetFileLocation();
    if (fileLocation == "") {
      // This ROI was just drawn, not previously loaded from disk.
      struct stat st1;
      if (stat("./ROIs",&st1) != 0) {
	cerr << "ROIs directory does not exist. Creating!" << endl;
	struct stat st2;
	// xxx dakers this is not cross-platform compatible.
	stat(".",&st2);
	// Get permissions of current directory, and set them to be the
	// same in the directory we're creating...
	mkdir("ROIs", st2.st_mode);
	// xxx dakers this is not cross-platform compatible.
      }
      std::string roiName = (*iter)->Name();
      (*iter)->SetFileLocation("ROIs/"+ roiName + ".nii.gz");
      struct stat st3;
      if (stat((*iter)->GetFileLocation().c_str(), &st3) == 0) {
	// File already exists - give user an overwrite warning!
	char message[2048];
	sprintf (message, "\n\nThe ROI in '%s' already exists. Do you want to overwrite it?\n", (*iter)->GetFileLocation().c_str());
	NButtonChoiceDialog ed(this->_qFrame, "Warning", message,"Overwrite","Skip");
	switch (ed.ShowModal()) {
	case BUTTON_CHOICE_1: // Save
	  SaveROINifti(*iter, (*iter)->GetFileLocation());
	  break;
	case BUTTON_CHOICE_2: // Don't Save
	  // Do nothing
	  break;
	};
      }
	else {
	  SaveROINifti(*iter, (*iter)->GetFileLocation());
	}
    }
    else {
      SaveROINifti(*iter, (*iter)->GetFileLocation());
    }
  }
}

void QuenchController::SaveState(std::string filename, bool saveROIs)
{
	//Set the directory of the state file
	char oldDir[2048]; 
	char *unused = _getcwd(oldDir,2048);
	string qst_dir = FileNameToPath(filename);
	int iunused = _chdir(qst_dir.c_str());
	const int version = QUENCH_QST_VERSION_NUMBER;

	std::ofstream stream(filename.c_str(),ios_base::trunc);
	if(stream.is_open())
	{
		stream<<"Version "<<version<<endl;
		stream<<"--- Main Window ---"<<endl;
		int x,y,w,h;
		_qFrame->GetPosition(&x,&y);
		stream<<"Position "<<x<<" "<<y<<endl;

		_qFrame->GetSize(&w,&h);
		stream<<"Size "<<w<<" "<<h<<endl;
		stream<<"Is Maximized "<<_qFrame->IsMaximized()<<endl;
		stream<<"Current background image index "<<_currBkImage<<endl;
		stream<<endl;
		
		//Serialize data
		_subjectData	.Serialize(stream, version);
		_pdbHelper		.Serialize(stream, version);
		_groupArray		.Serialize(stream, version);

		//Serialize interactor
		_qFrame->Interactor()->Serialize(stream, version);

		//Serialize panels
		_qFrame->StatsPanel_()  ->Serialize(stream, version);
		_qFrame->OverlayPanel_()->Serialize(stream, version);

		// Save all ROIs to NIFTI files (generating new files if
		// necessary):
		if (saveROIs) {
		  SaveAllROIs();
		}

		//Serialize ROIs
		_voiMgr.Serialize(stream, version);
	}
	stream.close();
	iunused = _chdir(oldDir);
}

// Add the specified ROI to the scene:
void QuenchController::AddNewROI(vtkROI *vInput)
{
  PDTIFilterROI voi(vInput);
  _voiMgr.push_back(voi);
  _qFrame->StatsPanel_()->ROIPanel_()->Update(_voiMgr);
  _qFrame->Interactor()->ROIViz()->Update(_voiMgr);

  // This new ROI should be selected:
  _voiMgr.SetSelected(vInput->Name());
}

// Replace an existing ROI with a new version. Used when pushing updated ROIs
// from mrDiffusion, and when editing ROIs.
void QuenchController::ReplaceROI (int oldID, vtkROI *vInput)
{

  DTIFilterROI *oldROI = _voiMgr.Find(oldID);
  _voiMgr.erase(oldROI->Name());
  PDTIFilterROI voi(vInput);
  _voiMgr.push_back(voi);
  _voiMgr.SetSelected(vInput->Name());
  // update 3d data
  _qFrame->StatsPanel_()->ROIPanel_()->Update(_voiMgr);
  _qFrame->Interactor()->ROIViz()->Update(_voiMgr);
  UpdateROIBoundingBoxDisplay();
  
}

// Invoked by the ROI editor when an ROI has been modified, and the visual
// representation needs to be updated. This works by replacing the current
// ROI with a new one:
void QuenchController::ROIEdited ()
{
  // Which ROI was just edited?
  vtkROI *editedROI = _qFrame->Interactor()->voiEditor()->GetROI();

  // Get its image representation:
  DTIScalarVolume *maskVolume = editedROI->GetImageRepresentation();

  // Turn that into a mesh, using marching-cubes:
  vtkPolyData *pData = MeshBuilder::ImageToMesh(maskVolume);
 
  // Generate a new ROI structure from that mesh:
  vtkROI *newMeshROI = vtkROI::CreateMesh(_qFrame->Interactor()->Renderer(), editedROI->ID(), Vector3d(0,0,0), pData);

  // Copy all the old settings from the previous ROI into the current one.
  newMeshROI->SetShowROI(_voi_visibility);
  newMeshROI->SetShowLabel(_voi_label_visibility);
  newMeshROI->SetFileLocation(editedROI->GetFileLocation());
  newMeshROI->SetName(editedROI->Name());
  Colord col = editedROI->Color();
  newMeshROI->SetColor(col);
  newMeshROI->SetImageRepresentation(maskVolume);

  // Do the swap
  ReplaceROI(editedROI->ID(), newMeshROI);

  // Make sure the editor knows about the replacement:
  _qFrame->Interactor()->voiEditor()->SetROI(newMeshROI);

  // Refresh the screen:
  _qFrame->Interactor()->Renderer()->ResetCameraClippingRange();
  _qFrame->Refresh();
}

// Called when the user requests drawing a new ROI
void QuenchController::DrawNewROI()
{
  _qFrame->Interactor()->Renderer()->SetBackground(0.74, 0.6, 0.6);

  qROIEditor *editor = _qFrame->Interactor()->voiEditor();
  if (_qFrame->Interactor()->voiEditor()->GetEditingROIMode()) {
    cerr << "Can't make a new ROI until you're done editing the current one." << endl;
    return;
  }

  vtkPolyData *pData = vtkPolyData::New();

  vtkROI *newMeshROI = vtkROI::CreateMesh(_qFrame->Interactor()->Renderer(), ROIManager_().NextID(), Vector3d(0,0,0), pData);
  newMeshROI->SetShowROI(_voi_visibility);
  newMeshROI->SetShowLabel(_voi_label_visibility);

  DTIScalarVolume *roiImageBase = _qFrame->Interactor()->VolumeViz()->GetActiveVolume();
  unsigned int iDim, jDim, kDim;
  roiImageBase->getDimension(iDim, jDim, kDim);

  // Make a new mask image with all zeros
  DTIScalarVolume *maskVolume = roiImageBase->GenerateMaskVolume();
  newMeshROI->SetImageRepresentation(maskVolume);
  AddNewROI(newMeshROI);
  _qFrame->Interactor()->voiEditor()->SetEditingROIMode(true);
  editor->SetROI(newMeshROI);
  _qFrame->Interactor()->voiEditor()->UpdateStatusDisplay();
  _qFrame->RefreshViz();
}

// Turn on ROI editing mode.
void QuenchController::ROIEditModeOn()
{
  // Set background to a reddish tint (as a cue to the user):
  _qFrame->Interactor()->Renderer()->SetBackground(0.74, 0.6, 0.6);

  // Error check - these situations should never be possible, unless
  // there was an error managing the available menu options:
  PDTIFilterROI selectedROI = _voiMgr.Selected();
  if (selectedROI == NULL) {
    cerr << "No active ROI to edit." << endl;
    return;
  }
  if (selectedROI->GetType() != ROI_TYPE_IMAGE) {
    cerr << "Can't edit this type of ROI." << endl;
    return;
  }

  // Turn on ROI editing inside the editor:
  _qFrame->Interactor()->voiEditor()->SetEditingROIMode(true);
  _qFrame->Interactor()->voiEditor()->SetROI((vtkROI *) selectedROI.get());

  // Update the status display text to indicate that ROI editing mode is off:
  _qFrame->Interactor()->voiEditor()->UpdateStatusDisplay();

  // Redraw the screen:
  _qFrame->RefreshViz();
}

// Turn off ROI editing mode.
void QuenchController::ROIEditModeOff()
{
  // Reset the background to the normal grey:
  _qFrame->Interactor()->Renderer()->SetBackground(0.74, 0.74, 0.66);

  // Update the flag inside the ROI editor itself:
  _qFrame->Interactor()->voiEditor()->SetEditingROIMode(false);

  // Update the status display text to indicate that ROI editing mode is off:
  _qFrame->Interactor()->voiEditor()->UpdateStatusDisplay();

  // Redraw the screen:
  _qFrame->RefreshViz();
}

// Increase the brush size for the ROI editor.
void QuenchController::IncreaseBrushSize()
{
  _qFrame->Interactor()->voiEditor()->IncreaseBrushSize();
  _qFrame->RefreshViz();
}

// Decrease the brush size for the ROI editor.
void QuenchController::DecreaseBrushSize()
{
  _qFrame->Interactor()->voiEditor()->DecreaseBrushSize();
  _qFrame->RefreshViz();
}

// Save the specified ROI to disk. 
void QuenchController::SaveROINifti(PDTIFilterROI voi, std::string path)
{
  DTIScalarVolume *vol = voi->GetImageRepresentation();
  if (!vol) {
    cerr << "Error - current volume has no image representation." << endl;
    return;
  }
  DTIVolumeIO::saveVolumeNifti3D(vol, path.c_str());
}

// Save the currently-selected ROI as a NIFTI file.
void QuenchController::SaveCurrentROINifti(std::string path, std::string filename)
{
  PDTIFilterROI selectedROI = _voiMgr.Selected();
  if (!selectedROI) {
    cerr << "Error - no ROI selected." << endl;
    return;
  }
  // Perform the actual save to disk:
  SaveROINifti(selectedROI, path);

  // Update the onscreen name of the ROI to match the filename:
  ((vtkROI *) selectedROI.get())->SetName(filename);
  selectedROI->SetFileLocation(RelativePath(path));
  _qFrame->RefreshViz();
}

// Set the visibility of the ROIs in the scene.
void QuenchController::SetROIVisibility(bool vis)
{
  _voi_visibility = vis;
  for(PDTIFilterROIList::iterator iter = _voiMgr.begin(); iter != _voiMgr.end(); iter++)
    ((vtkROI*)((*iter).get()))  ->  SetShowROI(vis );
  UpdateROIBoundingBoxDisplay();
  _qFrame->RefreshViz();
}

// Set the visibility of the bounding boxes used to highlight the
// currently-selected ROI.
void QuenchController::SetROIBoundingBoxVisibility(bool vis)
{
  _voi_bounding_box_visibility = vis;
  UpdateROIBoundingBoxDisplay();
  _qFrame->RefreshViz();
}

// Set the visibility of each ROI in the scene to the specified value.
void QuenchController::SetROILabelVisibility(bool vis)
{
  _voi_label_visibility = vis;
   for(PDTIFilterROIList::iterator iter = _voiMgr.begin(); iter != _voiMgr.end(); iter++)
    ((vtkROI*)((*iter).get()))  ->  SetShowLabel(vis );
  _qFrame->RefreshViz();
}

// Updates the visibility status of the bounding box, used to highlight the 
// currently-selected ROI.
void QuenchController::UpdateROIBoundingBoxDisplay()
{
  vtkROI *voi = (vtkROI*)_voiMgr.Selected().get();
  vtkActor *selectedActor = NULL;
  if (voi) {
    selectedActor = voi->Actor();
  }

  _qFrame->Interactor()->HighlightProp((_voi_visibility && _voi_bounding_box_visibility) ? selectedActor : NULL);
}


// Executed whenever the user wants to modify the properties (color, name) of
// the currently-selected ROI. Enforces a policy that names must be unique.
void QuenchController::OnROIProperties()
{
  PDTIFilterROI selected = _voiMgr.Selected();
  std::string oldName = selected->Name();
  ROIPropertiesDialog d(_qFrame, selected);
  int returnCode = d.ShowModal();
  if (returnCode == wxID_OK) {
    // User pressed OK after possibly modifying ROI properties:
    Colord newColor = d.GetColor();
    double opacity = d.GetOpacity();
    newColor.a = opacity;
    std::string newName = d.GetName();

    int countWithName = 0;

    // Count how many ROIs have the same name as the one we're adding:
    for(PDTIFilterROIList::iterator iter = _voiMgr.begin(); iter != _voiMgr.end(); iter++) {
      if ((*iter)->Name() == newName) {
	countWithName++;
      }
    }
    // If there's a different ROI with the same name, warn the user and 
    // refuse to continue.
    if (countWithName > 1 || countWithName == 1 && newName != oldName) {
      wxString caption(_T("Error"));
      wxString message("Duplicate ROI name is not allowed.");
      SystemInfoDialog infoDlg(this->_qFrame, caption, message);
      infoDlg.ShowModal();
      return;
    }

    // Actually update the name and color in the ROI:
    ((vtkROI *)selected.get())->SetName(newName);
    ((vtkROI *)selected.get())->SetColor(newColor);

    // Refresh the window:
    _qFrame->RefreshViz();
  }
}

// Deletes the currently selected ROI from the view:
void QuenchController::DeleteCurrentROI()
{
  // delete the voi from the manager
  _voiMgr.erase(_voiMgr.Selected()->Name());
  vtkROI * voi = 0;
  
  // if any more voi's left set the new selected voi to the start of the manager
  if(_voiMgr.size()) {
    voi = dynamic_cast<vtkROI*>((*_voiMgr.begin()).get());
    _voiMgr.SetSelected( voi->Name());
  }
  
  // update 3d data
  _qFrame->StatsPanel_()->ROIPanel_()->Update(_voiMgr);
  _qFrame->Interactor()->ROIViz()->Update(_voiMgr);
  UpdateROIBoundingBoxDisplay();
  _qFrame->SetROISelectedMenuState(_voiMgr.Selected() != NULL);
}
