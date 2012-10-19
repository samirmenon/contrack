/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

/*************************************************************************
 *    NAME: David Akers and Anthony Sherbondy
 *    USER: dla
 *    FILE: complexFunction.C
 *    DATE: Sat Nov  7 18:40:55 US/Eastern 1998
 *************************************************************************/
#include "DTIFilterTree.h"
#include "DTIPathwayDatabase.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#ifndef FALSE
#define TRUE 1
#define FALSE 0
#endif

int DTIFilterTree::containsPrimes() const {
  for (int i = 0; i < (int)strlen (functionString); i++) {
    if (functionString[i] == '\'') {
    return TRUE;
    }
  }
return FALSE;
}

int DTIFilterTree::containsPrime(char fname) const {
char curname = ' ';
  for (int i = 0; i < (int)strlen (functionString); i++) {
    if (functionString[i] != '\'' && functionString[i] != ' ')
      curname = functionString[i];
    if (functionString[i] == '\'' && curname == fname) {
    return TRUE;
    }
  }
return FALSE;
}

int operator== (const DTIFilterTree &instance1, const DTIFilterTree &instance2) {
  return (&instance1 == &instance2);
}

ostream &operator<< (ostream &theStream, DTIFilterTree &theClass) {
  theStream << theClass.fname_ << " :" << theClass.functionString << endl;
  return theStream;
}

istream &operator>> (istream &theStream, DTIFilterTree &theClass) {
  theStream >> theClass.fname_;
  char c;
  theStream >> c;
  theStream.getline(theClass.functionString,255);
  for (int i = 0; i < (int)strlen (theClass.functionString); i++) {
    if (theClass.functionString[i] == 13) theClass.functionString[i] = 0; 
  }
  theClass.chooseVar ("z");
  theClass.setFunc (theClass.functionString);
  return theStream;
}

/*************************************************************************
 * Function Name: DTIFilterTree::DTIFilterTree
 * Parameters: FunctionManager *mgr
 * Effects:
 *************************************************************************/

DTIFilterTree::DTIFilterTree()
{
  theFunction = NULL;
  numFunctionsAttachedTo = 0;
  //constant_manager = constantManager;
  roi_manager = NULL;
  strcpy (derivOf, "");
  function_OK = FALSE;
  strcpy (functionString, "");
  // Set up link to constants, attachedTO list..
  FUNCTION_CHANGED = TRUE;
  //  for (int i = 0; i < MAX_NUM_ATTACHED; i++) {
  //  attachedTo[i] = NULL;
  // }

}


/*************************************************************************
 * Function Name: DTIFilterTree::~DTIFilterTree
 * Parameters:
 * Effects:
 *************************************************************************/

DTIFilterTree::~DTIFilterTree()
{
}


/*************************************************************************
 * Function Name: DTIFilterTree::unattachAll
 * Parameters:
 * Returns: void
 * Effects:
 *************************************************************************/
void
DTIFilterTree::unattachAll()
{
  assert (FALSE);
}


/*************************************************************************
 * Function Name: DTIFilterTree::addObject
 * Parameters: void *newObject
 * Returns: void
 * Effects:
 *************************************************************************/
void
DTIFilterTree::addObject(void *newObject)
{
  dependent *d = new dependent;
  d->depends = TRUE;
  d->obj = newObject;
  dependents_.push_back (d);
}


/*************************************************************************
 * Function Name: DTIFilterTree::removeObject
 * Parameters: void *oldObject
 * Returns: void
 * Effects:
 *************************************************************************/
void
DTIFilterTree::removeObject(void *oldObject)
{
  for (DependentList::iterator iter = dependents_.begin(); iter != dependents_.end(); iter++) {
    if ((*iter)->obj == oldObject) {
    dependents_.erase(iter);
    delete (*iter);
    return;
    }
  }
}


/*************************************************************************
 * Function Name: DTIFilterTree::chooseVar
 * Parameters: const char *varName
 * Returns: void
 * Effects:
 *************************************************************************/
void
DTIFilterTree::chooseVar(const char *varName)
{
  assert (varName);
  strcpy (var_name, varName);

}


/*************************************************************************
 * Function Name: DTIFilterTree::setVarValue
 * Parameters: const complex &value
 * Returns: void
 * Effects:
 *************************************************************************/
void
DTIFilterTree::setVarValue(const NumberType &value)
{
  var_value = value;
}


/*************************************************************************
 * Function Name: DTIFilterTree::print
 * Parameters:
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIFilterTree::print()
{
  if (theFunction != NULL) 
    theFunction->print();
  else
    fprintf (stderr, "No function defined.");
}

/*************************************************************************
 * Function Name: DTIFilterTree::setFunc
 * Parameters: const char *function
 * Returns: void
 * Effects:
 *************************************************************************/
