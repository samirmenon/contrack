#ifndef UI_UTILS_H
#define UI_UTILS_H

#include <string>
#ifdef USE_MPI
#include <mpi.h>
#endif

using namespace std;
#ifdef _WIN32
#include <time.h>
#endif

class ProgressBar
{
public:
  ProgressBar(int n=40);
  void update(float fPerc);
  void report(float fPerc);
  void setMsg(const string m){_strMsg=m;}

  float _fPercInt;
  int _nEntries;
  string _strMsg;
  string _strEmpty;
  string _strFull;
  float _fPercNext;  
};

class Timer
{
 public:

  void start() 
  {
#ifdef USE_MPI
    _startT = MPI_Wtime ();
#else
    time (&_startT);
#endif
  }
  
  float elapsed()
  {
    float diffT;
#ifdef USE_MPI
    _endT = MPI_Wtime ();
    diffT =  _endT - _startT;
#else
    time (&_endT);
    diffT = difftime(_endT,_startT);
#endif
    return diffT;
  }
 private:
#ifdef USE_MPI
  double _startT, _endT;
#else
  time_t _startT,_endT;
#endif
};

#endif
