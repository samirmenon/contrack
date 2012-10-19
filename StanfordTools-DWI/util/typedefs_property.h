/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef TYPEDEFS_PROPERTY_H
#define TYPEDEFS_PROPERTY_H

/*! This macro style of variable declaration is inspired from the concep of properties
 in C#. A property generally consists of a protected member variable, a get method
 and a set method to set its value. Get/Set methods are also called accessor/mutator.
 
 Properties can be simple like name of an object or it may involve complex operations
 hence the macros comes in several flavors. 
 For simple properties in which the accessor returns the property and mutator sets the 
 property, use PROPERTY macro
 To define a read only property with no mutator use the PROPERY_READONLY macro.
 To define a property with a custom mutator use PROPERTY_DECLARE which simply declares
 the mutator but doesn't define it.
 */
//! Declares a protected variable of a given type and name
#define DECLARE_PROPERTY_NAME(TYPE, VARIABLE_NAME)\
protected:\
TYPE VARIABLE_NAME;\
public:\

//! Same as above + accessor function. Accessors functions don't have a 'Get' prefix
#define PROPERTY_READONLY(TYPE, VARIABLE_NAME, FUNCTION_NAME)\
DECLARE_PROPERTY_NAME(TYPE, VARIABLE_NAME)\
TYPE&  FUNCTION_NAME(){return VARIABLE_NAME;}

//! Same as above + mutator function. Mutator have a 'Set' prefix
#define PROPERTY(TYPE, VARIABLE_NAME, FUNCTION_NAME)\
DECLARE_PROPERTY_NAME(TYPE, VARIABLE_NAME)\
void   Set##FUNCTION_NAME( TYPE &v){VARIABLE_NAME = v;}\
TYPE&  FUNCTION_NAME(){return VARIABLE_NAME;}

//! Same as above but the mutators are not defined. Useful when set function has complex rules
#define PROPERTY_DECLARE(TYPE, VARIABLE_NAME, FUNCTION_NAME)\
DECLARE_PROPERTY_NAME(TYPE, VARIABLE_NAME)\
void   Set##FUNCTION_NAME( TYPE &v);\
TYPE&  FUNCTION_NAME() { return VARIABLE_NAME; }

//! No mutator here
#define PROPERTY_READONLY_DECLARE(TYPE, VARIABLE_NAME, FUNCTION_NAME)\
DECLARE_PROPERTY_NAME(TYPE, VARIABLE_NAME)\
TYPE&  FUNCTION_NAME();


#endif
