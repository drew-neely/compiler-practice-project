#pragma once

#include <vector>
#include <print>
#include <map>

#include "cfg.h"

using namespace std;

void toSSA(CFG* cfg);
void verifySSA(CFG* cfg);