#ifndef RAPID_HELPER
#define RAPID_HELPER

#include "typedefs.h"
class vtkPolyData;

class RAPIDHelper {
	
public:
	//! Creates a RAPID model from vtk polygon soup. The RAPID model can then be used for 3d intersection
	static	PRAPID_model RAPIDModelFromPolyData(vtkPolyData *tris);
};

#endif

