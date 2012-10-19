/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef DTI_CONTRACKPDF_H
#define DTI_CONTRACKPDF_H
#include <typedefs.h>



namespace DTIConTrackPDF  {

  // FB5 data format = (e1_x,e1_y,e1_z,e2_x,e2_y,e2_z,e3_x,e3_y,e3_z,k1,k2,Cl,eVal2,eVal3)
  // Note: e3 is actually e1 of the tensor and e1 is e3, but the distribution is commonly
  // written such that e3 is e1, we follow the literature of Mardia, etc.

  struct Params {
    DTIVector vE1;
    DTIVector vE2;
    DTIVector vE3;
    float fScannerK;
    float fCl;
    float fE2;
    float fE3;    
  };

  static bool isValidData(const Array1D<float> &vParams){
    return vParams[0] == 0 && vParams[1] == 0 && vParams[2] == 0;
  }
  static bool isValidVersion(const Array1D<float> &vParams){
    return vParams.dim()==14 || vParams.dim()==13;
  }

  // Note that k1 and k2 used to describe different properties of the distribution of the PDD, 
  // but now are the same property.  The only property we record from the PDD distribution is the 
  // negative concentration parameter (-kappa), which is a negative real value, the smaller the more 
  // concentrated the distribution is about the PDD up to value of 0.  It is equal to -1/sin(dispersion)^2.

  // For more information see, Schwartzman A, Dougherty R, Taylor J. Cross-Subject Comparison of 
  // Principal Diffusion Direction Maps, MRM 2005.
  static Params getPDFParams(const Array1D<float> &vParams){
    Params p;
    p.vE1 = DTIVector(3);
    p.vE2 = DTIVector(3);
    p.vE3 = DTIVector(3);
    if( !isValidVersion(vParams) ) {
      p.vE1[0]=-1; p.vE1[1]=-1; p.vE1[2]=-1;
    } else if( vParams.dim()==14 ) {
	p.vE1[0] = vParams[0]; p.vE1[1] = vParams[1]; p.vE1[2] = vParams[2];
	p.vE2[0] = vParams[3]; p.vE2[1] = vParams[4]; p.vE2[2] = vParams[5];
	p.vE3[0] = vParams[6]; p.vE3[1] = vParams[7]; p.vE3[2] = vParams[8];
	p.fScannerK = -vParams[10];
	p.fCl = vParams[11];
	p.fE2 = vParams[12];
	p.fE3 = vParams[13];
      } else {
	p.vE1[0] = vParams[0]; p.vE1[1] = vParams[1]; p.vE1[2] = vParams[2];
	p.vE2[0] = vParams[3]; p.vE2[1] = vParams[4]; p.vE2[2] = vParams[5];
	p.vE3[0] = vParams[6]; p.vE3[1] = vParams[7]; p.vE3[2] = vParams[8];
	p.fScannerK = vParams[9];
	p.fCl = vParams[10];
	p.fE2 = vParams[11];
	p.fE3 = vParams[12];
      }
    return p;
  }
}


#endif
