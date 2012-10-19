#include "RAPIDHelper.h"
#include "vtkPolyData.h"
#include "vtkCellArray.h"
#include "RAPID.H"

PRAPID_model RAPIDHelper::RAPIDModelFromPolyData(vtkPolyData *tris)
{
	PRAPID_model model( new RAPID_model );
	model->BeginModel();
	
	vtkIdType            npts = 0;
	vtkIdType            *pts = 0;
	
	// Add triangles to the filter
	vtkCellArray* cells = tris->GetPolys();
	
	// start traversal on the polydata
	cells->InitTraversal();
	int counter=0;
	
	// for each triangle in the polydata
	while(cells->GetNextCell( npts, pts ))
	{     
		if(npts != 3)
			std::cout << "May be doing intersection incorrectly." << endl;
		double *p;
		
		// add the triangle to the rapid model.
		p = tris->GetPoint(pts[0]); double a[3] = {p[0], p[1], p[2]};
		p = tris->GetPoint(pts[1]); double b[3] = {p[0], p[1], p[2]};
		p = tris->GetPoint(pts[2]); double c[3] = {p[0], p[1], p[2]};
		model->AddTri (a,b,c,counter);
		counter++;
	}
	model->EndModel();
	return model;  
}
