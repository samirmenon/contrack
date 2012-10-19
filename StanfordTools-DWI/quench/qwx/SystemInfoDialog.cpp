/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#include "SystemInfoDialog.h"

#include <wx/dialog.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/sizer.h>

BEGIN_EVENT_TABLE(SystemInfoDialog, wxDialog)
  EVT_BUTTON(DISMISS_BUTTON_ID, SystemInfoDialog::OnDismiss)
END_EVENT_TABLE()


SystemInfoDialog::SystemInfoDialog(wxWindow* parent, wxString caption, wxString system_info)
    : wxDialog(parent, -1, caption)
{
    wxStaticText* info_text = new wxStaticText(this, -1, system_info);
    info_text->SetSizeHints(-1, -1, 600, -1, -1, -1); // set maximum width
    wxButton* dismiss_button = new wxButton(this, DISMISS_BUTTON_ID, _T("Close Info Window"));

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(info_text, 0, wxADJUST_MINSIZE | wxALL, 10);
    sizer->Add(0, 10); // spacer
    sizer->Add(dismiss_button, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 10);
    info_text->SetLabel( system_info );

    SetSizeHints(-1, -1, 600, -1, -1, -1); // set maximum width 
    SetSize(600,50,-1,-1, wxSIZE_AUTO_HEIGHT); // set position
    SetSizerAndFit(sizer, false);
}

void
SystemInfoDialog::OnDismiss (wxCommandEvent &event) {

  if ( Validate() && TransferDataFromWindow() )
    {   
        if ( IsModal() )
        {
            EndModal(wxID_OK);
        }
        else
        {
            SetReturnCode(wxID_OK);
            Show(false);
        }
    }

}


    
