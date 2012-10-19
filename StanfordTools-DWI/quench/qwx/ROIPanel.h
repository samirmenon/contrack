#ifndef ROI_PANEL_H
#define ROI_PANEL_H

#include "typedefs_quench.h"
#include "wx/spinctrl.h"
#include "wx/statline.h"
#include "ROIManager.h"

//! A generic class to handle mulitple values associated with a spinner, current 3
class MultiValueSpinner : public wxPanel , public IEventSource
{
	public:
		MultiValueSpinner( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 163,29 ), long style = wxTAB_TRAVERSAL );
		~MultiValueSpinner();
		//! \return the min value of the spinner
		PROPERTY_DECLARE(Vector3d, _min, Min);
		//! \return the max value of the spinner
		PROPERTY_DECLARE(Vector3d, _max, Max);
		//! \return the current value of the spinner
		PROPERTY_DECLARE(Vector3d, _value, Value);
		//! get/set the step size of the spinner
		PROPERTY(double, _step, StepSize);

	protected:
		wxTextCtrl* m_textCtrl2;
		wxStaticText* m_staticText4;
		wxSpinButton* m_spinBtn2;
		int _index;
		
		void OnTextEnter( wxCommandEvent& event );
		void OnSpin(double amt) ;
		void OnSpinDown( wxSpinEvent& event ) { OnSpin(-_step); }
		void OnSpinUp( wxSpinEvent& event ) { OnSpin(_step); }
};

//! This panel is displayed in the refine selection dialog. 
/*! This is used to interact with ROI's
*/
class ROIPanel : public wxPanel , public IEventSource, public IEventListener
{
public:
		ROIPanel(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 447,344 ), long style = wxDEFAULT_DIALOG_STYLE );
		~ROIPanel();
		//! Sets the bounds for the current ROI
		void SetROIBounds(Vector3d min, Vector3d max) ;
		//! Update the panel
		void Update(ROIManager &mgr);
		//! get/set the voi manager
		PROPERTY(ROIManager*, _voiMgr, ROIManager_);

protected:
		// gui elements
		wxPanel* m_panel2;
		wxStaticText* m_staticText12;
		wxChoice* _queryOps;
		wxTextCtrl* _Query;
		wxCheckBox* _showAllChecked;
		wxCheckBox* _showLabelsChecked;
		wxChoice* _voiList;
		wxButton* _buttonCreateCube;
		wxButton* _buttonCreateSphere;
		wxButton* _buttonCreateMesh;
		wxButton* _buttonDelete;
		wxStaticLine* m_staticline1;
		wxStaticText* m_staticText2;
		wxTextCtrl* _nameCtrl;
		wxCheckBox* _checkboxVisible;
		wxButton* _colorButton;
		wxStaticText* m_staticText3;
		wxStaticText* m_staticText8;
		wxSpinCtrl  * m_opacityCtrl;
		wxStaticText* m_color;
		MultiValueSpinner * _scaleCtrl, *_posCtrl;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnQueryChanged( wxCommandEvent& event );
		virtual void OnSelectionChanged( wxCommandEvent& event );
		virtual void OnCreateCube( wxCommandEvent& event );
		virtual void OnCreateSphere( wxCommandEvent& event );
		virtual void OnCreateMesh( wxCommandEvent& event );
		virtual void OnDelete( wxCommandEvent& event );
		virtual void OnNameChanged( wxCommandEvent& event );
		virtual void OnColorChanged( wxCommandEvent& event );
		virtual void OnQueryOpChanged( wxCommandEvent& event );
		virtual void OnROIVisible( wxCommandEvent& event );
		virtual void OnShowAll( wxCommandEvent& event );
		virtual void OnShowLabels( wxCommandEvent& event );
		virtual void OnSpinCtrl( wxSpinEvent& event );
		virtual void OnSpinCtrlText( wxCommandEvent& event );

		void OnEvent(PEvent evt);
		void SetEnabled(bool enabled=true);

		Vector3d _min, _max;
};

#endif

