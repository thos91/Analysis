#include "wgDetectorConst.hpp"

namespace wagasci_detector_constants {

// Number of ...
const int NUM_VIEWS = 2;
const int NUM_PLANES = 8;
const int NUM_CHANNELS_PER_PLANE = 80;

// Planes
const double PLANE_CHANNEL_START = -487.5; //mm // center of ch#0
const double PLANE_START = -226.5;         //mm // center of 1st X-layer(pln#0 side view)
const double PLANE_DISTANCE = 57;      //mm // b/w two adjacent X-layers (planes)
}
