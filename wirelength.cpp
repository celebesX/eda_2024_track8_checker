#include <iomanip>
#include "global.h"
#include "wirelength.h"
#include "rsmt.h"


int reportWirelength() {
  int totalWirelengthBaseline = 0;
  int totalCritWirelengthBaseline = 0;
  int totalNonCritWirelengthBaseline = 0;

  int totalWirelengthOptimized = 0;
  int totalCritWirelengthOptimized = 0;
  int totalNonCritWirelengthOptimized = 0;
  
  getOptimizedWirelength(totalCritWirelengthOptimized, totalNonCritWirelengthOptimized);
  getBaselineWirelength(totalCritWirelengthBaseline, totalNonCritWirelengthBaseline);

  // append critical wirelength to total wirelength
  totalWirelengthBaseline = totalCritWirelengthBaseline + totalNonCritWirelengthBaseline;
  totalWirelengthOptimized = totalCritWirelengthOptimized + totalNonCritWirelengthOptimized;

  double ratioBaseline = 100.0 * (double)totalCritWirelengthBaseline   / (double)totalWirelengthBaseline;
  double ratioOptimized = 100.0 * (double)totalCritWirelengthOptimized / (double)totalWirelengthOptimized;
  std::cout << "  Baseline wirelength: total = " << totalWirelengthBaseline << "; crit = " << totalCritWirelengthBaseline << " (" << std::setprecision(2) << ratioBaseline <<"%)" << std::endl;    
  std::cout << "  Optimized wirelength: total  = " << totalWirelengthOptimized << "; crit = " << totalCritWirelengthOptimized << " (" << std::setprecision(2) << ratioOptimized <<"%)" << std::endl;    
  
  double enhanceCritRatio = 100.0 * (((double)totalCritWirelengthOptimized / (double)totalCritWirelengthBaseline) - 1.0);
  double enhanceNonCritRatio = 100.0 * (((double)totalNonCritWirelengthOptimized / (double)totalNonCritWirelengthBaseline) - 1.0);
  std::cout << "  Critical length change = " << std::setprecision(2) << enhanceCritRatio << "%" << std::endl;
  std::cout << "  Non-Critical length change = " << std::setprecision(2) << enhanceNonCritRatio << "%" << std::endl;
  return 0;
}

bool getOptimizedWirelength(int& critWireLength, int& nonCritWireLength) {
  critWireLength = 0;
  nonCritWireLength = 0;

  for (auto iter : glbNetMap) {
    Net* net = iter.second;
    if (net->isClock()) {
      continue;
    }
    critWireLength += net->getCritWireLength(true);   // isBaseline = true
    nonCritWireLength += net->getNonCritWireLength(true);    
  }

  return true;
}

bool getBaselineWirelength(int& critWireLength, int& nonCritWireLength) {
  critWireLength = 0;
  nonCritWireLength = 0;

  for (auto iter : glbNetMap) {
    Net* net = iter.second;
    if (net->isClock()) {
      continue;
    }
    critWireLength += net->getCritWireLength(false);   // isBaseline = false
    nonCritWireLength += net->getNonCritWireLength(false);    
  }

  return true;
}