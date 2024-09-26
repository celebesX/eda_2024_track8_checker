#include <iomanip>
#include "legal.h"
#include "global.h"
#include "object.h"

bool legalCheck() {
  for (int i = 0; i < chip.getNumCol(); i++) {
    for (int j = 0; j < chip.getNumRow(); j++) {
      Tile* tile = chip.getTile(i, j);
      tile->clearInstances();
    }
  }
 
  int numErrors = 0;

  std::cout << "  1.1 Check instance location and tile capacity." << std::endl;
  if (checkTypeAndCapacity() == false) {
    numErrors++;
  }

  std::cout << "  1.2 Check control set constraint." << std::endl;
  if (checkControlSet() == false) {
    numErrors++;
  }

  std::cout << "  1.3 Check clock region constraint." << std::endl;
  if (checkClockRegion() == false) {
    numErrors++;
  }

  if (numErrors > 0) {
    std::cout << "  LegalCheck failed with " << numErrors << " errors." << std::endl;
    return false;
  } else {
    std::cout << "  Legalization check passed." << std::endl;
    return true;
  }
}

bool checkTypeAndCapacity() {
  int errorCount = 0;

  for (auto inst : glbInstMap) {
    std::tuple<int, int, int> loc = inst.second->getLocation();
    Tile* tile = chip.getTile(std::get<0>(loc), std::get<1>(loc));
    if (tile->addInstance(inst.first, std::get<2>(loc), inst.second->getModelName()) == false)  {
      errorCount++;
    }        
  }

  // check tile capacity
  int overflowTileCount = 0;
  for (int i = 0; i < chip.getNumCol(); i++) {
    for (int j = 0; j < chip.getNumRow(); j++) {

      Tile* tile = chip.getTile(i, j);            
      
      std::list<std::pair<std::string, int> > overflow;
      for (auto mapIter = tile->getInstanceMapBegin(); mapIter != tile->getInstanceMapEnd(); mapIter++) {
        std::string modelType = mapIter->first;        
        slotArr slots = mapIter->second;
        for (int idx = 0; idx < (int)slots.size(); idx++) {
          Slot* slot = slots[idx];
          if (slot == nullptr) {
            continue;
          }
          // check if the slot is legally occupied
          std::list<int> instances = slot->getInstances();                    
          if (instances.size() > 1) {
            // 1) 2-LUTs are allowed but total number of input should not exceed 6
            if (modelType == "LUT") {
              if (instances.size() > 2) {
                overflow.push_back(std::pair<std::string, int>(modelType, idx));                        
              } else {
                std::set<int> totalInputs;
                for (auto instID : instances) {
                  Instance* instPtr = glbInstMap.find(instID)->second;
                  std::vector<Pin*> inpins = instPtr->getInpins();
                  for (auto pin : inpins) {
                    if (pin->getNetID() != -1) {
                      totalInputs.insert(pin->getNetID());
                    }
                  }                                    
                }
                if (totalInputs.size() > 6) {
                  overflow.push_back(std::pair<std::string, int>(modelType, idx));
                }
              }
            } else {
              overflow.push_back(std::pair<std::string, int>(modelType, idx));                        
            }
          } else {
            // check DRAM and lut
            if (modelType == "DRAM") {
              if (slot->getInstances().empty()) {
                continue;
              }
              // DRAM at slot0 blocks lut slot 0~3
              // DRAM at slot1 blocks lut slot 4~7
              slotArr* lutSlotArr = tile->getInstanceByType("LUT");
              if (idx == 0) {
                for (int lutIdx = 0; lutIdx < 4; lutIdx++) {
                  Slot* lutSlot = (*lutSlotArr)[lutIdx];
                  if (lutSlot->getInstances().size() > 0) {
                    overflow.push_back(std::pair<std::string, int>("LUT-DRAM", lutIdx));
                  }
                }
              } else if (idx == 1) {
                for (int lutIdx = 4; lutIdx < 8; lutIdx++) {
                  Slot* lutSlot = (*lutSlotArr)[lutIdx];
                  if (lutSlot->getInstances().size() > 0) {
                    overflow.push_back(std::pair<std::string, int>("LUT-DRAM", lutIdx));
                  }
                }
              } else {
                // dram with invalid slot index
              }
            }
          }
        }  // end for each slot
      }  // end for each slot type

      // print error
      if (overflow.empty() == false) {
        std::cout << "Error: Tile " << tile->getLocStr() << " is over the capacity." << std::endl;
        for (auto pair : overflow) {
          std::cout << "  Slot type: " << pair.first << " slot index: " << pair.second << std::endl;
        }
        overflowTileCount++;
      }            
    } // end for each row
  } // end for each column

  if (errorCount > 0) {
    return false;
  } else {
    return true;
  }    
}

