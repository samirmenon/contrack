// Author: dakers
//
// Original source: parametersmap.cpp from mrMesh... (VISTA)

#include "CParametersMap.h"

const char* CParametersMap::pszFloatFormat = "%g";
//const char* CParametersMap::pszFloatFormat = "%f";

#define BINARY_ARRAYS

CParametersMap::CParametersMap()
{
}

CParametersMap::~CParametersMap()
{
	CleanupBinaryParams();
	CleanupStringParams();
	CleanupArrayParams();
}

void CParametersMap::CleanupBinaryParams()
{
	HashMapBinaryParams::iterator it;

	for (it = mapBinary.begin(); it != mapBinary.end(); it++)
	{
		free(it->second);
	}
	mapBinary.clear();
}

void CParametersMap::CleanupArrayParams()
{
	HashMapArrayParams::iterator it;

	for (it = mapArrays.begin(); it != mapArrays.end(); it++)
	{
		delete it->second;
	}
	mapArrays.clear();
}

void CParametersMap::CleanupStringParams()
{
	mapStrings.clear();
}

bool CParametersMap::CreateFromString(char *pBuf, int iBufLen)
{
	wxASSERT(pBuf);

	if (!pBuf)
		return false;

	// variables to hold values during iteration
	int			iBinaryLen;
	wxString	strName;
	wxString	strValue;
	BinaryData	*pData = NULL;
	
	CFloatArray	*pArray = NULL;
	int			nDimensions = 0;
	const int	nMaxDimensions = 3;
	int			iArraySizes[3];
	int			iArrayItem = 0;
	int			arraySize = 0;
	int			nCnt1 = 0;
	
	double* tmpArrayD;
	float*	tmpArrayF;

	CleanupBinaryParams();
	CleanupStringParams();
	CleanupArrayParams();

	strName.Alloc(32);
	strValue.Alloc(128);


	states state = S_SKIP_TO_NAME;

	for (char *pCursor = pBuf; (state != S_FINISH) && (state != S_ERROR) && (pCursor - pBuf < iBufLen);)
	{
		switch (state)
		{
		case S_SKIP_TO_NAME:
			if (wxIsalpha(*pCursor) || (*pCursor == '_'))
			{
					state = S_NAME;
					break;
			}
			if (!*pCursor)		{state = S_FINISH; break;}
			pCursor++;
			break;
		
		case S_NAME:
			if (!*pCursor)		{state = S_ERROR; break;}
			if (wxIsalnum(*pCursor) || (*pCursor == '_'))
			{
				strName += *pCursor;
				pCursor++;
			}
			else
				state = S_SKIP_TO_DATA;
			break;
		
		case S_SKIP_TO_DATA:
			if (!*pCursor)		{state = S_ERROR; break;}
			
			else if (*pCursor == '=')
			{
				state = S_SKIP_TO_STRING;
				strValue = "";
			}
			else if (*pCursor == ':')
			{
				state = S_BINARY_LEN;
				iBinaryLen = 0;
			}
			else if (*pCursor == '[')
			{
				state = S_ARRAY_SIZES;
				nDimensions = 0;
				strValue = "";
			}
			pCursor++;
			break;
		
		case S_SKIP_TO_STRING:
			if (!*pCursor)			{state = S_ERROR; break;}
			if (*pCursor == '\'')	{state = S_QUOTED_STRING; pCursor++; break;}
			if (wxIsalnum(*pCursor) || (*pCursor == '_'))	{state = S_STRING; break;}
			pCursor++;
			break;

		case S_STRING:
			if (!wxIsspace(*pCursor) && (*pCursor != 0))
			{
				strValue += *pCursor;
			}
			else
			{
				strName.MakeLower();
				mapStrings[strName] = strValue;
				state = (*pCursor) ? S_SKIP_TO_NAME : S_FINISH;
				strName = "";
			}
			pCursor++;
			break;

		case S_QUOTED_STRING:
			if ((*pCursor != '\'') && (*pCursor != 0))
			{
				strValue += *pCursor;
			}
			else
			{
				strName.MakeLower();
				mapStrings[strName] = strValue;

				state = (*pCursor) ? S_SKIP_TO_NAME : S_FINISH;
				strName = "";
			}
			pCursor++;
			break;

		case S_BINARY_LEN:
			if (!*pCursor)		{state = S_ERROR; break;}

			strValue = "";
			while (!wxIsdigit(*pCursor))
			{
				pCursor++;
				if (!*pCursor){state = S_ERROR; break;}
			}
			while (wxIsdigit(*pCursor))
			{
				strValue += *pCursor;
				pCursor++;
				if (!*pCursor){state = S_ERROR; break;}
			}

			if (wxSscanf(strValue, "%d", &iBinaryLen) < 1)
			{
				state = S_FINISH;
				break;
			}
			strValue = "";

			if (!*pCursor)		{state = S_ERROR; break;}

			pCursor++;
			state = S_BINARY;
			break;

		case S_BINARY:
			if (!*pCursor)		{state = S_ERROR; break;}

			pData = (BinaryData *)malloc(sizeof(BinaryData)+iBinaryLen-1);
			
			if (!pData)			{state = S_ERROR; break;}
			
			pData->len = iBinaryLen;
			memmove(pData->data, pCursor, iBinaryLen);
			
			strName.MakeLower();
			mapBinary[strName] = pData;

			strName = "";

			pCursor += iBinaryLen;	//WARNING: if pCursor is not char *, but e.g. WORD *, some conversion operations are needed
			state = (*pCursor) ? S_SKIP_TO_NAME : S_FINISH;
			break;
		
		case S_ARRAY_SIZES:
			if (!*pCursor)		{state = S_ERROR; break;}
			
			if (wxIsspace(*pCursor))
			{
				pCursor++;
				break;
			}
			if (wxIsdigit(*pCursor))
			{
				strValue += *pCursor;
			}
			else if (*pCursor == ',' || *pCursor == ']')
			{
				if (nDimensions < nMaxDimensions)
				{
					long lValue;
					strValue.ToLong(&lValue);
					iArraySizes[nDimensions] = lValue;
					nDimensions++;
					strValue = "";
				}
				if (*pCursor == ']')
				{
#ifdef BINARY_ARRAYS
					state = S_SKIP_TO_BINARY_ARRAY;
#else
					state = S_SKIP_TO_ARRAY;
#endif
					
					pArray = new CFloatArray;
					iArrayItem = 0;
					if (!pArray || 
						!(pArray->Create(nDimensions, iArraySizes[0], iArraySizes[1], iArraySizes[2])))
					{
						state = S_ERROR;
						if (pArray)
							delete pArray;
						break;
					}
					mapArrays[strName] = pArray;
					strName = "";
				}
			}

			pCursor++;
			break;

		case S_SKIP_TO_ARRAY:
			if (!*pCursor) 		{state = S_ERROR; break;}

//			if (wxIsdigit(*pCursor) || (*pCursor == '.') || (*pCursor == '-'))
			if (IsCharNumeric(*pCursor))
			{
				state = S_ARRAY_ITEMS;
				break;
			}
			pCursor++;

			break;

		case S_SKIP_TO_BINARY_ARRAY:
			if (!*pCursor) 		{state = S_ERROR; break;}

			if (*pCursor == '\'')
			{
				state = S_ARRAY_BINARY_ITEMS;
			}
			pCursor++;

			break;

		case S_ARRAY_BINARY_ITEMS:
			arraySize = pArray->GetNumberOfItems ();
			tmpArrayD = new double[arraySize];
			memcpy (tmpArrayD, pCursor, arraySize*sizeof(double));
			tmpArrayF = pArray->GetPointer ();

			for (nCnt1=0; nCnt1<arraySize; nCnt1++)
				tmpArrayF[nCnt1] = (float)tmpArrayD[nCnt1];
			delete[] tmpArrayD;
			pCursor += arraySize*sizeof(double)+1;
			strValue = "";
			if (!*pCursor)
			{
				state = S_FINISH;
			}
			else
			{
				state = S_SKIP_TO_NAME;
			}
			break;

		case S_ARRAY_ITEMS:
//			if (wxIsdigit(*pCursor) || (*pCursor == '.') || (*pCursor == '-'))
			if (IsCharNumeric(*pCursor))
			{
				strValue += *pCursor;
				pCursor++;
			}
			else
			{
				double fTemp;
				strValue.ToDouble(&fTemp);
				pArray->SetAtAbsoluteIndex((float)fTemp, iArrayItem);
				strValue = "";
				iArrayItem++;

				if (!*pCursor)
				{
					state = S_FINISH;
					break;
				}
				else if (*pCursor != ';')
				{
					state = S_SKIP_TO_NAME;
				}
				
				pCursor++;
			}
			break;

		case S_FINISH:
			break;
		}
	}

	//Dump("parser", pBuf, pCursor-pBuf);

	//return (state == S_FINISH);
	return (state != S_ERROR);
}

