/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef BUTTON_CHOICE_DIALOG_H
#define BUTTON_CHOICE_DIALOG_H

#include <wx/dialog.h>
#include <wx/string.h>
#include "typedefs_quench.h"

const static int BUTTON_CHOICE_1 = 1;
const static int BUTTON_CHOICE_2 = 2;
const static int BUTTON_CHOICE_3 = 3;

//! A generic message box with upto 3 choices
/*! This dialog generalizes the message box by having custom message and labels for the button choices
	One can have 2 or 3 buttons
*/
class NButtonChoiceDialog : public wxDialog
{
	wxStaticText* info_text;
	wxButton* buttonChoice1, *buttonChoice2, *buttonChoice3;
public:
	NButtonChoiceDialog(wxWindow* parent, wxString caption, wxString question, wxString choice1, wxString choice2="", wxString choice3=""): wxDialog(parent, -1, caption)
	{
		info_text = new wxStaticText(this, -1, question);
		info_text->Wrap( -1 );
		//info_text->SetSizeHints(-1, -1, 600, -1, -1, -1); // set maximum width
		wxFont font2(10, 70, 90, 90, false, wxT("Arial"));
		info_text->SetFont( font2 );

		buttonChoice1 = new wxButton(this, BUTTON_CHOICE_1, choice1, wxDefaultPosition, DEFAULT_BUTTON_SIZE);
		buttonChoice1->SetFont( DEFAULT_FONT );
		buttonChoice2 = buttonChoice3 = 0;
		
		if(!choice2.empty())
			buttonChoice2 = new wxButton(this, BUTTON_CHOICE_2, choice2, wxDefaultPosition, DEFAULT_BUTTON_SIZE);
		
		if(!choice3.empty())
			buttonChoice3 = new wxButton(this, BUTTON_CHOICE_3, choice3, wxDefaultPosition, DEFAULT_BUTTON_SIZE);

		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		sizer->Add(info_text, 0, wxALIGN_CENTER | wxALL, 5);

		wxBoxSizer* button_sizer = new wxBoxSizer(wxHORIZONTAL);
		
		wxBoxSizer* one_button_sizer= new wxBoxSizer( wxVERTICAL );
		one_button_sizer->Add( buttonChoice1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
		button_sizer->Add( one_button_sizer, 1, wxEXPAND, 5 );

		if(!choice2.empty())
		{
			buttonChoice2->SetFont( DEFAULT_FONT );
			one_button_sizer= new wxBoxSizer( wxVERTICAL );
			one_button_sizer->Add( buttonChoice2, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
			button_sizer->Add( one_button_sizer, 1, wxEXPAND, 5 );
		}

		if(!choice3.empty())
		{
			buttonChoice3->SetFont( DEFAULT_FONT );
			one_button_sizer= new wxBoxSizer( wxVERTICAL );
			one_button_sizer->Add( buttonChoice3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );
			button_sizer->Add( one_button_sizer, 1, wxEXPAND, 5 );
		}

		sizer->Add(button_sizer, 0, wxALL, 10);

		SetSizeHints(-1, -1, 600, -1, -1, -1); // set maximum width 
		SetSize(-1,-1,-1,-1, wxSIZE_AUTO_HEIGHT); // set position
		SetSizerAndFit(sizer, false);
	}

	~NButtonChoiceDialog()
	{
		SAFE_DELETE(info_text);
		SAFE_DELETE(buttonChoice1);
		SAFE_DELETE(buttonChoice2);
		SAFE_DELETE(buttonChoice3);
	}
	//! Called when user clicks on the first button
	void OnChoice1 (wxCommandEvent &event) { Choice(BUTTON_CHOICE_1); }
	//! Called when user clicks on the second button
	void OnChoice2 (wxCommandEvent &event) { Choice(BUTTON_CHOICE_2); } 
	//! Called when user clicks on the third button
	void OnChoice3 (wxCommandEvent &event) { Choice(BUTTON_CHOICE_3); }

	//! hides the dialog
	void Choice(int choice)
	{
		if ( Validate() && TransferDataFromWindow() )
		{   
			if ( IsModal() )
				EndModal(choice);
			else
			{
				SetReturnCode(choice);
				Show(false);
			}
		}
	}

	DECLARE_EVENT_TABLE();
};

#endif
