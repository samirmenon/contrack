#include "DTIPathwaySelection.h"
#include <typedefs.h>
#include <cassert>
#include "PDBHelper.h"

/*
Union
Consider if:
visible==true && id != SelectedGroup && assignemntsLocked == false
if pass 
set to selected group

Intersect
Consider if:
visible==true && id == SelectedGroup && assignemntsLocked == Don't Care
if fail 
set to 0

Subtract
Consider if:
visible==true && id == SelectedGroup && assignemntsLocked == Don't Care
if pass
set to 0

Select All
Consider if:
visible==true && id == 0
set to SelectedGroup

Deselect All
Consider if:
visible==true && id == SelectedGroup
set to 0

*/
DTIPathwaySelection DTIPathwaySelection::PruneForGesture(BOOLEAN_OPERATION op, DTIPathwayAssignment *assignment, PathwayGroupArray &array)
{
	int numPathways = (int)assignment->size();
	DTIPathwaySelection sel(numPathways, false);

	bool shouldMatchGroup 	= op != UNION;
	int  comparisionGroup 	= /*op == SELECT_ALL  ?  0 :*/ assignment->SelectedGroup();
	bool assignmentsLocked 	= op == UNION  ?  !assignment->Locked() : true;

	for(int i = 0; i < numPathways; i++)
	{
		int id = (*assignment)[i];
		sel[i] = array[id].Visible() && ( (id == assignment->SelectedGroup()) == shouldMatchGroup) && assignmentsLocked;
	}

	//For union, all fibers from trash group can participate
	if( op == UNION || op == SELECT_ALL)
		for(int i = 0; i < numPathways; i++)
			if((*assignment)[i] == DTI_COLOR_UNASSIGNED)
				sel[i] = array[0].Visible();

	return sel;
}

/*
Filter by stats
Consider if:
Active == true && 
if( assignmentsLocked )
	if( id == 0 || id == selected group)
else
	all
*/
DTIPathwaySelection DTIPathwaySelection::PruneForStatistics(DTIPathwayAssignment *assignment, PathwayGroupArray &array)
{
	int numPathways = (int)assignment->size();
	DTIPathwaySelection sel(numPathways, false);

	if(assignment->Locked() == false)
	{
		for(int i = 0; i < numPathways; i++)
		{
			int id = (*assignment)[i];
			sel[i] = array[id].Active();
		}
	}
	else
	{
		bool group0Active = array[0].Active();
		for(int i = 0; i < numPathways; i++)
		{
			int id = (*assignment)[i];
			sel[i] = array[id].Active() && ( (id == 0 && group0Active) || id == assignment->SelectedGroup() );
		}
	}

	return sel;
}

DTIPathwaySelection DTIPathwaySelection::PruneHiddenFibers(DTIPathwayAssignment *assignment, PathwayGroupArray &array)
{
	int numPathways = (int)assignment->size();
	DTIPathwaySelection sel(numPathways, false);

	for(int i = 0; i < numPathways; i++)
	{
		int id = (*assignment)[i];
		sel[i] = array[id].Visible();
	}

	return sel;
}