char* CParametersMap::FormatString(int *pLen)
{
	wxASSERT(pLen);

	*pLen = 0;

	wxString	strStringParams;
	FormatStringsMap(strStringParams);

	wxString	strArrayParams;

#ifndef BINARY_ARRAYS
	FormatArraysMap(strArrayParams);
#endif

	HashMapBinaryParams::iterator it2;

	int		iBufLen = 0;
	char	*pOutBuf = NULL;

	// estimate required buffer size
	for (it2 = mapBinary.begin(); it2 != mapBinary.end(); it2++)
	{
		iBufLen += it2->first.Len() + 16 + it2->second->len;	//name : size data\n; size = max 10 chars + 3 (" : ") + 1 ('\n')
	}
	iBufLen += strStringParams.Len() + strArrayParams.Len() + 1; //str1 + str2 + '\0'

#ifdef BINARY_ARRAYS
	int	nBinaryBufLen = 0;
	char *pBinaryArraysBuf = NULL;

	FormatArraysMapBinary(&pBinaryArraysBuf, &nBinaryBufLen);
	iBufLen += nBinaryBufLen;
#endif

	if (iBufLen < 2)
		return NULL;

	// create & fill out buffer

	pOutBuf = new char[iBufLen];

	if (!pOutBuf)
		return NULL;

	int	iPos = 0;
	for (it2 = mapBinary.begin(); it2 != mapBinary.end(); it2++)
	{
		int iBytesProcessed;
		wxSprintf(pOutBuf+iPos, "%s : %d %n", it2->first.GetData(), it2->second->len, &iBytesProcessed);
		memmove(pOutBuf+iPos+iBytesProcessed, it2->second->data, it2->second->len);
		iPos += iBytesProcessed + it2->second->len;
	}
	//*pLen = iPos;

	memmove(pOutBuf+iPos, strStringParams.GetData(), strStringParams.Len());
	iPos += strStringParams.Len();

	memmove(pOutBuf+iPos, strArrayParams.GetData(), strArrayParams.Len());
	iPos += strArrayParams.Len();

#ifdef BINARY_ARRAYS
	if (nBinaryBufLen)
	{
		memmove(pOutBuf+iPos, pBinaryArraysBuf, nBinaryBufLen);
		iPos += nBinaryBufLen;
		delete[] pBinaryArraysBuf;
	}
#endif

	pOutBuf[iPos] = 0;

	*pLen = iPos+1;

	wxASSERT(iPos == iBufLen-1);

	return pOutBuf;

	//<test>
	//int		iLength = 30000000;
	//char	*pBuf	= new char[iLength];
	//
	//*pLen = iLength;

	//for (int i=0; i<iLength; i++)
	//{
	//	pBuf[i] = 'a';//rand()&0xFF;
	//}
	//return pBuf;
	//</test>
}

