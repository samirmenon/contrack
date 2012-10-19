#include "Network.h"
#include <wx/socket.h>
#include <iostream>
#include <wx/hashmap.h>
#include <algorithm>
#include "DTIPathwayIO.h"
#include "qPathwayViz.h"
#include "PDBHelper.h"
#include "VisibilityPanel.h"
#include "StatsPanel.h"
#include "StatsCheckboxArray.h"

#include "QuenchFrame.h"
#include "QuenchController.h"
#include "vtkInteractorStyleQuench.h"
#include "qVolumeViz.h"

#include "MeshBuilder.h"
#include "vtkROI.h"

using namespace std;

bool SendInParts(wxSocketBase *sock, char *pData, int iLen, int iPacketSize = 1024*1024);
bool ReceiveInParts(wxSocketBase *sock, char *pBuf, int iLen, int iPacketSize = 1024*1024);

enum
{
	ID_SERVER_EVENT,
	ID_SOCKET_EVENT
};
const int NEW_CLIENT = -1;

BEGIN_EVENT_TABLE(Network, wxEvtHandler)
  EVT_SOCKET(ID_SERVER_EVENT,  Network::OnServerEvent)
  EVT_SOCKET(ID_SOCKET_EVENT,  Network::OnSocketEvent)
END_EVENT_TABLE()

Network::Network(int portNumber, QuenchFrame* qFrame, QuenchController* qCtrlr) 
{
	_qFrame = qFrame;
	_qCtrlr = qCtrlr;
#ifdef _WIN32
	_set_printf_count_output(1);
#endif
	_nextClientID = 0;

	wxIPV4address	addr;
	addr.LocalHost();
	addr.Service(portNumber);
	m_pServer = new wxSocketServer(addr);

	if (!m_pServer->Ok())
		return;
	// Setup the event handler and subscribe to connection events
	m_pServer->SetEventHandler(*this, ID_SERVER_EVENT);
	m_pServer->SetNotify(wxSOCKET_CONNECTION_FLAG);
	m_pServer->Notify(TRUE);
}
Network::~Network()
{
}
void Network::OnServerEvent(wxSocketEvent &event)
{
	wxSocketBase *sock;
	assert(event.GetSocketEvent() == wxSOCKET_CONNECTION);  
	sock = m_pServer->Accept(TRUE);
	if (!sock)
	{
		cerr<<"Error: couldn't accept a connection"<<endl;
		return;
	}

	sock->SetEventHandler(*this, ID_SOCKET_EVENT);
	sock->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
	sock->Notify(TRUE);

	sock->SetFlags(wxSOCKET_WAITALL/* | wxSOCKET_BLOCK*/);
}
void Network::OnSocketEvent(wxSocketEvent &event)
{
	wxSocketBase *sock = event.GetSocket();
	switch(event.GetSocketEvent())
	{
	case wxSOCKET_LOST:
		sock->Destroy();
		break;
	case wxSOCKET_INPUT:
		{
		// Disable input events, not to trigger wxSocketEvent again
		sock->SetNotify(wxSOCKET_LOST_FLAG);

		int iClientID ;
		sock->ReadMsg(&iClientID, sizeof(int));
		if (sock->Error())
		{
			cerr<<"Error receiving client message"<<endl;
			sock->Close();
			return;
		}

		if ( iClientID == NEW_CLIENT )
		{
			_clientIDs.insert(_nextClientID);
			iClientID = _nextClientID++;
		}

		ServerReply	reply;
		reply.id = iClientID;
		reply.status = 1;

		sock->WriteMsg(&reply, sizeof(reply));
		if (sock->Error())
		{
			cerr<<"Error sending a reply"<<endl;
			return;
		}

		ReceiveClientCommand(iClientID, sock);

		// Enable input events again.
		sock->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);
		break;
		}
	}
}
bool Network::ReceiveClientCommand(int iID, wxSocketBase *sock)
{
	ClientHeader	client_header;	
	ServerHeader	server_header;	
	string			command;	
	string			params;	

	// receive message header
	sock->ReadMsg(&client_header, sizeof(client_header));
	if (sock->Error())
	{
		cerr<<"Failed to receive client message"<<endl;
		return false;
	}

	// receive command and parameters
	if (client_header.command_length)
	{
		command.reserve(client_header.command_length);
		command.resize(client_header.command_length-1);
		sock->ReadMsg((void*)command.c_str(), client_header.command_length);
		transform(command.begin(), command.end(), command.begin(), ::tolower);
	}
	if (client_header.params_length)
	{
		params.reserve(client_header.params_length);
		params.resize(client_header.params_length-1);

		bool bRes = ReceiveInParts(sock, (char*)params.c_str(), client_header.params_length);
		if (!bRes)
			cerr<<"Error receiving client data"<<endl;
	}

	server_header.data_length = 0;
	char	*pReplyData = NULL;
	//Push commands
	if		  (command == "push_paths")
		_qCtrlr->LoadPathway(params, false);
	else	if(command == "push_fg_info")
		CommandPushFgInfo(params);
	else 	if(command == "set_datpath")
		{}
	else 	if(command == "push_roi")
		{server_header.data_length = CommandPushROI(params, &pReplyData);}
	// Get commands
	else 	if(command == "get_fg")
			server_header.data_length = CommandGetPaths(&pReplyData);
	else 	if(command == "get_fg_ids")
			server_header.data_length = CommandGetFgInfo(&pReplyData);
	else 	if(command == "get_datapath")
			{}
	else 	if(command == "get_roi")
			{}

	//Send data
	server_header.status = 1;

	if (server_header.data_length ==1)
		server_header.data_length = 0;

	sock->WriteMsg(&server_header, sizeof(server_header));
	if (sock->Error())
	{
		cerr<<"Failed to respond"<<endl;
		if (pReplyData)
			delete[] pReplyData;
		return false;
	}
	if (server_header.data_length)
		if (!SendInParts(sock, pReplyData, server_header.data_length))
			cerr<<"Failed to send data "<<endl;
	if (pReplyData)
		delete[] pReplyData;

	
	return true;
}

