/***********************************************************************
* AUTHOR: sherbond <sherbond>
*   FILE: qPathwayViz.cpp
*   DATE: Thu Nov 20 16:42:12 2008
*  DESCR: 
***********************************************************************/
#include "typedefs.h"
#include "qPathwayViz.h"
#include "VisibilityPanel.h"
#include "ColorMapPanel.h"

// Rendering
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include <GL/glu.h>

// Pathway Grouping
#include "ModeSelector.h"
#include "icons/shapematch.h"
#include "icons/add.h"
#include "icons/intersect.h"
#include "icons/subtract.h"
#include "icons/surfintersect.h"
#include "icons/touch.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkPolyData.h"
#include "vtkFloatArray.h"
#include "vtkMath.h"
#include "vtkPointData.h"
#include "vtkProperty.h"
#include "DTIPathwayIO.h"
#include "DTIPathDistanceMatrix.h"
#include "DTIPathwayDatabase.h"
#include "DTIPathwaySelection.h"
#include "DTIPathway.h"
#include "DTIMath.h"
#include "vtkCellArray.h"
#include "vtkFilledCaptionActor2D.h"
#include <DTIPathwayStatisticHeader.h>
#include <RAPID.H>
#include <numeric>
#include "vtkMatrix4x4.h"
#include "vtkPropCollection.h"
#include "vtkWindowToImageFilter.h"
#include "ROIManager.h"
#include "vtkPropPicker.h"
#include "vtkPropCollection.h"
#include "vtkTubeFilter.h"

using namespace std;
vtkTubeFilter* tubeFilter;

qPathwayViz::qPathwayViz(vtkRenderer *renderer) : _per_point_info(" ",0,0,100)
{
	_filterOperation = UNION;
	_lineWidthSetting = 0;
	_renderer = renderer;
	_pdPathways = NULL;
	_mPathways = vtkPolyDataMapper::New();
	_aPathways = vtkActor::New();
	_aPathways->SetMapper(_mPathways);

	// allocate memory for surface intersection actor
	_mIntersections = vtkPolyDataMapper::New();
	_aIntersections = vtkActor::New();
	_aIntersections->SetMapper(_mIntersections);
	_pdIntersections = vtkPolyData::New();
	_mIntersections->SetInput (_pdIntersections);
	vtkMapper::SetResolveCoincidentTopologyToPolygonOffset();

	// allocate memory for fiber actor
	vtkPoints *intPoints = vtkPoints::New();
	vtkCellArray *intCells = vtkCellArray::New();
	_pdIntersections->SetPoints(intPoints);
	_pdIntersections->SetVerts (intCells);
	_aIntersections->SetVisibility (false);
	_aIntersections->GetProperty()->SetPointSize (5);
	intPoints->Delete();
	intCells->Delete();

	// Initialize the mode selector panel
	Point2i size(48,48);
	bool visible = false;
	int val = 0;
	Point2i loc;

	// initialize the Marking mode modeselector panel
	loc = Point2i(755,4);
	_msGesture = new ModeSelector (_renderer, "Mark mode");
	_msGesture->AddIcon (size, ICON_TOUCH, "Touch");
	_msGesture->AddIcon (size, ICON_SURFINTERSECT, "Surface");
	_msGesture->SetPosition(loc);
	_msGesture->SetSelectMode(val);
	_msGesture->SetVisible(visible);
	
	// initialize the selection mode modeselector panel
	loc = Point2i(857,4);
	_msSelection = new ModeSelector (_renderer, "Selection mode");
	_msSelection->AddIcon (size, ICON_ADD, "Add");
	_msSelection->AddIcon (size, ICON_SUBTRACT, "Remove");
	_msSelection->AddIcon (size, ICON_INTERSECT, "Intersect");
	_msSelection->SetPosition(loc);
	_msSelection->SetSelectMode(val);
	_msSelection->SetVisible(visible);
	//_msGesture->setOpacity (0.7);_msSelection->setOpacity (0.7);
	
	// add them to the renderer.
	_renderer->AddActor(_aPathways);
	_renderer->AddActor(_aIntersections);
	
	_vis_panel = new VisibilityPanel(renderer);
	bool bFalse = false;
	_vis_panel->SetVisible(bFalse);

	_propPicker = vtkPropPicker::New();
	_propCollection = vtkPropCollection::New();

	_propCollection->AddItem(_aPathways);
	_propCollection->AddItem(_aIntersections);

	_pathwayVisibility = false;
	_pointsVisibility = false;
	tubeFilter = vtkTubeFilter::New();

	_do_per_point_color = false;
}
qPathwayViz::~qPathwayViz()
{
	if(_aPathways!=NULL) 
	{
		VTK_SAFE_DELETE(_aPathways);
		VTK_SAFE_DELETE(_mPathways);
		VTK_SAFE_DELETE(_pdPathways);
		VTK_SAFE_DELETE(_aIntersections);
		VTK_SAFE_DELETE(_mIntersections);
		VTK_SAFE_DELETE(_pdIntersections);
	}
	VTK_SAFE_DELETE(_propPicker);
	VTK_SAFE_DELETE(_propCollection);
	VTK_SAFE_DELETE(tubeFilter);
	delete _vis_panel;
	delete _msGesture;
	delete _msSelection;
}

