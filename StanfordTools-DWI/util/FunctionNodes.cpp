/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

/*************************************************************************
 *    NAME: David Akers and Anthony Sherbondy
 *     
 *    FILE: FunctionNodes.cpp
 *    DATE: Sun Mar 28  8:29:25 HST 2004
 *************************************************************************/
#include "FunctionNodes.h"
#include "DTIPathway.h"
#include "DTIFilterROI.h"

/*************************************************************************
 * Function Name: FuncNode::FuncNode
 * Parameters: DTIFilterTree *func
 * Effects: 
 *************************************************************************/

FuncNode::FuncNode(DTIFilterTree *func)
{
  theFunction = func;
}


/*************************************************************************
 * Function Name: FuncNode::~FuncNode
 * Parameters: 
 * Effects: 
 *************************************************************************/

FuncNode::~FuncNode()
{
}


/*************************************************************************
 * Function Name: NodeBinary::NodeBinary
 * Parameters: DTIFilterTree *func
 * Effects: 
 *************************************************************************/

NodeBinary::NodeBinary(DTIFilterTree *func) : FuncNode (func)
{
  leftChild = rightChild = NULL;
}


/*************************************************************************
 * Function Name: NodeBinary::~NodeBinary
 * Parameters: 
 * Effects: 
 *************************************************************************/

NodeBinary::~NodeBinary()
{
}


/*************************************************************************
 * Function Name: NodeUnary::NodeUnary
 * Parameters: DTIFilterTree *func
 * Effects: 
 *************************************************************************/

NodeUnary::NodeUnary(DTIFilterTree *func) : FuncNode (func)
{
  child = NULL;
}


/*************************************************************************
 * Function Name: NodeUnary::~NodeUnary
 * Parameters: 
 * Effects: 
 *************************************************************************/

NodeUnary::~NodeUnary()
{
}


/*************************************************************************
 * Function Name: NodeLeaf::NodeLeaf
 * Parameters: DTIFilterTree *func
 * Effects: 
 *************************************************************************/

NodeLeaf::NodeLeaf(DTIFilterTree *func) : FuncNode (func)
{
  
}


/*************************************************************************
 * Function Name: NodeLeaf::~NodeLeaf
 * Parameters: 
 * Effects: 
 *************************************************************************/

NodeLeaf::~NodeLeaf()
{
}


/*************************************************************************
 * Function Name: NodeParen::NodeParen
 * Parameters: DTIFilterTree *func
 * Effects: 
 *************************************************************************/

NodeParen::NodeParen(DTIFilterTree *func) : NodeUnary (func)
{

}


/*************************************************************************
 * Function Name: NodeParen::~NodeParen
 * Parameters: 
 * Effects: 
 *************************************************************************/

NodeParen::~NodeParen()
{
}


/*************************************************************************
 * Function Name: NodeParen::print
 * Parameters: 
 * Returns: void
 * Effects: 
 *************************************************************************/
void
NodeParen::print()
{
  fprintf (stderr, "(");
  child->print();
  fprintf (stderr, ")");
}


/*************************************************************************
 * Function Name: NodeParen::getMatchingPathways
 * Parameters: 
 * Returns: DTIPathwayDatabase *
 * Effects: 
 *************************************************************************/
bool
NodeParen::pathwayMatches (DTIPathway *pathway)
{
  return child->pathwayMatches(pathway);
}


/*************************************************************************
 * Function Name: NodeParen::copy
 * Parameters: DTIFilterTree *newFunc
 * Returns: FuncNode *
 * Effects: 
 *************************************************************************/
FuncNode *
NodeParen::copy(DTIFilterTree *newFunc)
{
  NodeParen *newparen = new NodeParen (newFunc);
  newparen->setChild (child->copy (newFunc));
  return newparen;
}


/*************************************************************************
 * Function Name: NodeNot::NodeNot
 * Parameters: DTIFilterTree *function
 * Effects: 
 *************************************************************************/

NodeNot::NodeNot(DTIFilterTree *function) : NodeUnary (function)
{
}


/*************************************************************************
 * Function Name: NodeNot::~NodeNot
 * Parameters: 
 * Effects: 
 *************************************************************************/

