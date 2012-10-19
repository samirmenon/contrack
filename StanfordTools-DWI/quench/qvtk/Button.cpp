/***********************************************************************
* AUTHOR: David Akers <dakers>
*   FILE: .//ColorButton.cpp
*   DATE: Tue Nov 29 19:23:04 2005
*  DESCR: 
***********************************************************************/
#include "Button.h"
#include "vtkPolyData.h"
#include "vtkActor2D.h"
#include "vtkCellArray.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty2D.h"
#include "vtkActor.h"
#include "vtkTextActor.h"
#include "vtkTexture.h"
#include "typedefs_quench.h"
#include "vtkTextProperty.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkJPEGReader.h"
#include "vtkPNGReader.h"
#include "vtkTexture.h"
#include "vtkImageMapper.h"
#include "vtkImageData.h"
#include "vtkImageFlip.h"
#include "vtkTransform.h"
#include "vtkImageReslice.h"
#include "vtkRenderer.h"


Button::Button (vtkRenderer *renderer, std::string name, Point2i textOffset, Point2i size)
{
	_transparent = false;
	// A button has a border with optional text. It can be filled with a texture or a solid color
	int border = 1;
	_textOffset = textOffset;
	_size = size;
	_renderer=renderer;

	_actor = vtkActor2D::New();
	_outline_actor = 0;
	_crossthrough_actor = 0;

	// create the border for a button
#ifdef _WIN32
	CreateHollowRectangle2D(&_outline_actor, border, border, size.x, size.y);
#else
	// there is a one pixel offset in linux for some unknown reason, hence the special case
	CreateHollowRectangle2D(&_outline_actor, 0,0, size.x, size.y);
#endif
	_outline_actor ->GetProperty()->SetColor(0,0,0);
	_outline_actor ->GetProperty()->SetLineWidth(1);

	// create a cross out line
	 CreateLine2D(&_crossthrough_actor, border, border, size.x-border, size.y-border);
	_crossthrough_actor->GetProperty()->SetColor(0.3,0.3,0.3);
	_crossthrough_actor->GetProperty()->SetLineWidth(2);

	// text actor for button text
	_text = vtkTextActor::New();
	_text->SetInput (name.c_str());
	_text->GetTextProperty()->SetJustificationToCentered();
	_text->GetTextProperty()->SetColor(0,0,0);
	_text->GetPositionCoordinate()->SetValue(_textOffset.x, _textOffset.y);

	// add actors to renderers
	renderer->AddActor (_actor);
	renderer->AddActor (_text);
	renderer->AddActor (_outline_actor);
	renderer->AddActor (_crossthrough_actor);
	_crossthrough_actor->SetVisibility(false);
}
Button::~Button()
{	
	// remove actors from renderer and delete
	_renderer->RemoveActor(_actor); VTK_SAFE_DELETE(_actor); 
	_renderer->RemoveActor(_crossthrough_actor); VTK_SAFE_DELETE(_crossthrough_actor); 
	_renderer->RemoveActor(_outline_actor); VTK_SAFE_DELETE(_outline_actor); 
	_renderer->RemoveActor(_text); VTK_SAFE_DELETE(_text); 
}
void Button::SetState(Button::State state)
{
	// set the state of button, which can be selected, enabled or disabled
	_state = state;
	bool opaque = !_transparent;
	_actor->SetVisibility(opaque);
	_text->SetVisibility(true);

	_outline_actor->SetVisibility(state==SELECTED);
	_actor->GetProperty()->SetOpacity( (state != DISABLED ? 1 : 0.25)* opaque );
	_text ->GetProperty()->SetOpacity( (state != DISABLED ? 1 : 0.25));

	if(state == SELECTED)
		_text->GetTextProperty()->BoldOn();
	else
		_text->GetTextProperty()->BoldOff();
	_actor->Modified();

	//vtk bug: Need to hide text actor if empty
	_text->SetVisibility((int)strlen(_text->GetInput()));
}
void Button::SetCrossedOut(bool &b) { _crossthrough_actor->SetVisibility(b); SetState(_state); }
void Button::SetVisible(bool &bvis)
{
	_visible = bvis;
	if( _visible )
		SetState(_state);
	else
	// hide all actors
	{
		_actor->GetProperty()->SetOpacity(0);
		_text ->GetProperty()->SetOpacity(0);
		_outline_actor	  ->SetVisibility(false);
		_crossthrough_actor->SetVisibility(false);
		_actor->SetVisibility(false);
		_text->SetVisibility(false);
	}
}
void Button::SetTransparent(bool &trans)
{
	_transparent = trans;
	_actor->SetVisibility( !_transparent );
}
void Button::SetOpacity(double &o)
{
	// set the opactiy of all actors
	_opacity = o;
	_actor->GetProperty()->SetOpacity(o);
	_outline_actor->GetProperty()->SetOpacity(o);
	_crossthrough_actor->GetProperty()->SetOpacity(o);
	_text->GetProperty()->SetOpacity(o);
}
void Button::SetPosition(Point2i &pos)
{
	// set the 2d position of the button
	_position = pos;
	int x = _position.x, y = _position.y;
	_actor				->SetPosition(x,y);
	_outline_actor		->SetPosition(x,y);
	_crossthrough_actor	->SetPosition(x,y);
	_text				->SetPosition(x+_textOffset.x, y+_textOffset.y);
}
void Button::SetLineWidth (int linewidth) { _outline_actor ->GetProperty()->SetLineWidth(linewidth); }

