#ifndef NETWORK_H
#define NETWORK_H

class wxSocketServer;
class wxSocketEvent;
#include "wx/wxprec.h"

#include "typedefs_quench.h"
#include <wx/hashmap.h>
#include <set>
#include "DTIVolume.h"

class wxSocketBase;
//class ClientIDData;
class CClientFrame;
class wxListCtrl;
class QuenchFrame;
class QuenchController;

class vtkPolyData;
class CFloatArray;
#include "CParametersMap.h"

class Network: public wxEvtHandler, public IEventSource 
{
public:
	Network(int portNumber, QuenchFrame* qFrame, QuenchController* qCtrlr);
	virtual ~Network();

	/// Called by CClientFrame
	void	OnClientFrameClosed(int iClientID);
	QuenchController* _qCtrlr;
	QuenchFrame* _qFrame;

protected:
	//! receive the fiber group info from matlab and push it to Quench
	void CommandPushFgInfo(std::string &params);

	//! get the fiber group data and copy it to \param pReplyData. \return length of the pReplyData
	int CommandGetPaths(char **pReplyData);
	
	//! Get the fiber group info. \return length of the pReplyData
	int CommandGetFgInfo( char **pReplyData );

	//! Push an ROI to Quench, and copy the ID to \param pReplyData. \return length of the pReplyData
	int CommandPushROI(std::string &params, char **pReplyData );

	//! Update or add an ROI in Quench, as appropriate.
	int QuenchPushROI(vtkPolyData *data, int queryID, const float roiOrigin[3], DTIScalarVolume *image);

	// Actually add a new ROI to Quench. (Only used in cases where the incoming ROI does not currently exist in Quench.)
	int QuenchAddROI(vtkPolyData *data, const float roiOrigin[3], DTIScalarVolume *image);

	// Update an existing ROI in Quench (gives it new offset and image data). 
	void QuenchUpdateROI(vtkPolyData *data, const float roiOrigin[3], int roiID, DTIScalarVolume *image);
	

	DTIScalarVolume *ToScalarImage(CParametersMap &paramsIn, CParametersMap &paramsOut);

	std::set<int> _clientIDs;
	wxSocketServer	*m_pServer; /// Server socker
	int	_nextClientID; /// Used in CreateNewClient

	void OnServerEvent(wxSocketEvent& event);
	void OnSocketEvent(wxSocketEvent& event);

	struct	ServerReply
	{
		int id;		///< client id
		int status;	///< Status: <0 on error
		char reserved[24];
	};
	struct	ClientHeader
	{
		int	command_length;	///< Length of command sent after ClientHeader
		int	params_length;	///< Length of parameters string
		char reserved[24];
	};
	struct	ServerHeader
	{
		int status;			///< Status. On error: status<0, data contains error descripton
		int data_length;	///< Length of data sent after ServerHeader
	};

private:
	/// Calls command handler for pCommand
	//bool ProcessClientCommand(ClientIDData *pClient, char *pCommand, CParametersMap &paramsIn, CParametersMap &paramsOut);
	/// Receives client command, calls CParamsParser, executes ProcessClientCommand and send result
	bool ReceiveClientCommand(int iID, wxSocketBase *sock);
	/// Checks if mapClients contains given iID
	bool IsClientIDValid(int iID);

    DECLARE_EVENT_TABLE();
};

#endif
