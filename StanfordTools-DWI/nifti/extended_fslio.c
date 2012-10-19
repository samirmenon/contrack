#include "extended_fslio.h"

#define FSLIOERR(x) { fprintf(stderr,"Error:: %s\n",(x)); fflush(stderr); exit(EXIT_FAILURE); }


/***************************************************************
 * convertBufferToScaledFloat
 ***************************************************************/
/*! \fn int  convertBufferToScaledFloat(float *outbuf, void *inbuf, long len, float slope, float inter, int nifti_datatype )
    \brief allocate a 4D buffer, use 1 contiguous buffer for the data 

        Array is indexed as buf[0..th-1][0..zh-1][0..yh-1][0..xh-1].  
        <br>To access all elements as a vector, use buf[0][0][0][i] where
        i can range from 0 to th*zh*yh*xh - 1.

    \param outbuf pointer to array of floats of size len
    \param inbuf void pointer to an array of len items of datatype nifti_datatype
    \param len number of elements in outbuf and inbuf
    \param slope slope term of scaling to be applied
    \param inter intercept term of scaling to be applied:  out = (in*slope)+inter
    \param nifti_datatype NIFTI datatype code for the datatype of the elements in inbuf
    \return error code: 0=OK -1=error
 */
int convertBufferToScaledFloatWithOffset(float *outbuf, void *inbuf, uint xDimIn, uint yDimIn, uint zDimIn, uint tDimIn, uint uDimIn, uint xOff, uint yOff, uint zOff, uint tOff, uint uOff, uint xDim, uint yDim, uint zDim, uint tDim, uint uDim, float slope, float inter, int nifti_datatype, float *minVal, float *maxVal ) 
{
  int bFirstVal = 1;
  uint uu,tt,zz,yy,xx;

  for(uu=0; uu<uDim; uu++) {
    long uOffIn = (uu+uOff)*xDimIn*yDimIn*zDimIn*tDimIn;
    long uOffOut = uu;

    for(tt=0; tt<tDim; tt++) {
      long tOffIn = (tt+tOff)*xDimIn*yDimIn*zDimIn;
      long tOffOut = tt*uDim;

      for(zz=0; zz<zDim; zz++) {
	long zOffIn = (zz+zOff)*xDimIn*yDimIn;
	long zOffOut = zz*uDim*tDim*xDim*yDim;

	for(yy=0; yy<yDim; yy++) {
	  long yOffIn = (yy+yOff)*xDimIn;
	  long yOffOut = yy*uDim*tDim*xDim;

	  for(xx=0; xx<xDim; xx++) {
	    long iIn  = xx + xOff + yOffIn + zOffIn + tOffIn + uOffIn;
	    long iOut = xx*uDim*tDim + yOffOut + zOffOut + tOffOut + uOffOut;

	    switch(nifti_datatype) {
	    case NIFTI_TYPE_UINT8:
	      outbuf[iOut] = (float) ( *((THIS_UINT8 *)(inbuf)+iIn) * slope + inter);
	      break;
	    case NIFTI_TYPE_INT8:
	      outbuf[iOut] = (float) ( *((THIS_INT8 *)(inbuf)+iIn) * slope + inter);
	      break;
	    case NIFTI_TYPE_UINT16:
	      outbuf[iOut] = (float) ( *((THIS_UINT16 *)(inbuf)+iIn) * slope + inter);
	      break;
	    case NIFTI_TYPE_INT16:
	      outbuf[iOut] = (float) ( *((THIS_INT16 *)(inbuf)+iIn) * slope + inter);
	      break;
	    case NIFTI_TYPE_UINT64:
	      outbuf[iOut] = (float) ( *((THIS_UINT64 *)(inbuf)+iIn) * slope + inter);
	      break;
	    case NIFTI_TYPE_INT64:
	      outbuf[iOut] = (float) ( *((THIS_INT64 *)(inbuf)+iIn) * slope + inter);
	      break;
	    case NIFTI_TYPE_UINT32:
	      outbuf[iOut] = (float) ( *((THIS_UINT32 *)(inbuf)+iIn) * slope + inter);
	      break;
	    case NIFTI_TYPE_INT32:
	      outbuf[iOut] = (float) ( *((THIS_INT32 *)(inbuf)+iIn) * slope + inter);
	      break;
	    case NIFTI_TYPE_FLOAT32:
	      outbuf[iOut] = (float) ( *((THIS_FLOAT32 *)(inbuf)+iIn) * slope + inter);
	      break;
	    case NIFTI_TYPE_FLOAT64:
	      outbuf[iOut] = (float) ( *((THIS_FLOAT64 *)(inbuf)+iIn) * slope + inter);
	      break;
	      
	    case NIFTI_TYPE_FLOAT128:
	    case NIFTI_TYPE_COMPLEX128:
	    case NIFTI_TYPE_COMPLEX256:
	    case NIFTI_TYPE_COMPLEX64:
	    default:
	      fprintf(stderr, "\nWarning, cannot support %s yet.\n",nifti_datatype_string(nifti_datatype));
	      return(-1);
	    }

	    if(bFirstVal==1) {
	      bFirstVal = 0;
	      *minVal = outbuf[iOut];
	      *maxVal = outbuf[iOut];		
	    } else {
	      if(*minVal>outbuf[iOut])
		*minVal = outbuf[iOut];
	      if(*maxVal<outbuf[iOut])
		*maxVal = outbuf[iOut];
	    }
	    
	  }
	}
      }
    }
  }  
  return(0);
}

