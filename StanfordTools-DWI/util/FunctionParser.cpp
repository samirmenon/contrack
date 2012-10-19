/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

/*************************************************************************
 *    NAME: David Akers and Anthony Sherbondy
 *    USER: dla
 *    FILE: functionParser.C
 *    DATE: Sat Nov  7 19:21:39 US/Eastern 1998
 *************************************************************************/

#include "FunctionParser.h"
#include "ROIManager.h"
#include <string.h>
#include "tokens.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "typedefs.h"

#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif

#ifdef WIN32
#define strcasecmp strcmpi
#endif


int getCommaDelimitedText(const char *input, char *result, int startPos) {
  int index=0;
  int i=startPos;
  for (i = startPos; i < (int)strlen (input); i++) {
    if (input[i] != ',') {
    result[index] = input[i];
    }
    else break;
    index++;
  }
  result[index] = 0;
  if (i == strlen (input))
    return FALSE;
  else
    return TRUE;
}

// Make functionNames a hash mapping, and

/*************************************************************************
 * Function Name: FunctionParser::FunctionParser
 * Parameters: DTIFilterTree *function
 * Effects:
 *************************************************************************/

FunctionParser::FunctionParser(DTIFilterTree *function, ROIManager *roiManager)
{
  theFunction = function;
  function_OK = TRUE;
  roi_manager = roiManager;
  //roi_manager->getROIFilter ("");
}


/*************************************************************************
 * Function Name: FunctionParser::~FunctionParser
 * Parameters: 
 * Effects: 
 *************************************************************************/

FunctionParser::~FunctionParser()
{
}


/*************************************************************************
 * Function Name: FunctionParser::parse
 * Parameters: const char *inputString
 * Returns: void
 * Effects:
 *************************************************************************/
void
FunctionParser::parse(const char *inputString)
{
  strcpy (theInput, inputString);
  funcToUse = NULL;
  inputOffset = 0;
  inputLength = strlen (theInput); // watch here
  inAbsoluteBars = FALSE;

  strcpy (tokenString, "");
  strcpy (nextTokenString, "");
  tokenValue = 0;
  nextTokenValue = 0;
  tokenChar = ' ';
  nextTokenChar = ' ';

  tokenTenths = 0;
  nextTokenTenths = 0;
  
  strcpy (storeTokenString, "");
  storeTokenValue = 0;
  storeTokenTenths = 0;
  storeTokenChar = ' ';
  unmatched = FALSE;
  
  storeToken = TOKEN_BOF;
  nextToken = TOKEN_NULL;
  currToken = scan();
  theHead = NULL;
  
  FuncNode *newFunc = NULL;
  if (function_OK) {
    newFunc = ScalarExpressionDeriv();
  if (function_OK) {
    theHead = newFunc; }
  }
  
}


/*************************************************************************
 * Function Name: FunctionParser::Match
 * Parameters: FuncToken token
 * Returns: int
 * Effects:
 *************************************************************************/
int
FunctionParser::Match(FuncToken token)
{
  if (currToken == TOKEN_NULL) { return FALSE; }
  int returnVal = (token == currToken);

  if (returnVal) {
    if (unmatched) {
      FuncToken tempToken;
      char tempString[255];
      double tempValue;
      char tempChar;

      tempToken = currToken;
      strcpy (tempString, tokenString);
      tempChar = tokenChar;
      tempValue = tokenValue;


       currToken = storeToken;
       strcpy (tokenString, nextTokenString);
       tokenValue = nextTokenValue;
       tokenChar = nextTokenChar;



       strcpy (nextTokenString, storeTokenString);
       nextTokenValue = storeTokenValue;
       nextTokenChar = storeTokenChar;
       unmatched = FALSE;

	    storeTokenValue = tempValue;  //save our stuff
	    strcpy(storeTokenString, tempString);
	    storeTokenChar = tempChar;
	    storeToken = tempToken;

     }
   else

          {

	    storeTokenValue = tokenValue;  //save our stuff
	    strcpy (storeTokenString, tokenString);
	    storeTokenChar = tokenChar;
	    storeToken = currToken;

           tokenValue = nextTokenValue;
           strcpy(tokenString, nextTokenString);
           tokenChar = nextTokenChar;
           currToken = scan();

           if (currToken == TOKEN_NULL) {function_OK = FALSE;}
          }
  }
  return returnVal;


}


