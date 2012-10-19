/***********************************************************************
* AUTHOR: David Akers <dakers>
*   FILE: .//SceneWindow.cpp
*   DATE: Tue Jan 24 09:18:48 2006
*  DESCR: 
***********************************************************************/
#include "DTIPathwaySelection.h"
#include "qPathwayViz.h"
#include "SceneWindow.h"
#include <wx/imagjpeg.h>
#include <wx/imaggif.h>
#include <wx/imagpng.h>
#include <wx/wfstream.h>
#include <wx/cursor.h>
#include <wx/mstream.h>
#include "vtkInteractorStyleQuench.h"

#include "icons/cursor_intersect_add.h"
#include "icons/cursor_intersect_add_mask.h"
#include "icons/cursor_intersect_remove.h"
#include "icons/cursor_intersect_remove_mask.h"
#include "icons/cursor_intersect_intersect.h"
#include "icons/cursor_intersect_intersect_mask.h"
#include "icons/cursor_touch_add.h"
#include "icons/cursor_touch_add_mask.h"
#include "icons/cursor_touch_remove.h"
#include "icons/cursor_touch_remove_mask.h"
#include "icons/cursor_touch_intersect.h"
#include "icons/cursor_touch_intersect_mask.h"
#include "icons/cursor_rotate.h"
#include "icons/cursor_rotate_mask.h"

#ifdef __WXMSW__
#	include <windows.h>
#	include <winuser.h>
#else
#	define SetCursor(A) 
#endif


static const int HOTSPOT_ROTATE_X = 16;
static const int HOTSPOT_ROTATE_Y = 16;
static const int HOTSPOT_TOUCH_X = 12;
static const int HOTSPOT_TOUCH_Y = 3;
static const int HOTSPOT_INTERSECT_X = 7;
static const int HOTSPOT_INTERSECT_Y = 25;

BEGIN_EVENT_TABLE(SceneWindow, wxVTKRenderWindowInteractor)
	EVT_KEY_DOWN    (SceneWindow::OnKeyPressed)
	EVT_KEY_UP      (SceneWindow::OnKeyReleased)
	EVT_LEFT_DCLICK (SceneWindow::OnDoubleClick)
END_EVENT_TABLE()

/***********************************************************************
*  Method: SceneWindow::~SceneWindow
*  Params: 
* Effects: 
***********************************************************************/
SceneWindow::~SceneWindow()
{
	SAFE_DELETE(_touch_cursors[UNION]);
	SAFE_DELETE(_touch_cursors[SUBTRACT]);
	SAFE_DELETE(_touch_cursors[INTERSECT]);

	SAFE_DELETE(_intersect_cursors[UNION]);
	SAFE_DELETE(_intersect_cursors[SUBTRACT]);
	SAFE_DELETE(_intersect_cursors[INTERSECT]);

	SAFE_DELETE(_rotate_cursor);
}

wxCursor *loadCursor (const char *filename, const char *maskFilename, int hotspotX = 0, int hotspotY = 0) 
{
	wxBitmap *theBitmap = new wxBitmap();
	theBitmap->LoadFile(filename, wxBITMAP_TYPE_BMP);

	wxBitmap *theBitmapMask = new wxBitmap();
	theBitmapMask->LoadFile (maskFilename, wxBITMAP_TYPE_BMP);
	theBitmapMask->SetDepth(1);

	theBitmap->SetMask(new wxMask(*theBitmapMask));
	wxImage theImage = theBitmap->ConvertToImage();
	theImage.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, hotspotX);
	theImage.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, hotspotY);
	wxCursor *cursor = new wxCursor (theImage);
	return cursor;
}

wxCursor *loadCursorFromMemory (const unsigned char *cursorData, const unsigned char *maskData, int width, int height, int hotspotX = 0, int hotspotY = 0) {
	wxMemoryInputStream *cursorStream = new wxMemoryInputStream(cursorData, 192);
	wxMemoryInputStream *maskStream = new wxMemoryInputStream(maskData, 192);
	wxImage *theImage = new wxImage(*cursorStream, wxBITMAP_TYPE_BMP);
	wxImage *maskImage = new wxImage(*maskStream, wxBITMAP_TYPE_BMP);
	theImage->SetMaskFromImage(*maskImage, 0, 0, 0);

	theImage->SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, hotspotX);
	theImage->SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, hotspotY);
	wxCursor *cursor = new wxCursor (*theImage);
	
	delete maskStream;
	delete cursorStream;
	delete theImage; delete maskImage;
	return cursor;
}

