#include <stdio.h>
#include "Mesh.h"
#include "nearpoints.cxx"
#include "vtkFloatArray.h"
#include "vtkCellArray.h"
#include "vtkPointData.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyData.h"
#include "vtkActor.h"
#include "vtkVolume.h"
#include "vtkImageData.h"
#include "vtkRenderer.h"
#include "vtkMatrix4x4.h"
#include "vtkProperty.h"
#include "vtkRenderWindow.h"
#include "Overlay.h"
#include "ColorMapPanel.h"

//#include "Quench.h"
#include "QuenchFrame.h"

const double VALUE_THRESHOLD = 1;
const double DISTANCE_THRESHOLD = 3;

Mesh::Mesh(std::string file_name, vtkRenderer *renderer)
{
	lastLeft = 0; lastRight = 0; lastOpacity=0;
	lastColorMapIndex = 0;

	meshKdTree = 0;
	FILE *fp = fopen(file_name.c_str(),"rb");
	// mesh file not found
	if(!fp)
	{
		printf("File not found: %s\n",file_name.c_str());
		return;
	}
	
	fread(&nV,sizeof(int),1,fp);
	vertices = new double[nV*3];
	fread(vertices, sizeof(double),nV*3,fp);

	fread(&nV2,sizeof(int),1,fp);
	initVertices = new double[nV2*3];
	fread(initVertices, sizeof(double),nV2*3,fp);

	fread(&nN,sizeof(int),1,fp);
	normals = new double[nN*3];
	fread(normals, sizeof(double),nN*3,fp);

	fread(&nC,sizeof(int),1,fp);
	colors = new double[nC*4];
	blendColors = new double[nC*4];
	fread(colors, sizeof(double),nC*4,fp);

	fread(&nT,sizeof(int),1,fp);
	triangles = new int[nT*3];
	fread(triangles, sizeof(int),nT*3,fp);
	fclose(fp);


	vtkPolyData *pData = vtkPolyData::New();

	vtkUnsignedCharArray* vcolors = vtkUnsignedCharArray::New();
	vcolors->SetNumberOfComponents(3);
	vtkPoints *pts = vtkPoints::New();
	vtkFloatArray *vnormals = vtkFloatArray::New();
	vnormals->SetNumberOfComponents(3);
	for(int i = 0; i < nV; i++)
	{
		pts->InsertNextPoint (vertices[i*3], vertices[i*3+1], vertices[i*3+2]);
		vcolors->InsertNextTuple3 (colors[i*4], colors[i*4+1], colors[i*4+2]);
		vnormals->InsertNextTuple3 (normals[i*3], -normals[i*3+1], -normals[i*3+2]);
	}

	pData->SetPoints(pts);
	pData->GetPointData()->SetScalars(vcolors);
	pData->GetPointData()->SetNormals(vnormals);
	
	vcolors ->Delete();
	pts     ->Delete();
	vnormals->Delete();

	vtkCellArray *border = vtkCellArray::New();
	for(int i = 0; i < nT*3; i+=3)
	{
		border->InsertNextCell(3);
		border->InsertCellPoint(triangles[i]);
		border->InsertCellPoint(triangles[i+1]);
		border->InsertCellPoint(triangles[i+2]);
	}

	pData->SetPolys(border);
	border->Delete();

	vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
	mapper->SetInput(pData);
	actor = vtkActor::New();
	actor->SetMapper(mapper);
	mapper->Delete();
	pData->Delete();
	_renderer = renderer;
	_renderer->AddActor(actor);
	actor->GetProperty()->SetDiffuseColor(1.,1.,1.);
	actor->GetProperty()->LightingOff();
	_visible = true;

	lastOverlay = 0;
	mapping = 0;
	kdVertices=0;
}
Mesh::~Mesh()
{
	_renderer->RemoveActor(actor);
	VTK_SAFE_DELETE(actor);
	delete []initVertices;
	delete []vertices; 
	delete []normals;
	delete []colors;
	delete []triangles;
	delete []blendColors;
}
void Mesh::UpdateColors(Overlay* o, DTIVectorVolume *vol, ColorMapPanel* cmpanel)
{
	float opacity = o->Opacity();
	float left,right,min,max;
	o->Range(left,right,min,max);

	vtkMatrix4x4 *mx = vtkMatrix4x4::New();
	mx->DeepCopy (vol->getTransformMatrix());
	mx->Invert();
	vtkUnsignedCharArray* vcolors = (vtkUnsignedCharArray*)((vtkPolyData*)((vtkPolyDataMapper *)actor->GetMapper())->GetInput())->GetPointData()->GetScalars();//vtkUnsignedCharArray::New();
	vtkImageData* img = o->ImageData();  

	if(o != lastOverlay)
	{
		if(mapping) 
			delete[]mapping;
		if(kdVertices)
			delete []kdVertices;
		if(meshKdTree)
			delete meshKdTree;

		//Values about 0.1 > pass
		std::vector< Vector3<Real> > kdVerts;
		int dims[3];
		img->GetDimensions(dims);

		kdVerts.reserve(dims[0]*dims[1]*dims[2]);
		for(int x = 0; x < dims[0]; x++)
			for(int y = 0; y < dims[1]; y++)
				for(int z = 0; z < dims[2]; z++)
					if(*((float*)img->GetScalarPointer(x,y,z)) > VALUE_THRESHOLD )
						kdVerts.push_back( Vector3<Real>(x,y,z) );

		kdVertices = new Real[kdVerts.size()][3];
		for(int i = 0; i < kdVerts.size(); i++)
			for(int j = 0; j < 3; j++)
				kdVertices[i][j]=kdVerts[i][j];

		meshKdTree = new KdTree(kdVertices, kdVerts.size());
		kdVerts.clear();
	}

	//vcolors->SetNumberOfComponents(3);
	if(o != lastOverlay || fabs(left - lastLeft)>1e-3 || fabs(right - lastRight) > 1e-3 || lastColorMapIndex != o->ColorMapIndex() || fabs(lastOpacity - opacity) > 1e-3)
	{
		if(!mapping)
		{
			mapping = new int[nV][3];
			for(int i = 0; i < nV; i++)
			{
				double vertex[4]={initVertices[i*3+0], initVertices[i*3+1], initVertices[i*3+2],1}; 
				double *out_vertex = mx->MultiplyDoublePoint(vertex);
				Real ov[3]={out_vertex [0], out_vertex [1], out_vertex [2]};
				int idx; 
				float bestSqDistance;
				meshKdTree->getNearestPoint(ov, &idx, &bestSqDistance);
				if(bestSqDistance > DISTANCE_THRESHOLD)
					mapping[i][0] = -1;
				else
				{
					for(int j = 0; j < 3; j++)
						mapping[i][j]=kdVertices[idx][j];
				}
			}
		}
		//create the mapping
		for(int i = 0; i < nV; i++)
		{
			double color[3];
			if(mapping[i][0] == -1)
				for(int j = 0; j < 3; j++)
					color[j] = colors[i*4+j];
			else
			{
				int ijk[3]={mapping[i][0], mapping[i][1], mapping[i][2]};
				float val = *((float*)img->GetScalarPointer(ijk[0], ijk[1], ijk[2]));
				
				//Normalize value;
				val = (val-left)/(right-left);
				if(val > 1) val = 1;
				if(val < 0) val = 0;
				std::vector<Coloruc>&cols =  cmpanel->ColorMaps()[ o->ColorMapIndex() ].Colors;
				Coloruc c = cols[(int)(val*(cols.size()-1))];

				double out_col[4]={c.r,c.g,c.b,c.a};
				double opacity2 = opacity*c.a/255;
				for(int j = 0; j < 3; j++)
					color[j] = opacity2*out_col[j] + (1-opacity2)*colors[i*4+j];
			}
			vcolors->SetTuple3(i,color[0], color[1], color[2]);
		}

		((vtkPolyData*)((vtkPolyDataMapper *)actor->GetMapper())->GetInput())->GetPointData()->GetScalars()->Modified();
		actor->Modified();
	}
	//vcolors ->Delete();
	mx->Delete();

	lastOverlay = o;
	lastLeft = left;
	lastRight = right;
	lastOpacity = opacity;
	lastColorMapIndex = o->ColorMapIndex();
}
void Mesh::SetVisible(bool b)
{
	_visible = b;
	actor->SetVisibility(b);
	_renderer->GetRenderWindow()->Render();
}