int	CParametersMap::GetInt(const wxString &strName, int iDefaultValue)
{
	HashMapStringParams::iterator it;

	it = mapStrings.find(strName);
	if (it != mapStrings.end())
	{
		long	iValue;
		if (it->second.ToLong(&iValue))
			return iValue;
		else
		{
			double temp;
			if (it->second.ToDouble(&temp))
				return int(temp+0.5);
		}
	}

	return iDefaultValue;
}

float CParametersMap::GetFloat(const wxString &strName, float fDefaultValue)
{
	HashMapStringParams::iterator it;

	it = mapStrings.find(strName);
	if (it != mapStrings.end())
	{
		double	fValue;
		if (it->second.ToDouble(&fValue))
			return (float)fValue;
	}

	return fDefaultValue;
}

bool CParametersMap::GetInt(const wxString &strName, int *pValue)
{
	HashMapStringParams::iterator it;

	it = mapStrings.find(strName);
	if (it == mapStrings.end())
		return false;

	long	lValue;
	if (it->second.ToLong(&lValue))
	{
		*pValue = lValue;
		return true;
	}

	double fValue;
	if (it->second.ToDouble(&fValue))
	{
		*pValue = int(fValue+0.5);
		return true;
	}

	return false;
}

bool CParametersMap::GetFloat(const wxString &strName, float *pValue)
{
	HashMapStringParams::iterator it;

	it = mapStrings.find(strName);
	if (it == mapStrings.end())
		return false;

	double	fValue;
	if (!it->second.ToDouble(&fValue))
		return false;

	*pValue = (float)fValue;

	return true;
}

