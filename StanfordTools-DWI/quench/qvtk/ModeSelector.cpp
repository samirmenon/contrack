/***********************************************************************
* AUTHOR: David Akers <dakers>
*   FILE: .//ModeSelector.cpp
*   DATE: Thu Mar  2 16:39:55 2006
*  DESCR: 
***********************************************************************/
#include "ModeSelector.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"
#include "vtkRenderer.h"
#include "vtkProperty2D.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkCellArray.h"
#include "vtkRenderWindow.h"

const int _separation = 4; // pixels in between elements.
const int _textHeight = 12;
double opacity = 0.5;
	
ModeSelector::ModeSelector(vtkRenderer *renderer, const char *label):_text(label)
{
	_selected_mode = 0;
	_renderer = renderer;
	_background_swatch.reset((ColorButton*)0);
}

ModeSelector::~ModeSelector()
{
}

void ModeSelector::SetTextProperties(std::string label, vtkTextActor *actor)
{
	actor->SetInput (label.c_str());
	actor->GetTextProperty()->SetJustificationToCentered();
	actor->GetTextProperty()->SetFontSize (12);
	actor->GetTextProperty()->SetFontFamilyToArial();
}
void ModeSelector::AddIcon(std::string label, PTextureButton button)
{
	// add a textured button to the icons
	_icons.push_back(button);

	// set text properties of this button
	SetTextProperties(label, _icons[_icons.size()-1]->TextActor());

	// compute the size
	Point2i s1 = _icons[0]->Size();
	_size = Point2i((int)_icons.size()*(_separation+s1.x),s1.y+_textHeight*5/2+2);

	// scale the background button to make room for the newly added button
	_background_swatch.reset(new ColorButton(_renderer,_text, Colord(.2,.2,.5), Button::DISABLED, Point2i(_size.x/2,_size.y-_textHeight-2), _size));
	_background_swatch->SetOpacity(opacity);

	SetTextProperties(_text, _background_swatch->TextActor());
	button->SetLineWidth(2);
}
void ModeSelector::AddIcon(Point2i size, unsigned char *data, const char *label)
{
	// add a textured button with given data
	AddIcon(label, PTextureButton(new TextureButton(_renderer, label,Button::ENABLED,Point2i(size.x/2,-_textHeight), size,data)));
}
void ModeSelector::SetPosition(Point2i &pos)
{
	Point2i size(0,_textHeight);

	// set the position of each of the icons
	for (int i = 0; i < (int)_icons.size(); i++) 
	{
		Point2i temp = pos + size;
		_icons[i]->SetPosition (temp);
		size.x+=_icons[i]->Size().x +_separation;
	}

	// set position of background button
	_background_swatch->SetPosition(pos);
	_position = pos;
}
ModeSelectorComponent ModeSelector::OnLeftDown(int x, int y)
{
	// check if cursor lies within this selector
	if (x <= _position.x || x >= _position.x+_size.x || y >= _position.y+_size.y || y <= _position.y) 
		return MODE_SELECTOR_NONE;
	// did we click on the handle?
	else if (y >= _position.y+_size.y-_textHeight) 
		return MODE_SELECTOR_HANDLE;
	// must be a icon then, which one?
	else 
	{
		int xPos = _position.x;
		// for each icon
		for (int i = 0; i < (int)_icons.size(); i++) 
		{
			xPos += _icons[i]->Size().x;
			// ith icon was clicked
			if (x <= xPos) 
			{
				// select it
				SetSelectMode(i);
				// refresh screen
				_renderer->GetRenderWindow()->Render();
				return MODE_SELECTOR_PICKMODE;
			}
		}
	}
	return MODE_SELECTOR_NONE;
}
void ModeSelector::SetSelectMode(int &modeNum)
{
	// deselect the previous icon
	_icons[_selected_mode]->SetState(Button::ENABLED);
	_selected_mode = modeNum;
	// select the new icon
	_icons[_selected_mode]->SetState(Button::SELECTED);
}
void ModeSelector::SetVisible(bool &value)
{
	// set the visibility of all icons
	_visible = value;
	if(_visible)
	{
		_background_swatch->SetState(Button::DISABLED);
		_background_swatch->TextActor()->GetTextProperty()->SetOpacity(1);
		_background_swatch->TextActor()->GetTextProperty()->SetBold(1);
		_background_swatch->SetOpacity(opacity);
		for (int i = 0; i < (int)_icons.size(); i++) 
			_icons[i]->SetState(Button::ENABLED);
		_icons[_selected_mode]->SetState(Button::SELECTED);
	}
	else
	{
		_background_swatch->SetVisible(_visible);
		for (int i = 0; i < (int)_icons.size(); i++) 
			_icons[i]->SetVisible(_visible);
	}
}
void ModeSelector::Serialize(ostream &s, int version)
{
	s<<"Visible :"<<_visible<<endl;
	s<<"Position :";_position.Serialize(s,version); s<<endl;
	s<<"SelectMode :"<<_selected_mode<<endl;
	s<<endl;
}
void ModeSelector::Deserialize(istream &s, int version)
{
	s>>"Visible :">>_visible>>endl; 
	s>>"Position :"; _position.Deserialize(s,version); s>>endl;
	s>>"SelectMode :">>_selected_mode>>endl;
	s>>endl;

	SetVisible(_visible);
	SetPosition(_position);
	SetSelectMode(_selected_mode);
}