bool checkControlSet() {    
  // Return true if the control set is valid, otherwise return false
  int errorCount = 0;

  int tileCount = 0;
  std::map<int, int> tileClkCount;
  std::map<int, int> tileCeCount;
  std::map<int, int> tileResetCount;
  for (int i = 0; i < chip.getNumCol(); i++) {
    for (int j = 0; j < chip.getNumRow(); j++) {
      Tile* tile = chip.getTile(i, j);
      if (tile->matchType("PLB") == false) {
        continue;        
      }
      tileCount++;

      std::set<int> plbClkNets;
      std::set<int> plbCeNets;
      std::set<int> plbResetNets;
      for (int bank = 0; bank < 2; bank++) {
        std::set<int> clkNets;
        std::set<int> ceNets;
        std::set<int> srNets;
        if (tile->getControlSet(bank, clkNets, ceNets, srNets) == false) {             
          errorCount++;
        }

        int numClk = clkNets.size();      
        int numReset = srNets.size();    
        int numCe = ceNets.size();

        if (numClk > MAX_TILE_CLOCK_PER_PLB_BANK) {
          std::cout << "Error: Multiple clock nets in bank " << bank << " of tile " << tile->getLocStr() << std::endl;
          errorCount++;
        }
        if (numReset > MAX_TILE_RESET_PER_PLB_BANK) {
          std::cout << "Error: Multiple reset nets in bank " << bank << " of tile " << tile->getLocStr() << std::endl;
          errorCount++;
        }  
        if (numCe > MAX_TILE_CE_PER_PLB_BANK) {
          std::cout << "Error: Multiple CE nets in bank " << bank << " of tile " << tile->getLocStr() << std::endl;        
          errorCount++;
        }  

        // merge control sets in different banks
        plbClkNets.insert(clkNets.begin(), clkNets.end());
        plbCeNets.insert(ceNets.begin(), ceNets.end());
        plbResetNets.insert(srNets.begin(), srNets.end());
      }

      int plbCeCount = (int)plbCeNets.size();
      int plbClkCount = (int)plbClkNets.size();
      int plbResetCount = (int)plbResetNets.size();

      if (tileCeCount.find(plbCeCount) == tileCeCount.end()) {
        tileCeCount[plbCeCount] = 1;
      } else {
        tileCeCount[plbCeCount]++;
      } 
      if (tileClkCount.find(plbClkCount) == tileClkCount.end()) {
        tileClkCount[plbClkCount] = 1;
      } else {
        tileClkCount[plbClkCount]++;
      }
      if (tileResetCount.find(plbResetCount) == tileResetCount.end()) {
        tileResetCount[plbResetCount] = 1;
      } else {
        tileResetCount[plbResetCount]++;
      }
    }
  }

  // print stat 
  std::cout << "        Checked control set on " << tileCount << " tiles." << std::endl;
  std::cout << "          Clock:" <<std::endl;
  for (const auto& pair : tileClkCount) {
    std::cout << "            " << pair.second << " tiles have " << pair.first << " clock ";
    if (pair.first > 1) {
      std::cout << "nets." << std::endl;
    } else {
      std::cout << "net." << std::endl;
    }        
  }
  std::cout << "          CE:" <<std::endl;
  for (const auto& pair : tileCeCount) {
    std::cout << "            " << pair.second << " tiles have " << pair.first << " ce ";
    if (pair.first > 1) {
      std::cout << "nets." << std::endl;
    } else {
      std::cout << "net." << std::endl;
    }
  }
  std::cout << "          Reset:" <<std::endl;
  for (const auto& pair : tileResetCount) {
    std::cout << "            " << pair.second << " tiles have " << pair.first << " reset ";
    if (pair.first > 1) {
      std::cout << "nets." << std::endl;
    } else {
      std::cout << "net." << std::endl;
    }
  }

  if (errorCount > 0) {
    return false;
  } else {
    return true;
  }   
}