bool CParametersMap::GetString(const wxString &strName, wxString *pstrValue)
{
	HashMapStringParams::iterator it;

	it = mapStrings.find(strName);
	if (it == mapStrings.end())
		return false;

	*pstrValue = it->second;

	return true;
}

wxString CParametersMap::GetString(const wxString &strName, const wxString &strDefault)
{
	HashMapStringParams::iterator it;

	it = mapStrings.find(strName);
	if (it != mapStrings.end())
	{
		return it->second;
	}

	return strDefault;
}

CFloatArray* CParametersMap::GetArray(const wxString &strName)
{
	HashMapArrayParams::iterator it;

	it = mapArrays.find(strName);
	if (it != mapArrays.end())
	{
		return it->second;
	}

	return NULL;
}

void	CParametersMap::SetInt(const wxString &strName, int iValue)
{
	mapStrings[strName] = wxString::Format("%d", iValue);
}

void	CParametersMap::SetFloat(const wxString &strName, float fValue)
{
	mapStrings[strName] = wxString::Format(pszFloatFormat, fValue);
}

void	CParametersMap::SetArray(const wxString &strName, CFloatArray *pArray)
{
	// delete old array if exists
	HashMapArrayParams::iterator it;

	it = mapArrays.find(strName);
	if (it != mapArrays.end())
	{
		wxASSERT(it->second);
		if (it->second)
			delete it->second;
	}

	// set new array
	mapArrays[strName] = pArray;
}

void	CParametersMap::SetString(const wxString &strName, const wxString &strValue)
{
	mapStrings[strName] = strValue;
}

void CParametersMap::FormatArraysMap(wxString &strResult)
{
	HashMapArrayParams::iterator it;

	int		iStrLen = 0;

	for (it = mapArrays.begin(); it != mapArrays.end(); it++)
	{
		iStrLen += it->first.Len() + 20 + /*8*/12 * it->second->GetNumberOfItems();
	}

	strResult.Alloc(iStrLen);

	CFloatArray	*pArray;
	int			i;
	static char		pszBuf[32];	// for sprintf("%f")

	for (it = mapArrays.begin(); it != mapArrays.end(); it++)
	{		
		pArray = it->second;
		strResult += it->first + "[";

		for (i=0; i<pArray->GetNumberOfDimensions(); i++)
		{
			strResult += wxString::Format("%d", pArray->GetSizes()[i]);
			if (i < pArray->GetNumberOfDimensions()-1)
				strResult += ',';
			else
				strResult += "] = ";
		}

		for (i=0; i<pArray->GetNumberOfItems(); i++)
		{
			float fValue;
			pArray->GetAtAbsoluteIndex(&fValue, i);

//			strResult += wxString::Format(pszFloatFormat, fValue);
			sprintf(pszBuf, pszFloatFormat, fValue);
			strResult += pszBuf;

			if (i < pArray->GetNumberOfItems()-1)
				strResult += ';';
			else
				strResult += '\n';
		}
	}
}

