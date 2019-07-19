// ROOT includes
#include "TROOT.h"

// user includes
#include "wgEnableThreadSafety.hpp"

void wgEnableThreadSafety() {
  ROOT::EnableThreadSafety();
}

RootSideEffectGuard::RootSideEffectGuard(): m_directory(gDirectory) {
  gDirectory = 0;
}
RootSideEffectGuard::~RootSideEffectGuard() {
  gDirectory = m_directory;
}
