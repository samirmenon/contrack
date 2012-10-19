/***********************************************************************
 * AUTHOR: David Akers <dakers>
 *   FILE: .//ButtonChoiceDialog.cpp
 *   DATE: Sun Jul 15 09:32:18 2007
 *  DESCR: 
 ***********************************************************************/
#include "ButtonChoiceDialog.h"

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWindows headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/button.h>

BEGIN_EVENT_TABLE(ButtonChoiceDialog, wxDialog)
  EVT_BUTTON ( BUTTON_CHOICE_1, ButtonChoiceDialog::OnChoice1)
  EVT_BUTTON ( BUTTON_CHOICE_2, ButtonChoiceDialog::OnChoice2)
END_EVENT_TABLE()

/***********************************************************************
 *  Method: ButtonChoiceDialog::ButtonChoiceDialog
 *  Params: wxWindow *parent, wxString caption, wxString buttonChoice1, wxString buttonChoice2
 * Effects: 
 ***********************************************************************/
ButtonChoiceDialog::ButtonChoiceDialog(wxWindow *parent, wxString caption, wxString questionString, wxString buttonChoice1Str, wxString buttonChoice2Str)
  : wxDialog(parent, -1, caption)
{
   wxStaticText* info_text = new wxStaticText(this, -1, caption);
   info_text->SetSizeHints(-1, -1, 600, -1, -1, -1); // set maximum width

   wxButton* buttonChoice1 = new wxButton(this, BUTTON_CHOICE_1, buttonChoice1Str);
   wxButton* buttonChoice2 = new wxButton(this, BUTTON_CHOICE_2, buttonChoice2Str);
   
   wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
   sizer->Add(info_text, 0, wxADJUST_MINSIZE | wxALL, 10);
   sizer->Add(0, 10); // spacer
   sizer->Add(buttonChoice1, 0, wxALIGN_CENTER_HORIZONTAL, 10);
   sizer->Add(0, 10);
   sizer->Add(buttonChoice2, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 10); 

   info_text->SetLabel( questionString );
   
   SetSizeHints(-1, -1, 600, -1, -1, -1); // set maximum width 
   SetSize(600,50,-1,-1, wxSIZE_AUTO_HEIGHT); // set position
   SetSizerAndFit(sizer, false);
}


/***********************************************************************
 *  Method: ButtonChoiceDialog::OnChoice1
 *  Params: wxCommandEvent &event
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
ButtonChoiceDialog::OnChoice1(wxCommandEvent &event)
{
    if ( Validate() && TransferDataFromWindow() )
    {   
        if ( IsModal() )
        {
            EndModal(BUTTON_CHOICE_1);
        }
        else
        {
            SetReturnCode(BUTTON_CHOICE_1);
            Show(false);
        }
    }
}


/***********************************************************************
 *  Method: ButtonChoiceDialog::OnChoice2
 *  Params: wxCommandEvent &event
 * Returns: void
 * Effects: 
 ***********************************************************************/
void
ButtonChoiceDialog::OnChoice2(wxCommandEvent &event)
{
  if ( Validate() && TransferDataFromWindow() )
    {   
        if ( IsModal() )
        {
            EndModal(BUTTON_CHOICE_2);
        }
        else
        {
            SetReturnCode(BUTTON_CHOICE_2);
            Show(false);
        }
    }
}


