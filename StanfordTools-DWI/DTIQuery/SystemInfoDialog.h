/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef SYSTEM_INFO_DIALOG_H
#define SYSTEM_INFO_DIALOG_H

#include <wx/dialog.h>

class SystemInfoDialog : public wxDialog
{
    public:
    SystemInfoDialog(wxWindow* parent, wxString caption, wxString system_info);
    void OnDismiss (wxCommandEvent &event);
    private:
    const static int DISMISS_BUTTON_ID = 874;

    DECLARE_EVENT_TABLE()
};

#endif
