/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

/*************************************************************************
*    NAME: David Akers and Anthony Sherbondy
*     
*    FILE: DTIFilterROI.cpp
*    DATE: Sun Feb 29  7:54:22 PST 2004
*************************************************************************/
#include "DTIFilterROI.h"
#include <RAPID.H>
#include <obb.H>
#include <stdexcept>
#include "DTIPathway.h"
#include <iostream>
using namespace std;

DTIFilterROI::DTIFilterROI(ROIType type, PRAPID_model model, Colord col)
{  
        _type = type;
	_model = model;
	_color = col;
	_image_volume = NULL;
	memset(_rotation_matrix, 0, sizeof(double)*9); 
	for (int i = 0; i < 3; i++)
	{
		_rotation_matrix[i][i] = 1;
		_position[i] = 0;
		_scale[i]=1;
	}
	//First compute the center
	Vector3<double> center, minv, maxv;
	for(int i = 0; i < _model->num_tris; i++)
	{
		center+=Vector3<double>(_model->tris[i].p1);
		center+=Vector3<double>(_model->tris[i].p2);
		center+=Vector3<double>(_model->tris[i].p3);

		// compute the bounds of the voi
		minv = min(Vector3d(_model->tris[i].p1), minv); maxv = max(Vector3d(_model->tris[i].p1), maxv);
		minv = min(Vector3d(_model->tris[i].p2), minv); maxv = max(Vector3d(_model->tris[i].p2), maxv);
		minv = min(Vector3d(_model->tris[i].p3), minv); maxv = max(Vector3d(_model->tris[i].p3), maxv);
	}
	center=  center * ( 1.0 /(3 * _model->num_tris) );
	_center = center;
	_size   = maxv - minv;
}

//! Need to do scaling manually as RAPID has no support for non-uniform scaling
void DTIFilterROI::SetScale (const double scale[3])
{

	//Now scale the model around the center
	for(int i = 0; i < _model->num_tris; i++)
		for(int j = 0; j < 3; j++)
		{		
			_model->tris[i].p1[j] = _center[j] + scale[j] * (_model->tris[i].p1[j] - _center[j]) / _scale[j];
			_model->tris[i].p2[j] = _center[j] + scale[j] * (_model->tris[i].p2[j] - _center[j]) / _scale[j];
			_model->tris[i].p3[j] = _center[j] + scale[j] * (_model->tris[i].p3[j] - _center[j]) / _scale[j];
		}

	//Update the model
	_model->EndModel();
	memcpy(_scale, scale, sizeof(double)*3);
}

bool DTIFilterROI::matches(DTIPathway *pathway)
{
	RAPID_model *pathwayModel = pathway->getRAPIDModel();
	RAPID_Collide (_rotation_matrix, _position, _model.get(),
		ZERO_ROTATION, ZERO_TRANSLATION, pathwayModel, RAPID_FIRST_CONTACT);
	return RAPID_num_contacts > 0 ? true:false;
}

void DTIFilterROI::Serialize (std::ostream &s, int version)
{
  s<<"Name: "<<_name<<std::endl;
  s<<"Type: Image" << std::endl; // later, will accommodate primitives like spheres or cubes.
  _color.Serialize(s,version);
  s<<_file_location<<std::endl;
}

DTIFilterROI::DTIFilterROI()
{
  
}

