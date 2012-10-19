/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef FUNCTION_PARSER
#define FUNCTION_PARSER

#include "DTIFilterTree.h"
#include "FunctionNodes.h"
#include "tokens.h"
#include "typedefs.h"

class ROIManager;
class FuncNode;
class DTIFilterTree;

class FunctionParser {
public:

  // Methods:

  FunctionParser (DTIFilterTree *function, ROIManager *constantManager);
  ~FunctionParser();

  int OK() { return function_OK; }
  FuncNode *getHead() { return theHead; }
  void parse (const char *inputString);

  int Match (FuncToken token);
  void UnMatch();
  int WillMatch (FuncToken token);

  void DerivDeriv();
  FuncNode *ScalarExpressionDeriv();
  FuncNode *ScalarDeriv();
  FuncNode *ScalarDeriv2(FuncNode *temp);
  FuncNode *LogicalOperandDeriv1();
  FuncNode *LogicalOperandDeriv2(FuncNode *temp);
  FuncNode *FunctionDeriv();
  FuncNode *ComparedDeriv();
  FuncNode *FuncDeriv1();
  FuncNode *FuncDeriv2 (FuncNode *temp);

  FuncNode *SummandDeriv1();
  FuncNode *SummandDeriv2(FuncNode *temp);

  FuncNode *FactorDeriv1();
  FuncNode *FactorDeriv2(FuncNode *temp);
  
  FuncNode *OperandDeriv1();

  FuncNode *TerminalDeriv();

  // ***********  TOKENIZER  *********** //

  FuncToken scan();
  FuncToken integerState();
  FuncToken decimalState();
  FuncToken stringState();

private:

  DTIFilterTree *theFunction;
  int function_OK;
  FuncNode *theHead;
  char theInput[255];
  char currentChar, nextChar;
  int inputOffset, inputLength;
  int unmatched;
  FuncToken currToken, storeToken, nextToken;
  char tokenString[255];
  char nextTokenString[255];

  double tokenValue;
  double tokenTenths;
  double nextTokenValue;
  double nextTokenTenths;
  double storeTokenValue;
  double storeTokenTenths;

  char storeTokenString[255];

  char storeTokenChar;
  char tokenChar;
  char nextTokenChar;

  int inAbsoluteBars;

  DTIFilterTree *funcToUse;
  ROIManager *roi_manager;

};

#endif
