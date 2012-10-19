#include "QuenchApp.h"
#include "QuenchFrame.h"
#include "QuenchController.h"
#include "Network.h"
#include "typedefs_quench.h"
#include <wx/cmdline.h>
#include <wx/dir.h>

#include <iostream>
using namespace std;

// Create a new application object: this macro will allow wxWindows to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also declares the accessor function
// wxGetApp() which will return the reference of the right type (i.e. QuenchApp and
// not wxApp) ... um, I think I have to use DECLARE_APP in the header to get the wxGetApp to work
IMPLEMENT_APP(QuenchApp)

// 'Main program' equivalent: the program execution "starts" here
bool QuenchApp::OnInit()
{
	wxImage::AddHandler(new wxPNGHandler);
	
	// Friendly start or error
	cerr << "Initializing Quench..." << endl;
	
	// Create controller for main application window
	QuenchController* qCtrlr = new QuenchController();

	// Create the main application frame window
	_qFrame = new QuenchFrame(qCtrlr, "Quench ", wxPoint(100, 150), wxSize(QUENCH_DEFAULT_WIDTH, QUENCH_DEFAULT_HEIGHT)); 
	
	// Setup network communication window
	Network* network = new Network(4001, _qFrame, qCtrlr);
	network->push_back(qCtrlr);

	// Parse the arguments
	if (!ParseCmdLine(qCtrlr, argc, argv))
		exit(1);
	
	_qFrame->Show(TRUE);
	
	// XXX Forcing this size change is the only thing I can think of to get OS X to redraw the window correctly... 
	// I don't know why, but if you don't do this then some things do not work correctly in the vtk viz window until 
	// you manually resize it.
	wxSize size = _qFrame->GetSize();
	size.IncBy(1,1);
	_qFrame->SetSize(size);
	_qFrame->Refresh();

	return TRUE;
}

bool QuenchApp::ParseCmdLine(QuenchController* qCtrlr, int argc, char** argv)
{
	// Command line description
	static const wxCmdLineEntryDesc cmdline_desc [] =
	{
		
		{ wxCMD_LINE_SWITCH, _T( "h" ), _T( "help" ), _T( "Show this help message" ),
            wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
		{ wxCMD_LINE_OPTION, _T( "s" ), _T("state" ), _T("A state file (qst)." ),
            wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_NEEDS_SEPARATOR },
        { wxCMD_LINE_OPTION, _T( "p" ), _T( "paths" ), _T( "A pathways (pdb) file." ),
            wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_NEEDS_SEPARATOR },
		{ wxCMD_LINE_OPTION, _T( "d" ), _T( "default_dir" ), _T( "A directory that contains default backgrounds to load." ),
            wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_NEEDS_SEPARATOR },
		{ wxCMD_LINE_OPTION, _T( "a" ), _T( "all_dir" ), _T( "Load all volumes in this directory." ),
            wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_NEEDS_SEPARATOR },
		{ wxCMD_LINE_OPTION, _T( "m" ), _T( "mesh" ), _T( "A mesh file name." ),
            wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_NEEDS_SEPARATOR },
        
		
        { wxCMD_LINE_NONE }
	};
	
	// Setup the parser object
	wxCmdLineParser parser( cmdline_desc, argc, argv );
	
	// Set the cmd line help to be a message to be a dialog
	//wxMessageOutput::Set(new wxMessageOutputStderr);
	wxMessageOutput::Set(new wxMessageOutputMessageBox);
	
	// Make us use - for switches 
	parser.SetSwitchChars(_T("-"));
	
	// Parse the parameters
    int parse_success = parser.Parse( );
	
    // Print help if asked (-1) or if a syntax error occured (>0).
    if( parse_success != 0 )
    {
		//parser.Usage();
		//cerr << "Was there a help message?" << endl;
		//wxString msg= wxString::Format( _T("Usage Message"), argv[0]);
		//wxMessageBox(msg, "Usage Error", wxOK | wxICON_INFORMATION, NULL);
        return false;
    }
	
	// Let's get parsed options
	wxString state_file;
	if (parser.Found(_T("s"),&state_file)) {
		
		// Load the state file
		qCtrlr->LoadState(state_file.mb_str());
		
	} else {
		// If there is a state file we do not care about the other parameters
		
		bool bLoadedVolume = false;
		
		// We need to load the images before pathways file
		wxString default_dir;
		if (parser.Found(_T("d"),&default_dir)) {
			// Load the state file
			qCtrlr->LoadDefaultVolumes(default_dir.mb_str());
			bLoadedVolume = true;
		}
		
		wxString all_dir;
		if (parser.Found(_T("a"),&all_dir)) {
			// Load the state file
			wxArrayString files;
			size_t numFiles =  wxDir::GetAllFiles(all_dir, &files, "*.nii.gz", wxDIR_FILES);
			// load the images
			for(int i = 0; i < (int)numFiles ; i++)
				qCtrlr->LoadVolume(files[i].mb_str());
			bLoadedVolume = true;
		}
		
		if (bLoadedVolume) {
			
			//Select the first volume
			qCtrlr->SetBackgroundVolume(0);
			
			// Can only load pathways if we have a volume
			wxString pdb_file;
			if (parser.Found(_T("p"),&pdb_file)) {
				// Load the pdb file
				qCtrlr->LoadPathway(pdb_file.mb_str(), false);	
			}
			
		wxString mesh_file;
		if (parser.Found(_T("m"),&mesh_file)) 
			qCtrlr->LoadMesh(mesh_file.mb_str());
		}
		
	}
	
	return true;
}
