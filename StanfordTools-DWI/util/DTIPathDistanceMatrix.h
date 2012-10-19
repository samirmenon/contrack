#ifndef PATH_DISTANCE_MATRIX
#define PATH_DISTANCE_MATRIX

#include <fstream>

class DTIPathDistanceMatrix {

 public: 
  DTIPathDistanceMatrix(int numPathways);
  virtual ~DTIPathDistanceMatrix();

  float GetDistance (int path1Index, int path2Index);
  void SetDistance (int path1Index, int path2Index, float value);
  
  int GetNumPathways() { return _num_pathways; }

  static DTIPathDistanceMatrix *loadDistanceMatrix (std::istream &theStream);

 private:
  float *_distances;
  int _num_pathways;

};

#endif
