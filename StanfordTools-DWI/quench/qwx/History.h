#ifndef HISTORY_H
#define HISTORY_H

#include <vector>
#include "PDBHelper.h"
#include "PathwayGroup.h"
#include "typedefs_quench.h"
class QuenchController;

class Command
{
public:
	Command(QuenchController *quench) { _quench = quench; }
	virtual void Redo() = 0;
	virtual void Undo() = 0;
protected:
	QuenchController *_quench;
};

class CommandAssignmentChangedByGesture : public Command
{
public:
	CommandAssignmentChangedByGesture(QuenchController *quench);
	virtual void Redo();
	virtual void Undo();
protected:
	DTIPathwayAssignment _assn;
};

class CommandAssignmentChangedByStats : public Command
{
public:
	CommandAssignmentChangedByStats(QuenchController *quench);
	virtual void Redo();
	virtual void Undo();
protected:
	DTIPathwayAssignment _assn;
};

class CommandFiberGroupsChanged : public Command
{
public:
	CommandFiberGroupsChanged(QuenchController *quench);
	virtual void Redo();
	virtual void Undo();
protected:
	DTIPathwayAssignment _assn;		///The pathway assignment which is used for undo/redo
	PathwayGroupArray _pgarray;		///The group array which is used for undo/redo
};

class CommandManager : public std::vector< PCommand >
{
public:
	CommandManager() { index = -1;}
	//! Tries to redo \return true if redo was possible, false otherwise
	bool Redo();
	//! Tries to undo \return true if undo was possible, false otherwise
	bool Undo();
	//! Updates
	void push_back(PCommand cmd);
	//! Overrides the vector clear as we need to reset the index
	void clear();
protected:
	int index;						///Location of the command stack 
};

#endif