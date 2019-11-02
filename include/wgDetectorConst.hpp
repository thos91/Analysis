#ifndef WGDETECTORCONST_H
#define WGDETECTORCONST_H

namespace wagasci_detector_constants {

// Number of ...
extern const int NUM_VIEWS;
extern const int NUM_PLANES;
extern const int NUM_CHANNELS_PER_PLANE;

// Planes
extern const double PLANE_CHANNEL_START; //mm // center of ch#0
extern const double PLANE_START;         //mm // center of 1st X-layer(pln#0 side view)
extern const double PLANE_DISTANCE;      //mm // b/w two adjacent X-layers (planes)


// Scintillators
extern const double SCINTILLATOR_WIDTH;  //mm
}

#endif /* WGDETECTORCONST_H */
