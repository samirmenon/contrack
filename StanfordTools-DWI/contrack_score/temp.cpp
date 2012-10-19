/*************************************************************************
 * Function Name: DTIPathwayIO::loadAndAppendDatabaseCamino
 * Parameters: DTIPathwayDatabase* pdb, fstream &theStream, int numPathwaysToLoad
 * Returns: void
 * Effects: 
 *************************************************************************/
void DTIPathwayIO::loadAndAppendDatabaseCamino(DTIPathwayDatabase* pdb, std::ifstream &pdbStream, int numPathwaysToLoad) 
{
  if(pdb==NULL)
    return;

  int numAccepted = 0;
  while ( numPathwaysToLoad<0 || numAccepted<numPathwaysToLoad ){
    // Read a freakin pathway
    DTIPathway *pathway = new DTIPathway((DTIPathwayAlgorithm) 2);
    pathway->initializePathStatistics(0, pdb->getPathStatisticHeaders(), true);
    if(isLittleEndian()) {
      if(loadCaminoPathwayLE(pathway,pdbStream)<0) {
	delete pathway;
	break;
      }
    }else {
      if(loadCaminoPathwayBE(pathway,pdbStream)<0) {
	delete pathway;
	break;
      }
    }
    pathway->setID(numAccepted++);
    pdb->addPathway(pathway);	
  }
  //  std::cerr << "Finished loading pathways." << std::endl;
  return;
}

/*************************************************************************
 * Function Name: DTIPathwayIO::loadCaminoPathwayBE
 * Parameters: ifstream &theStream
 * Returns: DTIPathway*
 * Effects: 
 *************************************************************************/
int DTIPathwayIO::loadCaminoPathwayBE(DTIPathway* pathway, std::ifstream &pdbStream)
{
  float numStats = readFloat(pdbStream);
  if(pdbStream.fail()) 
    return -1; // Must check after we read

  float numPoints = readFloat(pdbStream);

  // Just ignore these numbers for now
  unsigned int ss;
  for( ss=0; ss<numStats; ss++) {
    float stat;
    stat = readFloat(pdbStream);
  }

  //float seedID = readFloat(pdbStream);
  //pathway->setSeedPointIndex(int(floor(seedID)));
  //pathway->setID();
  //pathway->initializePathStatistics(0, pdb->getPathStatisticHeaders(), true);
  unsigned int pp;
  for( pp=0; pp<numPoints; pp++ ) {
    float p[3];
    p[0] = readFloat (pdbStream);
    p[1] = readFloat (pdbStream);
    p[2] = readFloat (pdbStream);
    DTIVector v(3, p);
    pathway->append(v);
  }
  return 0;
}
 
/*************************************************************************
 * Function Name: DTIPathwayIO::loadCaminoPathwayLE
 * Parameters: ifstream &theStream
 * Returns: DTIPathway *
 * Effects: 
 *************************************************************************/
int DTIPathwayIO::loadCaminoPathwayLE(DTIPathway* pathway, std::ifstream &pdbStream)
{
  float numStats = readFloat(pdbStream);
  if(pdbStream.fail()) 
    return -1; // Must check after we read
  byteSwapAny(numStats);

  float numPoints = readFloat(pdbStream);
  byteSwapAny(numPoints);

  // Just ignore these numbers for now
  unsigned int ss;
  for( ss=0; ss<numStats; ss++) {
    float stat;
    stat = readFloat(pdbStream);
    byteSwapAny(stat);
  }

  
  //float seedID = readFloat(pdbStream);
  //byteSwapAny(seedID);
  //pathway->setSeedPointIndex(int(floor(seedID)));
  //pathway->setID(numAccepted);
  //numAccepted++;
  //pathway->initializePathStatistics(0, pdb->getPathStatisticHeaders(), true);
  unsigned int pp;
  for( pp=0; pp<numPoints; pp++ ) {
    float p[3];
    p[0] = readFloat (pdbStream);
    byteSwapAny(p[0]);
    p[1] = readFloat (pdbStream);
    byteSwapAny(p[1]);
    p[2] = readFloat (pdbStream);
    byteSwapAny(p[2]);
    DTIVector v(3, p);
    pathway->append(v);
  }
  return 0;
}




/*************************************************************************
 * Function Name: DTIPathwayIO::saveDatabaseCamino
 * Parameters: fstream &theStream, int numPathwaysToLoad
 * Returns: DTIPathwayDatabase *
 * Effects: 
 *************************************************************************/
void DTIPathwayIO::saveDatabaseCamino(DTIPathwayDatabase *db,std::ofstream &pdbStream) 
{
  std::vector<DTIPathway *>::iterator iter = db->_fiber_pathways.begin();
  if(isLittleEndian()) {
    for ( ; iter!= db->_fiber_pathways.end(); iter++) {
      DTIPathway *pathway = *iter;
      saveCaminoPathwayLE(pathway, pdbStream);
      
    }
  } else {
    for ( ; iter!= db->_fiber_pathways.end(); iter++) {
      DTIPathway *pathway = *iter;
      saveCaminoPathwayBE(pathway, pdbStream);
    }
  }
}


/*************************************************************************
 * Function Name: DTIPathwayIO::saveCaminoPathwayBE
 * Parameters: DTIPathway* pathway, ifstream &theStream
 * Returns: void
 * Effects: 
 *************************************************************************/
void DTIPathwayIO::saveCaminoPathwayBE(DTIPathway* pathway, std::ofstream &pdbStream)
{
  // Write a pathway header
  float numStats = (float)pathway->getNumPathStatistics();
  writeFloat(numStats,pdbStream);
  float numPoints = (float)pathway->getNumPoints();
  writeFloat(numPoints,pdbStream);
      
  // Write pathway stats
  unsigned int ss;
  for( ss=0; ss<pathway->getNumPathStatistics(); ss++) {
    float stat = pathway->getPathStatistic(ss);
    writeFloat(stat,pdbStream);
  }
    
  // Write pathway points
  unsigned int pp;
  for( pp=0; pp<pathway->getNumPoints(); pp++ ) {
    float p[3];
    pathway->getPoint(pp,p);
    writeFloat(p[0],pdbStream);
    writeFloat(p[1],pdbStream);
    writeFloat(p[2],pdbStream);
  }
}
 
/*************************************************************************
 * Function Name: DTIPathwayIO::saveCaminoPathwayLE
 * Parameters: DTIPathway* pathway, ifstream &theStream
 * Returns: void
 * Effects: 
 *************************************************************************/
void DTIPathwayIO::saveCaminoPathwayLE(DTIPathway* pathway, std::ofstream &pdbStream)
{
  // Write a pathway header
  float numStats = (float)pathway->getNumPathStatistics();
  byteSwapAny(numStats);
  writeFloat(numStats,pdbStream);
  float numPoints = (float)pathway->getNumPoints();
  byteSwapAny(numPoints);
  writeFloat(numPoints,pdbStream);
    
  // Write pathway stats
  unsigned int ss;
  for( ss=0; ss<pathway->getNumPathStatistics(); ss++) {
    float stat = pathway->getPathStatistic(ss);
    byteSwapAny(stat);
    writeFloat(stat,pdbStream);
  }
    
  // Write pathway points
  unsigned int pp;
  for( pp=0; pp<pathway->getNumPoints(); pp++ ) {
    float p[3];
    pathway->getPoint(pp,p);
    byteSwapAny(p[0]);
    writeFloat(p[0],pdbStream);
    byteSwapAny(p[1]);
    writeFloat(p[1],pdbStream);
    byteSwapAny(p[2]);
    writeFloat(p[2],pdbStream);
  }	
}
