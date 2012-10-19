#include "vtkRenderer.h"
#include "vtkActor.h"
#include "vtkProperty.h"
#include "vtkPolyDataMapper.h"
#include "vtkCubeSource.h"
#include "vtkArrowSource.h"
#include "qMoveTool.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTransform.h"
#include "vtkPolyData.h"
#include "vtkCellArray.h"
#include "VisibilityPanel.h"
#include "vtkFollower.h"
#include "vtkVectorText.h"
#include "vtkPropPicker.h"
#include "vtkPropCollection.h"
#include "vtkCamera.h"
#include "vtkCoordinate.h"
#include "vtkRenderWindow.h"

void WorldToDisplay(double *pt, double *origin, vtkCoordinate *coordinate, vtkRenderer *renderer)
{
	coordinate->SetValue(pt[0],pt[1],pt[2]); 
	double *temp = coordinate->GetComputedDoubleDisplayValue(renderer); 
	pt[0]=temp[0]-origin[0]; pt[1]=temp[1]-origin[1];
	pt[2] = sqrt(pt[0]*pt[0] + pt[1]*pt[1]);
}

qManipulationTool::	qManipulationTool(vtkRenderer *renderer, int numAxis)
{
	_scale = Vector3d(1,1,1);
	_selected_prop = 0;
	_renderer = renderer;
	_nAxis = numAxis;

	_propPicker = vtkPropPicker::New();
	_propCollection = vtkPropCollection::New();

	//Create the axis and plane actors
	_axis_arrows = new vtkFollower* [_nAxis];
	_axis_planes = new vtkFollower* [_nAxis];

	for(int i = 0; i < _nAxis; i++)
	{
		_axis_arrows[i] = vtkFollower::New();
		_axis_planes[i] = vtkFollower::New();

		//Add actors to renderer
		_renderer->AddActor(_axis_arrows[i]);
		_renderer->AddActor(_axis_planes[i]);

		//Prop collection
		_propCollection->AddItem(_axis_arrows[i]);
		_propCollection->AddItem(_axis_planes[i]);
	}
}
qManipulationTool::~qManipulationTool()
{
	for(int i = 0; i < _nAxis; i++)
	{
		//Remove from renderer
		_renderer->RemoveActor(_axis_arrows[i]);
		_renderer->RemoveActor(_axis_planes[i]);

		//Release memory
		VTK_SAFE_DELETE(_axis_arrows[i]);
		VTK_SAFE_DELETE(_axis_planes[i]);
	}
	delete []_axis_arrows;
	delete []_axis_planes;

	VTK_SAFE_DELETE(_propPicker);
	VTK_SAFE_DELETE(_propCollection);
}
bool qManipulationTool::OnLeftDown(int x, int y)
{
	_selected_prop = 0;
	if(!_propCollection || !_propPicker)return false;
	_propPicker->PickProp(x, y, _renderer, _propCollection);

	vtkProp *prop = _propPicker->GetViewProp();
	for(int i = 0; i < _nAxis; i++)
		if(_axis_planes[i] == prop ||  _axis_arrows[i] == prop)
		{
			//change the opacity
			if(_axis_planes[i] == prop)
			{
				_axis_planes[i]->GetProperty()->SetRepresentationToSurface();
				_axis_planes[i]->GetProperty()->SetDiffuseColor(1,1,0);
				_axis_planes[i]->GetProperty()->SetOpacity(0.5);
			}
			else
				_axis_arrows[i]->GetProperty()->SetOpacity(1);

			_renderer->GetRenderWindow()->Render();
			_selected_prop = (vtkFollower*)prop;
			return true;
		}
		return false;
}
bool qManipulationTool::OnLeftUp(int x, int y)
{
	if(!_selected_prop)
		return false;

	//Set the colors appropriately
	for(int i = 0; i < _nAxis; i++)
		if(_axis_planes[i] == _selected_prop)
		{
			_axis_planes[i]->GetProperty()->SetRepresentationToWireframe();
			_axis_planes[i]->GetProperty()->SetDiffuseColor(0,0,0);
			_axis_planes[i]->GetProperty()->SetOpacity(1);
			_renderer->GetRenderWindow()->Render();
			break;
		}
		else if(_axis_arrows[i] == _selected_prop)
		{
			_axis_arrows[i]->GetProperty()->SetOpacity(0.2);
			_renderer->GetRenderWindow()->Render();
			break;
		}


		_selected_prop = 0;
		return true;
}
void qManipulationTool::SetVisible(bool &show)
{
	_visible = show;
	for(int i = 0; i < _nAxis; i++)
	{	
		_axis_planes[i]->SetVisibility(show?1:0);
		_axis_arrows[i]->SetVisibility(show?1:0);
	}
}
void qManipulationTool::SetScale(Vector3d &scale)
{
	double *s = _axis_planes[0]->GetScale();
	_scale = Vector3d(s[0]*scale.x, s[1]*scale.y, s[2]*scale.z);

	for(int i = 0; i < _nAxis; i++)
	{	
		_axis_planes[i]->SetScale(_scale.x, _scale.y, _scale.z);
		_axis_arrows[i]->SetScale(_scale.x, _scale.y, _scale.z);
	}
}
void qManipulationTool::SetPosition(Vector3d &pos)
{
	_position = pos;
	for(int i = 0; i < _nAxis; i++)
	{	
		_axis_planes[i]->SetPosition(pos[0],pos[1],pos[2]);
		_axis_arrows[i]->SetPosition(pos[0],pos[1],pos[2]);
	}
}
void qManipulationTool::Serialize  (std::ostream &s, int version)
{
	s<<"Position: "; _position.Serialize(s,version); s<<endl;
	s<<"Scale :";	_scale.Serialize(s,version); s<<endl;
	s<<"Visible : "<<_visible<<endl;
}
void qManipulationTool::Deserialize(std::istream &s, int version)
{
	Vector3d newScale;
	s>>"Position: "; _position.Deserialize(s,version); s>>endl;
	s>>"Scale :";	 newScale.Deserialize(s,version); s>>endl;
	s>>"Visible : ">>_visible>>endl;

	newScale = newScale * Vector3d(1/_scale.x, 1/_scale.y, 1/_scale.z);
	SetPosition(_position);
	SetScale(newScale);
	SetVisible(_visible);
}
void InitAxis(vtkActor *arrow, vtkActor *plane, Colord col, Vector3d *points, int numPoints, Vector3d angles)
{
	vtkTransform *transform = vtkTransform::New();
	transform->Identity();		

	//Create the arrow

	transform->RotateX(angles.x);
	transform->RotateY(angles.y);
	transform->RotateZ(angles.z);

	vtkArrowSource* src = vtkArrowSource::New();
	vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
	vtkTransformPolyDataFilter *filter= vtkTransformPolyDataFilter::New();
	src->SetShaftResolution(24);
	src->SetTipResolution(24);
	filter->SetTransform(transform);
	filter->SetInputConnection(src->GetOutputPort());
	mapper->SetInputConnection(filter->GetOutputPort());
	arrow->SetMapper(mapper);
	arrow->GetProperty()->SetColor(col.r, col.g, col.b);
	arrow->GetProperty()->SetOpacity(0.2);
	mapper->Delete();
	src->Delete();
	filter->Delete();
	transform->Delete();

	//Create the plane
	mapper = vtkPolyDataMapper::New();
	CreateRectangle3D(&plane, points[0].v, points[1].v, points[2].v, points[3%numPoints].v,false);
	plane->GetProperty()->SetSpecularPower(0);
	plane->GetProperty()->SetDiffuseColor(0,0,0);
	plane->GetProperty()->SetOpacity(1);
	plane->GetProperty()->SetRepresentationToWireframe();
	mapper->Delete();	
}

