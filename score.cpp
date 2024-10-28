#include <iomanip>
#include <cmath>
#include "global.h"
#include "wirelength.h"
#include "pindensity.h"
#include "rsmt.h"
#include "score.h"


bool reportScore(const int bestCritWireLength, const int bestNonCritWireLength) {
  if (bestCritWireLength <=0 || bestNonCritWireLength <=0 ) {        
    std::cout << "Error: input wirelength should be positive numbers; crit_len = " << bestCritWireLength << ", non_crit_len = " << bestNonCritWireLength << std::endl;
    return false;
  }

  int critWireLength = 0; 
  int nonCritWireLength = 0;
  getOptimizedWirelength(critWireLength, nonCritWireLength);

  if (critWireLength < bestCritWireLength || nonCritWireLength < bestNonCritWireLength) {
    std::cout << "Error: invalid best wirelength input, current wirelength is shorter than the best." << std::endl;
    return false;
  }  


  int top5Pct = 0;
  double baselinePinDensity = 0.0;
  double optimizedPinDensity = 0.0;
  getPinDensity(top5Pct, baselinePinDensity, optimizedPinDensity);

  double critLenRatio = (double)critWireLength/(double)bestCritWireLength;
  double nonCritLenRatio = (double)nonCritWireLength/(double)bestNonCritWireLength;
  double lenScore = 100.0 -  100.0 * (critLenRatio - 1.0) - 50.0 * (nonCritLenRatio - 1.0);
  if (lenScore < 0.0) {
    lenScore = 0.0;
  }

  double qScore = lenScore; 
  
  double pinDensityRatio = 1.0;  
  double q = 1.0;

  if (optimizedPinDensity > baselinePinDensity) {
    pinDensityRatio = baselinePinDensity/optimizedPinDensity;
    const double eulerConst = std::exp(1.0);
    q = pow(pinDensityRatio, 2*eulerConst);
    qScore = q * lenScore;
  }  

  std::cout<<"  Case socre:" << std::endl;
  std::cout<<"    Critical wirelength = "<< critWireLength <<", best = " << bestCritWireLength << ", ratio = " << critLenRatio << std::endl;
  std::cout<<"    Non-Critical wirelength = "<< nonCritWireLength <<", best = " << bestNonCritWireLength << ", ratio = " << nonCritLenRatio << std::endl;
  std::cout<<"    Length score = "<< lenScore << std::endl;
  std::cout<<"    Pin density = "<< optimizedPinDensity <<", baseline = " << baselinePinDensity << ", ratio = " << pinDensityRatio << std::endl;
  std::cout<<"    Density coefficient = "<< q  << std::endl;
  std::cout<<"    Case score = "<< qScore << std::endl;
  return true;
}