void CParametersMap::FormatArraysMapBinary(char **pResult, int *pLength)
{
	wxASSERT(pResult);
	wxASSERT(pLength);

	int	iBufLen = 0;

	HashMapArrayParams::iterator it;

	wxString strResult;
	CFloatArray* pArray;

	for (it = mapArrays.begin(); it != mapArrays.end(); it++)
	{
		pArray = it->second;
		strResult = it->first + "[";
		int i;
		for (i=0; i<pArray->GetNumberOfDimensions(); i++)
		{
			strResult += wxString::Format("%d", pArray->GetSizes()[i]);
			if (i < pArray->GetNumberOfDimensions()-1)
				strResult += ',';
			else
				strResult += "] = ";
		}
		iBufLen += strResult.Length() + sizeof(double) * it->second->GetNumberOfItems() + 1 /*'\n'*/ + 2 /*'\''*/;
	}

	*pResult	= NULL;
	*pLength	= iBufLen;

	if (!iBufLen)
		return;

	//iBufLen += 1; //<- for trailing zero

	char* pBuf	= new char[iBufLen];
	if (!pBuf)
	{
		*pLength = 0;
		return;
	}
	*pResult = pBuf;

	int iPos = 0;
	int	nBytes = 0;

	for (it = mapArrays.begin(); it != mapArrays.end(); it++)
	{
		int	nItems = it->second->GetNumberOfItems();

		pArray = it->second;
		strResult = it->first + "[";
		int i;
		for (i=0; i<pArray->GetNumberOfDimensions(); i++)
		{
			strResult += wxString::Format("%d", pArray->GetSizes()[i]);
			if (i < pArray->GetNumberOfDimensions()-1)
				strResult += ',';
			else
				strResult += "] = ";
		}

		sprintf (pBuf + iPos, "%s%n", strResult.GetData(), &nBytes);
		wxASSERT(nBytes == strResult.Length());
		
		iPos += nBytes;

		pBuf[iPos++] = '\'';

		float *pArraySrc = it->second->GetPointer();
		double *pDst = (double*)(pBuf+iPos);
		
		for (int i=0; i<nItems; i++)
			pDst[i] = (double)pArraySrc[i];

		iPos += sizeof(double)*nItems;

		pBuf[iPos++] = '\'';
		pBuf[iPos++] = '\n';
	}
	
	wxASSERT(iPos == iBufLen);
}

void CParametersMap::FormatStringsMap(wxString &strResult)
{
	HashMapStringParams::iterator it;

	char *pszFormat;
	for (it = mapStrings.begin(); it != mapStrings.end(); it++)
	{
		// test whether to quote string or not
		pszFormat = "%s = %s\n";
		for (unsigned iPos = 0; iPos < it->second.Len(); iPos++)
		{
			if (wxIsspace(it->second[iPos]))
			{
				pszFormat = "%s = '%s'\n";
				break;
			}
		}

		strResult += wxString::Format(pszFormat, it->first.GetData(), it->second.GetData());
	}
}

bool CParametersMap::IsCharNumeric(char c)
{
	if (c >= '0' && c <='9')
		return true;

	static char num[]="+-.eE";
	char *pC = num;
	while (*pC)
	{
		if (*pC == c)
			return true;
		pC++;
	}
	return false;
}

void CParametersMap::AppendString(const wxString &strName, const wxString &strValue)
{
	wxString s = GetString(strName, "");
	if (!s.IsEmpty())
		s += ", ";
	SetString(strName, s+strValue);
}
