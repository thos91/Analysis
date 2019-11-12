#ifndef WGRAWDATA_H
#define WGRAWDATA_H

#include "wgConst.hpp"

// ===================================================================== //
//                                                                       //
//                             Raw Data Class                            //
//                                                                       //
// ===================================================================== //

// The Raw_t class is a container for all the info contained in the
// decoded raw data, plus the geometry info and the calibration info
// (if present).

class Raw_t
{
public:
  // SPILL
  int spill_number;
  int spill_mode;
  int spill_count;

  // RAW DATA IDENTIFIERS
  i1vector chipid;            // [NCHIPS];    //ASU 
  i1vector chanid;            //          [NCHANNELS]; //CHANNEL
  i1vector colid;             //                    [MEMDEPTH]; //SCA

  // RAW DATA
  i3CCvector charge;          // [NCHIPS][NCHANNELS][MEMDEPTH];
  i3CCvector time;            // [NCHIPS][NCHANNELS][MEMDEPTH];
  i2CCvector bcid;            // [NCHIPS]           [MEMDEPTH];
  i3CCvector hit;             // [NCHIPS][NCHANNELS][MEMDEPTH];
  i3CCvector gs;              // [NCHIPS][NCHANNELS][MEMDEPTH];
  
  // GEOMETRY DATA
  int        view;
  i2CCvector pln;             // [NCHIPS][NCHANNELS];
  i2CCvector chan;            // [NCHIPS][NCHANNELS];
  i2CCvector grid;            // [NCHIPS][NCHANNELS];
  d2CCvector x;               // [NCHIPS][NCHANNELS];
  d2CCvector y;               // [NCHIPS][NCHANNELS];
  d2CCvector z;               // [NCHIPS][NCHANNELS];

  // CALIBRATION DATA
  d3CCvector pedestal;        // [NCHIPS][NCHANNELS][MEMDEPTH];
  d3CCvector pe;              // [NCHIPS][NCHANNELS][MEMDEPTH];
  d3CCvector gain;            // [NCHIPS][NCHANNELS][MEMDEPTH];
  d3CCvector time_ns;         // [NCHIPS][NCHANNELS][MEMDEPTH];
  d3CCvector tdc_slope;       // [NCHIPS][NCHANNELS][2];
  d3CCvector tdc_intcpt;      // [NCHIPS][NCHANNELS][2];

  i1vector   debug_spill;     //         [N_DEBUG_SPILL];
  i2CCvector debug_chip;      // [NCHIPS][N_DEBUG_CHIP];

  // COUNTERS (MAX VALUES)
  int n_chips;
  int n_chans;
  int n_cols;

  // CONSTRUCTORS
  Raw_t();
  explicit Raw_t(std::size_t n_chips);
  Raw_t(std::size_t n_chips, std::size_t n_chans);

  // CLEAR CONTENT OF OBJECT
  void clear();
};

#endif /* WGRAWDATA_H */
