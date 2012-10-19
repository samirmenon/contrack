#include <stdio.h>
#include "ui_utils.h"
#include <cmath>

ProgressBar::ProgressBar(int n)
{
  _nEntries = n;
  if(_nEntries<0) _nEntries=0;
  if(_nEntries>100) _nEntries = 100;
  _fPercInt = 100.0/_nEntries;
  _fPercNext = 0;
  _strEmpty = "[]";
  _strEmpty.insert(1,_nEntries,' ');
  _strFull  = "[]";
  _strFull.insert(1,_nEntries,'=');
}

void 
ProgressBar::update(float fPerc)
{
  if( fPerc >= _fPercNext) {
    _fPercNext = (int)floor(fPerc/_fPercInt)*_fPercInt + _fPercInt;
    report(fPerc);
  }
}

void 
ProgressBar::report(float fPerc)
{
  if(_strMsg.empty()) {
    string strCurrent;
    if(fPerc < _fPercInt )
      strCurrent = _strEmpty;
    else if(fPerc >= 100) {
      strCurrent = _strFull;
    } else {
      int nCurI = (int)floor(fPerc/100.0*(float)_nEntries);
      strCurrent = _strEmpty;
      strCurrent.replace(1,nCurI,nCurI,'=');
      strCurrent.replace(nCurI+1,1,1,'|');
    }
    printf("%s%3.2f%%\r",strCurrent.c_str(),fPerc);
    fflush(stdout);
  } else {
    printf("%s%3.2f\n",_strMsg.c_str(),fPerc);
    fflush(stdout);
  }
}


