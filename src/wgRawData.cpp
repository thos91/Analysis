#include "wgConst.hpp"
#include "wgRawData.hpp"

//***************************************
Raw_t::Raw_t() : Raw_t(NCHIPS, NCHANNELS) {}

Raw_t::Raw_t(std::size_t n_chips) : Raw_t(n_chips, NCHANNELS) {}

Raw_t::Raw_t(std::size_t n_chips, std::size_t n_chans) :
    n_chips(n_chips), n_chans(n_chans), n_cols(MEMDEPTH) {
  chipid.resize        (n_chips);
  chanid.resize                 (n_chans);
  colid.resize                           (MEMDEPTH);
  
  charge.Initialize    (n_chips, n_chans, MEMDEPTH);
  time.Initialize      (n_chips, n_chans, MEMDEPTH);
  bcid.Initialize      (n_chips,          MEMDEPTH);
  hit.Initialize       (n_chips, n_chans, MEMDEPTH);
  gs.Initialize        (n_chips, n_chans, MEMDEPTH);
  
  pln.Initialize       (n_chips, n_chans);
  chan.Initialize      (n_chips, n_chans);
  grid.Initialize      (n_chips, n_chans);
  x.Initialize         (n_chips, n_chans);
  y.Initialize         (n_chips, n_chans);
  z.Initialize         (n_chips, n_chans);
  
  pedestal.Initialize  (n_chips, n_chans, MEMDEPTH);
  pe.Initialize        (n_chips, n_chans, MEMDEPTH);
  time_ns.Initialize   (n_chips, n_chans, MEMDEPTH);
  gain.Initialize      (n_chips, n_chans, MEMDEPTH);
  tdc_slope.Initialize (n_chips, n_chans, 2);
  tdc_intcpt.Initialize(n_chips, n_chans, 2);
  
  debug_spill.resize            (N_DEBUG_SPILL);
  debug_chip.Initialize(n_chips, N_DEBUG_CHIP);
  this->clear();
}

//***************************************
void Raw_t::clear(){
  Raw_t::spill_number =                                              -1 ;
  Raw_t::spill_mode =                                                -1 ;
  Raw_t::spill_count =                                               -1 ;
  
  std::fill_n(Raw_t::chipid.begin(), Raw_t::chipid.size(),           -1);
  std::fill_n(Raw_t::chanid.begin(), Raw_t::chanid.size(),           -1);
  std::fill_n(Raw_t::colid.begin(), Raw_t::colid.size(),             -1);
  
  Raw_t::charge.fill                                                (-1);
  Raw_t::time.fill                                                  (-1);
  Raw_t::bcid.fill                                                  (-1);
  Raw_t::hit.fill                                                   (-1);
  Raw_t::gs.fill                                                    (-1);

  Raw_t::view =                                                      -1 ;
  Raw_t::pln.fill                                                   (-1);
  Raw_t::chan.fill                                                  (-1);
  Raw_t::grid.fill                                                  (-1);
  Raw_t::x.fill                                                    (NAN);
  Raw_t::y.fill                                                    (NAN);
  Raw_t::z.fill                                                    (NAN);
  
  Raw_t::pedestal.fill                                              (-1);
  Raw_t::pe.fill                                                    (-1);
  Raw_t::gain.fill                                                  (-1);
  Raw_t::time_ns.fill                                               (-1);
  Raw_t::tdc_slope.fill                                              (0);
  Raw_t::tdc_intcpt.fill                                            (-1);

  std::fill_n(Raw_t::debug_spill.begin(), Raw_t::debug_spill.size(),  0);
  Raw_t::debug_chip.fill                                             (0);
}