ColorButton::ColorButton (vtkRenderer *renderer, std::string name, Colord col, Button::State state, Point2i textOffset, Point2i size)
	:Button(renderer,name,textOffset,size)
{
	// Create filled rectangle for the button
	CreateRectangle2D(&_actor, 0,0,size.x,size.y, false);
	_actor->GetProperty()->SetColor(col.r, col.g, col.b);
	SetState(state);
}
void ColorButton::SetColor(Colord col) 
{ 
	_actor->GetProperty()->SetColor(col.r, col.g, col.b); 
	_actor->Modified();
}


TextureButton::TextureButton(vtkRenderer *renderer, std::string name, Button::State state, Point2i textOffset, Point2i size, unsigned char *data, int bpp) 
	:Button(renderer,name,textOffset,size)
{
	// create a textured button
	unsigned char *data2 = data;

	//convert from rgb to rgba with alpha set to 255 by default. 
	// This is used later to change the opacity of the texture
	if(bpp == 3)
	{
		data2 = new unsigned char[size.x*size.y*4];
		for(int i = 0; i < size.x*size.y; i++)
		{
			for(int j = 0; j < 3; j++)
				data2[i*4+j] = data[i*3+j];
			data2[i*4+3] = 255;
		}
	}

	vtkImageMapper *mapper = vtkImageMapper::New();
	_imagedata = vtkImageData::New();
	_imagedata->SetScalarTypeToUnsignedChar();
	_imagedata->SetNumberOfScalarComponents(4);
	_imagedata->SetDimensions(size.x, size.y, 1);
	char *destinationPtr = (char *) _imagedata->GetScalarPointer();
	memcpy (destinationPtr, data2, size.x*size.y*4);

	// flip the y axis
	vtkImageFlip *flipY = vtkImageFlip::New();
	flipY->SetFilteredAxis(1);

	flipY->SetInput(_imagedata);
	mapper->SetInput (flipY->GetOutput());
	mapper->SetColorWindow (255.0);
	mapper->SetColorLevel (127.0);
	_actor->SetMapper (mapper);
	mapper->Delete();
	flipY->Delete();
	SetState(state);

	if( data2 != data)
		delete []data2;
}
void TextureButton::SetState(Button::State state)
{
	Button::SetState(state);
	
	unsigned char opacity = (state != DISABLED ? 255 : 63);
	int dims[3];
	vtkImageData *imagedata = ((vtkImageMapper*)_actor->GetMapper())->GetInput();
	imagedata->GetDimensions(dims);
	if(!imagedata->GetScalarPointer())
		  imagedata->Update();

	// set the opacity of the texture based on the button state
	unsigned char *data = (unsigned char *)imagedata->GetScalarPointer();
	for(int i = 3; i < dims[0]*dims[1]*4; i+=4)
		data[i] = opacity;
	imagedata->Modified();
	_actor->Modified();
}
void TextureButton::SetScale(Point2f s)
{
	// scale the texture
	vtkTransform *scale = vtkTransform::New();
	scale->Scale(s.x,s.y,1);
	vtkImageReslice *reslice = vtkImageReslice::New();
	reslice->SetResliceTransform(scale);
	reslice->SetInput(((vtkImageMapper*)_actor->GetMapper())->GetInput());
	((vtkImageMapper*)_actor->GetMapper())->SetInput(reslice->GetOutput());
	reslice->Delete();
	scale->Delete();
}

Point2i TextureButton::Size()
{
	// get the texture size
	_imagedata->Update();
	return Point2i(_imagedata->GetExtent()[1], _imagedata->GetExtent()[3]);
}

// these macros convert the bounds of a rectangle to its 4 vertices. 
#define TO_ARRAY(xmin,ymin,xmax,ymax)\
	double v1[]={xmin,ymin,0};\
	double v2[]={xmin,ymax,0};\
	double v3[]={xmax,ymax,0};\
	double v4[]={xmax,ymin,0};

#define TO_ARRAY8(x1,y1,x2,y2,x3,y3,x4,y4)\
	double v1[]={x1,y1,0};\
	double v2[]={x2,y2,0};\
	double v3[]={x3,y3,0};\
	double v4[]={x4,y4,0};

enum Primitive {LINE, BORDER, QUAD};

