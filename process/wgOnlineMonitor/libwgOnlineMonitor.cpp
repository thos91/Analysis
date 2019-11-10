// system includes
#include <string>
#include <exception>
#include <chrono>

// system C includes
#include <cstdio>

// boost includes
#include <boost/program_options.hpp>

// ROOT includes
#include <TH1I.h>
#include <TH2D.h>
#include <TString.h>
#include <TError.h>

// pyrame includes
#include "pyrame.h"

// MIDAS includes
// #include "mjsonrpc.h"

// user includes
#include "wgErrorCodes.hpp"
#include "wgConst.hpp"
#include "wgLogger.hpp"
#include "wgTopology.hpp"
#include "wgStrings.hpp"
#include "wgFit.hpp"
#include "wgOnlineMonitor.hpp"

using namespace std::chrono;

bool WG_ONLINE_MONITOR_CONTINUE_TO_RUN = true;

void newevt(void   *workspace, struct event *e);
void newblock(void *workspace, struct block *b);
void endblock(void *workspace, struct block *b);
void reinit(void   *workspace);
void endrun(void   *workspace);

void newblock(void *workspace, struct block *block)
{
  //struct om_ws *ws = (struct om_ws *) workspace;

  int spill_number = std::stoi(get_block_field(block, "spill_number"));
  int spill_flag   = std::stoi(get_block_field(block, "spill_flag"));
  int spill_count  = std::stoi(get_block_field(block, "spill_count"));

#ifdef DEBUG_WG_ONLINE_MONITOR
  std::cout << "\nNew block\nID: " << block->id << " | Spill number: " << spill_number <<
      " | Spill flag: " << spill_flag << " | Spill count: " << spill_count << std::endl;
#endif
} //newblock

// ==================================================================

void newevt(void *workspace, struct event *event) 
{
  int result;
  struct om_ws *ws = (struct om_ws *) workspace;

  int spill_number;
  if ((result = my_stoi(get_event_field(event, "spill_number"), spill_number)) != 0) {
    Log.eWrite("Failed to convert \"spill_number\": error " + std::to_string(result));
    spill_number = -1;
  }
  int spill_count;
  if ((result = my_stoi(get_event_field(event, "spill_count"), spill_count)) != 0) {
    Log.eWrite("Failed to convert \"spill_count\": error " + std::to_string(result));
    spill_count = -1;
  }
  int spill_flag;
  if ((result = my_stoi(get_event_field(event, "spill_flag"), spill_flag)) != 0) {
    Log.eWrite("Failed to convert \"spill_flag\": error " + std::to_string(result));
    spill_flag = -1;
  }
  int bcid;
  if ((result = my_stoi(get_event_field(event, "bcid"), bcid)) != 0) {
    Log.eWrite("Failed to convert \"bcid\": error " + std::to_string(result));
    bcid = -1;
  }
  int time;
  if ((result = my_stoi(get_event_field(event, "time"), time)) != 0) {
    Log.eWrite("Failed to convert \"time\": error " + std::to_string(result));
    time = -1;
  }
  int column;
  if ((result = my_stoi(get_event_field(event, "column"), column)) != 0) {
    Log.eWrite("Failed to convert \"column\": error " + std::to_string(result));
    column = -1;
  }
  int chip;
  if ((result = my_stoi(get_event_field(event, "chip"), chip)) != 0) {
    Log.eWrite("Failed to convert \"chip\": error " + std::to_string(result));
    chip = -1;
  }
  int channel;
  if ((result = my_stoi(get_event_field(event, "channel"), channel)) != 0) {
    Log.eWrite("Failed to convert \"channel\": error " + std::to_string(result));
    channel = -1;
  }
  int plane;
  if ((result = my_stoi(get_event_field(event, "plane"), plane)) != 0) {
    Log.eWrite("Failed to convert \"plane\": error " + std::to_string(result));
    plane = -1;
  }
  double x;
  if ((result = my_stod(get_event_field(event, "x"), x)) != 0) {
    Log.eWrite("Failed to convert \"x\": error " + std::to_string(result));
    x = NAN;
  }
  double y;
  if ((result = my_stod(get_event_field(event, "y"), y)) != 0) {
    Log.eWrite("Failed to convert \"y\": error " + std::to_string(result));
    y = NAN;
  }
  double z;
  if ((result = my_stod(get_event_field(event, "z"), z)) != 0) {
    Log.eWrite("Failed to convert \"z\": error " + std::to_string(result));
    z = NAN;
  }
  int hit;
  if ((result = my_stoi(get_event_field(event, "hit"), hit)) != 0) {
    Log.eWrite("Failed to convert \"hit\": error " + std::to_string(result));
    hit = -1;
  }
  int charge;
  if ((result = my_stoi(get_event_field(event, "charge"), charge)) != 0) {
    Log.eWrite("Failed to convert \"charge\": error " + std::to_string(result));
    charge = -1;
  }
  int gain;
  if ((result = my_stoi(get_event_field(event, "gain"), gain)) != 0) {
    Log.eWrite("Failed to convert \"gain\": error " + std::to_string(result));
    gain = -1;
  }

  if (hit == 1 && gain == 1) {
    ws->h_charge[chip][channel]->Fill(charge);
    ws->h_bcid[chip][channel].first->Fill(bcid);
  }
  
#ifdef DEBUG_WG_ONLINE_MONITOR
  char debug_message[1024];
  std::snprintf(debug_message, 1024,
                "\tNew event %d\n\tspill_number:%d | spill_count:%d | spill_flag:%d | "
                "bcid:%d | time:%d | column:%d | chip:%d | channel:%d | plane:%d | "
                "x:%.1f | y:%.1f | z:%.1f | hit:%d | charge:%d | gain:%d\n",
                ws->num_events, spill_number, spill_count, spill_flag,
                bcid, time, column, chip, channel, plane,
                x, y, z, hit, charge, gain);
  std::cout << debug_message;
#endif

  ws->num_events++;
} //newevt

