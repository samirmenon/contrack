/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef CAMERA_CONTROLS_H
#define CAMERA_CONTROLS_H

class vtkCamera;
class Scene;
class vtkMatrix4x4;
#include <iostream>
#include <vector>
using namespace std;

class cam_params
{
public:
	double px;
	double py;
	double pz;
	double dx;
	double dy;
	double dz;
};

class CameraControls {
 public:
  CameraControls();
  virtual ~CameraControls();
  void Update(vtkCamera *camera, const unsigned int dim[3], const double voxSize[3], vtkMatrix4x4 *mx);
  void NextPosition ();
  void PrevPosition ();
  void ResetPosition ();
  //  static void SaveCoords (vtkCamera *camera, ofstream &stream);
  //  static void RestoreCoords (vtkCamera *camera, ifstream &stream);

 private:
  vector<cam_params> camList;
  Scene *scene;
  short curCamPos;

};

#endif