/***************************************************************
 * convertBufferToScaledFloat
 ***************************************************************/
/*! \fn int  convertBufferToScaledFloat(float *outbuf, void *inbuf, long len, float slope, float inter, int nifti_datatype )
    \brief allocate a 4D buffer, use 1 contiguous buffer for the data 

        Array is indexed as buf[0..th-1][0..zh-1][0..yh-1][0..xh-1].  
        <br>To access all elements as a vector, use buf[0][0][0][i] where
        i can range from 0 to th*zh*yh*xh - 1.

    \param outbuf pointer to array of floats of size len
    \param inbuf void pointer to an array of len items of datatype nifti_datatype
    \param len number of elements in outbuf and inbuf
    \param slope slope term of scaling to be applied
    \param inter intercept term of scaling to be applied:  out = (in*slope)+inter
    \param nifti_datatype NIFTI datatype code for the datatype of the elements in inbuf
    \return error code: 0=OK -1=error
 */
int reverseBufferToScaledFloatWithOffset(float* mem_buf, void *nifti_buf, uint xDimMem, uint yDimMem, uint zDimMem, uint tDimMem, uint uDimMem, uint xOff, uint yOff, uint zOff, uint tOff, uint uOff, uint xDimNifti, uint yDimNifti, uint zDimNifti, uint tDimNifti, uint uDimNifti, float slope, float inter, int nifti_datatype, float *minVal, float *maxVal ) 
{
  int bFirstVal = 1;
  uint uu,tt,zz,yy,xx;

  for(uu=0; uu<uDimNifti; uu++) {
    long uIndexNifti = (uu+uOff)*xDimNifti*yDimNifti*zDimNifti*tDimNifti;
    long uIndexMem = uu;

    for(tt=0; tt<tDimNifti; tt++) {
      long tIndexNifti = (tt+tOff)*xDimNifti*yDimNifti*zDimNifti;
      long tIndexMem = tt*uDimMem;

      for(zz=0; zz<zDimNifti; zz++) {
	long zIndexNifti = (zz+zOff)*xDimNifti*yDimNifti;
	long zIndexMem = zz*uDimMem*tDimMem*xDimMem*yDimMem;

	for(yy=0; yy<yDimNifti; yy++) {
	  long yIndexNifti = (yy+yOff)*xDimNifti;
	  long yIndexMem = yy*uDimMem*tDimMem*xDimMem;

	  for(xx=0; xx<xDimNifti; xx++) {
	    long iNifti  = xx + xOff + yIndexNifti + zIndexNifti + tIndexNifti + uIndexNifti;
	    long iMem = xx*uDimMem*tDimMem + yIndexMem + zIndexMem + tIndexMem + uIndexMem;
	    
	    /* switch(nifti_datatype) { */
/* 	    case NIFTI_TYPE_UINT8: */
/* 	      nifti_buf[iNifti] = (float) ( *((THIS_UINT8 *)(nifti_buf)+iMem) * slope + inter); */
/* 	      break; */
/* 	    case NIFTI_TYPE_INT8: */
/* 	      nifti_buf[iNifti] = (float) ( *((THIS_INT8 *)(nifti_buf)+iMem) * slope + inter); */
/* 	      break; */
/* 	    case NIFTI_TYPE_UINT16: */
/* 	      nifti_buf[iNifti] = (float) ( *((THIS_UINT16 *)(nifti_buf)+iMem) * slope + inter); */
/* 	      break; */
/* 	    case NIFTI_TYPE_INT16: */
/* 	      nifti_buf[iNifti] = (float) ( *((THIS_INT16 *)(nifti_buf)+iMem) * slope + inter); */
/* 	      break; */
/* 	    case NIFTI_TYPE_UINT64: */
/* 	      nifti_buf[iNifti] = (float) ( *((THIS_UINT64 *)(nifti_buf)+iMem) * slope + inter); */
/* 	      break; */
/* 	    case NIFTI_TYPE_INT64: */
/* 	      nifti_buf[iNifti] = (float) ( *((THIS_INT64 *)(nifti_buf)+iMem) * slope + inter); */
/* 	      break; */
/* 	    case NIFTI_TYPE_UINT32: */
/* 	      nifti_buf[iNifti] = (float) ( *((THIS_UINT32 *)(nifti_buf)+iMem) * slope + inter); */
/* 	      break; */
/* 	    case NIFTI_TYPE_INT32: */
/* 	      nifti_buf[iNifti] = (float) ( *((THIS_INT32 *)(nifti_buf)+iMem) * slope + inter); */
/* 	      break; */
/* 	    case NIFTI_TYPE_FLOAT32: */
/* 	      nifti_buf[iNifti] = (float) ( *((THIS_FLOAT32 *)(nifti_buf)+iMem) * slope + inter); */
/* 	      break; */
/* 	    case NIFTI_TYPE_FLOAT64: */
/* 	      nifti_buf[iNifti] = (float) ( *((THIS_FLOAT64 *)(nifti_buf)+iMem) * slope + inter); */
/* 	      break; */

	    switch(nifti_datatype) { 
	    case NIFTI_TYPE_FLOAT32: 
 	      *((THIS_FLOAT32 *)(nifti_buf)+iNifti) = (mem_buf[iMem] - inter) / slope;
 	      break;
	    case NIFTI_TYPE_FLOAT128:
	    case NIFTI_TYPE_COMPLEX128:
	    case NIFTI_TYPE_COMPLEX256:
	    case NIFTI_TYPE_COMPLEX64:
	    default:
	      fprintf(stderr, "\nWarning, cannot support %s yet.\n",nifti_datatype_string(nifti_datatype));
	      return(-1);
	    }

	    if(bFirstVal==1) {
	      bFirstVal = 0;
	      *minVal = mem_buf[iMem];
	      *maxVal = mem_buf[iMem];
	    } else {
	      if(*minVal>mem_buf[iMem])
		*minVal = mem_buf[iMem];
	      if(*maxVal<mem_buf[iMem])
		*maxVal = mem_buf[iMem];
	    }
	    
	  }
	}
      }
    }
  }  
  return(0);
}

