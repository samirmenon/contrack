/** Copyright (c) 2010, The Board of Trustees of Leland Stanford Junior University. 
All rights reserved. **/

// Purpose ...

#ifndef QUENCHAPP_H
#define QUENCHAPP_H

#if wxUSE_UNICODE
#ifdef _WIN32
#pragma error('wxUSE_UNICODE should be off, compile wxWidgets with unicode disabled');
#else
'wxUSE_UNICODE should be off, compile wxWidgets with unicode disabled'
#endif

#endif

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

class QuenchFrame;
class QuenchController;

// Define a new application type, each program should derive a class from wxApp
class QuenchApp : public wxApp
{
public:
	// this one is called on application startup and is a good place for the app
	// initialization (doing it here and not in the ctor allows to have an error
	// return: if OnInit() returns false, the application terminates)
	virtual bool OnInit();
	virtual bool ParseCmdLine(QuenchController* qCtrlr, int argc, char** argv);
	
private:
	bool silent_mode;
	QuenchFrame*	  _qFrame;
};

#endif