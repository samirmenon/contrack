#ifndef PATHWAY_SELECTION_H
#define PATHWAY_SELECTION_H

#include <vector>
#include "PathwayGroup.h"

//! Possible gesture based boolean operation that can be done of a fiber group
enum BOOLEAN_OPERATION
{
	UNION,
	SUBTRACT,
	INTERSECT,
	SELECT_ALL,
	DESELECT_ALL
};

class DTIPathwayAssignment;
//! This class stores selectability of each fiber
/*! Initially the class stores the eligibility of each fiber for 
	filtering operation. After the filtering operation, the boolean
	value is interpreted as a pass/fail for the fiber.
	This helps in concatenating the results of filtering operation for
	eg. doing a stats based filtering followed by ROI based filtering
 */
class DTIPathwaySelection : public std::vector<bool>
{
public:
	//! The default constructor, all pathways are initialized with \param init
	DTIPathwaySelection(int numPathways, bool init = false):std::vector<bool>(numPathways, init){}
	
	//! Prunes away fibers depending on the boolean operation
	static DTIPathwaySelection PruneForGesture(BOOLEAN_OPERATION op, DTIPathwayAssignment *assignment, PathwayGroupArray &array);

	//! Prunes the pathways for stats based selection
	static DTIPathwaySelection PruneForStatistics(DTIPathwayAssignment *assignment, PathwayGroupArray &array);

	//! Prune hidden fibers
	static DTIPathwaySelection PruneHiddenFibers(DTIPathwayAssignment *assignment, PathwayGroupArray &array);

	//! Returns the number of selected pathways
	int NumSelectedPathways()
	{
		int selected = 0;
		for(size_t i = 0; i < size(); i++)
			if( (*this)[i])
				selected++;
		return selected;
	}
};

#endif
