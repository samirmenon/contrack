/***********************************************************************
* AUTHOR: David Akers <dakers>
*   FILE: .//VisibilityPanel.cpp
*   DATE: Mon Mar 13 13:42:51 2006
*  DESCR: 
***********************************************************************/
#include "VisibilityPanel.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "Button.h"
#include "PathwayGroup.h"
#include "vtkCellArray.h"
#include "vtkTextActor.h"
#include "PathwayGroup.h"
#include "vtkProperty2D.h"
#include "vtkTextProperty.h"
#include "vtkPNGReader.h"
#include "vtkTexturedActor2D.h"
#include "vtkImageData.h"
#include "PathwayGroup.h"
#include "PDBHelper.h"
#include "icons/add_group.h"
#include "icons/delete_group.h"
#include "icons/trash.h"
#include "qROIEditor.h"

//http://www.pictureresize.org/online-images-converter.html
static const int SWATCH_BORDER = 0;
static const int BUTTON_WIDTH = 32;
static const int BUTTON_HEIGHT = 32;
int SWATCHES_PER_ROW  = 8;
#include "icons/removeSwatch_png.cpp"
#include "icons/addSwatch_png.cpp"

template <class T> void free_vector_vtk(std::vector<T*> &v)
{
	for(unsigned int i = 0; i < v.size(); i++)
		v[i]->Delete();
	v.clear();
}

VisibilityPanel::VisibilityPanel(vtkRenderer *renderer)
{
	_renderer = renderer;

	_add_group_button = _delete_group_button = 0;

	_taSelectedGroup= CreateTextActor(20,40);
	_taLockedGroup  = CreateTextActor(2,20);
	_taNameGroup 	= CreateTextActor(2,64);
	_voiEditingGroup = CreateTextActor(2,84);

	_swCurrentAssign = new ColorButton(_renderer,"",Colord(), Button::SELECTED,Point2i(), Point2i(16,16));
	Point2i pos(2,43);
	_swCurrentAssign->SetPosition(pos);
	_xmin = 300; _ymin = 10;
}
VisibilityPanel::~VisibilityPanel()
{
	VTK_SAFE_DELETE(_taSelectedGroup);
	VTK_SAFE_DELETE(_taLockedGroup);
	VTK_SAFE_DELETE(_taNameGroup);
	VTK_SAFE_DELETE(_voiEditingGroup);
	
//	SAFE_DELETE(_add_group_button);
//	SAFE_DELETE(_delete_group_button);

	delete _swCurrentAssign;
	_swatches.clear();
}


DTIPathwayGroupID VisibilityPanel::OnLeftDown(int x, int y)
{
	//Check if inside add group button
	if( _add_group_button && PointInsideRect( Point2i(x,y), _add_group_button->Position(), Point2i(11,13)) )
	{
		if(_swatches.size() < SWATCHES_MAX)
			NotifyAllListeners( PEvent( new Event(PATHWAY_GROUP_ADDED) ) );
		return true;
	}

	//Check if inside delete group button
	else if( _delete_group_button && PointInsideRect( Point2i(x,y), _delete_group_button->Position(), Point2i(11,13)) )
	{
		if(_swatches.size() > SWATCHES_MIN+1)
			NotifyAllListeners( PEvent( new Event(PATHWAY_GROUP_DELETED) ) ); 
		return true;
	}

	//Check if inside any of the group swatches
	else 
	{
		int whichChosen = SwatchUnderCursor(x,y, _xmin, _ymin, (int)_swatches.size());
		if(whichChosen != -1)
			if(whichChosen == 0) //trash can only be toggled 
				NotifyAllListeners(PEvent(new EventPathwayGroupVisibilityChanged(whichChosen)));
			else // some other fiber group was clicked
				NotifyAllListeners(PEvent(new EventPathwayGroupSelected(whichChosen)));
		return whichChosen;
	}
	return -1;
}
bool VisibilityPanel::OnRightButtonUp(int x, int y)
{
	// if we clicked on one of the swatches show the refine selection panel
	int whichChosen = SwatchUnderCursor(x,y, _xmin, _ymin, (int)_swatches.size());
	if( whichChosen == -1)
		return false;
	if( whichChosen != 0) // if we didnt click on the trash icon then show the refine selection panel
		NotifyAllListeners( PEvent( new EventShowRefineSelectionPanel(whichChosen) ) ); // notify Quench about it
	else
		cerr<<"In future this would allow to delete fibers in trash group"<<endl;
	return true;
}

