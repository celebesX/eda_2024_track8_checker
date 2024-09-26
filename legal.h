#pragma once

#include <iostream>
#include <map>
#include "object.h"
#include "arch.h"

bool legalCheck();  // check tile type and capacity 

bool checkTypeAndCapacity();
bool checkControlSet();
bool checkClockRegion();

void reportClockRegion(const int col, const int row);