/*************************************************************************
 * Function Name: FunctionParser::UnMatch
 * Parameters:
 * Returns: void
 * Effects: 
 *************************************************************************/
void
FunctionParser::UnMatch()
{
  if (!(unmatched)) {
    FuncToken tempToken;
    char tempString[255];
    double tempValue;
    char tempChar;
    



    tempToken = currToken;
    strcpy (tempString, tokenString);
    tempChar = tokenChar;
    tempValue = tokenValue;

    currToken = storeToken;
    strcpy (tokenString,storeTokenString);
    tokenValue = storeTokenValue;
    tokenChar = storeTokenChar;

    
    //    storeToken = nextToken;
    strcpy (storeTokenString, nextTokenString);
    storeTokenValue = nextTokenValue;
    storeTokenChar = nextTokenChar;

    strcpy (nextTokenString, tempString);
    nextTokenChar = tempChar;
    nextTokenValue = tempValue;

    storeToken = tempToken; //keep it
    
    unmatched = TRUE;

    }
}


/*************************************************************************
 * Function Name: FunctionParser::WillMatch
 * Parameters: FuncToken token
 * Returns: int
 * Effects: 
 *************************************************************************/
int
FunctionParser::WillMatch(FuncToken token)
{
   if (currToken == TOKEN_NULL) {return FALSE;} //go no further

   int returnVal = (token == currToken);

   return returnVal;
}



/*************************************************************************
 * Function Name: FunctionParser::ScalarExpressionDeriv
 * Parameters:
 * Returns: FuncNode *
 * Effects: 
 *************************************************************************/
FuncNode *
FunctionParser::ScalarExpressionDeriv()
{

  FuncNode *returnVal;
  FuncToken token = currToken;


  if (token == TOKEN_NULL) {function_OK = FALSE;
                                       return  NULL;}

  else returnVal = ScalarDeriv();

  return returnVal;
}


/*************************************************************************
 * Function Name: FunctionParser::ScalarDeriv
 * Parameters: 
 * Returns: FuncNode *
 * Effects: 
 *************************************************************************/
FuncNode *
FunctionParser::ScalarDeriv()
{
  FuncNode *returnVal;
  FuncToken token = currToken;
  if (token == TOKEN_NULL) {function_OK = FALSE;
  returnVal = NULL;}
  else {
    FuncNode *temp = LogicalOperandDeriv1();
    returnVal = ScalarDeriv2(temp);
  }
  return returnVal;
}


/*************************************************************************
 * Function Name: FunctionParser::ScalarDeriv2
 * Parameters:
 * Returns: FuncNode *
 * Effects:
 *************************************************************************/
FuncNode *
FunctionParser::ScalarDeriv2(FuncNode *temp)
{
  FuncToken token = currToken;
  if (token == TOKEN_NULL) {function_OK = FALSE;
                                      return(NULL);
                                      }
  return temp;
  //  FuncToken token = currToken;
  //  if (token == TOKEN_NULL) {function_OK = FALSE;
  //                                        return(NULL);
  //                                        }

}


/*************************************************************************
 * Function Name: FunctionParser::LogicalOperandDeriv1
 * Parameters: 
 * Returns: FuncNode *
 * Effects:
 *************************************************************************/
FuncNode *
FunctionParser::LogicalOperandDeriv1()
{
  FuncNode *returnVal;
  FuncToken token = currToken;
  if (token == TOKEN_NULL) {function_OK = FALSE;
  returnVal = NULL;}
  else {
    FuncNode *temp = ComparedDeriv();
    returnVal = LogicalOperandDeriv2(temp);
  }
  return returnVal;

}


/*************************************************************************
 * Function Name: FunctionParser::LogicalOperandDeriv2
 * Parameters:
 * Returns: FuncNode *
 * Effects: 
 *************************************************************************/
FuncNode *
FunctionParser::LogicalOperandDeriv2(FuncNode *temp)
{
    FuncToken token = currToken;
    if (token == TOKEN_NULL) {function_OK = FALSE;
    return(NULL);
    }
    //function_OK = FALSE;
    return temp;
}



/*************************************************************************
 * Function Name: FunctionParser::ComparedDeriv
 * Parameters:
 * Returns: FuncNode *
 * Effects:
 *************************************************************************/