// ==================================================================

void endblock(void *workspace, struct block *block) {
  int result;
  struct om_ws *ws=(struct om_ws *)workspace;

  int spill_number;
  if ((result = my_stoi(get_block_field(block, "spill_number"), spill_number)) != 0) {
    Log.eWrite("Failed to convert \"spill_number\": error " + std::to_string(result));
    spill_number = -1;
  }
  int spill_count;
  if ((result = my_stoi(get_block_field(block, "spill_count"), spill_count)) != 0) {
    Log.eWrite("Failed to convert \"spill_count\": error " + std::to_string(result));
    spill_count = -1;
  }
  int spill_flag;
  if ((result = my_stoi(get_block_field(block, "spill_flag"), spill_flag)) != 0) {
    Log.eWrite("Failed to convert \"spill_flag\": error " + std::to_string(result));
    spill_flag = -1;
  }

  // ------- Calculate gain and dark noise -------

  Double_t time = duration<double>(system_clock::now().time_since_epoch()).count();
  
  std::array<double, 2> fit_gain{};
  for (auto const& h_chip_charge : ws->h_charge) {
    for (auto const& h_chan_charge : h_chip_charge) {
      if (h_chan_charge->GetEntries() >= FIT_WHEN_CHARGE_ENTRIES_ARE) {
        wgFit::Gain(h_chan_charge, fit_gain);
        if (!std::isnan(fit_gain[0])) {
          ws->h_gain_stability->Fill(time, fit_gain[0]);
          std::cout << "gain " << fit_gain[0] << "\n";
          h_chan_charge->Reset();
        }
      }
    }
  }

  double fit_dark_noise[2] = {};
  for (auto& h_chip_bcid : ws->h_bcid) {
    for (auto& h_chan_bcid : h_chip_bcid) {
      if (h_chan_bcid.first->GetEntries() >= FIT_WHEN_BCID_ENTRIES_ARE) {
        int previous_spill_count = h_chan_bcid.second;
        h_chan_bcid.second = spill_count;
        int nb_spills = TMath::Abs(spill_count - previous_spill_count);
        wgFit::NoiseRate(h_chan_bcid.first, fit_dark_noise, nb_spills);
        if (!std::isnan(fit_dark_noise[0])) {
          ws->h_dark_noise_stability->Fill(time, fit_dark_noise[0]);
          std::cout << "dark noise " << fit_dark_noise[0] << " nb spills " << nb_spills << "\n";
          h_chan_bcid.first->Reset();
        }
      }
    }
  }

  // ------- spill gap -------

  // When the ws->last_spill=65535 the spill number is reset to
  // zero. Check that the spill number is always incremented by one

  unsigned spill_gap;
  if ((spill_gap = (spill_count - ws->last_spill - 1) % MAX_VALUE_16BITS) != 0) {
    ws->num_spill_gaps += spill_gap;
    std::stringstream ss;
    ss << "spill gap: " << spill_gap << " | spill count: " << spill_count
       << " | last spill number: " << ws->last_spill;
    Log.eWrite(ss.str());
  }
  ws->last_spill = spill_count;

#ifdef DEBUG_WG_ONLINE_MONITOR
  std::cout << "End block\n\tID = " << block->id << " | Spill number: " << spill_number <<
      " | Spill count: " << spill_count << " | Spill flag: " << spill_flag << std::endl;
  std::cout << "\tNumber of events: " << ws->num_events << std::endl;
#endif

  if (ws->num_blocks - ws->lastblock_evtdisp != 1 && ws->num_blocks != 0) {
    Log.eWrite("\"ws->nblock\" = " + std::to_string(ws->num_blocks) +
               " | \"ws->lastblock_evtdisp\" = " + std::to_string(ws->lastblock_evtdisp) +
               "\n");
  }
  
} // endblock