/***************************************************************
 * d5matrix
 ***************************************************************/
double *****d5matrix(int uh, int th, int zh,  int yh, int xh)
/* allocate a double 5matrix with range t[0..uh][0..th][0..zh][0..yh][0..xh] */
/* adaptation of Numerical Recipes in C nrutil.c allocation routines */
{

	int j;
	int nvol = uh+1;
	int nt = th+1;
	int nslice = zh+1;
	int nrow = yh+1;
	int ncol = xh+1;
        double *****t;


	/** allocate pointers to vols */
        t=(double *****) malloc((size_t)((nvol)*sizeof(double****)));
        if (!t) FSLIOERR("d5matrix: allocation failure");

	/** allocate pointers to times */
	t[0]=(double ****) malloc ((size_t)((nvol*nt)*sizeof (double***)));
	if (!t[0]) FSLIOERR("d5matrix: allocation failure");

	/** allocate pointers to slices */
        t[0][0]=(double ***) malloc((size_t)((nvol*nt*nslice)*sizeof(double**)));
        if (!t[0][0]) FSLIOERR("d5matrix: allocation failure");

	/** allocate pointers for ydim */
        t[0][0][0]=(double **) malloc((size_t)((nvol*nt*nslice*nrow)*sizeof(double*)));
        if (!t[0][0][0]) FSLIOERR("d5matrix: allocation failure");


	/** allocate the data blob */
        t[0][0][0][0]=(double *) malloc((size_t)((nvol*nt*nslice*nrow*ncol)*sizeof(double)));
        if (!t[0][0][0][0]) FSLIOERR("d5matrix: allocation failure");


	/** point everything to the data blob */
        for(j=1;j<nrow*nslice*nt*nvol;j++) t[0][0][0][j]=t[0][0][0][j-1]+ncol;
        for(j=1;j<nslice*nt*nvol;j++) t[0][0][j]=t[0][0][j-1]+nrow;
	for(j=1;j<nt*nvol;j++) t[0][j]=t[0][j-1]+nslice;
        for(j=1;j<nvol;j++) t[j]=t[j-1]+nt;

        return t;
}