qMoveTool::qMoveTool(vtkRenderer *renderer) : qManipulationTool(renderer, 3)
{
	Vector3d points[4];

	double s = 0.5;
	//X axis is in xz-plane
	points[0] = Vector3d(0,0,0); points[1] = Vector3d(s,0,0); points[2] = Vector3d(s,0,s); points[3] = Vector3d(0,0,s); 
	InitAxis(_axis_arrows[0], _axis_planes[0], Colord(1,0,0), points, 4, Vector3d(0,0,0));	

	//Y axis is in yz-plane
	points[0] = Vector3d(0,0,0); points[1] = Vector3d(0,s,0); points[2] = Vector3d(0,s,s); points[3] = Vector3d(0,0,s); 
	InitAxis(_axis_arrows[1], _axis_planes[1], Colord(0,1,0), points, 4, Vector3d(0,0,90));	

	//Z axis is in xy-plane
	points[0] = Vector3d(0,0,0); points[1] = Vector3d(s,0,0); points[2] = Vector3d(s,s,0); points[3] = Vector3d(0,s,0); 
	InitAxis(_axis_arrows[2], _axis_planes[2], Colord(0,0,1), points, 4, Vector3d(0,-90,0));	
}
void qMoveTool::OnDrag(int dx, int dy, Vector3d &out)
{	
	//if nothing selected, then return
	if(!_selected_prop)
		return;

	vtkCoordinate * coordinate = vtkCoordinate::New();
	coordinate->SetCoordinateSystemToWorld(); 
	double origin[3]={0,0,0}, axes[3][3]={{10,0,0},{0,10,0},{0,0,10}};
	double *temp;
	int axesMap[][2] = { {0,2},{1,2},{0,1} };

	coordinate->SetValue(0,0,0); temp = coordinate->GetComputedDoubleDisplayValue( _renderer ); origin[0]=temp[0]; origin[1]=temp[1];

	out = Vector3d(.0,.0,.0);

	//Add the translation
	for(int i = 0; i < _nAxis; i++)
	{
		if(_axis_arrows[i] == _selected_prop)
		{
			WorldToDisplay(axes[i],origin,coordinate,_renderer);	
			out[i]=(axes[i][0]*dx+axes[i][1]*dy)/axes[i][2];
			break;
		}
		else if(_axis_planes[i] == _selected_prop)
		{	
			int j = axesMap[i][0];
			int k = axesMap[i][1];
			WorldToDisplay(axes[j],origin,coordinate,_renderer);	
			WorldToDisplay(axes[k],origin,coordinate,_renderer);	

			out[j]=(axes[j][0]*dx+axes[j][1]*dy)/axes[j][2];
			out[k]=(axes[k][0]*dx+axes[k][1]*dy)/axes[k][2];
			break;
		}
	}

	coordinate->Delete();
}

