#include "History.h"
#include "PDBHelper.h"

//#include "Quench.h"
#include "QuenchController.h"

CommandAssignmentChangedByGesture::CommandAssignmentChangedByGesture(QuenchController *quench) : Command(quench)
{
	_assn = _quench->PDBHelper_().Assignment();
}
void CommandAssignmentChangedByGesture::Redo()
{
	_quench->SwapAssignment(_assn);
}
void CommandAssignmentChangedByGesture::Undo()
{
	_quench->SwapAssignment(_assn);
}

CommandAssignmentChangedByStats::CommandAssignmentChangedByStats(QuenchController *quench) : Command(quench)
{
	_assn = _quench->PDBHelper_().InteractiveAssignment();
}
void CommandAssignmentChangedByStats::Redo()
{
	_quench->SwapAssignment(_assn);
}
void CommandAssignmentChangedByStats::Undo()
{
	_quench->SwapAssignment(_assn);
}


CommandFiberGroupsChanged::CommandFiberGroupsChanged(QuenchController *quench) : Command(quench)
{
	_assn = _quench->PDBHelper_().Assignment();
	_pgarray = _quench->PathwayGroupArray_();
}
void CommandFiberGroupsChanged::Redo()
{
	_quench->SwapGroupArray(_pgarray);
	_quench->SwapAssignment(_assn);
}
void CommandFiberGroupsChanged::Undo()
{
	_quench->SwapGroupArray(_pgarray);
	_quench->SwapAssignment(_assn);
}

bool CommandManager::Redo()
{
	// are we already at the end of the command list?
	if(index == (int)size()-1)
		return false; // redo failed
	// otherwise attempt a Redo
	(*this)[++index]->Redo();
	return true;
}
bool CommandManager::Undo()
{
	// are we already at the start of the command list?
	if(index == -1)
		return false; // undo failed
	// otherwise do an undo
	(*this)[index--]->Undo();
	return true;
}
void CommandManager::push_back(PCommand cmd)
{
	//Delete all the actions beyond the active one which located at index
	erase(begin()+(index+1), end());

	index = (int)size();
	((std::vector<PCommand>*)(this))->push_back(cmd);
}
void CommandManager::clear()
{
	// clear the history
	index = -1;
	((std::vector<PCommand>*)(this))->clear();
}
