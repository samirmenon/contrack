/*
 *  random.h
 *  anneal
 *
 *  Created by M. A. DiMauro on Tue Jun 18 2002.
 *  Copyright (c) 2001 __MyCompanyName__. All rights reserved.
 *
 */

//#include <stdio.h>
//#include <string>
//#include <cmath>
//#include <conio>
//#include <ctype.h>
//#include <stdl>

//#define IM 714025L      // These constants are from Press et. al.
//#define IA 1366L        // "Numerical Recipes in C"
//#define IC 150889L      // Do not tamper with them unless you are an expert
#define TABLE_LENGTH 97 // Length of shuffle table

static long seed = 797 ;           // Keep the current seed here
static long randout ;              // Random output
static long table[TABLE_LENGTH] ;  // Keep shuffle table here
static int table_initialized = 0 ; // Has it been initialized?

/*
   Set the random seed
*/

void pf_srand ( long iseed )
{
   seed = iseed ;          // We keep the static seed here
   table_initialized = 0 ; // Must also rebuild table with it!
}

/*
   Return the maximum random number
*/

//long pf_randmax ()
//{
//   return IM - 1L ;
//}

/*
   This is the actual random number generator
*/

long pf_rand ()     // Return the next random number
{
   int i ;

   if (! table_initialized) {  // Initialize shuffle table before use
      table_initialized = 1 ;  // Flag to avoid more inits
			srand(seed);
      for (i=0 ; i<TABLE_LENGTH ; i++) { // Fill entire table
         seed = rand();//(IA * seed + IC) % IM ;  // Make a random number
         table[i] = seed ;               // Put it in the table
         }
      seed = rand();//(IA * seed + IC) % IM ;     // One more random number
      randout = seed ;                   // for upcoming first use
      }

   i = (int) ((double) TABLE_LENGTH * (double) randout / (double) RAND_MAX) ;
   randout = table[i] ;            // This output comes from table
   seed = rand();//(IA * seed + IC) % IM ;  // Make new random number
   table[i] = seed ;               // to replace used entry
   return randout ;                // then return old entry
}

