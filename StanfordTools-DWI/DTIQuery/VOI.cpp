/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#include "VOI.h"
#include "vtkProperty.h"
#include "vtkCellArray.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkPolyData.h"
#include "vtkFilledCaptionActor2D.h"
#include "vtkTextMapper.h"
#include "vtkTextProperty.h"
#include <util/DTIFilterVOI.h>

VOI::VOI(int id, double center[3])
{
  this->cellID = -1;
  this->overlayActor = NULL;
  this->ghostActor = NULL;
  this->_symmetric_to = NULL;
  _id = id;
  
  //this->filter = createFilter (deltas);
  
  textActor  = vtkFilledCaptionActor2D::New();
  //textActor->SetStaticOffset(30);
  textActor->SetCaption("VOI");
  textActor->SetAttachmentPoint(center);
  textActor->SetHeight(.06);
  textActor->SetWidth(.06);
  textActor->GetCaptionTextProperty()->SetColor(1,1,1);
  textActor->GetCaptionTextProperty()->SetFontFamilyToCourier();
  textActor->SetVisibility (false);
}

VOI::~VOI() {
	mapper->Delete();
        actor->Delete();
	textActor->Delete();
	delete this->filter;
}

void VOI::SetLabel(const char *label)
{
  strcpy (_label, label);
  static char caption[255];
  if (strcmp (label, "")) {
    sprintf (caption, "%d (\"%s\")", _id, label);
  }
  else {
    sprintf (caption, "%d", _id);
  }
  
  this->textActor->SetCaption(caption);
  double calcVal = strlen(caption)/3.0;
  double maxVal = calcVal > 2 ? calcVal : 2;

  //textActor->SetWidth(.029*maxVal + 0.02);
  textActor->SetWidth (0.029*calcVal + 0.01);
  //textActor->SetHeight(.03*strlen(caption)/3);
  textActor->SetHeight (0.06);
  textActor->Modified();
}

const char *VOI::GetLabel() 
{
	//return this->textActor->GetCaption();
	return _label;
}