/***************************************************************
 * FslGetBufferAsScaledDouble_5D
 ***************************************************************/
/*! \fn double **** FslGetBufferAsScaledDouble(FSLIO *fslio)
    \brief Return the fslio data buffer of a 1-4D dataset as a 4D array of 
        scaled doubles. This function is modified from the original 4D version
	(FslGetBufferAsScaledDouble) in fslio.c

        Array is indexed as buf[0..sdim-1][0..tdim-1][0..zdim-1][0..ydim-1][0..xdim-1].  
        <br>The array will be byteswapped to native-endian.
        <br>Array values are scaled as per fslio header slope and intercept fields.

    \param fslio pointer to open dataset
    \return Pointer to 5D double array, NULL on error
 */

double *****FslGetBufferAsScaledDouble_5D(FSLIO *fslio)
{
  double *****newbuf;
  int xx,yy,zz,tt,uu;
  int ret;
  float inter, slope;
  uint nSize;

  if (fslio==NULL)  FSLIOERR("FslGetBufferAsScaledDouble: Null pointer passed for FSLIO");

  if ((fslio->niftiptr->dim[0] <= 0) || (fslio->niftiptr->dim[0] > 5))
	FSLIOERR("FslGetBufferAsScaledDouble: Incorrect dataset dimension, 1-5D needed");

  /***** nifti dataset */
  if (fslio->niftiptr!=NULL) {
	xx = (fslio->niftiptr->nx == 0 ? 1 : (long)fslio->niftiptr->nx);
	yy = (fslio->niftiptr->ny == 0 ? 1 : (long)fslio->niftiptr->ny);
	zz = (fslio->niftiptr->nz == 0 ? 1 : (long)fslio->niftiptr->nz);
	tt = (fslio->niftiptr->nt == 0 ? 1 : (long)fslio->niftiptr->nt);
	uu = (fslio->niftiptr->nu == 0 ? 1 : (long)fslio->niftiptr->nu);

	if (fslio->niftiptr->scl_slope == 0) {
		slope = 1.0;
		inter = 0.0;
	}
	else {
		slope = fslio->niftiptr->scl_slope;
		inter = fslio->niftiptr->scl_inter;
	}
	
    /** allocate new 5D buffer */
	nSize = uu*tt*zz*yy*xx*8;
	fprintf(stderr,"Attempting to allocate %u B.\n",nSize);
    newbuf = d5matrix(uu-1,tt-1,zz-1,yy-1,xx-1);

    /** cvt it */
    ret = convertBufferToScaledDouble(newbuf[0][0][0][0], fslio->niftiptr->data, (long)(xx*yy*zz*tt*uu), slope, inter, fslio->niftiptr->datatype);

    if (ret == 0)
        return(newbuf);
    else
        return(NULL);

  } /* nifti data */

  if (fslio->mincptr!=NULL) {
    fprintf(stderr,"Warning:: Minc is not yet supported\n");
  }

  return(NULL);
}