FuncNode *
FunctionParser::ComparedDeriv()
{
  FuncNode * returnVal;
  FuncToken token = currToken;
  
  if (token == TOKEN_NULL) {function_OK = FALSE;
  returnVal = NULL;}
  else
    
    returnVal = FuncDeriv1(); //not a vector, so just get a function
  
  return returnVal;

}


/*************************************************************************
 * Function Name: FunctionParser::FuncDeriv1
 * Parameters: 
 * Returns: FuncNode *
 * Effects: 
 *************************************************************************/
FuncNode *
FunctionParser::FuncDeriv1()
{
  FuncNode * returnVal;
  FuncToken token = currToken;

  if (token == TOKEN_NULL) {function_OK = FALSE;
  returnVal = NULL;}
  else {
    FuncNode * temp = SummandDeriv1();
    returnVal = FuncDeriv2(temp);  
  }
  return returnVal;

}


/*************************************************************************
 * Function Name: FunctionParser::FuncDeriv2
 * Parameters: FuncNode *temp
 * Returns: FuncNode *
 * Effects: 
 *************************************************************************/
FuncNode *
FunctionParser::FuncDeriv2(FuncNode *temp)
{

   if (Match(TOKEN_OR)) {
     /*if (WillMatch(TOKEN_STRING) &&
theFunction->getFunction(nextTokenString)) {
       UnMatch();
       return temp;  
     }
     else*/ {
       NodeOr *orNode = new NodeOr(theFunction);
       orNode->setLeft(temp);
       FuncToken newToken = currToken;
       if (newToken == TOKEN_NULL) {
	 function_OK = FALSE;
	 return(NULL);
       }
       FuncNode * newNode = SummandDeriv1();
       if (newNode == NULL) {function_OK= FALSE;
       return(NULL);
       }
       orNode->setRight(newNode);
       return FuncDeriv2(orNode);
     }

   } //end if addop
   else if (Match (TOKEN_SUB_OP)) {
    /* if (WillMatch(TOKEN_STRING) &&
theFunction->getFunction(nextTokenString)) {
       UnMatch();
       return temp;
     }
     else */{    
       NodeSubOp *addNode = new NodeSubOp(theFunction);
       addNode->setLeft(temp);
       FuncToken newToken = currToken;
       if (newToken == TOKEN_NULL) {function_OK = FALSE;
       return(NULL);
       }
       FuncNode * newNode = SummandDeriv1();
       if (newNode == NULL) {function_OK= FALSE;
       return(NULL);
       }
       addNode->setRight(newNode);
       return FuncDeriv2(addNode);
     }
   } //end if SubOp            
   else return temp;
}


/*************************************************************************
 * Function Name: FunctionParser::SummandDeriv1
 * Parameters:
 * Returns: FuncNode *
 * Effects: 
 *************************************************************************/
FuncNode *
FunctionParser::SummandDeriv1()
{
  FuncNode * returnVal;
  FuncToken token = currToken;
  if (token == TOKEN_NULL) {function_OK = FALSE;
  returnVal = NULL;}
  else {
    FuncNode * temp = FactorDeriv1();
    returnVal = SummandDeriv2(temp);
  }
  return returnVal; 
}


/*************************************************************************
 * Function Name: FunctionParser::SummandDeriv2
 * Parameters: FuncNode *temp
 * Returns: FuncNode *
 * Effects:
 *************************************************************************/
FuncNode *
FunctionParser::SummandDeriv2(FuncNode *temp)
{
  FuncToken token = currToken;

  if (Match(TOKEN_AND)) {
    /*if (WillMatch(TOKEN_STRING) &&
theFunction->getFunction(nextTokenString)) {
      UnMatch();
      return temp;
    }
    else */{    
      NodeAnd *andNode = new NodeAnd(theFunction);
      andNode->setLeft(temp);
      FuncToken newToken = currToken;
      if (newToken == TOKEN_NULL) {function_OK = FALSE;
      return(NULL);
      }
      FuncNode * newNode = FactorDeriv1();
      if (newNode == NULL) {function_OK= FALSE;
      return(NULL);
      }
      andNode->setRight(newNode);
      return SummandDeriv2(andNode);
    }
  } //end if mulop
  else      if (token == TOKEN_NULL) {function_OK = FALSE;
  return(NULL);
  }
  else
    
    if (!( (Match(TOKEN_EOF)) || (WillMatch(TOKEN_RPAREN))
	   || (WillMatch(TOKEN_SUB_OP))
	   || (WillMatch(TOKEN_AND)) || (WillMatch(TOKEN_OR))
	   ))
      {
	// allow for juxtaposition to be equivalent to mult.
	FuncNode * newTemp = FactorDeriv1();
	if (newTemp == NULL) {function_OK = FALSE;
	return NULL;}
	else
	  {
	    NodeAnd *newNode = new NodeAnd(theFunction);
	    newNode->setLeft(temp);
	    newNode->setRight(newTemp);
	    return SummandDeriv2(newNode);
	  }
      }
    else {return temp;}


}