// ==================================================================

void reinit(void *) {}

// ==================================================================

void endrun(void *) {}

// ==================================================================

unsigned initialize_work_space(struct om_ws &ws, Topology *topol, unsigned dif_id) {
  TString name;
  unsigned n_chips = topol->dif_map[dif_id].size();
  if ( n_chips == 0 || n_chips > NCHIPS ) {
    Log.eWrite("[wgOnlineMonitor] wrong number of chips : " + std::to_string(n_chips) );
    return ERR_WRONG_CHIP_VALUE;
  }
  ws.dif_id = dif_id;
  ws.h_charge.resize(n_chips);
  ws.h_bcid.resize(n_chips);
  for (auto const &chip : topol->dif_map[dif_id]) {
    unsigned chip_id = chip.first;
    unsigned n_channels = chip.second;
    ws.h_charge[chip_id] = std::vector<TH1I*>(n_channels, NULL);
    ws.h_bcid[chip_id] = std::vector<std::pair<TH1I*, int>>(n_channels);
    for (unsigned ichan = 0; ichan < n_channels; ++ichan) {
      name.Form("charge_%d_%d", chip_id, ichan);      
      ws.h_charge[chip_id][ichan] = new TH1I(name, name, 400, 400, 800);
      name.Form("bcid_%d_%d", chip_id, ichan);      
      ws.h_bcid[chip_id][ichan].first = new TH1I(name, name, 400, 400, 800);
      ws.h_bcid[chip_id][ichan].second = 0;
    }
  }

  Int_t ini_time = (Int_t) time(NULL);
  Int_t bin_time = 1000; //ms
  Int_t fin_time = ini_time + bin_time * 20; //every 20sec
  Int_t max_gain = 80;
  Int_t min_gain = 0;
  Int_t bin_gain = max_gain  - min_gain;

  ws.h_gain_stability = new TH2D("GainStability", "GainStability",
                                 bin_time, ini_time, fin_time,
                                 bin_gain, min_gain, max_gain);
  ws.h_gain_stability->GetXaxis()->SetTimeDisplay(1);
  ws.h_gain_stability->GetXaxis()->SetTimeFormat("%d/%b");
  ws.h_gain_stability->GetXaxis()->SetNdivisions(510);
  ws.h_gain_stability->GetYaxis()->SetNdivisions(205);
  ws.h_gain_stability->GetXaxis()->SetLabelOffset(0.01);
  ws.h_gain_stability->GetXaxis()->SetLabelSize(0.04);
  ws.h_gain_stability->GetYaxis()->SetLabelSize(0.04);

  Int_t max_dark_noise = 10000;
  Int_t min_dark_noise = 0;
  Int_t bin_dark_noise = max_gain  - min_gain;
  
  ws.h_dark_noise_stability = new TH2D("DarkNoiseStability", "DarkNoiseStability",
                                       bin_time, ini_time, fin_time,
                                       bin_dark_noise, min_dark_noise, max_dark_noise);
  ws.h_dark_noise_stability->GetXaxis()->SetTimeDisplay(1);
  ws.h_dark_noise_stability->GetXaxis()->SetTimeFormat("%d/%b");
  ws.h_dark_noise_stability->GetXaxis()->SetNdivisions(510);
  ws.h_dark_noise_stability->GetYaxis()->SetNdivisions(205);
  ws.h_dark_noise_stability->GetXaxis()->SetLabelOffset(0.01);
  ws.h_dark_noise_stability->GetXaxis()->SetLabelSize(0.04);
  ws.h_dark_noise_stability->GetYaxis()->SetLabelSize(0.04);

  return n_chips;
}

