#ifndef ROI_PROPERTIES_DIALOG_H
#define ROI_PROPERTIES_DIALOG_H

#include <wx/notebook.h>
#include "typedefs_quench.h"
#include "DTIVolume.h"

class wxSpinCtrl;
class wxTextCtrl;
class wxButton;

class ROIPropertiesDialog : public wxDialog, public IEventSource
{
 public:
  ROIPropertiesDialog(wxWindow *parent, PDTIFilterROI voi);
  ~ROIPropertiesDialog();

  std::string GetName();
  std::string GetFilepath();
  Colord GetColor() { return _selectedColor; }
  double GetOpacity();

  void OnColorChanged(wxCommandEvent &event);

 private:
  Colord _selectedColor;
  wxTextCtrl *_nameCtrl;
  wxButton *_colorButton;
  wxSpinCtrl *_opacityCtrl;
  wxTextCtrl *_filepathCtrl;

};


#endif