void qPathwayViz::SetPathwayVisibility(bool &b)
{
	if(_aPathways!=NULL)
	{
		_aPathways->SetVisibility(b);
		_pathwayVisibility = b;
	}
}
void qPathwayViz::SetPointsVisibility(bool &b)
{
	if(_aIntersections !=NULL)
	{
		_aIntersections ->SetVisibility(b);
		_pointsVisibility = b;
	}
}

/*! Generates the pathways in vtk 3d format. It also runs the other 2 pathway update functions later.
Call this function when 
*/
void qPathwayViz::GeneratePathways(PDBHelper& helper, PathwayGroupArray &groupArray)
{
	unsigned char rgb[3];
	bool visible = true;
	// show the modeselector panels if not shown already
	_msGesture->SetVisible(visible);
	_msSelection->SetVisible(visible);
	_pathwayVisibility = _pointsVisibility = true;

	DTIPathwayDatabase *pdb = helper.PDB().get();
	DTIPathwayAssignment assn = helper.Assignment();
	// allocate array to hold fiber colors
	vtkUnsignedCharArray* colors = vtkUnsignedCharArray::New();
	colors->SetNumberOfComponents(3);
	colors->SetName("colors");

	// cache the assignment and grouparray so that we only update the fibers which have actually changed
	_cached_assn = assn;
	_cached_group_array = groupArray;
	_is_color_cached = true;

	//Delete existing pathways
	VTK_SAFE_DELETE(_pdPathways);
	_pdPathways = vtkPolyData::New();
	_mapPathwayLuminance.clear();

	// Set up the scene pathways for the first time by recording
	// all polylines for the entire pathway database
	vtkPoints* points = vtkPoints::New();
	vtkFloatArray* pcoords = vtkFloatArray::New();	
	pcoords->SetNumberOfComponents(3);
	vtkCellArray* cells = vtkCellArray::New();

	// This takes the given database and sets the pathway actor to use these new

	// pathways.
	// Note that by default, an array has 1 component.
	// We have to change it to 3 for points
	for(int j=0,currPoint=0; j < pdb->getNumFibers(); j++) 
	{
		DTIPathway* pathway = pdb->getPathway(j);
		_mapPathwayLuminance[pathway] = DTIMath::randzeroone()*0.3 - 0.15;
		if(pathway->getNumPoints() > 0) 
		{
			// Add a cell to handle this pathway
			cells->InsertNextCell(pathway->getNumPoints());
			double *pts = new double[3*pathway->getNumPoints()];

			// jitter the pathway color so we can distinguish between fibers
			JitteredPathwayColor(pathway, groupArray[ assn[j] ].Color(), rgb);

			for(int i=0;i<pathway->getNumPoints();i++, currPoint++)
			{
				// Get a point from the pathway database
				pathway->getPoint(i,pts+3*i);
				pcoords->InsertTuple(currPoint, pts+3*i);
				cells->InsertCellPoint(currPoint);
				colors->InsertNextTuple3(rgb[0],rgb[1],rgb[2]);
			}
			delete []pts;
		}
	}

	// Create vtkPoints and assign pcoords as the internal data array.
	points->SetData(pcoords);
	// Create the dataset. In this case, we create a vtkPolyData
	// Assign points and cells
	_pdPathways->SetPoints(points);
	_pdPathways->SetLines(cells);
	_pdPathways->GetPointData()->SetScalars(colors);
	// Create the mapper 
	_mPathways->SetInput(_pdPathways);
	_mPathways->ScalarVisibilityOn();
	// Create an actor.
	_aPathways->GetProperty()->SetLineWidth(1.0);

    // release memory
	points->Delete();
	pcoords->Delete();
	cells->Delete();
	colors->Delete();

	// We have added the vertices, now add the lines
	UpdatePathwaysVisibility(helper,groupArray);
	NotifyAllListeners (PEvent (new Event(UPDATE_VISIBILITY_PANEL)));
	//_vis_panel->Update(groupArray, helper.Assignment());
}
void qPathwayViz::UpdatePathwaysColor(PDBHelper& helper, PathwayGroupArray &groupArray)
{
  if (_pdPathways == NULL) {
    // Check added by dla 7/27/11 - was causing crash on exit with NULL pathways
    return;
  }
	unsigned char rgb[3];
	DTIPathwayDatabase *pdb = helper.PDB().get();
	DTIPathwayAssignment assn = helper.Assignment();
	vtkUnsignedCharArray* colors = (vtkUnsignedCharArray* )_pdPathways->GetPointData()->GetScalars();
	int stat_idx = -1;
	if(_do_per_point_color && (-1 != (stat_idx = pdb->getStatisticIndex(_per_point_info.StatisticName())) ) )
	{
		ColorMap &c = this->ColorMapPanel_()->ColorMaps()[ _per_point_info.ColorMapIndex() ];
		float stat_min = _per_point_info.Min();
		float stat_max = _per_point_info.Max();
		float scale = (c.Colors.size()-1)/(stat_max-stat_min);

		// for each fiber
		for(int j=0, point_idx = 0; j < pdb->getNumFibers(); j++) 
		{
			DTIPathway* pathway = pdb->getPathway(j);

			//Pathway is assigned to a different group so change color
			if(pathway->getNumPoints()) 
				for(int i=0;i<pathway->getNumPoints();i++)
				{
					ComputePerPointColor(pathway, i, stat_idx, stat_min, scale, c, rgb);
					colors->SetTuple3(point_idx+i, rgb[0], rgb[1], rgb[2]);
				}
				point_idx += pathway->getNumPoints();
		}
	}
	else
	{
		// for each fiber
		for(int j=0, point_idx = 0; j < pdb->getNumFibers(); j++) 
		{
			DTIPathway* pathway = pdb->getPathway(j);

			//Pathway is assigned to a different group so change color
			if(pathway->getNumPoints() > 0 && (_is_color_cached == false || assn[j] != _cached_assn[j]) ) 
			{
				// jitter the pathway color so we can distinguish between fibers
				JitteredPathwayColor(pathway, groupArray[ assn[j] ].Color(), rgb);
				for(int i=0;i<pathway->getNumPoints();i++)
					colors->SetTuple3(point_idx+i, rgb[0],rgb[1],rgb[2]);
			}
			point_idx += pathway->getNumPoints();
		}
	}
	colors->Modified();
	//	_vis_panel->Update(groupArray, helper.Assignment());
	NotifyAllListeners (PEvent (new Event(UPDATE_VISIBILITY_PANEL)));

	//Update the visibility if either group visibility has changed
	for(int i = 0; i < (int)groupArray.size(); i++)
		if( groupArray[i].Visible() ^ _cached_group_array[i].Visible())
		{
			UpdatePathwaysVisibility(helper, groupArray);
			return;
		}

	// Update visibility if a fiber is assigned from a visible group to an invisible group or

	// vice versa

	for(int i = 0; i < pdb->getNumFibers(); i++)
		if( groupArray[ assn[i] ].Visible()  ^  _cached_group_array[ _cached_assn[i] ].Visible())
		{
			UpdatePathwaysVisibility(helper, groupArray);
			return;
		}
	_cached_assn = assn;
	_is_color_cached = true;
}
void qPathwayViz::UpdatePathwaysVisibility(PDBHelper& helper, PathwayGroupArray &groupArray)
{
	DTIPathwayDatabase *pdb = helper.PDB().get();
	DTIPathwayAssignment assn = helper.Assignment();
	vtkCellArray* cells = vtkCellArray::New();
	int pt_offset	=0;
	int fiber_shown	=0;

	// for each fiber
	for(int j=0; j<pdb->getNumFibers();j++) 
	{
		DTIPathway*pathway = pdb->getPathway(j);
		int npts		   = pathway->getNumPoints();

		//If fiber group is visible add this fiber to the 3d dataset
		if(groupArray[assn[j]].Visible())
		{
			cells->InsertNextCell( npts );
			for(int i=0;i<npts;i++)
				cells->InsertCellPoint(i+pt_offset);
		}
		pt_offset+=npts;
	}
	_pdPathways-> SetLines(cells);
	_pdPathways-> Modified();
	 cells	   -> Delete();
	 //	_vis_panel -> Update(groupArray, helper.Assignment());
	 NotifyAllListeners (PEvent (new Event(UPDATE_VISIBILITY_PANEL)));
		    
	// update the cache
	_cached_assn		= assn;
	_cached_group_array = groupArray;
	_is_color_cached = true;
}
void qPathwayViz::BeginPerPointColoringMode(PDBHelper& helper, PathwayGroupArray &groupArray, EventShowPerPointColorPanel &event_info)
{
	_per_point_info = event_info;
	_do_per_point_color = true;
	_vis_panel->SetTransparent(_do_per_point_color);
	if(_msGesture->SelectMode() == qPathwayViz::TOUCH_MODE)
		UpdatePathwaysColor(helper, groupArray);
	else 
		SetIntersectionGeometry(helper, groupArray);
}
void qPathwayViz::EndPerPointColoringMode()
{
	_do_per_point_color = false;
	_vis_panel->SetTransparent(_do_per_point_color);
}
bool qPathwayViz::OnLeftDown(int x, int y)
{
	prev_x = prev_y = -1; _panel_selected = false;
	ModeSelectorComponent msc;

	// did we click on the marking modeselector panel?
	if (_msGesture->Visible() == false)
	  return false;
	msc = _msGesture->OnLeftDown(x,y);
	if(msc != MODE_SELECTOR_NONE)
	{
		int mode = _msGesture->SelectMode();
		// update the visibility
		_pathwayVisibility = mode==qPathwayViz::TOUCH_MODE;
		_pointsVisibility = mode==qPathwayViz::SURFACE_MODE;

		_aIntersections->SetVisibility ( _pointsVisibility );
		_aPathways->SetVisibility ( _pathwayVisibility );

		// compute the points intersection with selected image slice
		if(mode==qPathwayViz::SURFACE_MODE) 		// notify Quench of this change
			NotifyAllListeners(PEvent ( new Event(UPDATE_SURFACE_MODE_GEOMETRY) ));
		// refresh
		_renderer->GetRenderWindow()->Render();
		NotifyAllListeners( PEvent( new EventSelectionModeChanged(mode) ) );
		prev_x = x; prev_y = y; _panel_selected = true;
		return true;
	}
	// did we click on selection modeselector panel?
	msc = _msSelection->OnLeftDown(x,y);
	if(msc != MODE_SELECTOR_NONE)
	{
		// update the filtering operation
		_filterOperation = (BOOLEAN_OPERATION)_msSelection->SelectMode();
		// notify Quench of this change
		NotifyAllListeners( PEvent( new EventSelectionOperationChanged(_filterOperation) ) );
		prev_x = x; prev_y = y; _panel_selected = true;
		return true;
	}
	int group = _vis_panel->OnLeftDown(x,y);
	if(group != -1)
		return true;
	return false;
}
bool qPathwayViz::OnRightButtonUp(int x, int y)
{
	if(_vis_panel->OnRightButtonUp(x,y))
		return true;

	return false;
}
bool qPathwayViz::OnMouseMove(int x, int y)
{
  if (_msGesture->Visible() == false)
    return false;
	// we cant do a left down here because the x,y values are for the current mouse position which is different
	// from the first time we clicked on the panel
	if( _panel_selected )
	{
		// figure out how much the mouse has moved
		Point2i pos, delta(x-prev_x, y-prev_y);
		
		// update the positions
		pos = _msGesture->Position() + delta;
		_msGesture->SetPosition(pos);
		
		pos = _msSelection->Position() + delta; 
		_msSelection->SetPosition(pos);
		
		prev_x = x; prev_y = y;
		//refresh
		_renderer->GetRenderWindow()->Render();
		return true;
	}

	return false;
}
void qPathwayViz::SetActiveImageExtents(double pts[4][3], double normal[4])
{
	memcpy(_planePts, pts, sizeof(double)*4*3);
	memcpy(_planeNormal, normal, sizeof(double)*4);
}
void qPathwayViz::SetIntersectionGeometry(PDBHelper& helper, PathwayGroupArray &groupArray)
{
	// Create a RAPID model of the image slice currently selected.
	// The model is simply a quad
	RAPID_model *planeModel = new RAPID_model;
	planeModel->BeginModel();
		planeModel->AddTri (_planePts[0], _planePts[1], _planePts[2], 0);
		planeModel->AddTri (_planePts[0], _planePts[2], _planePts[3], 1);
	planeModel->EndModel();

	// color array to store the color of the pathway intersections
	vtkPoints *newPts = vtkPoints::New();
	vtkCellArray *newCells = vtkCellArray::New();
	vtkUnsignedCharArray* colors = vtkUnsignedCharArray::New();
	colors->SetName("colors");
	colors->SetNumberOfComponents(3);
	unsigned char rgb[3];

	int stat_idx = helper.PDB()->getStatisticIndex(_per_point_info.StatisticName());
	ColorMap &c = this->ColorMapPanel_()->ColorMaps()[ _per_point_info.ColorMapIndex() ];
	float stat_min = _per_point_info.Min();
	float stat_max = _per_point_info.Max();
	float scale = (c.Colors.size()-1)/(stat_max-stat_min);

	// Now intersect the RAPID model with each pathway in the scene, and record any intersection points.
	for (int i = 0; i < helper.PDB()->getNumFibers(); i++) 
	{
		DTIPathway *pathway = helper.PDB()->getPathway(i);
		bool selected = helper.Assignment()[i] == helper.Assignment().SelectedGroup();
		if (!groupArray[ helper.Assignment()[i] ].Visible())
			continue;

		RAPID_Collide (ZERO_ROTATION, ZERO_TRANSLATION, planeModel,
			ZERO_ROTATION, ZERO_TRANSLATION, pathway->getRAPIDModel(), RAPID_ALL_CONTACTS);

		for(int contact=0; contact<RAPID_num_contacts; contact++)
		{
			Vector3<double> p0;
			Vector3<double> p1;
			int index1 = RAPID_contact[contact].id2-1;
			int index2 = index1+1;
			pathway->getPoint (index1, p0);
			pathway->getPoint (index2, p1);
			double s1 = DTIMath::computeLinePlaneIntersection (p0, p1, _planeNormal, _planePts[0]);
			// Now use s1 to figure out the actual point, and create a point cell.
			Vector3<double> intersectionPoint = (p1-p0)*s1 + p0;

			newPts->InsertNextPoint (intersectionPoint.v);
			newCells->InsertNextCell (1);
			newCells->InsertCellPoint (newPts->GetNumberOfPoints()-1);

			if(_do_per_point_color && -1 != stat_idx )
				ComputePerPointColor(pathway, i, stat_idx, stat_min, scale, c, rgb);
			else// jitter the pathway color so we can distinguish between fibers
				JitteredPathwayColor(pathway, groupArray[ helper.Assignment()[i] ].Color(), rgb);

			colors->InsertNextTuple3(rgb[0],rgb[1],rgb[2]);
		}
	}

	// update data and release memory
	_pdIntersections->SetPoints (newPts);
	newPts->Delete();
	_pdIntersections->SetVerts (newCells);
	newCells->Delete();
	_pdIntersections->GetPointData()->SetScalars (colors);
	_pdIntersections->Modified();
	colors->Delete();
	//	_vis_panel->Update(groupArray, helper.Assignment());
	NotifyAllListeners (PEvent (new Event(UPDATE_VISIBILITY_PANEL)));

	_renderer->GetRenderWindow()->Render();
	delete planeModel;
}
void qPathwayViz::IncreaseLineWidth(int amount)
{
	int lineWidth = _aPathways->GetProperty()->GetLineWidth();

	// cycle over the line width between 1 and MAX_LINE_WIDTH
	lineWidth = (lineWidth + amount + MAX_LINE_WIDTH-1) % MAX_LINE_WIDTH + 1;
	_aPathways->GetProperty()->SetLineWidth(lineWidth);

	// refresh
	_renderer->GetRenderWindow()->Render();
}
int  qPathwayViz::Mode() { return _msGesture->SelectMode(); }
void qPathwayViz::ComputePerPointColor(DTIPathway *pathway, int idx, int stat_idx, float stat_min, float inv_range, ColorMap &c, unsigned char rgb[3])
{
	float stat = pathway->getPointStatistic(idx,stat_idx);
					
	// get the color index
	int k = (stat-stat_min)*inv_range;
					
	//check if its in range
	if (k <= 0) k = 1;
	if (k >= (int)c.Colors.size()) k = (int)c.Colors.size()-1;

	Coloruc col = c.Colors[k];
	rgb[0]=col.r; rgb[1]=col.g; rgb[2]=col.b; 
}
void qPathwayViz::JitteredPathwayColor(DTIPathway* t, Colord &col, unsigned char rgb[3])
{
	// See if the pathway is within any of the groups
	double vRand = 0;
	// 0 is the default pathway group
	if(!_mapPathwayLuminance.empty()) 
	{
		map<DTIPathway*, double>::iterator fiber  = _mapPathwayLuminance.find(t);
		if(fiber != _mapPathwayLuminance.end())
			vRand = (*fiber).second;
	}

	// jitter the hue
	double hsv[3],drgb[3] = {col.r, col.g, col.b};
	vtkMath::RGBToHSV(drgb,hsv);
	hsv[2]+=vRand; hsv[2] = max(.0,min(1.,hsv[2]));
	vtkMath::HSVToRGB(hsv,drgb);	

	for(int i = 0; i < 3; i++)
		rgb[i]= (unsigned char) floor(255 * drgb[i]);
}
void qPathwayViz::OnEvent(PEvent e)
{
}
void qPathwayViz::Serialize(ostream &s, int version)
{
	s<<"--- Gesture Panel ---"<<endl;
	_msGesture  ->Serialize(s,version);
	s<<"--- Selection Panel ---"<<endl;
	_msSelection->Serialize(s,version);
}
void qPathwayViz::Deserialize(istream &s, int version)
{
	s>>"--- Gesture Panel ---">>endl;
	_msGesture  ->Deserialize(s,version);
	s>>"--- Selection Panel ---">>endl;
	_msSelection->Deserialize(s,version);

	int mode = _msGesture->SelectMode();
	_filterOperation = (BOOLEAN_OPERATION)_msSelection->SelectMode();
	NotifyAllListeners( PEvent( new EventSelectionModeChanged( mode ) ) );
	NotifyAllListeners( PEvent( new EventSelectionOperationChanged(_filterOperation) ) );

	_pathwayVisibility = mode==qPathwayViz::TOUCH_MODE;
	_pointsVisibility = mode==qPathwayViz::SURFACE_MODE;

	_aIntersections->SetVisibility ( _pointsVisibility );
	_aPathways->SetVisibility ( _pathwayVisibility );

	if( mode == qPathwayViz::SURFACE_MODE)
		NotifyAllListeners(PEvent ( new Event(UPDATE_SURFACE_MODE_GEOMETRY) ));
}
void qPathwayViz::LoadAndReplaceDistanceMatrix(istream &matrixStream)
{
#if 0
	delete _DistanceMatrix;
	_DistanceMatrix = NULL;
	if (matrixStream) 
	{
		cerr << "Loading distance matrix from file..." << endl;
		_DistanceMatrix = DTIPathDistanceMatrix::loadDistanceMatrix (matrixStream);
	}
#endif
}
void qPathwayViz::TakeScreenshot(void **pixels, int &width, int &height, int quality)
{
	// Hide UI elements
	bool bVis = false;
	_msGesture	->SetVisible(bVis);
	_msSelection->SetVisible(bVis);
	_vis_panel	->SetVisible(bVis);

	//Grab Screenshot
	vtkRenderWindow* myWindow = _renderer->GetRenderWindow();
	vtkMapper* origMapper = _aPathways->GetMapper();

	//Generate tubes if visible and quality is higher than some threshold
	if ( quality > 0 && _aPathways && _aPathways->GetVisibility() )
	{
		VTK_SAFE_DELETE(tubeFilter);
		tubeFilter = vtkTubeFilter::New();
		tubeFilter->SetInput(_pdPathways);
		tubeFilter->SetRadius(.25); //default is .5
		//tubeFilter->SetNumberOfSides(quality);
		tubeFilter->SetNumberOfSides(7);
		tubeFilter->Update();

		//Create a mapper and actor
		vtkPolyDataMapper* tubeMapper = vtkPolyDataMapper::New();
		tubeMapper->SetInputConnection(tubeFilter->GetOutputPort());
		_aPathways->SetMapper(tubeMapper);
		_aPathways->Modified();
	}

	// Turn on anti aliasing
	myWindow->SetAAFrames(3);
	//Grab Screenshot
	myWindow->Render();
	// Reset anti-aliasing
	//myWindow->SetAAFrames(0);  

	vtkWindowToImageFilter *filter = vtkWindowToImageFilter::New();
	filter->SetInput( myWindow );
	filter->SetInputBufferTypeToRGB();
	filter->SetReadFrontBuffer (false);
	filter->Modified();
	filter->Update();
	
	vtkImageData *data = filter->GetOutput();
	data->Update();
	data->Modified();

	//copy the pixel data to the user supplied buffer
	int *dims = data->GetDimensions();
	width = dims[0];
	height = dims[1];
	unsigned char *ucpixels = new unsigned char[width*height*3];
	memcpy(ucpixels, data->GetScalarPointer(), width*height*3*sizeof(unsigned char));
	*pixels = ucpixels;
	filter->Delete();

	if( quality > 0 && _aPathways && _aPathways->GetVisibility() )
	{
		// Restore state
		//tubeFilter->Delete();
		_aPathways->SetMapper(origMapper);
		_aPathways->Modified();
	}
	myWindow->SetAAFrames(0);
	// Show UI elements
	bVis = true;
	_msGesture	->SetVisible(bVis);
	_msSelection->SetVisible(bVis);
	_vis_panel	->SetVisible(bVis);
	myWindow->Render();
}
