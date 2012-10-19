#ifndef PATHWAY_GROUP_H
#define PATHWAY_GROUP_H

#include "typedefs.h"
#include "io_utils.h"

//! A pathway group is primarily characterised by its color and name.
/*!  Fiber group 0 is the trash group also called as DTI_COLOR_UNASSIGNED
*/
class PathwayGroup : public ISerializable
{
public:
	PathwayGroup():_name("FG"){_active = _visible = true;}
	PathwayGroup(std::string name, Colord color, bool visible=true, bool active=true):_name(name), _visible(visible), _color(color), _active(active){}

	//! Color of the fibergroup
	PROPERTY(Colord, _color, Color);
	//! stores the visibility of the fiber group
	PROPERTY(bool, _visible, Visible);
	//! Name of the fiber group
	PROPERTY(std::string, _name, Name);
	//! boolean to check if a fiber group can participate in stats based filtering
	PROPERTY(bool, _active, Active);

	//--- ISerializable interface ---//
	void Deserialize(std::istream &s, int version = 0);
	void Serialize  (std::ostream &s, int version = 0);
};

//! The class contains an array of \PathwayGroup 's
class PathwayGroupArray: public std::vector<PathwayGroup>, public ISerializable
{
public:
	PathwayGroupArray(){ ResetColors(true); }
	/*! Reset the array to initial colors. If \param bResetNames is true the 
	names of the groups are set to "FG "+index of the group.
	*/
	void ResetColors(bool bResetNames = false);

	//! Add a new group, with default color based on index. 
	void Add(std::string name);

	//--- ISerializable interface ---//
	void Deserialize(std::istream &s, int version = 0);
	void Serialize  (std::ostream &s, int version = 0);
};

#endif
