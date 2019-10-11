// ROOT includes
#include "TROOT.h"
#include "TPluginManager.h"
#include "TMinuitMinimizer.h"

// user includes
#include "wgConst.hpp"
#include "wgLogger.hpp"
#include "wgEnableThreadSafety.hpp"

std::mutex MUTEX;

void wgEnableThreadSafety() {
  ROOT::EnableThreadSafety();
  // We will be using Minuit2 for fitting since it is thread safe, so check it is there
  if (gROOT->GetPluginManager()->FindHandler("ROOT::Math::Minimizer", "Minuit2") == 0)  {
    Log.eWrite("We need Minuit2 for parallel fitting!");
  } else 
    ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2");
}

RootSideEffectGuard::RootSideEffectGuard(): m_directory(gDirectory) {
  gDirectory = 0;
}
RootSideEffectGuard::~RootSideEffectGuard() {
  gDirectory = m_directory;
}