/*************************************************************************
 * Function Name: FunctionParser::FactorDeriv1
 * Parameters:
 * Returns: FuncNode *
 * Effects: 
 *************************************************************************/
FuncNode *
FunctionParser::FactorDeriv1()
{
  FuncNode * returnVal;
  FuncToken token = currToken;
  if (token == TOKEN_NULL) {function_OK = FALSE;
  returnVal = NULL;}
  else {
    FuncNode * temp = OperandDeriv1();
    returnVal = FactorDeriv2(temp);
  }

  return returnVal;
}


/*************************************************************************
 * Function Name: FunctionParser::FactorDeriv2
 * Parameters: FuncNode *temp
 * Returns: FuncNode *
 * Effects:
 *************************************************************************/
FuncNode *
FunctionParser::FactorDeriv2(FuncNode *temp)
{
  FuncToken token = currToken;

  if (token == TOKEN_NULL) {function_OK = FALSE;
  return(NULL);
  }
  else  if (Match(TOKEN_EOF)) {
    return(temp);
    
  } //end if (eof)

  /*  else  if (Match(TOKEN_POW_OP)) {
    NodePowOp *powNode = new NodePowOp(theFunction);
    powNode->setLeft(temp);
    FuncToken newToken = currToken;
    if (newToken == TOKEN_NULL) {function_OK = FALSE;
    return(NULL);
    }
    FuncNode * newNode = OperandDeriv1();
    if (newNode == NULL) {function_OK= FALSE;
    return(NULL);
    
    }
    powNode->setRight(newNode);
    return FactorDeriv2(powNode);
  } //end if powop
  */
  else {               
    return(temp);
  }
}


/*************************************************************************
 * Function Name: FunctionParser::OperandDeriv1
 * Parameters:
 * Returns: FuncNode *
 * Effects:
 *************************************************************************/
FuncNode *
FunctionParser::OperandDeriv1()
{
  if (Match(TOKEN_NOT) ) {
    NodeNot *temp = new NodeNot(theFunction);
    FuncToken newToken = currToken;
    if (newToken == TOKEN_NULL) {function_OK = FALSE;
    return(NULL);
    }
    temp->setChild(OperandDeriv1());
    return(temp);
  }
  if (Match(TOKEN_OR) ) {
    FuncToken newToken = currToken;
    if (newToken == TOKEN_NULL) {function_OK = FALSE;
    return(NULL);
    }
    return OperandDeriv1();
  }
  else if (Match (TOKEN_SUB_OP )) {
    function_OK = FALSE;
    return(NULL);
  }
    else if (WillMatch(TOKEN_STRING) )
    {
      Match(TOKEN_STRING);
      //if (!strcmp (nextTokenString, "f")) exit (0);
      
	  DTIFilterROI *filter;

	  filter = roi_manager->getROIFilter (tokenString);
	  if (filter) {
		  return new NodeFilterROI(theFunction, filter);
	  }
	  else {
		  function_OK = false;
		  return NULL;
	  }
      //assert (false);
      //return NULL;

      //      DTIFilterTree *lookupFunc;

      //      FunctionConstant *constant;
      
      //      if (constant = constant_manager->getConstant (tokenString)) 
      //	{return new NodeConst (theFunction, tokenString);}
      
    } //end not a function name
  
   else {
     return TerminalDeriv();}
}


/*************************************************************************
F * Function Name: FunctionParser::TerminalDeriv
 * Parameters:
 * Returns: FuncNode *
 * Effects:
 *************************************************************************/
