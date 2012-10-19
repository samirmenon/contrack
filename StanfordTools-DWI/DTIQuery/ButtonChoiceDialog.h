/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef BUTTON_CHOICE_DIALOG_H
#define BUTTON_CHOICE_DIALOG_H

#include <wx/dialog.h>

class ButtonChoiceDialog : public wxDialog
{
    public:
    ButtonChoiceDialog(wxWindow* parent, wxString caption, wxString questionString, wxString buttonChoice1, wxString buttonChoice2);

    void OnChoice1 (wxCommandEvent &event);
    void OnChoice2 (wxCommandEvent &event);

    //    void OnDismiss (wxCommandEvent &event);
    //    private:
    //    const static int DISMISS_BUTTON_ID = 874;

    const static int BUTTON_CHOICE_1 = 10;
    const static int BUTTON_CHOICE_2 = 11;

    DECLARE_EVENT_TABLE();
};

#endif