// ==================================================================

void free_workspace(struct om_ws &ws) {
  for (auto const& h_chip_charge : ws.h_charge)
    for (auto const& h_chan_charge : h_chip_charge)
      delete h_chan_charge;
  for (auto const& h_chip_bcid : ws.h_bcid)
    for (auto const& h_chan_bcid : h_chip_bcid)
      delete h_chan_bcid.first;
  delete ws.h_gain_stability;
  delete ws.h_dark_noise_stability;
}
  
// ==================================================================

void signal_handler(int signal_num) { WG_ONLINE_MONITOR_CONTINUE_TO_RUN = false; }

// ==================================================================

int wgOnlineMonitor(const char * x_pyrame_config_file, unsigned dif_id) {
  std::string pyrame_config_file(x_pyrame_config_file);
  
  gErrorIgnoreLevel = kError;
  signal(SIGINT, signal_handler);

  // =========== Topology =========== //

  Topology * topol;
  try { topol = new Topology(pyrame_config_file); }
  catch (const std::exception& e) {
    Log.eWrite("[wgOnlineMonitor] " + std::string(e.what()));
    return ERR_TOPOLOGY;
  }
  
  // =========== Work space initialization =========== //
  
  om_ws ws = {};
  try { initialize_work_space(ws, topol, dif_id); }
  catch (const std::exception& e) {
    Log.eWrite("[wgOnlineMonitor] Error during workspace initialization : "
               + std::string(e.what()));
    return ERR_WORKSPACE_INITIALIZATION;
  }

  // =========== Event loop =========== //

  char data_source_name[64];
  std::snprintf(data_source_name, 64, "converter_dif_%d", dif_id);

  if ((ws.loop = new_event_loop(data_source_name,
                                &ws,
                                newevt,
                                newblock,
                                endblock,
                                reinit,
                                endrun,
                                'f')) == NULL) {
    return ERR_EVENT_LOOP;
  }

  Log.Write("Press ! to exit");
  
  while (WG_ONLINE_MONITOR_CONTINUE_TO_RUN) {
    run_loop(ws.loop);
    std::string command;
    std::cin >> command;
    if (command == "!") break;
  }

  Log.Write("End of loop!");

  stop_loop(ws.loop);
  free_event_loop(ws.loop);
  free_workspace(ws);

  return WG_SUCCESS;
}
