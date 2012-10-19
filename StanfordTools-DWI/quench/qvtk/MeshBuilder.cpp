#include "DTIVolume.h"
#include "MeshBuilder.h"
#include "vtkStructuredPoints.h"
#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include "vtkMarchingCubes.h"
#include "vtkPointData.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkContourFilter.h"

#include <qwx/CParametersMap.h>
#include <qwx/CFloatArray.h>

// Original source: grayvolume.cpp from mrMesh... (VISTA)
#define INNER_VALUE 0
#define OUTER_VALUE 1
#define ISOLEVEL_VALUE 0.5f

vtkPolyData *MeshBuilder::ImageToMesh(DTIScalarVolume *inputImage)
{
   vtkUnsignedCharArray *pClassValues = vtkUnsignedCharArray::New();
      
   unsigned int pDims[3];
   inputImage->getDimension(pDims[0], pDims[1], pDims[2]);

   int			iSizes[3] = {pDims[0]+2, pDims[1]+2, pDims[2]+2};
   int			nTotalValues = iSizes[0] * iSizes[1] * iSizes[2];
      
   pClassValues->SetNumberOfValues(nTotalValues);
   memset(pClassValues->GetPointer(0), OUTER_VALUE, nTotalValues);
      
   vtkStructuredPoints *pClassData = vtkStructuredPoints::New();
   pClassData->SetDimensions(iSizes[0], iSizes[1], iSizes[2]);
      
   pClassData->SetOrigin(-1, -1, -1);
   pClassData->SetSpacing(1, 1, 1);
 
   int iDstIndex;
      
   for (unsigned int iSrcZ = 0; iSrcZ < pDims[2]; iSrcZ++)
     {
       for (unsigned int iSrcY = 0; iSrcY < pDims[1]; iSrcY++)
	 {
	   //iSrcIndex = iSrcZ * pDims[1] * pDims[0] + iSrcY * pDims[0];
	   iDstIndex = (iSrcZ+1) * iSizes[1] * iSizes[0] + (iSrcY+1) * iSizes[0] + 1;
	      
	      for (unsigned int iSrcX = 0; iSrcX < pDims[0]; iSrcX++)
		{
		  //pArray->GetAtAbsoluteIndex(&fTemp, iSrcIndex);
		  float fTemp = inputImage->getScalar(iSrcX, iSrcY, iSrcZ, 0);
		  pClassValues->SetValue(iDstIndex, (fTemp > 0) ? INNER_VALUE : OUTER_VALUE);
		  
		  // iSrcIndex++;
		  iDstIndex++;
		}
	    }
	}
      
      pClassData->GetPointData()->SetScalars(pClassValues);
      
      pClassValues->Delete();
     
      vtkPolyData *outputPData = BuildMesh(pClassData);
	    //vtkPolyData *outputPData = vtkPolyData::New();

      if (inputImage->getTransformMatrix() != NULL)
	{
	  //	  cerr << "Transforming by matrix! " << endl;
	  vtkTransform *transform = vtkTransform::New();
	  vtkTransformPolyDataFilter *transformFilter = vtkTransformPolyDataFilter::New();
	  transform->SetMatrix(inputImage->getTransformMatrix());
	  transform->PostMultiply();
	  transformFilter->SetTransform(transform);
	  transformFilter->SetInput(outputPData);
	  return transformFilter->GetOutput();
	}
      return outputPData;

}

#if 0

