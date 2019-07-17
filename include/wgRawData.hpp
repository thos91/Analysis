#ifndef WGRAWDATA_H
#define WGRAWDATA_H

#include "wgConst.hpp"

// ===================================================================== //
//                                                                       //
//                             Raw Data Class                            //
//                                                                       //
// ===================================================================== //

class Raw_t
{
public:
  int spill_number;
  int spill_mode;
  int spill_count;
  i1vector chipid;            // [NCHIPS];    //ASU 
  i1vector difid;             // [NCHIPS];    //DIF
  i1vector chip;              // [NCHIPS];
  i1vector chan;              //          [NCHANNELS];
  i1vector col;               //                    [MEMDEPTH];
  i3CCvector charge;          // [NCHIPS][NCHANNELS][MEMDEPTH];
  i3CCvector time;            // [NCHIPS][NCHANNELS][MEMDEPTH];
  i2CCvector bcid;            // [NCHIPS]           [MEMDEPTH];
  i3CCvector hit;             // [NCHIPS][NCHANNELS][MEMDEPTH];
  i3CCvector gs;              // [NCHIPS][NCHANNELS][MEMDEPTH];
  i1vector   debug_spill;     //         [N_DEBUG_SPILL];
  i2CCvector debug_chip;      // [NCHIPS][N_DEBUG_CHIP];
  int        view;
  i2CCvector pln;             // [NCHIPS][NCHANNELS];
  i2CCvector ch;              // [NCHIPS][NCHANNELS];
  i2CCvector grid;            // [NCHIPS][NCHANNELS];
  d2CCvector x;               // [NCHIPS][NCHANNELS];
  d2CCvector y;               // [NCHIPS][NCHANNELS];
  d2CCvector z;               // [NCHIPS][NCHANNELS];
  d3CCvector pedestal;        // [NCHIPS][NCHANNELS][MEMDEPTH];
  d3CCvector pe;              // [NCHIPS][NCHANNELS][MEMDEPTH];
  d3CCvector time_ns;         // [NCHIPS][NCHANNELS][MEMDEPTH];
  d3CCvector gain;            // [NCHIPS][NCHANNELS][MEMDEPTH];
  d3CCvector tdc_slope;       // [NCHIPS][NCHANNELS][2];
  d3CCvector tdc_intcpt;      // [NCHIPS][NCHANNELS][2];

  int n_chips;
  int n_chans;
  int n_cols;

  Raw_t();
  Raw_t(std::size_t n_chips);
  Raw_t(std::size_t n_chips, std::size_t n_chans);
  void clear();
};

#endif /* WGRAWDATA_H */
