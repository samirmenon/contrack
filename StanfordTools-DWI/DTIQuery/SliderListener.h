/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef SLIDER_LISTENER_H
#define SLIDER_LISTENER_H

class SliderPanel;

class SliderListener {
 protected:
  SliderListener() {}
  virtual ~SliderListener() {}
 public:

  virtual void sliderChanged (SliderPanel *slider) = 0;

};

#endif
