#include "ROIPropertiesDialog.h"
#include "wx/colordlg.h"
#include "wx/spinctrl.h"
#include "wx/textctrl.h"
#include "DTIFilterROI.h"

using namespace std;

/*BEGIN_EVENT_TABLE(ROIPropertiesDialog, wxDialog)
END_EVENT_TABLE()*/

ROIPropertiesDialog::ROIPropertiesDialog(wxWindow *parent, PDTIFilterROI voi) :
  wxDialog(parent, wxID_ANY, _T("ROI Properties"), wxDefaultPosition, wxSize(550,125), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
  this->SetSizeHints( wxDefaultSize, wxDefaultSize );	

  wxBoxSizer* bROISizer = new wxBoxSizer( wxVERTICAL );

  wxBoxSizer* bNameSizer = new wxBoxSizer( wxHORIZONTAL );
  
  wxStaticText *textName = new wxStaticText(this, wxID_ANY, wxT("   Name:"), wxDefaultPosition, wxDefaultSize, 0 ); 
  bNameSizer->Add(textName, 0, wxALL, 9);

  _nameCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxPoint( -1,-1 ), wxSize(400, 20) );
  _nameCtrl->SetFont( DEFAULT_FONT );
  bNameSizer->Add(_nameCtrl, 0, wxALL, 9);

  wxBoxSizer* bColorSizer = new wxBoxSizer( wxHORIZONTAL );

  wxStaticText *textColor = new wxStaticText(this, wxID_ANY, wxT("   Color:"), wxDefaultPosition, wxDefaultSize, 0 );
  textColor->SetFont( DEFAULT_FONT );
  bColorSizer->Add( textColor, 0, wxALL, 9 );
  _colorButton = new wxButton( this, wxID_ANY, wxT(""), wxPoint( -1,-1 ), wxSize( 24,24 ), 0 );
  //  _colorButton->SetBackgroundColour(*wxRED);
  Colord col = voi->Color();
  _colorButton->SetBackgroundColour(wxColour(col.r*255, col.g*255, col.b*255));
  _selectedColor = col;

  bColorSizer->Add( _colorButton, 0, wxALL, 5 );
  wxStaticText *textOpacity = new wxStaticText(this, wxID_ANY, wxT("   Opacity:"), wxDefaultPosition, wxDefaultSize, 0 );
  textOpacity->SetFont( DEFAULT_FONT );
  bColorSizer->Add( textOpacity, 0, wxALL, 9 );
  _opacityCtrl = new wxSpinCtrl(this, wxID_ANY, wxT("50"), wxDefaultPosition, wxSize(50,-1));
  bColorSizer->Add( _opacityCtrl, 0, wxALL, 5 );

  wxBoxSizer *bFilepathSizer = new wxBoxSizer( wxHORIZONTAL );

   wxStaticText *textFilepath = new wxStaticText(this, wxID_ANY, wxT("   File path:"), wxDefaultPosition, wxDefaultSize, 0 ); 
  bFilepathSizer->Add(textFilepath, 0, wxALL, 9);

  _filepathCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxPoint( -1,-1 ), wxSize(400, 20) );
  _filepathCtrl->SetFont( DEFAULT_FONT );
  bFilepathSizer->Add(_filepathCtrl, 0, wxALL, 9);
  _filepathCtrl->Enable(FALSE);

  bROISizer->Add( bNameSizer, 1, wxEXPAND, 0 );
  bROISizer->Add( bColorSizer, 1, wxEXPAND, 0 );
  bROISizer->Add( bFilepathSizer, 1, wxEXPAND, 0);
  bROISizer->Add (CreateSeparatedButtonSizer(wxOK | wxCANCEL));

  _nameCtrl->SetValue(voi->Name());
  _opacityCtrl->SetValue(col.a*100);
  std::string fileLoc = voi->GetFileLocation();
  if (fileLoc == "") {
    fileLoc = "[Not saved]";
  }
  _filepathCtrl->SetValue(fileLoc);
  this->SetSizer(bROISizer);
  this->Layout();

  _colorButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ROIPropertiesDialog::OnColorChanged ), NULL, this );
}

ROIPropertiesDialog::~ROIPropertiesDialog()
{
}

std::string ROIPropertiesDialog::GetName()
{
  return _nameCtrl->GetValue().c_str();
}

std::string ROIPropertiesDialog::GetFilepath()
{
  return _filepathCtrl->GetValue().c_str();
}

void 
ROIPropertiesDialog::OnColorChanged (wxCommandEvent &event)
{
	wxPoint pt=wxGetMousePosition();
	pt.y-=400;

	// show a color choosing dialog
	wxColourDialog colordlg(this,0);
	wxColourData &colorData = colordlg.GetColourData();
	/*colorData.SetCustomColour(15, wxColour((unsigned char) floor(PATHWAY_COLORS[0].r*255),
					      (unsigned char) floor(PATHWAY_COLORS[0].g*255),
					      (unsigned char) floor(PATHWAY_COLORS[0].b*255)));	*/			      
	for (int i = 1; i < 16; i++) {
	  colorData.SetCustomColour(i-1, wxColour((unsigned char) floor(PATHWAY_COLORS[i].r*255),
						(unsigned char) floor(PATHWAY_COLORS[i].g*255),
						(unsigned char) floor(PATHWAY_COLORS[i].b*255)));
	}
	colordlg.SetPosition(pt);

	if(colordlg.ShowModal()==wxID_OK)
	{
		wxColour color2 = colordlg.GetColourData().GetColour();
		// show the new color on screen
		_colorButton->SetBackgroundColour(color2);
		_colorButton->Refresh();
		_selectedColor = Colord(color2.Red()/255.0, color2.Green()/255.0, color2.Blue()/255.0);

	}
}

double ROIPropertiesDialog::GetOpacity()
{
  return _opacityCtrl->GetValue()/100.0;
}
