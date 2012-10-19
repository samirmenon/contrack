#include "Overlay.h"
#include "vtkOpenGLImageOverlayActor.h"
#include "vtkImageData.h"
#include "vtkMatrix4x4.h"
#include "vtkImageMapToColors.h"
#include "vtkLookupTable.h"
#include "vtkWindowLevelLookupTable.h"
#include "vtkRenderer.h"
#include "ColorMapPanel.h"
#include "vtkRenderWindow.h"
#include "vtkMapper.h"
#include "vtkOpenGLImageActor.h"
#include "qVolumeViz.h"

class vtkCustomImageActor : public vtkOpenGLImageActor
{
public:

  static vtkCustomImageActor *New()

  {

	  // First try to create the object from the vtkObjectFactory

	  return new vtkCustomImageActor();

  }

  virtual int RenderTranslucentPolygonalGeometry(vtkViewport *viewport)

  {

      double f, u;

      vtkMapper::GetResolveCoincidentTopologyPolygonOffsetParameters(f,u);

      vtkMapper::SetResolveCoincidentTopologyPolygonOffsetParameters(1,1);	
      //vtkMapper::SetResolveCoincidentTopologyPolygonOffsetParameters(-10,-10);



	  int retval = vtkImageActor::RenderTranslucentPolygonalGeometry(viewport);



      vtkMapper::SetResolveCoincidentTopologyPolygonOffsetParameters(f,u);

	  return retval;

  }

};

#define for_j 	for(int j = 0; j < 2; j++)
#define for_ij 	for(int i = 0; i < 3; i++) for(int j = 0; j < 2; j++)
#define for_ij_m 	for(int i = 0; i < 3; i++) for(int j = 0; j < 2; j++){

bool Overlay::bvisible[3] = {true, true, true};

Overlay::Overlay(vtkRenderer *renderer, DTIScalarVolume *vol, float offset, float opacity, int cmi, ColorMapPanel* cmpanel)
{
	_renderer = renderer;
	_visible = false;
	_opacity = opacity;
	_colorMapIndex = cmi;
	_cmapPanel = cmpanel;
	
	// TONY
	_offset = 0.01;
	//_offset = offset;

	// compute the range of values in the volume
	_min = vol->getCalcMinVal(); _max = vol->getCalcMaxVal();
	_left = _min+0.0*(_max-_min); _right = _max-0.0*(_max-_min);

	memcpy(_mat, vol->getTransformMatrix(), 16*sizeof(double));
	vtkMatrix4x4 *mx = vtkMatrix4x4::New();
	mx->DeepCopy (vol->getTransformMatrix());

	// create the overlays. Each slice has a front and back overlay.
	// For 3 slice we have a total of 6 overlays
	for_ij_m
		//_overlays[i][j] = vtkCustomImageActor::New();
		_overlays[i][j] = vtkOpenGLImageOverlayActor::New();
		_overlays[i][j]->SetUserMatrix(mx);
	}

	vol->getVoxelSize(_vox_sz[0], _vox_sz[1], _vox_sz[2]);
	//SetOffset(offset);

	//double viewDir[3]={1,1,1};
	//AdjustOffset();
	mx->Delete();

	// cache the image data of the volume into _img
	vol->getDimension(_dim[0], _dim[1], _dim[2], _dim[3]);
	_img = vtkImageData::New();
	_img->SetScalarTypeToFloat();
	_img->SetNumberOfScalarComponents(_dim[3]); 
	_img->SetDimensions (_dim[0], _dim[1], _dim[2]);
	_img->SetSpacing (1,1,1);
	if(_dim[3] != 1)return;

	_img->SetScalarTypeToFloat();
	_img->AllocateScalars();
	float *dataPtr = vol->getDataPointer();
    float *destinationPtr = static_cast<float *>(_img->GetScalarPointer());  
	// copy vol to _img
    memcpy (destinationPtr, dataPtr, _dim[0]*_dim[1]*_dim[2]*_dim[3]*sizeof(float));
	_img->Modified();

	// add the overlays to the renderer
	for_ij
		_renderer->AddActor(_overlays[i][j]);

	
	SetOpacity(_opacity);
	SetRange(_left, _right);
	SetVisible(_visible);
}

