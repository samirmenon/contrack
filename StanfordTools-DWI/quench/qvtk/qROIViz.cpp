#include "qMoveTool.h"
#include "qROIViz.h"
#include "vtkRenderer.h"
#include "vtkROI.h"
#include "vtkPropPicker.h"
#include "vtkPropCollection.h"
#include "vtkRenderWindow.h"

qROIViz::qROIViz(vtkRenderer *renderer)
{
	_renderer = renderer;

	// the default size of the move/scale tool is 1 unit which is very small.
	Vector3d scale(30,30,30);

	//create the move tool
	_move_tool = new qMoveTool(_renderer);
	_move_tool->SetScale(scale);
	//create the scale tool
	_scale_tool = new qScaleTool(_renderer);
	_scale_tool->SetScale(scale);

	is_move_tool_selected = true;
	bool visible = false;
	_move_tool->SetVisible(visible);
	_scale_tool->SetVisible(visible);

	_propPicker = vtkPropPicker::New();
	_propCollection = vtkPropCollection::New();
}
qROIViz::~qROIViz()
{
	// release memory
	SAFE_DELETE(_move_tool);
	SAFE_DELETE(_scale_tool);
	VTK_SAFE_DELETE(_propPicker);
	VTK_SAFE_DELETE(_propCollection);
}
bool qROIViz::OnLeftDown(int x, int y)
{
	// handle left click on the move tool
	if(_move_tool->OnLeftDown(x,y))
		return true;
	// handle left click on the scale tool
	if(_scale_tool->OnLeftDown(x,y))
		return true;

	// check if a voi was selected
	_propPicker->PickProp(x, y, _renderer, _propCollection);
	vtkProp *prop = _propPicker->GetViewProp();
	if(!prop)
		return false;
	
	NotifyAllListeners( PEvent( new EventROISelectByActor( (vtkActor*)prop) ) );
	return true;
}

bool qROIViz::OnClick(int x, int y)
{
  //  cerr << "Checking for ROI hit." << endl;
  // check if a voi was selected
  _propPicker->PickProp(x, y, _renderer, _propCollection);
  vtkProp *prop = _propPicker->GetViewProp();
  if(!prop)
    return false;
  //  cerr << "found ROI hit." << endl;
  NotifyAllListeners( PEvent( new EventROISelectByActor( (vtkActor*)prop) ) );
  return true;
}

bool qROIViz::OnRightButton(int x, int y)
{
	// check if a voi was clicked
	_propPicker->PickProp(x, y, _renderer, _propCollection);
	vtkProp *prop = _propPicker->GetViewProp();
	if(!prop)
		return false;
	
	NotifyAllListeners( PEvent( new Event(SHOW_ROI_PANEL) ) );
	return true;
}
bool qROIViz::OnLeftUp(int x, int y)
{
	if(_move_tool->OnLeftUp(x,y))
		return true;
	if(_scale_tool->OnLeftUp(x,y))
		return true;
	return false;
}
bool qROIViz::OnMouseMove(int dx, int dy, int state)
{
	Vector3d out;
	// if the move tool is visible
	if(_move_tool->Visible())
	{
		// is it selected?
		if(!_move_tool->SelectedProp())
			return false;
		// drag the move tool
		_move_tool->OnDrag(dx,dy, out);

		//Add the original position
		Vector3d pos = _move_tool->Position();
		for(int i = 0; i < 3; i++)
			out[i] = out[i] + pos[i];

		// check if the voi is still within the bounds of the image
		for(int i = 0; i < 3; i++)
		{
			if( out[i] < _pos_min[i])  out[i] = _pos_min[i];
			if( out[i] > _pos_max[i])  out[i] = _pos_max[i];
		}

		// check if the position has actually changed
		if( fabs( pos[0]-out[0] ) < 0.001 && fabs( pos[1]-out[1] ) < 0.001 && fabs( pos[2]-out[2] ) < 0.001)
			return true;

		// position has changed
		NotifyAllListeners( PEvent( new EventROITranslate( out ) ) );
	}
	// is scale tool visible?
	else if(_scale_tool->Visible())
	{
		// is scale tool selected?
		if(!_scale_tool->SelectedProp())
			return false;
		// if so, drag the scale tool
		_scale_tool->OnDrag(dx,dy, out);

		//Multiply by the original scale
		Vector3d scale = _scale_tool->Scale();

		// check if the voi still remains within the image with this new scale
		for(int i = 0; i < 3; i++)
		{
			out[i] = scale[i]*pow(1.02, out[i]);
			if( out[i] > _scale_max[i] ) out[i] = _scale_max[i];
		}
		
		//Check if the position has actually changed
		if( fabs( scale[0]-out[0] ) < 0.001 && fabs( scale[1]-out[1] ) < 0.001 && fabs( scale[2]-out[2] ) < 0.001)
			return true;

		//Scale has changed
		NotifyAllListeners( PEvent( new EventROIScale( out ) ) );
	}
	return false;
}
bool qROIViz::OnKeyDown(char key)
{
	bool visible;
	bool no = false;
	switch(key)
	{
		// toggle the visibility of move tool
	case 'e':
	case 'E':
		visible = !_move_tool->Visible();
		_move_tool->SetVisible(visible);
		_scale_tool->SetVisible(no);
		_renderer->GetRenderWindow()->Render();
		return true;
		break;

		// toggle the visibility of scale tool
	case 'r':
	case 'R':
		visible = !_scale_tool->Visible();
		_scale_tool->SetVisible(visible);
		_move_tool->SetVisible(no);
		_renderer->GetRenderWindow()->Render();
		return true;
		break;
	}
	return false;
}

