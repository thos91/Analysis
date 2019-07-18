// ROOT includes
#include "TROOT.h"

// user includes
#include "wgEnableThreadSafety.hpp"

void wgEnableThreadSafety() {
  ROOT::EnableThreadSafety();
}
