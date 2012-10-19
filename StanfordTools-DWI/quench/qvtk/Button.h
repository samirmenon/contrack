#ifndef BUTTON_H
#define BUTTON_H

#include "typedefs_quench.h"

class vtkActor2D;
class vtkActor;
class vtkRenderer;
class vtkTextActor;
class vtkTransform;
#include "vtkImageData.h"

static Point2i SWATCH_SIZE(35,25);

//	These functions help in create 2d/3d rectangles, hollow rectangles and line in vtk

void CreateRectangle2D(vtkActor2D** actor, double xmin, double  ymin, double xmax, double ymax, bool bTextured=false);
void CreateHollowRectangle2D(vtkActor2D** actor, double xmin, double  ymin, double xmax, double ymax);
void CreateLine2D(vtkActor2D** actor, double xmin, double  ymin, double xmax, double ymax);

void CreateRectangle3D(vtkActor** actor, double xmin, double  ymin, double xmax, double ymax, bool bTextured=false, vtkTransform *transform=0);
void CreateRectangle3D(vtkActor** actor, double v1[4], double  v2[4], double v3[4], double v4[4], bool bTextured=false, vtkTransform *transform=0);

void CreateHollowRectangle3D(vtkActor** actor, double xmin, double  ymin, double xmax, double ymax, vtkTransform *transform=0);
void CreateHollowRectangle3D(vtkActor** actor, double v1[4], double  v2[4], double v3[4], double v4[4], vtkTransform *transform=0);

void CreateLine3D(vtkActor** actor, double xmin, double  ymin, double xmax, double ymax, vtkTransform *transform=0);
void CreateLine3D(vtkActor** actor, double v1[4], double  v2[4], double v3[4], double v4[4], vtkTransform *transform=0);

//! function to check if a point is inside a rect
template <class T> bool PointInsideRect(_Point2<T,2> pt, _Point2<T,2> origin, _Point2<T> size)
{
	if(origin.x <= pt.x && origin.y <= pt.y && pt.x <= origin.x+size.x && pt.y <= origin.y+size.y)
		return true;
	return false;
}

//! A vtk based button class consists of actor with text and border
/*! A base class for ColorButton and TextureButton.
*/
class Button
{
public:
	//! The state of the fiber group
	enum State
	{
		SELECTED, 	//! This group is currently selected
		ENABLED, 	//! The fiber group is visible
		DISABLED,  	//! Fiber group is hidden
	};

	Button(vtkRenderer *renderer, std::string name, Point2i textOffset, Point2i size = SWATCH_SIZE);
	virtual ~Button();

	//! Set the state of the button
	virtual void SetState(State s);
	//! Set visibility
	PROPERTY_DECLARE(bool, _visible, Visible);
	//! Makes the visibility panel transparent/opaque for per point coloring mode
	PROPERTY_DECLARE(bool, _transparent, Transparent);
	//! Set opacity
	PROPERTY_DECLARE(double, _opacity, Opacity);
	//! Text on the button
	PROPERTY_READONLY(vtkTextActor*, _text, TextActor);
	//! Size of the button
	PROPERTY_READONLY(Point2i, _size, Size);
	/*! Specifies if the button is crossed out. This is applicable only when the button is used for showing fiber groups.
		When a fiber group is hidden, the button is crossed out
	*/
	PROPERTY_DECLARE (bool, _crossed_out, CrossedOut);
	//! Position of the button
	PROPERTY_DECLARE (Point2i, _position, Position);
	//! Set the width of the border
	void SetLineWidth (int linewidth);

protected:
	//! The main vtk actor
	vtkActor2D 		*_actor;
	//! actor representing the border of the button
	vtkActor2D		*_outline_actor;
	//! actor representing the button as crossed out
	vtkActor2D		*_crossthrough_actor;
	//! Pointer to the global vtk renderer                    
	vtkRenderer *_renderer;
	//! 2D screen offset of the text w.r.t. the button
	Point2i _textOffset;
	//! State of the button
	State _state;
};

//! A vtk gui element to represent the color and visibility of a fiber group
/*! It consists of rectangle with the color of the fiber group, a cross 
	through line and a border. 
*/
class ColorButton : public Button
{
public:
	ColorButton(vtkRenderer *renderer, std::string name, Colord col, Button::State state, Point2i textOffset, Point2i size = SWATCH_SIZE);
	void SetColor(Colord col);
};

//! Class representing a textured vtk button. 
/*! Only supports a single texture for all states
*/
class TextureButton : public Button
{
public:
	TextureButton  (vtkRenderer *renderer, std::string name, Button::State, Point2i textOffset,  Point2i size, unsigned char *data, int bpp=3);
	~TextureButton () { VTK_SAFE_DELETE(_imagedata); }

	// Handles the opacity of the texture
	void SetState(Button::State state);

	//! sets the scale of the button
	void 		SetScale(Point2f scale);
	//! Returns the size of the button which by default is the size of the texture
	Point2i 	Size();
protected:
	//! Imagedata containing the texture 
	vtkImageData *_imagedata;
};

#endif
