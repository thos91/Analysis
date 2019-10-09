// system includes
#include <string>
#include <exception>

// system C includes
#include <cstdio>

// boost includes
#include <boost/program_options.hpp>

// ROOT includes
// #include <TROOT.h>
// #include <TH1F.h>
// #include <TH2F.h>
// #include <TCanvas.h>
// #include <TGraph.h>
// #include <TText.h>
#include <TApplication.h>

// pyrame includes
#include "pyrame.h"

// MIDAS includes
#include "mjsonrpc.h"

// user includes
#include "wgErrorCodes.hpp"
#include "wgConst.hpp"
#include "wgLogger.hpp"
#include "wgTopology.hpp"
#include "wgStrings.hpp"
#include "wgOnlineMonitor.hpp"

const int EVEN_TIME_OFFSET = 4000; // TODO: 
const int  ODD_TIME_OFFSET = 500; // TODO: 
const int EVEN_RAMP_TIME = -1; // TODO: 
const int  ODD_RAMP_TIME = 1; // TODO: 
const int BCID_NS = 580; // ns

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
  float x;
  if ((result = my_stof(get_event_field(event, "x"), x)) != 0) {
    Log.eWrite("Failed to convert \"x\": error " + std::to_string(result));
    x = NAN;
  }
  float y;
  if ((result = my_stof(get_event_field(event, "y"), y)) != 0) {
    Log.eWrite("Failed to convert \"y\": error " + std::to_string(result));
    y = NAN;
  }
  float z;
  if ((result = my_stof(get_event_field(event, "z"), z)) != 0) {
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

  // Float_t offset_time = bcid % 2 == 0 ? EVEN_TIME_OFFSET : ODD_TIME_OFFSET;
  // Float_t ramp_time   = bcid % 2 == 0 ? EVEN_RAMP_TIME : ODD_RAMP_TIME;
  // Float_t hittiming = bcid * BCID_NS  + (time - offset_time) * ramp_time;
  
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
  
  if (ws->num_blocks - ws->lastblock_evtdisp != 1 && ws->num_blocks != 0) {
    Log.eWrite("\"ws->nblock\" = " + std::to_string(ws->num_blocks) +
               " | \"ws->lastblock_evtdisp\" = " + std::to_string(ws->lastblock_evtdisp) + "\n");
  }

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

  // ------- spill gap -------

  // When the ws->last_spill=65535 the spill number is reset to
  // zero. Check that the spill number is always incremented by one

  if ((spill_number - ws->last_spill - 1) % MAX_VALUE_16BITS + 1 != 0) {
    ws->num_spill_gaps++;
    Log.eWrite("\tSpill gap: " + std::to_string(ws->num_spill_gaps));
  }
  ws->last_spill = spill_number;

#ifdef DEBUG_WG_ONLINE_MONITOR
  std::cout << "End block\n\tID = " << block->id << " | Spill number: " << spill_number <<
      " | Spill count: " << spill_count << " | Spill flag: " << spill_flag << std::endl;
  std::cout << "\tNumber of events: " << ws->num_events << std::endl;
#endif
} // endblock

// ==================================================================

void reinit(void *workspace) 
{
} //reinit

// ==================================================================

void endrun(void *workspace) 
{
} //reinit


void initialize_work_space(struct om_ws &ws) {
    ws.dif_id = -1;
}

int wgOnlineMonitor(const char * x_pyrame_config_file, unsigned dif_id) {

  std::string pyrame_config_file(x_pyrame_config_file);

  TApplication om_app("WAGASCI online monitor", 0, NULL);

  // =========== Topology =========== //

  Topology * topol;
  try { topol = new Topology(pyrame_config_file); }
  catch (const std::exception& e) {
    Log.eWrite("[wgAnaHist] " + std::string(e.what()));
    return ERR_TOPOLOGY;
  }
  unsigned n_chips = topol->dif_map[dif_id].size();

  if ( n_chips == 0 || n_chips > NCHIPS ) {
    Log.eWrite("[wgAnaHist] wrong number of chips : " + std::to_string(n_chips) );
    return ERR_WRONG_CHIP_VALUE;
  }
  
  // =========== Event loop =========== //
  
  struct om_ws *ws = new struct om_ws;
  initialize_work_space(*ws);
  ws->dif_id = dif_id;

  char data_source_name[64];
  std::snprintf(data_source_name, 64, "converter_dif_%d", dif_id);

  if ((ws->loop = new_event_loop(data_source_name,
                                 ws,
                                 newevt,
                                 newblock,
                                 endblock,
                                 reinit,
                                 endrun,
                                 'f')) == NULL) {
    return ERR_EVENT_LOOP;
  }

  // If you want to avoid running the TApplication uncomment the while (true)
  // below. You will then need to exit by Ctrl-C in the terminal window.
  //
  // while (true) 
  run_loop(ws->loop);

  om_app.Run();

  Log.Write("End of loop!");

  return WG_SUCCESS;
}

static MJsonNode* send_channel_data(const MJsonNode* params)
{
   if (!params) {
      MJSO* doc = MJSO::I();
      doc->D("send channel data to mhttpd for the online monitor");
      doc->P("DIF", MJSON_INT, "DIF number");
      doc->P("CHIP", MJSON_INT, "CHIP number");
      doc->P("CHANNEL", MJSON_INT, "CHANNEL number");
      doc->R("charge", MJSON_STRING, "PEU values as a base64 string");
      doc->R("time", MJSON_STRING, "time from beam trigger as a base64 string");
      doc->R("gain", MJSON_STRING, "gain history");
      doc->R("dark rate", MJSON_STRING, "dark rate history");
      return doc;
   }

   MJsonNode* error = NULL;

   int dif = mjsonrpc_get_param(params, "DIF", NULL)->GetInt();
   if (error) return error;
   int chip = mjsonrpc_get_param(params, "CHIP", NULL)->GetInt();
   if (error) return error;
   int channel = mjsonrpc_get_param(params, "CHANNEL", NULL)->GetInt();
   if (error) return error;
   
   if (mjsonrpc_debug)
     std::cout << "send_channel_data(dif=" << dif << ",chip=" <<
         chip << ",channel=" << channel << ")\n";

   MJsonNode* result = MJsonNode::MakeObject();

   std::string charge, time, gain, dark_rate;
   
   result->AddToObject("charge", MJsonNode::MakeString(charge));
   result->AddToObject("time", MJsonNode::MakeString(time));
   result->AddToObject("gain", MJsonNode::MakeString(gain));
   result->AddToObject("dark_rate", MJsonNode::MakeString(dark_rate));

   return mjsonrpc_make_result(result);
}
