/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#include "CameraControls.h"
#include "vtkCamera.h"
#include "vtkMatrix4x4.h"
#include "scene.h"

CameraControls::CameraControls () 
{

  cam_params cam;
  // axial view
  cam.px=0; cam.py=0; cam.pz=3*175;
  cam.dx=0; cam.dy=+1; cam.dz=0;
  this->camList.push_back(cam);
  
  // coronal view
  cam.px=0; cam.py=3*175; cam.pz=0;
  cam.dx=0; cam.dy=0; cam.dz=+1;
  this->camList.push_back(cam);
  
  // sagittal view
  cam.px=-3*175; cam.py=0; cam.pz=0;
  cam.dx=0; cam.dy=0; cam.dz=+1;
  this->camList.push_back(cam);
  curCamPos = 0;

}

CameraControls::~CameraControls()
{
}

void
CameraControls::Update(vtkCamera *camera, const unsigned int dim[3], const double voxSize[3], vtkMatrix4x4 *mx)
{
  //  float center[3] = {0, 0, 0};
  //  float center[3] = {0, -15, 5}; //center not at AC, but rather at center of 
                                 // typical brain (#'s are in mm)
  //  center[0] = dim[0]*voxSize[0]*0.5;
  //  center[1] = dim[1]*voxSize[1]*0.5;
  //  center[2] = dim[2]*voxSize[2]*0.5;
  
  // pass center of image through the transform to get the center point.

  const double imageCenter[4] = {dim[0]/2.0, dim[1]/2.0, dim[2] / 2.0, 1};

  double transformedCenter[4];

  mx->MultiplyPoint(imageCenter, transformedCenter);

  camera->SetViewUp (this->camList[curCamPos].dx, this->camList[curCamPos].dy, this->camList[curCamPos].dz);
  camera->SetPosition (transformedCenter[0] + this->camList[curCamPos].px, 
		       transformedCenter[1] + this->camList[curCamPos].py, 
		       transformedCenter[2] + this->camList[curCamPos].pz);
  camera->SetFocalPoint (transformedCenter[0],transformedCenter[1],transformedCenter[2]);
  camera->ComputeViewPlaneNormal();
  camera->Modified();
}


void
CameraControls::NextPosition() 
{
  this->curCamPos++;
  if(this->curCamPos >= this->camList.size()) {
    this->curCamPos = 0;
  }
}

void
CameraControls::PrevPosition()
{
  this->curCamPos--;
  if (this->curCamPos < 0) {
    this->curCamPos = this->camList.size() - 1;
  }
}

void
CameraControls::ResetPosition()
{
  this->curCamPos = 0;
}