Overlay::~Overlay()
{
	_img->Delete();

	// remove all overlays
	for_ij
	{
		_renderer->RemoveActor(_overlays[i][j]);
		_overlays[i][j]->Delete();
	}
}
void Overlay::SetOpacity(float &o)
{
	if(fabsf(_opacity-o)<0.0001)return;
	_opacity = o;

	// set the opacity of the overlays
	for_ij
		_overlays[i][j]->SetOpacity(_opacity);
}
void Overlay::SetVisible(bool &vis)
{
	if(_visible==vis)return;
	_visible = vis;

	// set the visibility of the overlays
	for_ij
		_overlays[i][j]->SetVisibility(_visible && bvisible[i]);
}
void Overlay::SetVisible(DTISceneActorID imageId, bool vis)
{
	// set the visibility of a specific slice
	//for(int j = 0; j < 2; j++)
	for_j
		_overlays[(int)imageId][j]->SetVisibility(vis && _visible);
	bvisible[imageId] = vis;
//	_renderer->GetRenderWindow()->Render();
}
void Overlay::SetOffset(float &o)
{
	for (int i=0; i<3; i++) 
	{
		double deltaP[] = {0,0,0};
		double deltaN[] = {0,0,0};
		deltaP[i] = o*_vox_sz[i];
		deltaN[i] = -o*_vox_sz[i];
		_overlays[i][0]->AddPosition(deltaP);
		_overlays[i][1]->AddPosition(deltaN);
	}
/*	for(int i = 0; i < 3; i++)
	{
		double delta[]={0,0,0}, delta1[]={0,0,0};
		delta[i]=10; delta1[i]=-10;
		_overlays[i][0]->AddPosition(delta);
		_overlays[i][0]->AddPosition(delta1);
	}*/
}
void Overlay::AdjustOffset(double viewDir[3])
{
	return;
	//for(int i = 0; i < 3; i++)
	//for(int j = 0; j < 2; j++)
	//{
	for_ij
	{
		vtkMatrix4x4 *mat = vtkMatrix4x4::New();
		mat->DeepCopy (_mat);
		double off = 3;

		mat->SetElement(i,3,_mat[i*4+3]+(1-2*j)*fabs(off*viewDir[i]));
		_overlays[i][j]->SetUserMatrix(mat);
		_overlays[i][j]->Modified();
		mat->Delete();
	}
}
void Overlay::SetColorMapIndex(int &o)
{
	// set a colormap
	if(_colorMapIndex==o)return;
	_colorMapIndex = o; 
	setupColorMap();
}
void Overlay::SetRange(float left, float right)
{
	// set the range of values in the source image that will be mapped to the color map
	if(fabsf(_left-left)<0.0001 && fabsf(_right-right) < 0.0001)return;
	_left = left; _right = right;
	setupColorMap();
}
void Overlay::setupColorMap()
{
	// for each overlay
	for_ij
	{
		//Set up the look up table
		vtkLookupTable *lut = vtkLookupTable::New();
		//ColorMap &c = the_global_frame->ColorMapPanel_()->ColorMaps()[_colorMapIndex];
		ColorMap &c = _cmapPanel->ColorMaps()[_colorMapIndex];
		lut->SetNumberOfTableValues(c.Colors.size());

		// add colors to the look up table
		for(int k = 0; k < (int)c.Colors.size(); k++)
			lut->SetTableValue(k,c.Colors[k].r/255.0, c.Colors[k].g/255.0, c.Colors[k].b/255.0, c.Colors[k].a/255.0);
		lut->SetTableRange(_left, _right);

		// map the overlay colors using the look up table
		vtkImageMapToColors *colors = vtkImageMapToColors::New();
		colors->SetInput(_img);
		colors->SetLookupTable(lut);
		_overlays[i][j]->SetInput(colors->GetOutput());
		_overlays[i][j]->SetVisibility(_visible && bvisible[i]);
		colors->Delete();
	}
	//the_global_frame->Refresh();
}
void Overlay::SetPosition(double *wPos, double *tPos)
{
	DTIVoxel v =DTIVoxel(3);
	DTIVoxel lDim = DTIVoxel(3);
	lDim[0]=_dim[0]-1; lDim[1]=_dim[1]-1; lDim[2]=_dim[2]-1;
	DTIVoxel lZero = DTIVoxel(3);
	lZero[0]=0; lZero[1]=0; lZero[2]=0;
	double wDim[3];
	double wZero[3];
	qVolumeViz::WorldToLocal(_mat,wPos,_dim, v);
	qVolumeViz::LocalToWorld(_mat,lDim,wDim);
	qVolumeViz::LocalToWorld(_mat,lZero,wZero);
	float v1[3] = {v[0], v[1], v[2]};
	//for(int i=0; i<3; i++) { flPos[i]/=(double)_vox_sz[i]; }
	
	double flPos[3];
	qVolumeViz::WorldToLocal(_mat,wPos,_dim,flPos);
	

	// set the slice to be displayed
	//for(int j = 0; j < 2; j++)
	//{
	for_j 
	{
		// TONY
		// Need to simply handle the difference between voxel sizes here 
		float o = _offset + -2*j*_offset; // j==0 --> _offset, j==1 --> -_offset
		_overlays[0][j]->SetQuadExtent(flPos[0]+o,flPos[0]+o, 0, _dim[1]-1, 0, _dim[2]-1);
		_overlays[1][j]->SetQuadExtent(0,_dim[0]-1, flPos[1]+o,flPos[1]+o, 0,_dim[2]-1);
		_overlays[2][j]->SetQuadExtent(0,_dim[0]-1, 0,_dim[1]-1, flPos[2]+o,flPos[2]+o);
		
		//_overlays[0][j]->SetQuadExtent(wPos[0],wPos[0], wZero[1], wDim[1], wZero[2], wDim[2]);
		//_overlays[1][j]->SetQuadExtent(wZero[0],wDim[0], wPos[1],wPos[1], wZero[2],wDim[2]);
		//_overlays[2][j]->SetQuadExtent(wZero[0],wDim[0], ,wDim[1], wPos[2],wPos[2]);
		
		_overlays[0][j]->SetDisplayExtent(v1[0],v1[0], 0, _dim[1]-1, 0, _dim[2]-1);
		_overlays[1][j]->SetDisplayExtent(0,_dim[0]-1, v1[1],v1[1], 0,_dim[2]-1);
		_overlays[2][j]->SetDisplayExtent(0,_dim[0]-1, 0,_dim[1]-1, v1[2],v1[2]);
	}

	for_ij
		_overlays[i][j]->Modified();
}
void Overlay::OnEvent(PEvent evt)
{
	// color map has been changed?
	if(evt->EventType() == COLOR_MAP_CHANGED )
		SetColorMapIndex((dynamic_cast<EventColorMapChanged*>(evt.get()))->Value());

	// overlay's visibility has changed
	else if(evt->EventType() == OVERLAY_VISIBILITY_CHANGED)
	{
		SetVisible((dynamic_cast<EventOverlayVisibilityChanged*>(evt.get()))->Value());
		// refresh
		_renderer->GetRenderWindow()->Render();
	}
	
	// opacity was changed
	//else if(evt->EventType() == OVERLAY_OPACITY_CHANGED)
//		SetOpacity((dynamic_cast<EventOverlayOpacityChanged*>(evt.get()))->Value());

	// range of colors in the source image to be mapped, was changed
	//else if(evt->EventType() == OVERLAY_RANGE_CHANGED)
//	{
//		EventOverlayRangeChanged* e = dynamic_cast<EventOverlayRangeChanged*>(evt.get());
//		SetRange(e->Min(), e->Max());
//	}
}
void Overlay::Serialize(std::ostream &s, int version)
{
	s<<"Opacity :"<<_opacity<<endl;
	s<<"Range :"<<_left<<" "<<_right<<endl;
	s<<"ColorMapIndex :"<<_colorMapIndex<<endl;
	s<<"Visible :"<<_visible<<endl;
	s<<endl;
}
void Overlay::Deserialize(std::istream &s, int version)
{
	float op, l,r;
	int idx;
	bool vis;

	s>>"Opacity :">>op>>endl; SetOpacity(op);
	s>>"Range :">>l>>" ">>r>>endl; SetRange(l, r);
	s>>"ColorMapIndex :">>idx>>endl; SetColorMapIndex(idx);
	s>>"Visible :">>vis>>endl; SetVisible(vis);
	s>>endl;
}