/***************************************************************
 * s4matrix
 ***************************************************************/
short ****s4matrix(int th, int zh,  int yh, int xh)
/* allocate a short 4matrix with range t[0..th][0..zh][0..yh][0..xh] */
/* adaptation of Numerical Recipes in C nrutil.c allocation routines */
{

	int j;
	int nvol = th+1;
	int nslice = zh+1;
	int nrow = yh+1;
	int ncol = xh+1;
        short ****t;


	/** allocate pointers to vols */
        t=(short ****) malloc((size_t)((nvol)*sizeof(short***)));
        if (!t) FSLIOERR("d4matrix: allocation failure");

	/** allocate pointers to slices */
        t[0]=(short ***) malloc((size_t)((nvol*nslice)*sizeof(short**)));
        if (!t[0]) FSLIOERR("d4matrix: allocation failure");

	/** allocate pointers for ydim */
        t[0][0]=(short **) malloc((size_t)((nvol*nslice*nrow)*sizeof(short*)));
        if (!t[0][0]) FSLIOERR("d4matrix: allocation failure");


	/** allocate the data blob */
        t[0][0][0]=(short *) malloc((size_t)((nvol*nslice*nrow*ncol)*sizeof(short)));
        if (!t[0][0][0]) FSLIOERR("d4matrix: allocation failure");


	/** point everything to the data blob */
        for(j=1;j<nrow*nslice*nvol;j++) t[0][0][j]=t[0][0][j-1]+ncol;
        for(j=1;j<nslice*nvol;j++) t[0][j]=t[0][j-1]+nrow;
        for(j=1;j<nvol;j++) t[j]=t[j-1]+nslice;

        return t;
}

/***************************************************************
 * FslGetBufferAsScaledShort
 ***************************************************************/
/*! \fn short **** FslGetBufferAsScaledDouble(FSLIO *fslio)
    \brief Return the fslio data buffer of a 1-4D dataset as a 4D array of 
	scaled shorts. 

	Array is indexed as buf[0..tdim-1][0..zdim-1][0..ydim-1][0..xdim-1].  
	<br>The array will be byteswapped to native-endian.
	<br>Array values are scaled as per fslio header slope and intercept fields.

    \param fslio pointer to open dataset
    \return Pointer to 4D short array
 */
