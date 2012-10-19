/***********************************************************************
* AUTHOR: David Akers <dakers>
*   FILE: .//vtkROI.cpp
*   DATE: Thu Sep  1 08:57:10 2005
*  DESCR: 
***********************************************************************/
#include "vtkROI.h"
#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkFilledCaptionActor2D.h"
#include "vtkCellArray.h"
#include "vtkTextProperty.h"
#include "vtkProperty.h"
#include "vtkCubeSource.h"
#include "vtkSphereSource.h"
#include "vtkTriangleFilter.h"
#include "RAPIDHelper.h"

vtkROI::vtkROI(ROIType type, vtkRenderer *r, int id, Vector3d pos, PvtkPolyData pData, std::string name, Colord col) : 
  DTIFilterROI(type, RAPIDHelper::RAPIDModelFromPolyData(pData.GetPointer()), col)
{
	_name = name; 
	_renderer = r;
	//	bVisible=bShowAll=true;
	bLabelVisible = bMeshVisible = true;
	_id = id;
	if(_name=="") { char name[100]; sprintf(name,"%d",_id); _name = name; }
	// create the 2d text actor
	_textActor  = vtkFilledCaptionActor2D::New();
	_textActor->SetHeight(.03);
	_textActor->GetCaptionTextProperty()->SetColor(1,1,1);
	_textActor->GetCaptionTextProperty()->SetFontFamilyToArial();
	_textActor->SetVisibility (true);

	_actor = vtkActor::New();
	_mesh_pd = pData;

	_mapper = vtkPolyDataMapper::New();
	_mapper->SetInput(_mesh_pd);

	_actor->SetMapper(_mapper);
	_actor->GetProperty()->SetSpecular(.3);
	_actor->GetProperty()->SetSpecularPower(20);

	_renderer->AddActor(_actor);
	_renderer->AddActor(_textActor);

	SetName(_name);
	//	cerr << "Establishing position at " << pos.v[0] << ", " << pos.v[1] << ", " << pos.v[2] << endl;
	SetPosition(pos.v);
	SetColor(col);
}
vtkROI::~vtkROI()
{
	//if( the_global_frame && !(the_global_frame->_bRendererDeleted) )
	{
		_renderer->RemoveActor(_actor);
		_renderer->RemoveActor(_textActor);
	}
	VTK_SAFE_DELETE(_actor);
	VTK_SAFE_DELETE(_mapper);
	//VTK_SAFE_DELETE(_mesh_pd);
	VTK_SAFE_DELETE(_textActor);
}
void vtkROI::SetPosition(const double T[3])
{
	// set the position of the voi
	_actor->SetPosition((double*)T);
	// set the position of the text actor
	double *bounds = _actor->GetBounds();
	double minPoint[3] = {bounds[0], bounds[2], bounds[4]};
	_textActor->SetAttachmentPoint((double*)minPoint);
	// set the position in util, for RAPID based intersection
	DTIFilterROI::SetPosition(T);
}
void vtkROI::SetScale(const double scales[3])
{
//    cerr << "SetScale: " << scales[0] << ", " << scales[1] << ", " << scales[2] << endl;
	// update the scale of the RAPID model
	DTIFilterROI::SetScale(scales);   
	// update the scale of the voi actor
	_actor->SetScale((double*)scales);
}
void vtkROI::SetColor(Colord &col)
{
	_actor->GetProperty()->SetOpacity(col.a);
	_actor->GetProperty()->SetDiffuseColor(col.value);
	DTIFilterROI::SetColor(col);
}
void vtkROI::SetName(std::string &name)
{
	int len = (int)name.size();
	if(!len)return;
	_name = name;

	_textActor->SetCaption(_name.c_str());
	_textActor->SetWidth(0.06*len);
	// xxx What is StaticOffset?? It seems to have complete control
	// over the size of the caption.

	_textActor->SetStaticOffset(8.0 + 8.5*len);
        //cerr << "width: " << _textActor->GetWidth() << endl;
	//cerr << "height: " << _textActor->GetHeight() << endl;
	_textActor->Modified();
}
/*
void vtkROI::SetVisible(bool &b){ bVisible = b; SetShowAll(bShowAll); }
void vtkROI::SetShowAll(bool &b)
{
	// this global flag toggles visibility of all the voi at one go
	bShowAll = b; 
	_actor->SetVisibility(bVisible && bShowAll);  
	_textActor->SetVisibility(bShowLabel && bShowAll);
}

void vtkROI::SetShowLabel(bool &b){bShowLabel = b; _textActor->SetVisibility(bShowLabel && bShowAll);}
*/

