/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef Q_VOLUMEVIZ_H
#define Q_VOLUMEVIZ_H

#include <DTIVolume.h>
#include "typedefs_quench.h"
#include "Overlay.h"

class vtkImageActor;
class vtkWindowLevelLookupTable;
class vtkColorTransferFunction;
class vtkImageData;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkActor;
class vtkMatrix4x4;
class vtkTextActor;
class vtkRenderer;
class vtkPropPicker;
class vtkPropCollection;

class ColorMapPanel;

//! This class handles interaction with the current background nifti image
class qVolumeViz : public IEventSource
{
public:
	qVolumeViz(vtkRenderer *renderer);
	~qVolumeViz();
	//! Sets the current background image
	void SetVolume(DTIScalarVolume* vol, float left, float right);
	//! Adds a new background image
	void AddVolume(DTIScalarVolume *vol, ColorMapPanel* cmpanel);
	//! Gets the active volume
	DTIScalarVolume *GetActiveVolume();

	//! Get position of cursor (in voxel space)
	void GetCursorPosition(DTIVoxel &pos);

	//! Computes the 3d bounds of the current backgroun image
	void ComputeCurrentVolumeBounds(Vector3d &vmin, Vector3d &vmax);
	//! Displays a red border along the currently selected slice
	void DisplayBorder();
	//! \return the current active slice
	PROPERTY_DECLARE(DTISceneActorID, _nActiveImage, ActiveImage ); 
	//! moves the currently active slice by \param amount
	void MoveActiveImage(int amount);
	//! Gets the 3d world pos which corresponds to the intersection of the axial, saggital and coronal planes
	DTIVector GetWorldPos();
	//! \return the associated user matrix
	vtkMatrix4x4* GetUserMatrix();
	//! \return same as GetWorldPos but is in the image coordinate system
	PROPERTY_DECLARE(Vector3d, _vPos, Position);

	//! Removes all overlays
	void Clear(){_overlays.clear();}
	//! \return the overlay specified at index \param i 
	POverlay GetOverlay(int i){return _overlays[i];}
	//! Adjust the 3d zoffset of the overlays
	void AdjustOverlayOffset(double viewDir[3]){ for(int i = 0; i < (int)_overlays.size(); i++)_overlays[i]->AdjustOffset(viewDir); }

	//! Applies contrast and brightness modification to the background image
	void GetWindowLevelParams(double &dW, double &dWMin, double &dWMax, double &dL, double &dLMin, double &dLMax);

	//! Gets the bound of the active image rectangle in world space
	void ActiveImageExtents(double points[4][3], double normal[4]);
	//! Gets the plane equation corresponding to the \param id
	void GetPlaneEquation (DTISceneActorID id, double pt[4], double normal[4]);
	//! Gets the 3d extent of the slice specified by \param id
	void GetDisplayExtent (DTISceneActorID id, int displayExtent[6]);
	//! \return the visibility of the slice specified by \param id
	bool Visibility(DTISceneActorID id);
	//! Sets the visiblity of a slice
	void SetVisibility(DTISceneActorID id, bool b);

	//! Brings up the overlay panel if a volume is clicked.
	bool OnRightButtonUp(int x, int y);
	
	//--- ISerializable interface ---//
	void Deserialize(std::istream &s, int version = 0);
	void Serialize  (std::ostream &s, int version = 0);

	// returns the number of overlays
	int NumOverlays() { return (int)_overlays.size(); }

	// Convert from local coordinate system of image to world coordinate system.
	static void LocalToWorld(double *mat, DTIVoxel &lPos, double *wPos);
	static void LocalToWorld(double *mat, double *lPos, double *wPos);
	static bool WorldToLocal(double *mat, double *wPos, uint *dim, DTIVoxel &lPos, bool round = false);
	static void WorldToLocal(double *mat, double *wPos, uint *dim, double *flPos);


private:
	std::vector<POverlay> _overlays;			/// Array of overlays
	DTIScalarVolume* _vol;						/// The active background image
	vtkImageData* _img;							/// The vtk data corresponding to the background image
	vtkImageActor *_aSag, *_aAxial, *_aCor;		/// The vtk actors  corresponding to the 3 slices
	vtkTextActor  *_aPosition;					/// The text displaying the current position in the image space
	vtkWindowLevelLookupTable *_lutBW;			/// LUT for brighness
	vtkColorTransferFunction *_lutColor;		///
	vtkPolyData         *_pdBorder;				/// vtk helpers to show a red border along the selected slice
	vtkPolyDataMapper   *_mBorder;				/// vtk helpers to show a red border along the selected slice
	vtkActor            *_aBorder;				/// vtk helpers to show a red border along the selected slice
	vtkRenderer			*_renderer;				/// pointer to the global vtk renderer
	vtkPropPicker *_propPicker;					/// Helper class to pick a ROIs
	vtkPropCollection *_propCollection;			/// Helper class which stores a collection of voi
};

#endif