short ****FslGetBufferAsScaledShort(FSLIO *fslio)
{
  short ****newbuf;
  int xx,yy,zz,tt;
  int i,j,k,l,m;
  float inter, slope;

  if (fslio==NULL)  FSLIOERR("FslGetBufferAsScaledShort: Null pointer passed for FSLIO");

  if ((fslio->niftiptr->dim[0] <= 0) || (fslio->niftiptr->dim[0] > 4))
	FSLIOERR("FslGetBufferAsScaledShort: Incorrect dataset dimension, 1-4D needed");

  /***** nifti dataset */
  if (fslio->niftiptr!=NULL) {
	xx = (fslio->niftiptr->nx == 0 ? 1 : (long)fslio->niftiptr->nx);
	yy = (fslio->niftiptr->ny == 0 ? 1 : (long)fslio->niftiptr->ny);
	zz = (fslio->niftiptr->nz == 0 ? 1 : (long)fslio->niftiptr->nz);
	tt = (fslio->niftiptr->nt == 0 ? 1 : (long)fslio->niftiptr->nt);

	if (fslio->niftiptr->scl_slope == 0) {
		slope = 1.0;
		inter = 0.0;
	}
	else {
		slope = fslio->niftiptr->scl_slope;
		inter = fslio->niftiptr->scl_inter;
	}
	
    /** allocate new 4D buffer */
    newbuf = s4matrix(tt-1,zz-1,yy-1,xx-1);

    /** fill the buffer */
    for (l=0,m=0;l<tt;l++)
    for (k=0;k<zz;k++)
    for (j=0;j<yy;j++)
    for (i=0;i<xx;i++,m++)
        switch(fslio->niftiptr->datatype) {
	    case NIFTI_TYPE_UINT8:
		newbuf[l][k][j][i] = (short) ( *((THIS_UINT8 *)(fslio->niftiptr->data)+m) * slope + inter);
		break;
	    case NIFTI_TYPE_INT8:
		newbuf[l][k][j][i] = (short) ( *((THIS_INT8 *)(fslio->niftiptr->data)+m) * slope + inter);
		break;
	    case NIFTI_TYPE_UINT16:
		newbuf[l][k][j][i] = (short) ( *((THIS_UINT16 *)(fslio->niftiptr->data)+m) * slope + inter);
		break;
	    case NIFTI_TYPE_INT16:
		newbuf[l][k][j][i] = (short) ( *((THIS_INT16 *)(fslio->niftiptr->data)+m) * slope + inter);
		break;
	    case NIFTI_TYPE_UINT64:
		newbuf[l][k][j][i] = (short) ( *((THIS_UINT64 *)(fslio->niftiptr->data)+m) * slope + inter);
		break;
	    case NIFTI_TYPE_INT64:
		newbuf[l][k][j][i] = (short) ( *((THIS_INT64 *)(fslio->niftiptr->data)+m) * slope + inter);
		break;
	    case NIFTI_TYPE_UINT32:
		newbuf[l][k][j][i] = (short) ( *((THIS_UINT32 *)(fslio->niftiptr->data)+m) * slope + inter);
		break;
	    case NIFTI_TYPE_INT32:
		newbuf[l][k][j][i] = (short) ( *((THIS_INT32 *)(fslio->niftiptr->data)+m) * slope + inter);
		break;
	    case NIFTI_TYPE_FLOAT32:
		newbuf[l][k][j][i] = (short) ( *((THIS_FLOAT32 *)(fslio->niftiptr->data)+m) * slope + inter);
		break;
	    case NIFTI_TYPE_FLOAT64:
		newbuf[l][k][j][i] = (short) ( *((THIS_FLOAT64 *)(fslio->niftiptr->data)+m) * slope + inter);
		break;

	    case NIFTI_TYPE_FLOAT128:
	    case NIFTI_TYPE_COMPLEX128:
	    case NIFTI_TYPE_COMPLEX256:
	    case NIFTI_TYPE_COMPLEX64:
	    default:
		fprintf(stderr, "\nWarning, cannot support %s yet.\n",nifti_datatype_string(fslio->niftiptr->datatype));
		newbuf = NULL;
		break;
        }

  return(newbuf);
  } /* nifti data */


  if (fslio->mincptr!=NULL) {
    fprintf(stderr,"Warning:: Minc is not yet supported\n");
  }

  return(NULL);
}