void vtkROI::SetShowROI(bool b)
{
  bMeshVisible = b;
  UpdateVisibility();
}

void vtkROI::SetShowLabel(bool b)
{
  bLabelVisible = b;
  UpdateVisibility();
}

void vtkROI::UpdateVisibility()
{
  _actor->SetVisibility(bMeshVisible);
  _textActor->SetVisibility(bMeshVisible && bLabelVisible);
  // xxx dla bounding boxes?
}

vtkROI* vtkROI::CreateCube(vtkRenderer *r, int id, Vector3d pos, Vector3d scale, std::string name, Colord col)
{
	// create a cube based voi
	vtkSmartPointer<vtkCubeSource> cubeData = vtkSmartPointer<vtkCubeSource>::New();

	//Create a triangle filter to convert the cube data to polygon mesh
	vtkTriangleFilter *triFilter;
	triFilter = vtkTriangleFilter::New();
	triFilter->SetInput(cubeData->GetOutput());
	triFilter->Update();

	// convert the polydata to rapid model
	PvtkPolyData pdata = PvtkPolyData::New();
	pdata->DeepCopy(triFilter->GetOutput());
	vtkROI* voi = new vtkROI(ROI_TYPE_CUBE,r,id,pos,pdata,name,col);
	voi->SetScale(scale.v);
	triFilter->Delete();
	return voi;
}

vtkROI* vtkROI::CreateSphere(vtkRenderer *r, int id, Vector3d pos, double radius, int resolution, std::string name, Colord col)
{
	// create a sphere based voi
	vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
	//sphere->SetCenter(pos.v);
	sphere->SetRadius(0.5l);
	sphere->SetThetaResolution(resolution);
	sphere->SetPhiResolution(resolution);

	// create a triangle filter to convert the sphere data to polygon mesh
	vtkTriangleFilter *triFilter;
	triFilter = vtkTriangleFilter::New();
	triFilter->SetInput(sphere->GetOutput());
	triFilter->Update();

	// convert polydata to RAPID polygon mesh
	PvtkPolyData pdata = PvtkPolyData::New();
	pdata->DeepCopy(triFilter->GetOutput());
	vtkROI* voi = new vtkROI(ROI_TYPE_SPHERE,r,id,pos,pdata,name,col);
	voi->SetScale(Vector3d(radius,radius, radius).v);
	triFilter->Delete();
	return voi;
}

// Create a new arbitrary mesh ROI:

vtkROI* vtkROI::CreateMesh(vtkRenderer *r, int id, Vector3d pos, vtkPolyData *polyData, std::string name, Colord col)
{
        // Convert the mesh data to triangles
        // xxxdla - is the polydata already stored as triangles? This could save
        // time, since we wouldn't need to apply the triangle filter.
        vtkTriangleFilter *triFilter;
	triFilter = vtkTriangleFilter::New();
	triFilter->SetInput(polyData);
	triFilter->Update();
	
	// convert polydata to RAPID polygon mesh
	PvtkPolyData pdata = PvtkPolyData::New();
	pdata->DeepCopy(triFilter->GetOutput());
	vtkROI* voi = new vtkROI(ROI_TYPE_IMAGE,r,id,pos,pdata,name,col);
	
	// Scale is currently set to (1,1,1) for arbitrary meshes, since the
	// vertices are already scaled). Alternatively, one could compute
	// a bounding box to use as the scale, and normalize the coordinates.
	voi->SetScale(Vector3d(1.0, 1.0, 1.0).v);

	triFilter->Delete();
	return voi;
}
