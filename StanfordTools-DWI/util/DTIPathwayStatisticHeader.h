#ifndef PATHWAY_STAT_HEADER_H
#define PATHWAY_STAT_HEADER_H

#include <iostream>

#include<string.h>
// number of stats
// highest unique ID

// * statheaders*
//
// then:
// 
// number of paths
//
// * paths *

struct DTIPathwayStatisticHeader {
 public:
  //bool _is_luminance_encoding; /* is this statistic good as a luminance encoding for paths?*/
  //bool _is_computed_per_point; /* is this statistic stored per point, also? */
  //bool _is_viewable_stat;      /* is this statistic something that shows up 
  //				  in the stat panel, by default? */

  int _is_luminance_encoding; /* is this statistic good as a luminance encoding for paths?*/
  int _is_computed_per_point; /* is this statistic stored per point, also? */
  int _is_viewable_stat;      /* is this statistic something that shows up 
  				  in the stat panel, by default? */

  
  char _aggregate_name[255];
  char _local_name[255];
  int _unique_id;
  bool isValid(){return strcmp(_aggregate_name,"Blank") == 0 ? false:true ;}
  void print()
  {
	  std::cout << "IsLuminanceEncode: " << _is_luminance_encoding << "(int32)" << std::endl;
	  std::cout << "IsComputedPerPoint: " << _is_computed_per_point << "(int32)" << std::endl;
	  std::cout << "IsViewable: " << _is_viewable_stat << "(int32)" << std::endl;
	  std::cout << "AggregateName: " << _aggregate_name << "(char[255])" << std::endl;
	  std::cout << "LocalName: " << _local_name << "(char[255])" << std::endl;
	  std::cout << "UniqueID: " << _unique_id << "(int32)" << std::endl;
  }
};

struct DTIPathwayStatisticThreshold 
{
   float _left,_right,_min,_max;
   bool _is_valid;
   DTIPathwayStatisticThreshold(){_min=_max=_left=_right=0; _is_valid = false;}
};
struct DTIPathwayStatisticHeaderOld {
 public:
  bool _is_luminance_encoding; /* is this statistic good as a luminance encoding for paths?*/
  bool _is_computed_per_point; /* is this statistic stored per point, also? */
  bool _is_viewable_stat;      /* is this statistic something that shows up 
  				  in the stat panel, by default? */

  
  char _aggregate_name[255];
  char _local_name[255];
  int _unique_id;
};

#endif
