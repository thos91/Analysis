#ifndef WGENABLETHREADSAFETY_H
#define WGENABLETHREADSAFETY_H

#include "TDirectory.h"

#ifdef __cplusplus
extern "C" {
#endif
  void wgEnableThreadSafety();
#ifdef __cplusplus
}
#endif

class RootSideEffectGuard {
public:
  RootSideEffectGuard();
  ~RootSideEffectGuard();
private:
  TDirectory* m_directory;
};


#endif /* WGENABLETHREADSAFETY_H */
