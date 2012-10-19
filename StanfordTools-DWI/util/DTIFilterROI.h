/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef FILTER_ROI_H
#define FILTER_ROI_H

#include "DTIFilter.h"
#include "typedefs.h"
#include "DTIVolume.h"

class DTIPathway;

//! This class handles mesh based filtering of pathways
/*! Quench can create 3 kinds of ROIs: cube, sphere and user generated.
 
    This class doesn't contain the actual geometry of the ROI as that is
	vtk specific. See \vtkROI to look into the actual implementation
	
	DTIFilterROI combines the 3 previous files DTIFilterCube, DTIFilterSphere 
	and DTIFilterComplex. The latter 3 seems to share a lot of functionality. 
	Since RAPID doesn't distinguish between a cube or a complex mesh it made 
	sense to have just a single ROI class. There are specialized functions to 
	create cube, sphere and complex ROI in vtkROI.
*/

enum ROIType {
  ROI_TYPE_SPHERE,
  ROI_TYPE_CUBE,
  ROI_TYPE_IMAGE,
  ROI_TYPE_GESTURE
};

static const Colord DEFAULT_ROI_COLOR(1,0,0);

class DTIFilterROI : public DTIFilter
{
public:
	//! \param model. The RAPID_model corresponding to the ROI. 
  DTIFilterROI(ROIType type, PRAPID_model model, Colord col = DEFAULT_ROI_COLOR);
  DTIFilterROI();
  virtual ~DTIFilterROI() {}
	
	//! Returns the rotation matrix
	double* RotationMatrix () { return (double*)_rotation_matrix; }
	//! Stores the rotation matrix
	virtual void SetRotationMatrix (const double R[3][3]) { memcpy(_rotation_matrix, R, sizeof(double)*9); }

	//! \return position of the ROI
	double* Position () { return _position; }

	Colord Color() { return _color; }
	virtual void SetColor (Colord &color) {_color = color; }

	//! Sets the new position of the ROI
	virtual void SetPosition (const double T[3]) { memcpy(_position, T, sizeof(double)*3); }

	//! \return scale of the ROI. Scale can be non-uniform.
	double* Scale() { return _scale; }
	/*! Set the new scale of the ROI. The RAPID_model is updated as RAPID doesn't support 
	non-uniform scaling
	*/
	virtual void SetScale (const double s[3]);

	//!\return true if \param pathway intersects the RAPID_model
	bool matches (DTIPathway *pathway);

	void SetImageRepresentation (DTIScalarVolume *imageVolume) { _image_volume = imageVolume; }
	DTIScalarVolume *GetImageRepresentation() { return _image_volume; }
	
	ROIType GetType() { return _type; }

	void SetFileLocation(const std::string &fileLocation) { _file_location = fileLocation; }
	std::string &GetFileLocation() { return _file_location; }

	//! ID of the ROI
	PROPERTY(int , _id , ID);
	
	//! Name of the ROI. Assumed to be unique for filtering purposes
	PROPERTY_READONLY(std::string, _name , Name);
	//! ROI's center
	PROPERTY_READONLY(Vector3d, _center, Center);
	//! Size of the bounding box enclosing the ROI
	PROPERTY_READONLY(Vector3d, _size , Size);

	void Serialize  (std::ostream &s, int version = 0);
	
protected:
	std::string _file_location;

	//! RAPID model used for pathway intersection.
	PRAPID_model _model;
	//! rotation matrix of the ROI
	double _rotation_matrix[3][3];
	//! Position of the ROI
	double _position[3], _scale[3];
	//! Color of the ROI
	Colord _color;

	//! Image volume (stored for some kinds of ROIs)
	DTIScalarVolume *_image_volume;

	//! Type of this ROI (sphere, cube, etc.) Could use inheritance
	// in the future for this.
	ROIType _type;

};


#endif
