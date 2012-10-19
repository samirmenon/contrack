/***********************************************************************
 * AUTHOR: David Akers <dakers>
 *   FILE: .//PathwayGroup.cpp
 *   DATE: Sun Mar 12 23:33:25 2006
 *  DESCR: 
 ***********************************************************************/
#include "PathwayGroup.h"
#include "typedefs.h"
using namespace std;

void PathwayGroup::Serialize(std::ostream &s, int version)
{
	s<<"Name: "<<_name<<std::endl;
	_color.Serialize(s,version);
	s<<"Visible: "<<_visible<<endl;
	s<<"Active: "<<_active<<endl;
	s<<endl;
}

void PathwayGroup::Deserialize(std::istream &s, int version)
{
	s>>"Name: ";
	char line[2048];
	s.getline(line, 2048);
	_name = line;

	_color.Deserialize(s,version);
	s>>"Visible: ">>_visible>>endl;
	s>>"Active: ">>_active>>endl;
	s>>endl;
}

void PathwayGroupArray::Serialize(std::ostream &s, int version)
{
	s<<"--- Pathway Groups ---"<<endl;
	s<<"Num Groups: "<<size()<<std::endl;

	// Save information about each pathway group
	foreach(std::vector<PathwayGroup> , iter, *this)
		(*iter).Serialize(s,version);
	s<<endl;
}

void PathwayGroupArray::Deserialize(std::istream &s, int version)
{
	clear();
	int n;
	s>>"--- Pathway Groups ---">>endl;
	s>>"Num Groups: ">>n>>std::endl;

	for(int i = 0; i < n; i++)
	{
		PathwayGroup group; 
		group.Deserialize(s, version);
		push_back(group);
	}
	s>>endl;
}

void PathwayGroupArray::ResetColors(bool bResetNames)
{
	if(bResetNames) // reset the names too , to FG 1, FG 2 and so on
	{
		clear();
		push_back(PathwayGroup("Trash", PATHWAY_COLORS[0]));
		string name = "FG 1";
		for(int i = 1; i < 9; i++,name[3]++)
			push_back(PathwayGroup(name, PATHWAY_COLORS[i]));
	}
	else
	{
		// Just reset the colors
		resize(9);
		Colord default_col;
		for(int i = 0; i < 9; i++)
			(*this)[i].SetColor(default_col);
	}
}


void PathwayGroupArray::Add(string name)
{
	int len = sizeof(PATHWAY_COLORS)/sizeof(Colord);
	int i = size()==0 ? 0: ( (size()-1)%(len-1) + 1 );

	// Cycle through the list of default colors from PATHWAY_COLORS array
	push_back(PathwayGroup(name, PATHWAY_COLORS[i]));
}