// CommandPushFgInfo (...)
// 
// Push all pathway information from mrDiffusion to Quench, and updates the
// Quench display to reflect the changes. (Does NOT include the pathways 
// themselves.)
//
// This includes:
//
// Which pathways are assigned to which groups
// How many pathway groups are used
// Visibility, color, and name for each group

void Network::CommandPushFgInfo(std::string &params)
{
	float *in = (float*)params.c_str();
	int pos = 0;
	int num_fibers = in[pos++];

	// read the assignment
	DTIPathwayAssignment assn;
	for(int i = 0; i < num_fibers; i++)
		assn.push_back( in[pos++] );

	// read the number groups
	PathwayGroupArray &array = _qCtrlr->PathwayGroupArray_();
	int num_groups = in[pos++];
	for(int i = 0; i < num_groups; i++)
	{
		int group_id = in[pos++];

		// keep adding pathway groups until we have sufficient groups to handle this
		while(group_id >= (int)array.size())
		{
			PathwayGroup pg;
			array.push_back( pg );
		}

		// read visiblity
		bool bvis = in[pos++]?true:false;
		// read color
		array[group_id].SetVisible( bvis );
		Colord col;
		col.r = in[pos++]/255.;
		col.g = in[pos++]/255.;
		col.b = in[pos++]/255.;
		array[group_id].SetColor( col );

		// get the name of the fiber group
		int name_size = in[pos++];
		char *name = new char[name_size+1];
		for(int j = 0; j < name_size; j++)
			name[j] = in[pos++];
		// add null character
		name[name_size] = '\0';

		// set the new name
		string sname(name);
		array[group_id].SetName(sname);
		delete []name;
	}

	// update 3d geometry
	PDBHelper &pdbHelper = _qCtrlr->PDBHelper_();
	int selected = pdbHelper.Assignment().SelectedGroup();
	assn.SetSelectedGroup(selected);
	pdbHelper.SetAssignment(assn);
	qPathwayViz *viz = _qFrame->Interactor()->PathwayViz();

	if(viz->Mode() == qPathwayViz::SURFACE_MODE)
		viz->SetIntersectionGeometry(pdbHelper, array);
	else
	{
		viz->InvalidateColorCache();
		viz->UpdatePathwaysColor(pdbHelper, array);
		viz->UpdatePathwaysVisibility(pdbHelper, array);
	}
	//viz->VisibilityPanel_()->Update(array, pdbHelper.Assignment());
	NotifyAllListeners (PEvent (new Event(UPDATE_VISIBILITY_PANEL)));
	
	_qFrame->StatsPanel_()->StatsCheckboxArray_()->Update(&pdbHelper.Assignment(), &array);
	_qFrame->Refresh();
}

int Network::CommandGetPaths(char **pReplyData)
{
	DTIPathwayDatabase *pdb = _qCtrlr->PDBHelper_().PDB().get();

	// no pdb has been loaded yet so return
	if(!pdb)
		return 0;

   double ACPC[3] = {0.,0.,0.};
   ostringstream strStream(std::string(),ios::binary|ios::out);
   DTIPathwayIO::saveDatabasePDB (pdb, strStream, ACPC);

	// copy the pdb to output buffer
   int len =  (int)strStream.str().size();
   char *str = new char[len]; 
   memcpy(str,strStream.str().c_str(),len);
   *pReplyData = str;
   return len;
}
 
