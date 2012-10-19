/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef COMPUTE_LISTENER_H
#define COMPUTE_LISTENER_H

class ComputeListener {

 protected:
  ComputeListener() {}
  ~ComputeListener() {}

 public:
  virtual void progressUpdate (double proportionComplete, char *message, bool done) = 0;
  
};

#endif
