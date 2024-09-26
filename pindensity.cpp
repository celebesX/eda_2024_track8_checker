#include <iomanip>
#include "global.h"
#include "object.h"
#include "pindensity.h"

bool reportPinDensity() {  
  int checkedTileCnt = 0;
  std::multimap<double, Tile*> pinDensityMap;
  for (int i = 0; i < chip.getNumCol(); i++) {
      for (int j = 0; j < chip.getNumRow(); j++) {
          Tile* tile = chip.getTile(i, j);
          if (tile->matchType("PLB") == false) {
              continue;        
          }
          if (tile->isEmpty()) {
            continue;
          }

          int numInterTileConn = tile->getConnectedLutSeqInput().size() + tile->getConnectedLutSeqOutput().size();          
          double ratio = (double)(numInterTileConn) / (MAX_TILE_PIN_INPUT_COUNT + MAX_TILE_PIN_OUTPUT_COUNT);
          pinDensityMap.insert(std::pair<double, Tile*>(ratio, tile));            
          checkedTileCnt++;
      }
  }
  
  std::cout << "  Checked pin density on " << checkedTileCnt <<" tiles." << std::endl;
  //std::cout << "    List of top 5% congested tiles:" << std::endl;
  int top5Pct = checkedTileCnt * 0.05;
  int top5PctCnt = 0;
  double totalPct = 0.0;

  // print some statistics in table
  
  std::cout << "  List of Congested Tiles" << std::endl;  
  std::cout << "  " << lineBreaker << std::endl;

  std::cout << "  Location | Input  | Output | Pin Density %" << std::endl;
  const int printCnt = 10;  

  for (auto it = pinDensityMap.rbegin(); it != pinDensityMap.rend(); it++) {
      Tile* tile = it->second;
      double ratio = it->first * 100.0;                
      // convert ratio to percentage        
      if (top5PctCnt < top5Pct) {         

          totalPct += ratio;
          top5PctCnt++;

          if (top5PctCnt < printCnt) {
            std::set<int> inPinSet = tile->getConnectedLutSeqInput();
            std::set<int> outPinSet = tile->getConnectedLutSeqOutput();
            std::string locStr = tile->getLocStr();
            std::cout << "  " << std::left << std::setw(8) << locStr << " ";
            std::cout << "| " << std::left << std::setw(2) << inPinSet.size() << "/" << (int)MAX_TILE_PIN_INPUT_COUNT <<"  ";
            std::cout << "| " << std::left << std::setw(2) << outPinSet.size() << "/" << (int)MAX_TILE_PIN_OUTPUT_COUNT<<"  ";
            std::cout << "| " << std::left << std::setw(4) << ratio << "%" << std::endl;            
          } else if (top5PctCnt == printCnt) {
            std::cout << "  ..." << std::endl;
            std::cout << "  " << lineBreaker << std::endl;
          }
      } else {
          break;
      }
  }
  double avgPct = totalPct / top5Pct;
  std::cout << "  Average pin density of top 5% congested tiles(" << top5Pct << " tiles): " << std::setprecision(2) << avgPct << "%" << std::endl;
    
  return true;      
}