#include <iomanip>
#include "global.h"
#include "wirelength.h"
#include "rsmt.h"


int reportWirelength() {
  int totalWirelength = 0;
  int totalCritWirelength = 0;
  for (auto iter : glbNetMap) {
    Net* net = iter.second;
    if (net->isClock()) {
      continue;
    }

    int netCritWirelength = net->getCritWireLength();
    int netNonCritWirelength = net->getNonCritWireLength();
    totalCritWirelength += netCritWirelength;
    totalWirelength = totalWirelength + netCritWirelength + netNonCritWirelength;        
  }

  double ratio = 100.0 * (double)totalCritWirelength / (double)totalWirelength;
  std::cout << "  Total wirelength = " << totalWirelength << "; Crit = " << totalCritWirelength << " (" << std::setprecision(2) << ratio <<"%)" << std::endl;    
  return 0;
}