/** Copyright (c) 2005, The Board of Trustees of Leland Stanford Junior 
University. All rights reserved. **/

#ifndef DTI_H
#define DTI_H

#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include "io_utils.h"
#include "typedefs_property.h"


#define foreach(type , element, list) 	\
for (type::iterator element = (list).begin(), e = (list).end();  element != e;  ++element ) 

#define SAFE_DELETE(ptr) if(ptr){delete ptr; ptr=0;}

// OS specific stuff is handled in Makefile.definitions
// so that here we can just do what we are told
// Removed conditional to compile on OSX - 8.6.12
//#ifndef _HAS_UINT 
typedef unsigned int uint;
//#endif //_HAS_UINT
#ifndef _HAS_UINT64_T
#ifdef _WIN32
typedef unsigned long long int uint64_t;
#else
#if __x86_64__
typedef unsigned long long int	uint64_t;
#else
__extension__
typedef unsigned long int	uint64_t;
#endif
#endif
#endif

/*
#ifdef _WIN32
typedef unsigned long long int uint64_t;
#else
#if __x86_64__
typedef unsigned long int	uint64_t;
#else
__extension__
typedef unsigned long long int	uint64_t;
#endif

#endif //_HAS_UINT64
*/

// We should come up with a OS specific define
/*
#ifndef _HAS_UINT
typedef unsigned int uint;
typedef unsigned long long int uint64_t;
#endif

#ifdef _WIN32
typedef unsigned int uint;
typedef unsigned long long int uint64_t;
#else

#if __x86_64__
typedef unsigned long int	uint64_t;
#else
__extension__
typedef unsigned long long int	uint64_t;
#endif

#ifndef _HAS_UINT
typedef unsigned int uint;
#endif

#endif
 */

// Can only include this typedefs after the uint has been defined 
#include "typedefs_array.h"
 
static double ZERO_ROTATION[3][3] = {{1.0, 0.0, 0.0},
									 {0.0, 1.0, 0.0},
									 {0.0, 0.0, 1.0}};

static double ZERO_TRANSLATION[] = {0.0, 0.0, 0.0};

enum DTIMCMethod {
  FORWARD_STEP = 0,
  MCMC = 1,
  OLA = 2,
  KEEP = 3,
  SISR = 4
};

enum DTIPathWeight {
  UNIF = 0,
  PRIOR = 1,
  LIKE = 2,
  POST = 3
};

enum NumericalIntegrationMethod {
  INTEGRATION_EULER = 0,
  INTEGRATION_RUNGE_KUTTA_2 = 1,
  INTEGRATION_RUNGE_KUTTA_4 = 2,
  INTEGRATION_NUM_CHOICES = 3
};

enum AllStatisticIDs {
  ALL_STAT_ID_LENGTH = 0,
  ALL_STAT_ID_CURVATURE = 1,
  ALL_STAT_ID_PRIOR = 2,
  ALL_STAT_ID_LIKELIHOOD = 3,
  ALL_STAT_ID_FA = 4,
  ALL_STAT_ID_POSTERIOR = 5,
  ALL_STAT_ID_STD = 6,
  ALL_STAT_ID_ANGLE = 7,
  ALL_STAT_ID_SAMPLEORDER = 8,
  ALL_STAT_ID_ANGLE_INT = 9,
  ALL_STAT_ID_EPDISTANCE = 10,
  ALL_STAT_ID_IWEIGHT = 11,
  ALL_STAT_ID_ENDXPOS = 12,
  ALL_STAT_ID_ENDYPOS = 13,
  ALL_STAT_ID_ENDZPOS = 14,
  ALL_NUM_STAT_IDS = 15
};

enum LiteStatisticIDs {
  LITE_STAT_ID_LENGTH = 0,
  LITE_STAT_ID_PRIOR = 1,
  LITE_STAT_ID_LIKELIHOOD = 2,
  LITE_STAT_ID_POSTERIOR = 3,
  LITE_STAT_ID_CCBWEIGHT = 4,
  LITE_STAT_ID_ABSORPTION = 5,
  LITE_NUM_STAT_IDS = 6
};

const std::string emptyStrOut = "NO";

typedef int PathwayProperty;
typedef int PointProperty;

static const int SWATCHES_MIN = 8;
static const int SWATCHES_MAX = 200;

typedef int DTILuminanceEncoding;

enum DTIQueryOperation 
{
  DTI_FILTER_AND = 0,
  DTI_FILTER_OR = 1,
  DTI_FILTER_CUSTOM = 2
};

enum DTIPathwayColor 
{
  DTI_COLOR_UNASSIGNED = 0,
  DTI_COLOR_GREY = 0,
  DTI_COLOR_BLUE = 1,
  DTI_COLOR_YELLOW = 2,
  DTI_COLOR_GREEN = 3,
  DTI_COLOR_PURPLE = 4,
  DTI_COLOR_ORANGE = 5,
  DTI_COLOR_RED = 6,
  NUM_PATHWAY_COLORS = 7
};
typedef int DTIPathwayGroupID;
typedef int DTIPathwayAlgorithm;
static const int DTI_PATHWAY_ALGORITHM_ANY = -1;