/*!	This template function does the entire job of creating rectangles, 
hollow rectangles, lines with/without transform and texture. 
*/
template <class Actor, class Mapper> 
void MakeRectOrLine(Actor **_actor, double v1[3], double v2[3], double v3[3], double v4[3], bool bTextured, vtkTransform *transform, Primitive type)
{
	if(!*_actor)
		*_actor = Actor::New();
	Actor *actor = *_actor;

	int numCells = type == LINE ? 2 : 5;
	vtkPolyData *pData = vtkPolyData::New();
	vtkPoints *pts = vtkPoints::New();
	pts->GetData()->SetName("Dummy_name_to_avoid_valgrind_warnings");
	
	pts->InsertNextPoint (v1[0],v1[1],v1[2]);
	if(type!=LINE)pts->InsertNextPoint (v2[0],v2[1],v2[2]);
	pts->InsertNextPoint (v3[0],v3[1],v3[2]);
	if(type!=LINE)pts->InsertNextPoint (v4[0],v4[1],v4[2]);
	pData->SetPoints(pts);
	pts->Delete();
	vtkCellArray *border = vtkCellArray::New();
	border->InsertNextCell(numCells);
	for(int i = 0; i < numCells; i++)
		border->InsertCellPoint(i%4);
	if(type == QUAD)
	{
		if(bTextured)
		{
			vtkFloatArray *tcoords = vtkFloatArray::New();
			tcoords->SetNumberOfComponents(2);
			tcoords->InsertNextTuple2(0,0);
			tcoords->InsertNextTuple2(0,1);
			tcoords->InsertNextTuple2(1,1);
			tcoords->InsertNextTuple2(1,0);
			tcoords->InsertNextTuple2(0,0);
			pData->GetPointData()->SetTCoords(tcoords);
			tcoords->Delete();
		}
		pData->SetPolys(border);
	}
	else
		pData->SetLines(border);  
	border->Delete();

	Mapper *mapper = Mapper::New();
	if(transform)
	{
		vtkTransformPolyDataFilter *filter= vtkTransformPolyDataFilter::New();
		filter->SetTransform(transform);
		filter->SetInput(pData);
		mapper->SetInputConnection(filter->GetOutputPort());
		filter->Delete();
	}
	else
		mapper->SetInput(pData);
	actor->SetMapper(mapper);
	pData->Delete();
	mapper->Delete();
}

// Create 2d rectangles or lines
void CreateRectangle2D(vtkActor2D** actor, double xmin, double  ymin, double xmax, double ymax, bool bTextured)
{
	TO_ARRAY(xmin,ymin,xmax,ymax);
	MakeRectOrLine<vtkActor2D, vtkPolyDataMapper2D>(actor, v1,v2,v3,v4, bTextured, 0, QUAD);
}
void CreateHollowRectangle2D(vtkActor2D** actor, double xmin, double  ymin, double xmax, double ymax)
{
	TO_ARRAY(xmin,ymin,xmax,ymax);
#ifndef _WIN32	
	//v2[1]++;
#endif
	MakeRectOrLine<vtkActor2D, vtkPolyDataMapper2D>(actor, v1,v2,v3,v4, false, 0, BORDER);
}
void CreateLine2D(vtkActor2D** actor, double xmin, double  ymin, double xmax, double ymax)
{
	TO_ARRAY(xmin,ymin,xmax,ymax);
	MakeRectOrLine<vtkActor2D, vtkPolyDataMapper2D>(actor, v1,v2,v3,v4, false, 0, LINE);
}

// Create 3d rectangles or lines
void CreateRectangle3D(vtkActor** actor, double xmin, double  ymin, double xmax, double ymax, bool bTextured, vtkTransform *transform)
{
	TO_ARRAY(xmin,ymin,xmax,ymax);
	MakeRectOrLine<vtkActor, vtkPolyDataMapper>(actor, v1,v2,v3,v4, bTextured, transform, QUAD);
}
void CreateRectangle3D(vtkActor** actor, double v1[4], double  v2[4], double v3[4], double v4[4], bool bTextured, vtkTransform *transform)
{
	MakeRectOrLine<vtkActor, vtkPolyDataMapper>(actor, v1,v2,v3,v4, bTextured, transform, QUAD);
}

void CreateHollowRectangle3D(vtkActor** actor, double xmin, double  ymin, double xmax, double ymax, vtkTransform *transform)
{
	TO_ARRAY(xmin,ymin,xmax,ymax);
	MakeRectOrLine<vtkActor, vtkPolyDataMapper>(actor, v1,v2,v3,v4, false, transform, LINE);
}
void CreateHollowRectangle3D(vtkActor** actor, double v1[4], double  v2[4], double v3[4], double v4[4], vtkTransform *transform)
{
	MakeRectOrLine<vtkActor, vtkPolyDataMapper>(actor, v1,v2,v3,v4, false, transform, BORDER);
}

void CreateLine3D(vtkActor** actor, double xmin, double  ymin, double xmax, double ymax, vtkTransform *transform)
{
	TO_ARRAY(xmin,ymin,xmax,ymax);
	MakeRectOrLine<vtkActor, vtkPolyDataMapper>(actor, v1,v2,v3,v4, false, transform, LINE);
}
void CreateLine3D(vtkActor** actor, double v1[4], double  v2[4], double v3[4], double v4[4], vtkTransform *transform)
{
	MakeRectOrLine<vtkActor, vtkPolyDataMapper>(actor, v1,v2,v3,v4, false, transform, LINE);
}