int
DTIFilterTree::setFunc(const char *function)
{
  numFunctionsAttachedTo = 0;
  strcpy (derivOf, "");
  // Remove all attachments..

  assert (roi_manager != NULL);
  FunctionParser *parser = new FunctionParser (this, roi_manager);
  //  cerr << "About to parse." << endl;
  parser->parse (function);
  //  cerr << "Parsing..." << endl;
  function_OK = parser->OK();
  /*if (returnVal) { theFunction = parser->getHead();
                   strcpy (functionString, function);
  }*/ /* removed 3/1/99 */
  strcpy (functionString, function);
  //  cerr << "setting function string to " << function << endl;
  //  cerr << "(this = " << (int) this << ")" << endl;
  if (!function_OK) {
    //theFunction = NULL;
    return FALSE;
	  //exit(0);
  }
    else theFunction = parser->getHead();

  //function_OK = (returnVal || function_OK);
  FUNCTION_CHANGED = TRUE;
  return TRUE;
}


/*************************************************************************
 * Function Name: DTIFilterTree::setFunc
 * Parameters: const DTIFilterTree *oldFunc
 * Returns: void
 * Effects:
 *************************************************************************/
int
DTIFilterTree::setFunc(const DTIFilterTree *oldFunc)
{
  assert (FALSE);
  return 0;
}


/*************************************************************************
 * Function Name: DTIFilterTree::setFunction
 * Parameters: FuncNode *newHead
 * Returns: void
 * Effects: 
 *************************************************************************/
void
DTIFilterTree::setFunction(FuncNode *newHead)
{
  //  if (theFunction != NULL) theFunction->cleanUpFuncs();
  theFunction = newHead;
}


/*************************************************************************
 * Function Name: DTIFilterTree::copy
 * Parameters: DTIFilterTree newFunc
 * Returns: FuncNode *
 * Effects: 
 *************************************************************************/
FuncNode *
DTIFilterTree::copy(DTIFilterTree *newFunc)
{
  if (theFunction != NULL) {
    FuncNode *newTree = theFunction->copy(newFunc);
    return newTree;}
  else return NULL;
}


/*************************************************************************
 * Function Name: DTIFilterTree::getMatchingPathways
 * Parameters: 
 * Returns: DTIPathwayDatabase *
 * Effects: 
 *************************************************************************/

DTIPathwayDatabase *
DTIFilterTree::getMatchingPathways (DTIPathwayDatabase *source)
{
	if (theFunction != NULL) {
	  //return this->theFunction->getMatchingPathways(source);
	  DTIPathwayDatabase *results = new DTIPathwayDatabase();
	  for (int i = 0; i < source->getNumFibers(); i++) {
		  DTIPathway *pathway = source->getPathway(i);
		  if (this->theFunction->pathwayMatches(pathway)) {
			  results->addPathway (pathway);
		  }
	  }
	  return results;
	}
	else {
		return source->copy();
	}
}


/*************************************************************************
 * Function Name: DTIFilterTree::copy
 * Parameters: 
 * Returns: DTIFilterTree *
 * Effects: 
 *************************************************************************/
DTIFilterTree *
DTIFilterTree::copy()
{
  DTIFilterTree *newFunc = new DTIFilterTree();
  newFunc->chooseVar (var_name);
  newFunc->FUNCTION_CHANGED = TRUE;
  newFunc->function_OK = function_OK;
  strcpy (newFunc->functionString, functionString);
  if (function_OK) 
    newFunc->setFunction (copy (newFunc));
  return newFunc;
}

void DTIFilterTree::setDependent (void *dependent, int value) {
  for (DependentList::iterator iter = dependents_.begin(); iter != dependents_.end(); iter++) {
    if ((*iter)->obj == dependent) {
      (*iter)->depends = value;
      return;
    }
  }
}

dependent *DTIFilterTree::getDependent (void *key) {
   for (DependentList::iterator iter = dependents_.begin(); iter != dependents_.end(); iter++) {
    if ((*iter)->obj == key) {
      return (*iter);
    }
  }
  // Not found!
return NULL;
}


/*************************************************************************
 * Function Name: DTIFilterTree::doesPathwayMatch
 * Parameters: DTIPathway *pathway
 * Returns: bool
 * Effects: 
 *************************************************************************/
bool
DTIFilterTree::doesPathwayMatch(DTIPathway *pathway)
{

  if (function_OK && theFunction != NULL) {
    //return this->theFunction->getMatchingPathways(source);
      if (this->theFunction->pathwayMatches(pathway)) {
	return true;
      }
      else {
	return false;
      }
  }
  else {
    return true;
  }
}