void VisibilityPanel::Update(PathwayGroupArray &array, DTIPathwayAssignment &assignment)
{
	//location
	SWATCHES_PER_ROW  = ((int)_swatches.size()/32+1)*8;
	if(SWATCHES_PER_ROW > 32)
		SWATCHES_PER_ROW = 32;

	GetExtents(_xmin,_ymin,_xmax,_ymax, (int)array.size());
	int x = _xmax+SWATCH_SIZE.x ,y = _ymax-SWATCH_SIZE.y;

	//Add the _add, delete group buttons if not so already
	if(!_add_group_button)
	{
		// add the add group button
		_add_group_button = new TextureButton(_renderer, "", Button::ENABLED, Point2i(), Point2i(11,13), ICON_ADD_GROUP);
		Point2i pos = Point2i(_xmin-15, _ymin+12);
		_add_group_button->SetPosition(pos);

		// add the delete group button
		_delete_group_button = new TextureButton(_renderer, "", Button::ENABLED, Point2i(), Point2i(11,13), ICON_DELETE_GROUP);
		pos = Point2i(_xmin-15, _ymin);
		_delete_group_button->SetPosition(pos);

		// trash button
		_swatches.push_back(PTextureButton( new TextureButton(_renderer, "", Button::DISABLED, Point2i(SWATCH_SIZE.x/2, SWATCH_SIZE.y/4),Point2i(35,25),ICON_TRASH,3)));
		Point2i tpos(x,y);
		_swatches[0]->SetPosition(tpos);
	}
	// set the visibility of the trash bin depending if it has non zero fibers
	_swatches[0]->SetState( (assignment.NumAssigned(0) && array[0].Visible()) ? Button::ENABLED: Button::DISABLED);

	char name[100];
	int origNumSwatches = (int)_swatches.size();
	
	//Remove all extra groups
	for (int i = (int)array.size(); i < origNumSwatches; i++)
		_swatches.pop_back();
	
	// update all the fiber groups 
	y+=SWATCH_SIZE.y;
	for (int i = 1; i < (int)array.size(); i++) 
	{
		sprintf(name,"%d",i);

		PathwayGroup group = array[i];
		ColorButton::State state = Button::ENABLED;

		//Add if needed
		if(origNumSwatches <= i)
			_swatches.push_back(PColorButton( new ColorButton(_renderer, name, group.Color(), Button::ENABLED, Point2i(SWATCH_SIZE.x/2, SWATCH_SIZE.y/4))));
		else // or update the color if it already exists
			((ColorButton*)_swatches[i].get())->SetColor(group.Color());

		int numPaths = assignment.NumAssigned(i);

		// is this the selected group
		if(i == assignment.SelectedGroup())
		{
			// set state to selected
			_swatches[i]->SetState(Button::SELECTED);
			
			// set the text of the name group
			char selected[255];
			sprintf (selected, "%d / %d (%0.1f%%)", numPaths, (int)assignment.size(), 100.0f*numPaths/assignment.size());
			_taSelectedGroup->SetInput(selected); _taSelectedGroup->Modified();
			_taNameGroup->SetInput(group.Name().c_str()); _taNameGroup->Modified();
			
			// is assignment locked?
			if( assignment.Locked() )
				_taLockedGroup->SetInput("Assignments locked");
			else
				_taLockedGroup->SetInput("Assignments unlocked");
			_swCurrentAssign->SetColor (group.Color());
		} 
		else
			_swatches[i]->SetState(numPaths ? Button::ENABLED: Button::DISABLED);
		
		// if the group is not visible, cross it out
		bool visible = !group.Visible() && numPaths;
		_swatches[i]->SetCrossedOut(visible);

		//Location stuff
		if( (i-1) % SWATCHES_PER_ROW == 0)
		{
			x  = _xmin;
			y -= SWATCH_SIZE.y;
		}
		Point2i tpos(x,y);
		_swatches[i]->SetPosition(tpos);
		x += SWATCH_SIZE.x;
	}
}
void VisibilityPanel::SetVisible(bool &bvis)
{
	_visible = bvis;
	SetTextInfoVisibility(_visible);
	if(_add_group_button)
		_add_group_button->SetVisible(_visible);
	if(_delete_group_button)
		_delete_group_button->SetVisible(_visible);
	for (int i = 0; i < (int)_swatches.size(); i++) 
		_swatches[i]->SetVisible( _visible );
}
void VisibilityPanel::SetTransparent(bool &trans)
{
	_transparent = trans;
	for (int i = 0; i < (int)_swatches.size(); i++) 
		_swatches[i]->SetTransparent( _transparent );
}
void VisibilityPanel::SetTextInfoVisibility(bool b)
{
	if(b)
		_swCurrentAssign->SetState(ColorButton::SELECTED);
	else
		_swCurrentAssign->SetVisible(b);
	_voiEditingGroup->SetVisibility(b);
	_taSelectedGroup->SetVisibility(b);
	_taLockedGroup->SetVisibility(b);
	_taNameGroup->SetVisibility(b);
}
vtkTextActor *VisibilityPanel::CreateTextActor(int x, int y) 
{
	vtkTextActor *actor = vtkTextActor::New();
	actor->GetTextProperty()->SetFontSize(16);
	actor->GetTextProperty()->SetFontFamilyToArial();
	actor->GetTextProperty()->SetJustificationToLeft();
	//actor->GetTextProperty()->BoldOn();
	//actor->GetTextProperty()->ItalicOn();
	//  actor->GetTextProperty()->ShadowOn();
	actor->GetTextProperty()->SetColor(0,0,0);
	actor->GetPositionCoordinate()->SetCoordinateSystemToDisplay();
	actor->GetPositionCoordinate()->SetValue(x,y);
	actor->Modified();
	_renderer->AddActor(actor);
	return actor;
}
void GetExtents(int xmin, int ymin, int &xmax, int &ymax, int numSwatches)
{
	// get the number of rows
	int numRows = (numSwatches - 2)/ SWATCHES_PER_ROW;
	// compute the number of columns
	int numCols = numRows == 0? numSwatches : SWATCHES_PER_ROW;
	// estimate the bounds based on the rows and columns
	xmax = xmin + numCols * SWATCH_SIZE.x;
	ymax = ymin + (numRows+1)*SWATCH_SIZE.y;
}
int  SwatchUnderCursor(int x, int y, int xmin, int ymin, int numSwatches)
{
	int xmax, ymax;
	// get the bounding rectangle
	GetExtents(xmin,ymin,xmax,ymax, numSwatches);
	int whichChosen = -1;
	// did we click on the trash bin?
	if(xmax+SWATCH_SIZE.x <= x && xmax+2*SWATCH_SIZE.x >=x  && ymax-SWATCH_SIZE.y <= y && ymax >= y)
		whichChosen = 0;
	else if( xmin <= x && xmax >=x  && ymin <= y && ymax >= y) // any other fiber group?
	{
		x=(x-xmin)/SWATCH_SIZE.x; y=(ymax-y)/SWATCH_SIZE.y;
		whichChosen = 1+x+y*SWATCHES_PER_ROW;
		if(whichChosen >= numSwatches) // we clicked in the last row, but no fiber groups exists in this column
			whichChosen = -1; 
	}

	return whichChosen;
}
