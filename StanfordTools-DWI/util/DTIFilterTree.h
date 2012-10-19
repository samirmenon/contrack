/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef DTI_FILTER_TREE_H
#define DTI_FILTER_TREE_H

#define MAX_NUM_ATTACHED 50


#include "typedefs.h"
//#include <functionManager.h>
#include "FunctionParser.h"
#include "FunctionNodes.h"
//#include <functionConstant.h>
class DTIPathwayDatabase;
#include <list>
using namespace std;
#include <string.h>

class DTIPathway;
class ROIManager;

struct dependent {
  void *obj;
  int depends;
};

typedef list<dependent *> DependentList;

class DTIFilterTree {

public:

  DTIFilterTree();
  ~DTIFilterTree();

  void setFunctionName (const char *name) { strcpy (fname_, name); }
  char * getFunctionName () { return fname_; }

  friend int operator== (const DTIFilterTree &instance1, const DTIFilterTree &instance2);
  /*private:
   DTIFilterTree &operator= (const DTIFilterTree &other);*/
  public:
  friend ostream &operator<< (ostream &theStream, DTIFilterTree &theFunction);
  friend istream &operator>> (istream &theStream, DTIFilterTree &theFunction);

  void setROIManager (ROIManager *manager) {roi_manager = manager; }
  int isAttachedTo (const DTIFilterTree *otherFunc) const;
  void attachMeTo (DTIFilterTree *otherFunc);
  void unattachAll();
  void unattachMeTo (DTIFilterTree *otherFunc);

  int functionOK() { return function_OK; }

  void addObject (void *newObject);
  void removeObject (void *oldObject);

  int functionChanged (void *dependent);
  int derivChanged();

  void chooseVar (const char *varName);
  char *getVarName () { return var_name; }

  void setVarValue (const NumberType &value);
  void setSecondaryValue (const NumberType &value) { secondary_value = value; }

  void print();

  bool doesPathwayMatch (DTIPathway *pathway);

  int setFunc (const char *function);
  int setFunc (const DTIFilterTree *oldFunc);
  void setFunction (FuncNode *newHead);

  FuncNode *copy (DTIFilterTree *newFunc);

  DTIFilterTree *copy();

  void setDependent (void *dependent, int value);
  dependent *getDependent (void *dependent);

  char *getFunctionString () { return functionString; }

  int containsPrimes() const;
  int containsPrime (char function) const;
  
  DTIPathwayDatabase *getMatchingPathways (DTIPathwayDatabase *source);

private:

  char fname_[255];

  DependentList dependents_;
  NumberType var_value;
  NumberType secondary_value;
  char var_name[255];

  ROIManager *roi_manager;
  FuncNode *theFunction;

  // Dependent objects hash-table
  int FUNCTION_CHANGED;
  int function_OK;
  // a way to get at the functionnames hash table (in mgr)

  int numFunctionsAttachedTo;

  char derivOf[255]; // If we're a derivative of something, what is it?

  char functionString[255];


};

#endif
