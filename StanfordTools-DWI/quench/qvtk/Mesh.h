#ifndef MESH_H
#define MESH_H

class vtkRenderer;
class vtkActor;
class KdTree;
class Overlay;
class ColorMapPanel;
#include "DTIVolume.h"

class Mesh
{
protected:
	double *vertices;
	double *initVertices;
	double *normals;
	double *colors;
	double *blendColors;
	int *triangles;
	int nV, nV2, nN, nC, nT;
	vtkActor *actor;
	vtkRenderer *_renderer;
	bool _visible;
	Overlay *lastOverlay;
	int (*mapping)[3];
	unsigned short (*kdVertices)[3];
	KdTree *meshKdTree;

	float lastLeft, lastRight, lastOpacity;
	int lastColorMapIndex;

public:
	Mesh(std::string file_name, vtkRenderer *renderer);
	~Mesh();
	bool Visible(){ return _visible;}
	void SetVisible(bool b);
	void UpdateColors(Overlay* o, DTIVectorVolume *vol, ColorMapPanel* cmpanel);
};

#endif