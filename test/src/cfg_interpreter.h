#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <print>
#include <map>

#include "cfg.h"
#include "liveness.h"

int interpretCFG(CFG* cfg, vector<int>& parameters, bool testLiveness=false);