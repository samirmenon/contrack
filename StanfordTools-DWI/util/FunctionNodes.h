/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef NODES_H
#define NODES_H

#include "DTIFilterTree.h"

class DTIFilterROI;
class DTIFilterTree;
class ConstantManager;
class DTIPathway;
#include "typedefs.h"

class FuncNode {

public:

  FuncNode (DTIFilterTree *func);
  virtual ~FuncNode();

  virtual bool pathwayMatches (DTIPathway *pathway) = 0;

  virtual void print () = 0;
  virtual FuncNode *copy(DTIFilterTree *newFunc) = 0;

protected:

  DTIFilterTree *theFunction;
};

class NodeBinary : public FuncNode {
public:

  NodeBinary (DTIFilterTree *func);
  ~NodeBinary ();

  void setLeft (FuncNode *newNode) { leftChild = newNode; }
  void setRight (FuncNode *newNode) { rightChild = newNode; }

  virtual bool pathwayMatches (DTIPathway *pathway) = 0;


protected:

  FuncNode *leftChild;
  FuncNode *rightChild;


};

class NodeUnary : public FuncNode {
public:
  NodeUnary (DTIFilterTree *func);
  ~NodeUnary();
 
  virtual bool pathwayMatches (DTIPathway *pathway) = 0;

  void setChild (FuncNode *newNode) { child = newNode; }

protected:
  FuncNode *child;
};

class NodeLeaf : public FuncNode {
public:
  NodeLeaf (DTIFilterTree *func);
  ~NodeLeaf();
  virtual bool pathwayMatches (DTIPathway *pathway) = 0;
private:
};




class NodeParen : public NodeUnary {
public:
  // do this now
  NodeParen (DTIFilterTree *func);
  ~NodeParen();
  void print();
  virtual bool pathwayMatches (DTIPathway *pathway);
  FuncNode *copy(DTIFilterTree *newFunc);
};

class NodeNot : public NodeUnary {
public:
  NodeNot (DTIFilterTree *function);
  ~NodeNot();
  void print();
  virtual bool pathwayMatches (DTIPathway *pathway);
  FuncNode *copy(DTIFilterTree *newFunc);
};

class NodeAnd : public NodeBinary {
public:
  NodeAnd (DTIFilterTree *function);
  ~NodeAnd();
  void print();
  virtual bool pathwayMatches (DTIPathway *pathway);
  FuncNode *copy(DTIFilterTree *newFunc);
};

class NodeOr : public NodeBinary {
public:
  NodeOr (DTIFilterTree *function);
  ~NodeOr();
  void print();
  virtual bool pathwayMatches (DTIPathway *pathway);
  FuncNode *copy(DTIFilterTree *newFunc);
};


class NodeSubOp : public NodeBinary {
  public:
  NodeSubOp (DTIFilterTree *function);
  ~NodeSubOp();
  void print();
  virtual bool pathwayMatches (DTIPathway *pathway);
  FuncNode *copy(DTIFilterTree *newFunc);

};

class NodeFilterROI : public NodeLeaf {
public:
	NodeFilterROI (DTIFilterTree *function, DTIFilterROI *filter);
	~NodeFilterROI();
	void print(); 
	virtual bool pathwayMatches (DTIPathway *pathway);
	FuncNode *copy (DTIFilterTree *newFunc);
private:
	DTIFilterROI *_filter;
};

#endif




