/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef DTI_TYPEDEFS_H
#define DTI_TYPEDEFS_H

enum DTISceneActorID {
  DTI_ACTOR_SAGITTAL_TOMO = 0,
  DTI_ACTOR_AXIAL_TOMO = 1,
  DTI_ACTOR_CORONAL_TOMO = 2,
  DTI_ACTOR_VOI = 3,
  DTI_ACTOR_TRACTS = 4,
  DTI_ACTOR_SURFACE = 5,
  DTI_ACTOR_NONE = 6
  
};

enum VOIMotionType {
  VOI_MOTION_FREE = 0,
  VOI_MOTION_SURFACE = 1,
  VOI_MOTION_SYMMETRY = 2
};

#endif