vtkPolyData *MeshBuilder::ImageToMesh(CParametersMap &paramsIn, CParametersMap &paramsOut)
{
  vtkUnsignedCharArray	*pClassValues = NULL;
  float					fTemp;
  
  CFloatArray *pArray = paramsIn.GetArray("voxels");
  if (!pArray)
    {
      paramsOut.AppendString("error", "No array named 'voxels' supplied");
      return NULL;
    }
 
  assert (pArray->GetNumberOfDimensions() == 3);
  
  if (pArray->GetNumberOfDimensions() != 3)
    {
      paramsOut.AppendString("error", "Array must be in 3-dimensional");
      return NULL;
    }

  // Scaling of the input image is currently disabled:

  /*  
  if (pScalesArray)
    {
      if (pScalesArray->GetNumberOfItems() != 3)
	{
	  paramsOut.AppendString("warning", "'scale' dimensions are invalid - array ignored");
	  pScalesArray = NULL;
	}
    }
  */


  // Convert pArray to a DTIScalarVolume
  // Call ImageToMesh(...), and return the result.

  const int *pDims = pArray->GetSizes();

  // xxxdla - sizeMM might matter? And, should we store the offset in the transform?

  DTIScalarVolume *image = new DTIScalarVolume((unsigned int) pDims[0], (unsigned int) pDims[1], (unsigned int) pDims[2], 1, 1.0, 1.0, 1.0, 0.0);

  cerr << pDims[0] << ", " << pDims[1] << ", " << pDims[2] << endl;

  int iSrcIndex;
      
   for (unsigned int iSrcZ = 0; iSrcZ < pDims[2]; iSrcZ++)
     {
       for (unsigned int iSrcY = 0; iSrcY < pDims[1]; iSrcY++)
	 {
	   iSrcIndex = iSrcZ * pDims[1] * pDims[0] + iSrcY * pDims[0];
	   //iDstIndex = (iSrcZ+1) * iSizes[1] * iSizes[0] + (iSrcY+1) * iSizes[0] + 1;
	      
	      for (unsigned int iSrcX = 0; iSrcX < pDims[0]; iSrcX++)
		{
		  //cerr << iSrcIndex << endl;
		  pArray->GetAtAbsoluteIndex(&fTemp, iSrcIndex);
		  //float fTemp = image->getScalar(iSrcX, iSrcY, iSrcZ);
		  //cerr << fTemp << endl;
		  image->setScalar(fTemp, iSrcX, iSrcY, iSrcZ);
		  //pClassValues->SetValue(iDstIndex, (fTemp > 0) ? INNER_VALUE : OUTER_VALUE);
		  
		  iSrcIndex++;
		  //iDstIndex++;
		}
	      //cerr << endl;
	 }
     }
      
  
  try
    {
      vtkPolyData *output = ImageToMesh(image);
      //vtkPolyData *output = vtkPolyData::New();
      delete image;
      return output;
    }
  catch(...)
    {
      if (pClassValues)
	pClassValues->Delete();
      paramsOut.SetString("error", "Not enough memory");
      return NULL;
    }
}
#endif

vtkPolyData* MeshBuilder::BuildMesh(vtkStructuredPoints *pClassData)
{

  vtkContourFilter *pMC = NULL;
  //vtkMarchingCubes		        *pMC	= NULL;
  vtkPolyData				*pOut	= NULL;
  
  try{
    //    pMC = vtkMarchingCubes::New();
    pMC = vtkContourFilter::New();
    pMC->SetInput(pClassData);
    pMC->SetValue(0, ISOLEVEL_VALUE);
    
    pMC->ComputeGradientsOff();
    pMC->ComputeNormalsOff();
    pMC->ComputeScalarsOff();
    
    assert (pMC->GetNumberOfContours() == 1);
    
    pMC->Update();
    
    pOut = vtkPolyData::New();
    pOut->ShallowCopy(pMC->GetOutput());
    
    ReverseTriangles(pOut);
    
    pMC->Delete();
    
    return pOut;
  }
  catch(...)
    {
      if (pMC)
	pMC->Delete();
      if (pOut)
	pOut->Delete();
      return NULL;
    }
}


void MeshBuilder::ReverseTriangles(vtkPolyData *pPD)
{
	int nPolys = pPD->GetNumberOfPolys();

	vtkIdType	*pPolys = pPD->GetPolys()->GetPointer();
	for (int iPoly = 0; iPoly < nPolys; iPoly++)
	{
		vtkIdType nVertices = *pPolys;
		if (nVertices == 3)
		{
			vtkIdType iLast = pPolys[3];
			pPolys[3] = pPolys[1];
			pPolys[1] = iLast;
		}
		pPolys += nVertices+1;
	}
}