// CommandGetFgInfo (...)
// 
// Fetch pathway information out of Quench. (Does NOT include the pathways themselves.)
// This includes:
//
// Which pathways are assigned to which groups
// How many pathway groups are used
// Visibility, color, and name for each group


int Network::CommandGetFgInfo( char **pReplyData )
{
	// no pdb has been loaded yet so return
	if(!_qCtrlr->PDBHelper_().PDB().get())
		return 0;

	//store the entire data in a double array for use with matlab
	set<int> unique_groups;
	DTIPathwayAssignment & assn = _qCtrlr->PDBHelper_().Assignment();
	for(int i = 0; i < (int)assn.size(); i++)
		unique_groups.insert(assn[i]);

	int length =  1 + (int)assn.size()  + 1+(int)unique_groups.size()*(1 + 1 + 3 + 2048) ;
	float *out = new float[ length ];
	int pos = 0;
	// copy the size of the assignment
	out[pos++] = assn.size();

	// copy the assignment
	for(int i = 0; i < (int) assn.size(); i++)
		out[pos++] = assn[i];

	// copy the size of unique groups
	out[pos++] = unique_groups.size();

	// copy the information about the unique groups
	for(set<int>::iterator iter = unique_groups.begin(); iter != unique_groups.end(); iter++)
	{
		int i = *iter;
		out[pos++] = i;
		PathwayGroup &pg = _qCtrlr->PathwayGroupArray_()[i];
		out[pos++] = pg.Visible();
		out[pos++] = pg.Color().r * 255;
		out[pos++] = pg.Color().g * 255;
		out[pos++] = pg.Color().b * 255;
		out[pos++] = pg.Name().size();
		for(int name_index = 0; name_index < (int)pg.Name().size(); name_index++)
			out[pos++] = pg.Name()[name_index];
	}
	*pReplyData = (char*)out;
	return pos*sizeof(float);
}

// CommandPushROI
// Pushes the state of a single ROI from mrDiffusion to Quench.
// This includes:
//
// An image mask representing the ROI
// An offset positioning that ROI within the volume.
//
// Sends back the query_id of the ROI. 

int Network::CommandPushROI(std::string &params, char **pReplyData )
{

  // convert params to CParametersMap (query_id, image info..)

  CParametersMap paramsIn;
  CParametersMap paramsOut;

  if (!paramsIn.CreateFromString((char *) params.c_str(), params.length()))
      paramsOut.SetString("warning", "Invalid arguments string");

  // Convert image to mesh...

  DTIScalarVolume *image = ToScalarImage(paramsIn, paramsOut);

  vtkPolyData *pVTKMesh = MeshBuilder::ImageToMesh(image);
  
  int queryID;
  queryID = paramsIn.GetInt ("query_id", -1);

  float roiOrigin[3];

  CFloatArray *centerArray = paramsIn.GetArray ("roi_origin");
  if (!centerArray) {
    wxASSERT(0);
  }
  else {
    centerArray->GetValue (&(roiOrigin[0]), 0);
    centerArray->GetValue (&(roiOrigin[1]), 1);
    centerArray->GetValue (&(roiOrigin[2]), 2);
    //    cerr << "Center array is: " << roiOrigin[0] << ", " << roiOrigin[1] << ", " << roiOrigin[2] << endl;
  }
  
  //  cerr << "mesh created..." << endl;
  
  queryID = QuenchPushROI (pVTKMesh, queryID, roiOrigin, image);

  float *out = new float[1];
  out[0] = queryID;
  
  *pReplyData = (char *) out;
  //  paramsOut.SetInt ("query_id", queryID);
  //  return bRes;
  return 1*sizeof(float);
}

// Update or add an ROI in Quench, as appropriate.
int
Network::QuenchPushROI(vtkPolyData *data, int queryID, const float roiOrigin[3], DTIScalarVolume *image)
{
 // queryID is -1 if the ROI doesn't currently exist in DTI-Query
  //  cerr << "queryID is " << queryID << endl;
  if (queryID == -1) {
    //    cerr << "adding a new ROI!" << endl;
    return QuenchAddROI (data, roiOrigin, image);
  }
  else {
    //    cerr << "Updating an existing ROI (id = " << queryID << ")" << endl;
    QuenchUpdateROI (data, roiOrigin, queryID, image);
    return queryID;
  }
}

// Actually add a new ROI to Quench. (Only used in cases where the incoming
// ROI does not currently exist in Quench.)

int
Network::QuenchAddROI(vtkPolyData *data, const float roiOrigin[3], DTIScalarVolume *image)
{
  // add a new ROI, return its ID

  //  cerr << "ROI origin is: " << roiOrigin[0] << ", " << roiOrigin[1] << ", " << roiOrigin[2] << endl;
  vtkROI *newMeshROI = vtkROI::CreateMesh(_qFrame->Interactor()->Renderer(), _qCtrlr->ROIManager_().NextID(), Vector3d(0.0, 0.0, 0.0), data);
  newMeshROI->SetImageRepresentation(image);
  _qCtrlr->AddNewROI(newMeshROI);
  
}