void qROIViz::Update(ROIManager &mgr)
{
	//Clear the prop list
	_propCollection->RemoveAllItems();

	// are there any ROI in the voi manager?
	if(mgr.size() == 0) // no
	{
		//Hide the tools
		bool visible = false;
		_move_tool->SetVisible(visible);
		_scale_tool->SetVisible(visible);
	}
	else // yes
	{
		//Add items to prop collection
		for(PDTIFilterROIList::iterator iter = mgr.begin(); iter != mgr.end(); iter++)
			_propCollection->AddItem(  ((vtkROI*) (*iter).get())->Actor() );

		PDTIFilterROI selected = mgr.Selected();
		
		//Set the move and scale tool's position to that of the selected voi
		double *pos = selected->Position();
		Vector3d vpos(pos);
		_move_tool->SetPosition(vpos);
		_scale_tool->SetPosition(vpos);
	
		Vector3d actualSize = selected->Size()*Vector3d(selected->Scale())/2;
		// update the min, max bounds within which the voi can be moved and still be inside the image
		_pos_min = _min + actualSize;
		_pos_max = _max - actualSize;
		
		// update the min, max bounds within which the voi can be scaled and still be inside the image
		_scale_max = min( Vector3d(selected->Position())-_min, _max-Vector3d(selected->Position()) );
		_scale_max = _scale_max / selected->Size() * 2;

	}
}
void qROIViz::Dolly(double scale)
{
	// when zooming in/out we need to keep the scale of the tools constant, 
	// so we need to multiply with the inverse scale
	Vector3d invscale(1/scale, 1/scale, 1/scale);
	
	_move_tool->SetScale(invscale);
	_scale_tool->SetScale(invscale);
}
void qROIViz::OnEvent(PEvent evt)
{
}

void qROIViz::SetROIBounds(Vector3d min, Vector3d max) 
{ 
	// set the bounds within which the voi can be moved or scaled
	_pos_min = _min = min; 
	_pos_max = _max = max; 
}


void qROIViz::show_move_tool(bool b)
{
	bool yes = b, no = !b;
	_move_tool->SetVisible(yes);
	_scale_tool->SetVisible(no);
	_renderer->GetRenderWindow()->Render();
}
void qROIViz::Serialize  (std::ostream &s, int version)
{
	s<<"--- Move tool ---"<<endl;
	_move_tool->Serialize(s,version);
	s<<"--- Scale tool ---"<<endl;
	_scale_tool->Serialize(s,version);
	s<<endl;
}
void qROIViz::Deserialize(std::istream &s, int version)
{
	s>>"--- Move tool ---">>endl;
	_move_tool->Deserialize(s,version);
	s>>"--- Scale tool ---">>endl;
	_scale_tool->Deserialize(s,version);
	s>>endl;
}