qScaleTool::qScaleTool(vtkRenderer *renderer) : qManipulationTool(renderer, 4)
{
	Vector3d points[4];

	double s1 = 0.3;
	double s2 = 0.4;
	//X axis is in xz-plane
	points[0] = Vector3d(s1,0,0); points[1] = Vector3d(s2,0,0); points[2] = Vector3d(0,0,s2); points[3] = Vector3d(0,0,s1); 
	InitAxis(_axis_arrows[0], _axis_planes[0], Colord(1,0,0), points, 4, Vector3d(0,0,0));	

	//Y axis is in yz-plane
	points[0] = Vector3d(0,s1,0); points[1] = Vector3d(0,s2,0); points[2] = Vector3d(0,0,s2); points[3] = Vector3d(0,0,s1); 
	InitAxis(_axis_arrows[1], _axis_planes[1], Colord(0,1,0), points, 4, Vector3d(0,0,90));	

	//Z axis is in xy-plane
	points[0] = Vector3d(s1,0,0); points[1] = Vector3d(s2,0,0); points[2] = Vector3d(0,s2,0); points[3] = Vector3d(0,s1,0); 
	InitAxis(_axis_arrows[2], _axis_planes[2], Colord(0,0,1), points, 4, Vector3d(0,-90,0));	

	//middle plane for uniform scaling
	points[0] = Vector3d(s2,0,0); points[1] = Vector3d(0,s2,0); points[2] = Vector3d(0,0,s2); points[3] = Vector3d(0,0,s2); 
	InitAxis(_axis_arrows[3], _axis_planes[3], Colord(0,0,1), points, 4, Vector3d(0,0,0));	

	//Hide the last arrow
	_axis_arrows[3]->SetPosition(-1e5,-1e5,-1e5);

	//Set the camera
	for(int i = 0; i < _nAxis; i++)
	{
		//_axis_arrows[i]->SetCamera(_renderer->GetActiveCamera());
		//_axis_planes[i]->SetCamera(_renderer->GetActiveCamera());
	}
}
void qScaleTool::OnDrag(int dx, int dy, Vector3d &out)
{
	//if nothing selected, then return
	if(!_selected_prop)
		return;

	//Main plane is selected, so uniform scale
	if(_selected_prop == _axis_planes[3])
	{
		out = Vector3d(dx+dy, dx+dy, dx+dy);
		return;
	}

	vtkCoordinate * coordinate = vtkCoordinate::New();
	coordinate->SetCoordinateSystemToWorld(); 
	double origin[3]={0,0,0}, axes[3][3]={{10,0,0},{0,10,0},{0,0,10}};
	double *temp;
	int axesMap[][2] = { {0,2},{1,2},{0,1} };

	coordinate->SetValue(0,0,0); temp = coordinate->GetComputedDoubleDisplayValue( _renderer ); origin[0]=temp[0]; origin[1]=temp[1];

	out = Vector3d(.0,.0,.0);

	//Add the translation
	for(int i = 0; i < 3; i++)
	{
		if(_axis_arrows[i] == _selected_prop)
		{
			WorldToDisplay(axes[i],origin,coordinate,_renderer);	
			out[i]=(axes[i][0]*dx+axes[i][1]*dy)/axes[i][2];
			break;
		}
		else if(_axis_planes[i] == _selected_prop)
		{	
			int j = axesMap[i][0];
			int k = axesMap[i][1];
			WorldToDisplay(axes[j],origin,coordinate,_renderer);	
			WorldToDisplay(axes[k],origin,coordinate,_renderer);	

			out[j]=(axes[j][0]*dx+axes[j][1]*dy)/axes[j][2];
			out[k]=(axes[k][0]*dx+axes[k][1]*dy)/axes[k][2];
			break;
		}
	}

	coordinate->Delete();
}