// Update an existing ROI in Quench (gives it new offset and image data). 
void
Network::QuenchUpdateROI(vtkPolyData *data, const float roiOrigin[3], int roiID, DTIScalarVolume *image)
{

  //  cerr << "Replacing existing ROI: " << roiID << endl;
  vtkROI *newMeshROI = vtkROI::CreateMesh(_qFrame->Interactor()->Renderer(), roiID, Vector3d(0.0, 0.0, 0.0), data);
  newMeshROI->SetImageRepresentation(image);
  _qCtrlr->ReplaceROI(roiID, newMeshROI);
  
}

bool SendInParts(wxSocketBase *sock, char *pData, int iLen, int iPacketSize)
{
	do
	{
		sock->WriteMsg(pData, min(iLen, iPacketSize));
		if (sock->Error())
		{
			cerr<<"Failed to send data "<<min(iLen, iPacketSize)<<" bytes"<<endl;
			return false;
		}
		iLen	-= iPacketSize;
		pData	+= iPacketSize;
	}
	while (iLen > 0);
	
	return true;
}
bool ReceiveInParts(wxSocketBase *sock, char *pBuf, int iLen, int iPacketSize)
{
	char *pData = pBuf;
	do
	{
		sock->ReadMsg(pData, min(iLen, iPacketSize));
		if (sock->Error())
		{
			cerr<<"Failed to receive client data"<<endl;
			return false;
		}
		iLen	-= iPacketSize;
		pData	+= iPacketSize;
	}
	while (iLen > 0);

	return true;
}

DTIScalarVolume *
Network::ToScalarImage(CParametersMap &paramsIn, CParametersMap &paramsOut)
{
  vtkUnsignedCharArray	*pClassValues = NULL;
  float					fTemp;
  
  CFloatArray *pArray = paramsIn.GetArray("voxels");
  if (!pArray)
    {
      paramsOut.AppendString("error", "No array named 'voxels' supplied");
      return NULL;
    }
 
  assert (pArray->GetNumberOfDimensions() == 3);
  
  if (pArray->GetNumberOfDimensions() != 3)
    {
      paramsOut.AppendString("error", "Array must be in 3-dimensional");
      return NULL;
    }

  const int *pDims = pArray->GetSizes();
  
  float roiOrigin[3];

  CFloatArray *centerArray = paramsIn.GetArray ("roi_origin");
  if (!centerArray) {
    wxASSERT(0);
  }
  else {
    centerArray->GetValue (&(roiOrigin[0]), 0);
    centerArray->GetValue (&(roiOrigin[1]), 1);
    centerArray->GetValue (&(roiOrigin[2]), 2);
    //    cerr << "Center array is: " << roiOrigin[0] << ", " << roiOrigin[1] << ", " << roiOrigin[2] << endl;
  }
  // xxxdla - sizeMM might matter?

  // look at origin, set up transform matrix accordingly
  

  DTIScalarVolume *image = new DTIScalarVolume((unsigned int) pDims[0], (unsigned int) pDims[1], (unsigned int) pDims[2], 1, 1.0, 1.0, 1.0, 0.0);

  // This is a temporary hack until we figure out the right thing to do.
  image->setQformOffset(roiOrigin);

  cerr << pDims[0] << ", " << pDims[1] << ", " << pDims[2] << endl;

  int iSrcIndex;
      
   for (unsigned int iSrcZ = 0; iSrcZ < pDims[2]; iSrcZ++)
     {
       for (unsigned int iSrcY = 0; iSrcY < pDims[1]; iSrcY++)
	 {
	   iSrcIndex = iSrcZ * pDims[1] * pDims[0] + iSrcY * pDims[0];
	   //iDstIndex = (iSrcZ+1) * iSizes[1] * iSizes[0] + (iSrcY+1) * iSizes[0] + 1;
	      
	      for (unsigned int iSrcX = 0; iSrcX < pDims[0]; iSrcX++)
		{
		  //cerr << iSrcIndex << endl;
		  pArray->GetAtAbsoluteIndex(&fTemp, iSrcIndex);
		  //float fTemp = image->getScalar(iSrcX, iSrcY, iSrcZ);
		  //cerr << fTemp << endl;
		  image->setScalar(fTemp, iSrcX, iSrcY, iSrcZ);
		  //pClassValues->SetValue(iDstIndex, (fTemp > 0) ? INNER_VALUE : OUTER_VALUE);
		  
		  iSrcIndex++;
		  //iDstIndex++;
		}
	      //cerr << endl;
	 }
     }
      
   return image;
}
