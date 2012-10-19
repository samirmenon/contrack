/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef DTI_FILTER_H
#define DTI_FILTER_H

class DTIPathway;


class DTIFilter {

 public:
  DTIFilter();
  virtual ~DTIFilter();

  virtual bool matches (DTIPathway *pathway) =0;
  bool isDirty() { return _dirty; }
  void setDirty (bool flag) { _dirty = flag; }
  
 private:

  bool _dirty;
};

#endif
