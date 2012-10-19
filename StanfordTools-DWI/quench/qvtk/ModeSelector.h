#ifndef MODE_SELECTOR_H
#define MODE_SELECTOR_H

#include "Button.h"
class vtkActor2D;
class vtkTextActor;
class vtkRenderer;

//! Enum specifying which part of the mode selector panel was picked by mouse
enum ModeSelectorComponent 
{
	MODE_SELECTOR_HANDLE	= 0, //! The top handle of the panel was selected to drag it around
	MODE_SELECTOR_PICKMODE	= 1, //! One of the buttons in the panel was clicked
	MODE_SELECTOR_NONE		= 2, //! Nothing was selected
};

//! A panel with a group of buttons.
/*! This is equivalent to a toolbar, its implemented in vtk
*/
class ModeSelector : public ISerializable
{
public:
	ModeSelector(vtkRenderer *renderer, const char *label);
	~ModeSelector();

	//! Adds a button with background image stored in \param data
	void AddIcon (Point2i size, unsigned char *data, const char *label);
	//! Position of the panel in 2d screen space
	PROPERTY_DECLARE(Point2i, _position, Position);
	//! Panel visibility
	PROPERTY_DECLARE(bool, _visible, Visible);
	//! Specifies which button on the panel is currently selected
	PROPERTY_DECLARE(int, _selected_mode, SelectMode);
	//! Sets the opacity of the panel
	void SetOpacity (double value);
	//! Processes mouse down event
	ModeSelectorComponent OnLeftDown(int x, int y);

	//--- ISerializable interface ---//
	void Deserialize(std::istream &s, int version = 0);
	void Serialize  (std::ostream &s, int version = 0);

private:
	//! Sets the text label of a button
	void SetTextProperties(std::string label, vtkTextActor *actor);
	//! Adds a textured button with a label
	void AddIcon(std::string label, PTextureButton);
	Point2i _size;							/// Position of the panel in 2d screen space
	PColorButton _background_swatch;		/// The background of the panel is a big button
	std::vector<PTextureButton> _icons;		/// Array of buttons in the panel
	vtkRenderer *_renderer;					/// The global vtk renderer
	std::string _text;						/// Panel's title
};

#endif