bool checkClockRegion() {    
  // Return true if the clock region is valid, otherwise return false
  int errorCount = 0;
  
  // clean up 
  for (int j = chip.getNumClockRow() - 1; j >=0 ; j--) {    
    for (int i = 0; i < chip.getNumClockCol(); i++) {
      ClockRegion* clockRegion = chip.getClockRegion(i, j);
      clockRegion->clearClockNets();
    }
  }

  for (auto inst : glbInstMap) {
    int instCol  = std::get<0>(inst.second->getLocation());
    int instRow  = std::get<1>(inst.second->getLocation());
    int clockCol = -1;
    int clockRow = -1;
    if (chip.getClockRegionCoordinate(instCol, instRow, clockCol, clockRow) == false) {
      std::cout << "Error: Instance " << inst.second->getInstanceName() << " is not in any clock region." << std::endl;
      errorCount++;
      continue;
    }

    ClockRegion* clockRegion = chip.getClockRegion(clockCol, clockRow);
    // check each pin of the instance
    for (int idx = 0; idx < inst.second->getNumInpins(); idx++) {
      Pin* pin = inst.second->getInpin(idx);
      int netID = pin->getNetID();
      if (pin->getProp() == PIN_PROP_CLOCK && netID != -1) { // connected clock pin
        Net* netPtr = glbNetMap.find(netID)->second;
        if (netPtr->isClock()) {
          clockRegion->addClockNet(pin->getNetID());   
        }
      }
    }

    // check each pin of the instance
    for (int idx = 0; idx < inst.second->getNumOutpins(); idx++) {
      Pin* pin = inst.second->getOutpin(idx);
      int netID = pin->getNetID();
      if (pin->getProp() == PIN_PROP_CLOCK && netID != -1) { // connected clock pin
        Net* netPtr = glbNetMap.find(netID)->second;
        if (netPtr->isClock()) {
          clockRegion->addClockNet(pin->getNetID());   
        }
      }
    }
  }

  // report clock region
  int overflowRegionCount = 0;
  for (int j = chip.getNumClockRow() - 1; j >=0 ; j--) {
    std::cout << "        | ";
    for (int i = 0; i < chip.getNumClockCol(); i++) {
      ClockRegion* clockRegion = chip.getClockRegion(i, j);
      std::cout << std::left << std::setw(2) << clockRegion->getNumClockNets() <<"| ";
      if (clockRegion->getNumClockNets() > MAX_REGION_CLOCK_COUNT) {
        overflowRegionCount++;
      }
    }
    std::cout << std::endl;
  }

  if (overflowRegionCount == 0) {
    std::cout << "        All clock regions passed legal check." << std::endl;
  } else {
    std::cout << "Error: " << overflowRegionCount << " clock regions have more than " << MAX_REGION_CLOCK_COUNT << " clock nets." << std::endl;
    errorCount++;
  }    

  if (errorCount > 0) {
    return false;
  } else {
    return true;
  }   
}

void reportClockRegion(const int col, const int row) {
  checkClockRegion();

  // report a specific clock region
  ClockRegion* clockRegion = chip.getClockRegion(col, row);
  if (clockRegion) {
    clockRegion->reportClockRegion();
  }
}
