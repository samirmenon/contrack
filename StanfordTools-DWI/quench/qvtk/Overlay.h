#ifndef OVERLAY_H
#define OVERLAY_H

#include "DTIVolume.h"
#include "typedefs_quench.h"

class vtkOpenGLImageOverlayActor;
class vtkImageData;
class vtkRenderer;
class vtkCustomImageActor;
class ColorMapPanel;

//! A vtk based overlay class
/*! Overlays are 2d slices of brain images super imposed on a background image slice. 
	Overlays can be psuedo colored using color maps. The range of color mapping and 
	opacity are tweakable.
*/
class Overlay : public IEventListener, public ISerializable 
{
public:
	Overlay(vtkRenderer *renderer, DTIScalarVolume *vol, float offset, float opacity, int cmi, ColorMapPanel* cmpanel);
	~Overlay();

	//! Reference nifti image that acts as the source for the overlay
	PROPERTY_READONLY(vtkImageData*, _img, ImageData);
	//! Opacity of the overlay
	PROPERTY_DECLARE(float, _opacity, Opacity);
	//! Offset of the overlay
	PROPERTY_DECLARE(float, _offset , Offset);
	//! Index of the colormap that this overlay uses
	PROPERTY_DECLARE(int  , _colorMapIndex, ColorMapIndex);
	//! Visibility of the overlay
	PROPERTY_DECLARE(bool, _visible, Visible);

	//! Apply a z-offset so that the overlay is always shown on top of the background image
	void AdjustOffset(double viewDir[3]);
	//! Set the range of lower and upper values that this overlay is mapped too
	void SetRange(float left, float right);
	//! Gets the range of the overlay
	void Range(float &left, float &right, float &min, float &max){left=_left; right=_right; min = _min; max = _max;}
	//! Set the visibility of a particular slice of the overlay
	void SetVisible(DTISceneActorID imageId, bool vis);
	//! Specifies the sagittal, coronal and axial slice to be displayed
	void SetPosition(double *v, double *flPos);
	//! Applies the colormapping on the overlay
	void setupColorMap();				

	void OnEvent(PEvent e);

	//--- ISerializable interface ---//
	void Deserialize(std::istream &s, int version = 0);
	void Serialize  (std::ostream &s, int version = 0);

protected:
	vtkRenderer *_renderer;				/// Pointer to the global vtk renderer
	ColorMapPanel* _cmapPanel;
	float _min, _max, _left, _right;	/// The range of values on which the colormap is mapped
	//vtkCustomImageActor *_overlays[3][2];/// The actual color map of the overlay that is shown in vtk.
	vtkOpenGLImageOverlayActor *_overlays[3][2];/// The actual color map of the overlay that is shown in vtk.
	uint _dim[4];						/// Dimension of the nifti image
	float _vox_sz[3];
	double _mat[16];					/// ACPC transformation matrix of the overlay
	static bool bvisible[3];			/// Stores the background volume's plane's visibility
};

#endif