bool SceneWindow::loadCursors()
{
//#ifdef __WXMSW__
	// below code works on windows to load a custom mouse pointer.
	_touch_cursors[UNION] = loadCursorFromMemory (CURSOR_TOUCH_ADD, CURSOR_TOUCH_ADD_MASK, 32, 32, HOTSPOT_TOUCH_X, HOTSPOT_TOUCH_Y);
	_touch_cursors[SUBTRACT] = loadCursorFromMemory (CURSOR_TOUCH_REMOVE, CURSOR_TOUCH_REMOVE_MASK, 32, 32, HOTSPOT_TOUCH_X, HOTSPOT_TOUCH_Y);
	_touch_cursors[INTERSECT] = loadCursorFromMemory (CURSOR_TOUCH_INTERSECT, CURSOR_TOUCH_INTERSECT_MASK, 32, 32, HOTSPOT_TOUCH_X, HOTSPOT_TOUCH_Y);

	_intersect_cursors[UNION] = loadCursorFromMemory (CURSOR_INTERSECT_ADD, CURSOR_INTERSECT_ADD_MASK, 32, 32, HOTSPOT_INTERSECT_X, HOTSPOT_INTERSECT_Y);
	_intersect_cursors[SUBTRACT] = loadCursorFromMemory (CURSOR_INTERSECT_REMOVE, CURSOR_INTERSECT_REMOVE_MASK, 32, 32, HOTSPOT_INTERSECT_X, HOTSPOT_INTERSECT_Y);
	_intersect_cursors[INTERSECT] = loadCursorFromMemory (CURSOR_INTERSECT_INTERSECT, CURSOR_INTERSECT_INTERSECT_MASK, 32, 32, HOTSPOT_INTERSECT_X, HOTSPOT_INTERSECT_Y);

	_rotate_cursor = loadCursorFromMemory (CURSOR_ROTATE, CURSOR_ROTATE_MASK, 32, 32, HOTSPOT_ROTATE_X, HOTSPOT_ROTATE_Y);
//#endif
	return true;
}


SceneWindow::SceneWindow(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style, const wxString &name) : wxVTKRenderWindowInteractor (parent, id, pos, size, style, name)
{
	loadCursors();
	SetFocus();
	SetCursor (*_rotate_cursor);
	_selection_mode = _selection_tool = 0;
}


void SceneWindow::OnKeyPressed(wxKeyEvent &event)
{
	// if control key is pressed change the cursor
	if (event.GetKeyCode() == WXK_CONTROL) 
		SetCursorToSelection();
	event.Skip();
}


void SceneWindow::OnKeyReleased(wxKeyEvent &event)
{
	// revert the cursor back to rotate
	if (event.GetKeyCode() == WXK_CONTROL)
		SetCursor (*_rotate_cursor);
	event.Skip();
}


void SceneWindow::SetCursorToSelection()
{
	switch (_selection_tool) 
	{
	case qPathwayViz::TOUCH_MODE:
		SetCursor(*(_touch_cursors[_selection_mode]));
		break;
	case qPathwayViz::SURFACE_MODE:
		SetCursor(*(_intersect_cursors[_selection_mode]));
		break;
	default:
		break;
	};
}

void SceneWindow::HideMouseCursor()
{
#ifndef __WXMSW__
	// wxCursor down_cursor = wxCursor(down_bits, 32, 32,
	//	 6, 14, down_mask, wxWHITE, wxBLACK);
	// this->SetCursor (down_cursor);
#else
	int count = ::ShowCursor(false);
	cerr << "count is: " << count << endl;
#endif
}
void SceneWindow::ShowMouseCursor()
{
#ifdef __WXMSW__
	::ShowCursor(true);
#endif
}

void SceneWindow::OnEvent(PEvent evt)
{
	// cache the mode and operation that has been selected
	switch(evt->EventType())
	{
		case SELECTION_MODE_CHANGED:
			_selection_tool = ((EventSelectionModeChanged*)(evt.get()))->Value();
			break;
		case SELECTION_OPERATION_CHANGED:
			_selection_mode = ((EventSelectionOperationChanged*)(evt.get()))->Value();
			break;
	}
}