FuncNode *
FunctionParser::TerminalDeriv()
{
  if (Match(TOKEN_LPAREN)) {
    NodeParen *parenNode = new NodeParen(theFunction);
    FuncNode * temp = ScalarExpressionDeriv();  //get the insides of the parens
    parenNode->setChild(temp);
    if (Match(TOKEN_RPAREN)) {
      return parenNode;

    } //end rparen
    else   // lparen without matching right
      {function_OK = FALSE;
      return NULL;}


  } //end LParen

  function_OK = FALSE;
  return NULL;

}


/*************************************************************************
 * Function Name: FunctionParser::scan
 * Parameters: 
 * Returns: FuncToken
 * Effects:
 *************************************************************************/
FuncToken
FunctionParser::scan()
{
  if (inputOffset == inputLength) {return TOKEN_EOF;} //done
  else
    {
      FuncToken returnVal;
 
      currentChar = theInput[inputOffset++]; //get char
                    if (inputOffset == inputLength) {nextChar = '\0';} //eof
                    else {nextChar = theInput[inputOffset];}
                    switch (currentChar) {
		    case '+':returnVal = TOKEN_OR;
		      break;
		    case '-':returnVal = TOKEN_SUB_OP;
		      break;
		    case '(':returnVal = TOKEN_LPAREN;
		      break;
		    case ')':returnVal = TOKEN_RPAREN;
		      break;
		    case '|':
		      if (nextChar == '|') {
			inputOffset++; //do not need the second | again
			returnVal = TOKEN_OR;
		      }
		      else
			returnVal = TOKEN_NULL;
		      break;
		    case '&': if (nextChar == '&') inputOffset++; //allow both & and &&
		      returnVal = TOKEN_AND;
		      break;
		    case ' ':returnVal = scan(); //do it again. skip spaces.
		      break;

		    default: 
		      
		      if ( ((currentChar >= 'A') && (currentChar <= 'Z')) ||
			   ((currentChar >= 'a') && (currentChar <='z')) ||
			   ((currentChar >= '0') && (currentChar <='9')))
			{

			  if  ((((nextChar < 'A') || (nextChar > 'z')) ||
			       ((nextChar > 'Z') && (nextChar < 'a'))) &&
			       (!((nextChar >= '0') && (nextChar <= '9'))))
			    {
			      nextTokenString[0] = currentChar;
			      nextTokenString[1] = 0;

			      returnVal = TOKEN_STRING;
			    } //end if (only one letter)
			  else{
			    //more than one letter
			    strcpy (nextTokenString, "");
			    //			    cerr << "Stringstate()" << endl;
			    returnVal =  stringState(); //call string scanner
			  }
			} //end if (it's a letter)
		      else
			{returnVal = TOKEN_NULL; } //UH OH! Bad character
		      
		      break;
		      
		    } //end switch (currentChar)

		    return(returnVal);

		    
    } //end else (not done yet)
}


/*************************************************************************
 * Function Name: FunctionParser::stringState
 * Parameters: 
 * Returns: FuncToken
 * Effects: 
 *************************************************************************/
FuncToken
FunctionParser::stringState()
{
  /* we know that we have a letter character at offset (nextChar) and in
       currentChar (which is offset-1 now) */
    FuncToken returnVal;
    char temp[5];
    temp[0] = currentChar;
    temp[1] = 0; // Null terminate.
    strcat (nextTokenString, temp); //add the new char
    if  (((nextChar >= 'A') && (nextChar <= 'Z')) ||
	 ((nextChar >= 'a') && (nextChar <= 'z')) ||
	 ((nextChar >= '0') && (nextChar <= '9')))
	 
      { currentChar = nextChar; //move on
      if (inputOffset < (inputLength-1)) {
	nextChar = theInput[++inputOffset];} //get next char
      else {nextChar = '\0';
      inputOffset++;} //end of the string
      returnVal = stringState();
      } // end if (((nextChar
    else {
      if (strcasecmp (nextTokenString, "and") == 0) returnVal = TOKEN_AND;
      else if (strcasecmp (nextTokenString, "or") == 0) returnVal = TOKEN_OR;
      else if (strcasecmp (nextTokenString, "not") == 0) returnVal = TOKEN_NOT;
      
      
      else 
	{
	  returnVal = TOKEN_STRING;
	  
	}
    }
    return returnVal;
} //end stringState
