NodeNot::~NodeNot()
{
}


/*************************************************************************
 * Function Name: NodeNot::print
 * Parameters: 
 * Returns: void
 * Effects: 
 *************************************************************************/
void
NodeNot::print()
{
  fprintf (stderr, " NOT (");
  child->print();
  fprintf (stderr, ")");
}


/*************************************************************************
 * Function Name: NodeNot::getMatchingPathways
 * Parameters: 
 * Returns: DTIPathwayDatabase *
 * Effects: 
 *************************************************************************/
bool
NodeNot::pathwayMatches (DTIPathway *pathway)
{
  return !(child->pathwayMatches (pathway));
}


/*************************************************************************
 * Function Name: NodeNot::copy
 * Parameters: DTIFilterTree *newFunc
 * Returns: FuncNode *
 * Effects: 
 *************************************************************************/
FuncNode *
NodeNot::copy(DTIFilterTree *newFunc)
{
  NodeNot *newnot = new NodeNot (newFunc);
  FuncNode *newChild = child->copy(newFunc);
  newnot->setChild (newChild);
  return newnot;
}


/*************************************************************************
 * Function Name: NodeAnd::NodeAnd
 * Parameters: DTIFilterTree *function
 * Effects: 
 *************************************************************************/

NodeAnd::NodeAnd(DTIFilterTree *function) : NodeBinary (function)
{
  
}


/*************************************************************************
 * Function Name: NodeAnd::~NodeAnd
 * Parameters: 
 * Effects: 
 *************************************************************************/

NodeAnd::~NodeAnd()
{
}


/*************************************************************************
 * Function Name: NodeAnd::print
 * Parameters: 
 * Returns: void
 * Effects: 
 *************************************************************************/
void
NodeAnd::print()
{
  leftChild->print();
  fprintf (stderr, " AND ");
  rightChild->print();
}


/*************************************************************************
 * Function Name: NodeAnd::getMatchingPathways
 * Parameters: 
 * Returns: DTIPathwayDatabase *
 * Effects: 
 *************************************************************************/
bool
NodeAnd::pathwayMatches (DTIPathway *pathway)
{
  return (leftChild->pathwayMatches (pathway) && rightChild->pathwayMatches (pathway));
  //DTIPathwayDatabase *leftResults = leftChild->getMatchingPathways (source);
  //return rightChild->getMatchingPathways (leftResults);
}


/*************************************************************************
 * Function Name: NodeAnd::copy
 * Parameters: DTIFilterTree *newFunc
 * Returns: FuncNode *
 * Effects: 
 *************************************************************************/
FuncNode *
NodeAnd::copy(DTIFilterTree *newFunc)
{ 
  NodeAnd *newNode = new NodeAnd(newFunc);
  newNode->setLeft(leftChild->copy(newFunc) );
  newNode->setRight(rightChild->copy(newFunc) );
  return (newNode);
}


/*************************************************************************
 * Function Name: NodeOr::NodeOr
 * Parameters: DTIFilterTree *function
 * Effects: 
 *************************************************************************/

NodeOr::NodeOr(DTIFilterTree *function) : NodeBinary (function)
{
  
}


/*************************************************************************
 * Function Name: NodeOr::~NodeOr
 * Parameters: 
 * Effects: 
 *************************************************************************/

NodeOr::~NodeOr()
{
}


/*************************************************************************
 * Function Name: NodeOr::print
 * Parameters: 
 * Returns: void
 * Effects: 
 *************************************************************************/
void
NodeOr::print()
{
  leftChild->print();
  fprintf (stderr, " AND ");
  rightChild->print();
}


/*************************************************************************
 * Function Name: NodeOr::getMatchingPathways
 * Parameters: 
 * Returns: DTIPathwayDatabase *
 * Effects: 
 *************************************************************************/