template <class T> void free_vector(std::vector<T*> &v)
{
	for(unsigned int i = 0; i < v.size(); i++)
		delete v[i];
	v.clear();
}


#define TRY_WITH_RETURN(EXPR, MSG) { if(!EXPR){std::cerr<<__FILE__<<":"<<__LINE__<<" "<<MSG<<std::endl; return;} }
#define TRY_WITH_BREAK(EXPR, MSG) { if(!EXPR){std::cerr<<__FILE__<<":"<<__LINE__<<" "<<MSG<<std::endl; break;} }
#define TRY_WITH_CONTINUE(EXPR, MSG) { if(!EXPR){std::cerr<<__FILE__<<":"<<__LINE__<<" "<<MSG<<std::endl; continue;} }
#define TRY_WITH_RETURN_VAL(EXPR, MSG, VAL) { if(!EXPR){std::cerr<<__FILE__<<":"<<__LINE__<<" "<<MSG<<std::endl; return VAL;} }

class RAPID_model;
class DTIFilterROI;
class DTIPathwayDatabase;
class DTIPathDistanceMatrix;
/*	
 shared_ptr is not a part of main STL library in linux
 hence we need two different implementation.
 
 shared_ptr are extensively used to automate memory 
 management. shared_ptr of any class has a suffix P
 for pointer.
 */

#ifdef _WIN32
#	include <memory>
	typedef std::shared_ptr<RAPID_model				> PRAPID_model;
	typedef std::shared_ptr<DTIFilterROI			> PDTIFilterROI;
	typedef std::shared_ptr<DTIPathwayDatabase		> PDTIPathwayDatabase;
	typedef std::shared_ptr<DTIPathDistanceMatrix	> PDTIPathDistanceMatrix;
#	define typeof __typeof__
#	include <io.h>
#	include <direct.h>
#else
#	include <tr1/memory>
	typedef std::tr1::shared_ptr<RAPID_model			> PRAPID_model;
	typedef std::tr1::shared_ptr<DTIFilterROI			> PDTIFilterROI;
	typedef std::tr1::shared_ptr<DTIPathwayDatabase		> PDTIPathwayDatabase;
	typedef std::tr1::shared_ptr<DTIPathDistanceMatrix	> PDTIPathDistanceMatrix;
#	define _access access
#	define _getcwd getcwd
#	define _chdir  chdir
#endif


//! A basic color class, with RGBA components and alpha set to 1 by default
template <class T>
class _tColor : public ISerializable
{ 
public:
	union{ T value[4]; struct{T r,g,b,a; };};
	void Set(T _r, T _g, T _b, T _a=1){r=_r; g=_g; b=_b; a=_a;}
	_tColor<T>(T _r, T _g, T _b, T _a=1){Set(_r,_g,_b,_a);}
	_tColor<T>(){Set(0,0,0,1);}
	void Deserialize(std::istream &s, int version = 0)
	{	s>>"Color: ">>r>>" ">>g>>" ">>b>>" ">>a>>std::endl;	 }
    void Serialize  (std::ostream &s, int version = 0)
	{	s<<"Color: "<<r<<" "<<g<<" "<<b<<" "<<a<<std::endl;  }
};

//! typedefs into different types of colors
typedef _tColor<unsigned char> 	Coloruc;
typedef _tColor<float> 			Colorf;
typedef _tColor<double> 		Colord;

static const Colord PATHWAY_COLORS[] = 
{
  Colord(128/255., 128/255., 128/255.),
  Colord( 20/255.,  90/255., 200/255.), /*		 blue	*/
  Colord(152/255., 	78/255., 163/255.), /*		 purple	*/
  Colord(255/255., 255/255.,  51/255.), /*		 yellow	*/
  Colord( 77/255., 175/255.,  74/255.), /*		 green	*/
  Colord(190/255.,  40/255.,  40/255.), /*		 red	*/
  Colord(255/255., 127/255.,   0/255.), /*		 orange	*/
  Colord(120/255., 180/255., 180/255.), /*		 cyan	*/
  Colord(120/255., 100/255.,  50/255.), /*		 gold	*/

  Colord(144/255., 180/255., 255/255.), /* light blue	*/
  Colord(212/255., 174/255., 218/255.), /* light purple */
  Colord(246/255., 246/255., 176/255.), /* light yellow */
  Colord(173/255., 223/255., 171/255.), /* light green	*/
  Colord(233/255., 155/255., 155/255.), /* light red	*/
  Colord(249/255., 200/255., 156/255.), /* light orange */
  Colord(193/255., 225/255., 225/255.), /* light cyan	*/
  Colord(199/255., 188/255., 162/255.)  /* light gold	*/
};


#endif
