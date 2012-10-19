/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOpenGLImageOverlayActor.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkOpenGLImageOverlayActor - OpenGL texture map
// .SECTION Description
// vtkOpenGLImageOverlayActor is a concrete implementation of the abstract class 
// vtkImageActor. vtkOpenGLImageOverlayActor interfaces to the OpenGL rendering library.

#ifndef __vtkOpenGLImageOverlayActor_h
#define __vtkOpenGLImageOverlayActor_h

#include "vtkOpenGLImageActor.h"

class vtkWindow;
class vtkOpenGLRenderer;
class vtkRenderWindow;

class VTK_RENDERING_EXPORT vtkOpenGLImageOverlayActor : public vtkOpenGLImageActor
{
public:
  static vtkOpenGLImageOverlayActor *New();
  vtkTypeMacro(vtkOpenGLImageOverlayActor,vtkImageActor);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Implement base class method.
  void Load(vtkRenderer *ren);
  
  // Description:
  // Implement base class method.
  void Render(vtkRenderer *ren);

  // Description:
  // Release any graphics resources that are being consumed by this texture.
  // The parameter window could be used to determine which graphic
  // resources to release. Using the same texture object in multiple
  // render windows is NOT currently supported. 
  void ReleaseGraphicsResources(vtkWindow *);
  
  // TONY
  void SetQuadExtent(double extent[6]);
  void SetQuadExtent(double minX, double maxX, double minY, double maxY, 
                     double minZ, double maxZ);

protected:
  vtkOpenGLImageOverlayActor();
  ~vtkOpenGLImageOverlayActor();

  unsigned char *MakeDataSuitable(int &xsize, int &ysize,
                                  int &release, int &reuseTexture);

  vtkTimeStamp   LoadTime;
  long          Index;
  vtkRenderWindow *RenderWindow;   // RenderWindow used for previous render
  double Coords[12];
  double QuadExtent[6];
  double TCoords[8];
  int TextureSize[2];
  
  // Non-recursive internal method
  void InternalRender(vtkRenderer *ren);
  
  // Is a certain size texture supported?
  int TextureSizeOK( int size[2] );
  
private:
  vtkOpenGLImageOverlayActor(const vtkOpenGLImageOverlayActor&);  // Not implemented.
  void operator=(const vtkOpenGLImageOverlayActor&);  // Not implemented.
};

#endif