bool
NodeOr::pathwayMatches(DTIPathway *pathway)
{
	/*
  DTIPathwayDatabase *sourceCopy = source->copy();
  DTIPathwayDatabase *results = leftChild->getMatchingPathways (sourceCopy);
  for (int i = 0; i < results->getNumFibers(); i++) {
	  DTIPathway *pathway = results->getPathway(i);
	  sourceCopy->removePathway(pathway);
  }
  DTIPathwayDatabase *rightResults = rightChild->getMatchingPathways(sourceCopy);
  for (int j = 0; j < rightResults->getNumFibers(); j++) {
	  DTIPathway *pathway = rightResults->getPathway(j);
	  results->addPathway(pathway);
  }

  return results;
  //return rightChild->getMatchingPathways (leftResults);
  */
  return (leftChild->pathwayMatches (pathway) || rightChild->pathwayMatches (pathway));
}


/*************************************************************************
 * Function Name: NodeOr::copy
 * Parameters: DTIFilterTree *newFunc
 * Returns: FuncNode *
 * Effects: 
 *************************************************************************/
FuncNode *
NodeOr::copy(DTIFilterTree *newFunc)
{
  NodeOr *newNode = new NodeOr(newFunc);
  newNode->setLeft(leftChild->copy(newFunc) );
  newNode->setRight(rightChild->copy(newFunc) );
  return (newNode);
}


/*************************************************************************
 * Function Name: NodeSubOp::NodeSubOp
 * Parameters: DTIFilterTree *function
 * Effects: 
 *************************************************************************/

NodeSubOp::NodeSubOp(DTIFilterTree *function) : NodeBinary (function)
{
}


/*************************************************************************
 * Function Name: NodeSubOp::~NodeSubOp
 * Parameters: 
 * Effects: 
 *************************************************************************/

NodeSubOp::~NodeSubOp()
{
}


/*************************************************************************
 * Function Name: NodeSubOp::print
 * Parameters: 
 * Returns: void
 * Effects: 
 *************************************************************************/
void
NodeSubOp::print()
{
  leftChild->print();
  fprintf (stderr, " - ");
  rightChild->print();
}


/*************************************************************************
 * Function Name: NodeSubOp::getMatchingPathways
 * Parameters: 
 * Returns: DTIPathwayDatabase *
 * Effects: 
 *************************************************************************/
bool
NodeSubOp::pathwayMatches (DTIPathway *pathway)
{
  return (leftChild->pathwayMatches(pathway) && !rightChild->pathwayMatches(pathway));
}


/*************************************************************************
 * Function Name: NodeSubOp::copy
 * Parameters: DTIFilterTree *newFunc
 * Returns: FuncNode *
 * Effects: 
 *************************************************************************/
FuncNode *
NodeSubOp::copy(DTIFilterTree *newFunc)
{
  NodeSubOp *newNode = new NodeSubOp(newFunc);
  newNode->setLeft(leftChild->copy(newFunc) );
  newNode->setRight(rightChild->copy(newFunc) );
  return (newNode);
}




/*************************************************************************
 * Function Name: NodeFilterROI::NodeFilterROI
 * Parameters: DTIFilterTree *function, DTIFilterROI *filter
 * Effects: 
 *************************************************************************/

NodeFilterROI::NodeFilterROI(DTIFilterTree *function, DTIFilterROI *filter) : NodeLeaf (function)
{
	_filter = filter;
}


/*************************************************************************
 * Function Name: NodeFilterROI::~NodeFilterROI
 * Parameters: 
 * Effects: 
 *************************************************************************/

NodeFilterROI::~NodeFilterROI()
{
}


/*************************************************************************
 * Function Name: NodeFilterROI::print
 * Parameters: 
 * Returns: void
 * Effects: 
 *************************************************************************/
void
NodeFilterROI::print()
{
  assert (false);
  return;
}


/*************************************************************************
 * Function Name: NodeFilterROI::getMatchingPathways
 * Parameters: 
 * Returns: DTIPathwayDatabase *
 * Effects: 
 *************************************************************************/
bool
NodeFilterROI::pathwayMatches (DTIPathway *pathway)
{
	return this->_filter->matches(pathway);
}


/*************************************************************************
 * Function Name: NodeFilterROI::copy
 * Parameters: DTIFilterTree *newFunc
 * Returns: FuncNode *
 * Effects: 
 *************************************************************************/
FuncNode *
NodeFilterROI::copy(DTIFilterTree *newFunc)
{
	assert (false);
	return NULL;
}


