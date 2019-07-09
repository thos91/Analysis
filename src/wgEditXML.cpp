// system C++ includes
#include <iostream>
#include <string>
#include <vector>

// system C includes
#include <cstdlib>
#include <ctime>

// tinyxml2 includes
#include "tinyxml2.hpp"

// user includes
#include "wgConst.hpp"
#include "wgEditXML.hpp"
#include "wgEditConfig.hpp"
#include "wgErrorCode.hpp"
#include "wgExceptions.hpp"
#include "wgFileSystemTools.hpp"
#include "wgTopology.hpp"
#include "wgLogger.hpp"

//**********************************************************************
void wgEditXML::Write(){
  xml->SaveFile(wgEditXML::filename.c_str());
}

//**********************************************************************
void wgEditXML::Open(const string& filename){
  CheckExist Check;
  if(!Check.XmlFile(filename))
    throw wgInvalidFile("[" + filename +"][wgEditXML::Open] error in opening XML file");
  wgEditXML::filename=filename; 
  wgEditXML::xml = new XMLDocument();
  XMLError eResult = wgEditXML::xml->LoadFile(filename.c_str());
  if (eResult != tinyxml2::XML_SUCCESS)
    throw wgInvalidFile("[" + filename + "][wgEditXML::Open] error in opening XML file");
}

//**********************************************************************
void wgEditXML::Close(){ 
  delete xml;
}

//**********************************************************************
void wgEditXML::Make(const string& filename, const unsigned idif, const unsigned ichip, const unsigned ichan){
  char str[XML_ELEMENT_STRING_LENGTH];
  xml = new XMLDocument();
  XMLDeclaration* decl = xml->NewDeclaration();
  xml->InsertEndChild(decl);

  XMLElement* data = xml->NewElement("data");
  xml->InsertEndChild(data);

  XMLElement* config = data->GetDocument()->NewElement("config");
  data->InsertEndChild(config);

  XMLElement* difid = xml->NewElement("difid");
  config->InsertEndChild(difid);
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "%d", idif );
  difid->InsertEndChild(difid->GetDocument()->NewText(str));
  
  XMLElement* chipid = xml->NewElement("chipid");
  config->InsertEndChild(chipid);
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "%d", ichip );
  chipid->InsertEndChild(chipid->GetDocument()->NewText(str));

  XMLElement* chanid = xml->NewElement("chanid");
  config->InsertEndChild(chanid);
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "%d", ichan );
  chanid->InsertEndChild(chanid->GetDocument()->NewText(str));

  XMLElement* start_time = xml->NewElement("start_time");
  config->InsertEndChild(start_time);
  start_time->InsertEndChild(start_time->GetDocument()->NewText("0"));
  
  XMLElement* stop_time = xml->NewElement("stop_time");
  config->InsertEndChild(stop_time);
  stop_time->InsertEndChild(stop_time->GetDocument()->NewText("0"));
  
  XMLElement* trigth = xml->NewElement("trigth");
  config->InsertEndChild(trigth);
  trigth->InsertEndChild(trigth->GetDocument()->NewText("-1"));

  XMLElement* gainth = xml->NewElement("gainth");
  config->InsertEndChild(gainth);
  gainth->InsertEndChild(gainth->GetDocument()->NewText("-1"));

  XMLElement* inputDAC = xml->NewElement("inputDAC");
  config->InsertEndChild(inputDAC);
  inputDAC->InsertEndChild(inputDAC->GetDocument()->NewText("-1"));

  XMLElement* HG = xml->NewElement("HG");
  config->InsertEndChild(HG);
  HG->InsertEndChild(HG->GetDocument()->NewText("-1"));

  XMLElement* LG = xml->NewElement("LG");
  config->InsertEndChild(LG);
  LG->InsertEndChild(LG->GetDocument()->NewText("-1"));

  XMLElement* trig_adj = xml->NewElement("trig_adj");
  config->InsertEndChild(trig_adj);
  trig_adj->InsertEndChild(trig_adj->GetDocument()->NewText("-1"));

  XMLElement* chan = xml->NewElement("chan");
  data->InsertEndChild(chan);

  for(unsigned k = 1; k <= MEMDEPTH; k++){
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "col_%d", k );
    XMLElement* col = xml->NewElement(str);
    chan->InsertEndChild(col);
  }
  xml->SaveFile(filename.c_str());
  delete xml;
}

//**********************************************************************
bool wgEditXML::GetConfig(const string& configxml,
                          const unsigned igdcc,
                          const unsigned idif,
                          const unsigned ichip,
                          const unsigned n_chans,
                          vector<vector<int>>& v) {
  try {
    bool found=false;
    string bitstream("");
    CheckExist Check;

    if(!Check.XmlFile(configxml))
      throw wgInvalidFile(configxml + " wasn't found or is not valid");
    if(ichip > NCHIPS)
      throw std::invalid_argument("ichip is greater than " + to_string(NCHIPS));

    XMLDocument configfile;
    configfile.LoadFile(configxml.c_str()); 
    XMLElement* ecal = configfile.FirstChildElement("ecal");
    XMLElement* domain = ecal->FirstChildElement("domain");
    XMLElement* acqpc = domain->FirstChildElement("acqpc");
    // GDCCs loop
    for(XMLElement* gdcc = acqpc->FirstChildElement("gdcc"); gdcc != NULL; gdcc = gdcc->NextSiblingElement("gdcc")) {
      if( string(gdcc->Attribute("name")) == "gdcc_1_" + to_string(igdcc) ) {
        // DIFs loop
        for(XMLElement* dif = gdcc->FirstChildElement("dif"); dif != NULL; dif = dif->NextSiblingElement("dif")) {
          if( string(dif->Attribute("name")) == "dif_1_" + to_string(igdcc) + "_" + to_string(idif) ) {
            // ASUs loop
            for(XMLElement* asu = dif->FirstChildElement("asu"); asu != NULL; asu = asu->NextSiblingElement("asu")) {
              if( string(asu->Attribute("name")) == "asu_1_" + to_string(igdcc) + "_" + to_string(idif) + "_" + to_string(ichip) ) {
                XMLElement* spiroc2d = asu->FirstChildElement("spiroc2d");
                // loop to find the spiroc2d_bitstream parameter
                for(XMLElement* param = spiroc2d->FirstChildElement("param"); param != NULL; param = param->NextSiblingElement("param")) {
                  if( string(param->Attribute("name")) == "spiroc2d_bitstream" ) {
                    bitstream = param->GetText();
                    found=true;
                    break;
                  }
                }
              }
              // If the bitstream was found exit the ASU loop
              else if (found) break;
            }
          }
        }
      }
    }
    if (found == false) return false;

    wgEditConfig EditCon(bitstream, true);
    v.clear();
    v.resize(n_chans);
    for(unsigned i = 0; i < n_chans; i++) {
      v[i].push_back(EditCon.Get_trigth());  
      v[i].push_back(EditCon.Get_gainth());
      v[i].push_back(EditCon.Get_inputDAC(i));
      v[i].push_back(EditCon.Get_ampDAC(i));
      v[i].push_back(EditCon.Get_trigadj(i));
    }
  }
  catch (const exception& e) {
    Log.eWrite("[" + configxml + "][GetConfig] failed to get spiroc2d_bitstream (DIF = " + to_string(idif) + ", chip = " + to_string(ichip) + " : " + string(e.what()));
    return false;
  }
  return true;
}

//**********************************************************************
void wgEditXML::GetLog(const string& filename, vector<int>& v){
  string str("");
  string target("");
  v.clear();
  v.resize(4);
  XMLDocument *xml = new XMLDocument();
  xml->LoadFile(filename.c_str()); 
  XMLElement* log = xml->FirstChildElement("log");
  XMLElement* acq  = log->FirstChildElement("acq");
  for(XMLElement* param = acq->FirstChildElement("param"); param!=NULL; param= param->NextSiblingElement("param")){
    target = param->Attribute("name");
    if(target=="start_time"){
      str = param->GetText();
      struct tm tm;
      strptime(str.c_str(),"%Y/%m/%d %H:%M:%S",&tm);
      time_t time = mktime(&tm);
      v[0] = (int)time;
    }else if(target=="stop_time"){
      str = param->GetText();
      struct tm tm;
      strptime(str.c_str(),"%Y/%m/%d %H:%M:%S",&tm);
      time_t time = mktime(&tm);
      v[1] = (int)time;
    }
    else if(target=="nb_data_pkts"){ 
      str = param->GetText();
      v[2]=atoi(str.c_str());
    }
    else if(target=="nb_lost_pkts"){ 
      str = param->GetText();
      v[3]=atoi(str.c_str());
    }
  }
  delete xml;
}

//**********************************************************************
void wgEditXML::SetConfigValue(const string& name, const int value, const bool create_new) {
  XMLElement* data   = xml->FirstChildElement("data");
  XMLElement* config = data->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());
  if (target) {
    target->SetText(to_string(value).c_str());
  }
  else if(create_new == true) {
    XMLElement* newElement = xml->NewElement(name.c_str());
    newElement->SetText(value);
    config->InsertEndChild(newElement);      
  }
  else throw wgElementNotFound("Element " + name + " doesn't exist");
}

//**********************************************************************
void wgEditXML::SetColValue(const string& name, const int icol, const int value, const bool create_new) {
  if (icol <= 0 || icol > MEMDEPTH) return;
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* chan = data->FirstChildElement("chan");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "col_%d", icol );
  XMLElement* col = chan->FirstChildElement(str);
  XMLElement* target = col->FirstChildElement(name.c_str());
  if ( target ) {
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "%d", value );
    target->SetText(str);
  }else{
    if(create_new == true){
      XMLElement* newElement = xml->NewElement(name.c_str());
      newElement->SetText(value);
      col->InsertEndChild(newElement);
    }else{
      throw wgElementNotFound("Element " + name + " doesn't exist");
    }
  }
}

//**********************************************************************
void wgEditXML::SetChValue(const string& name, const int value, const bool create_new){
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* chan = data->FirstChildElement("chan");
  XMLElement* target = chan->FirstChildElement(name.c_str());
  if ( target ) {
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "%d", value );
    target->SetText(str);
  }else{
    if(create_new == true){
      XMLElement* newElement = xml->NewElement(name.c_str());
      newElement->SetText(value);
      chan->InsertEndChild(newElement); 
    }else{
      Log.eWrite("[wgEditXML::SetChValue] Warning! Element " + name + " doesn't exist!");
    }
  }  
}


//**********************************************************************
void wgEditXML::AddColElement(const string& name, const int icol) {
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* chan = data->FirstChildElement("chan");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "col_%d", icol );
  XMLElement* col = chan->FirstChildElement(str);
  XMLElement* newElement = xml->NewElement(name.c_str());
  col->InsertEndChild(newElement);
}

//**********************************************************************
void wgEditXML::AddChElement(const string& name) {
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* chan = data->FirstChildElement("chan");
  XMLElement* newElement = xml->NewElement(name.c_str());
  chan->InsertEndChild(newElement);
}

//**********************************************************************
int wgEditXML::GetColValue(const string& name,const int icol){
  if(icol<0 || icol>MEMDEPTH) return 0.0;
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* chan = data->FirstChildElement("chan");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "col_%d", icol );
  XMLElement* col = chan->FirstChildElement(str);
  XMLElement* target = col->FirstChildElement(name.c_str());
  if ( target ) {
    string value = target->GetText();
    return atof(value.c_str());
  }else{
    throw wgElementNotFound("Element " + name + " doesn't exist");
    return -1.;
  }
}

//**********************************************************************
int wgEditXML::GetChValue(const string& name){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* chan = data->FirstChildElement("chan");
  XMLElement* target = chan->FirstChildElement(name.c_str());
  if ( target ) {
    string value = target->GetText();
    return atof(value.c_str());
  }else{
    throw wgElementNotFound("Element " + name + " doesn't exist");
    return -1.;
  }
}

//**********************************************************************
int wgEditXML::GetConfigValue(const string& name){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* config = data->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());
  if ( target ) {
    string value = target->GetText();
    return atoi(value.c_str());
  }else{
    throw wgElementNotFound("Element " + name + " doesn't exist");
    return -1;
  }
}

//**********************************************************************
void wgEditXML::SUMMARY_Make(const string& filename, const unsigned n_chans) {
  xml = new XMLDocument();
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLDeclaration* decl = xml->NewDeclaration();
  xml->InsertEndChild(decl);

  XMLElement* data; 
  //chip
  XMLElement* g_config;
  XMLElement* start_time;
  XMLElement* stop_time;
  XMLElement* gs_threshold;
  XMLElement* trigger_threshold;
  XMLElement* difid;
  XMLElement* chipid;
  XMLElement* n_channels;
  //chan
  vector<XMLElement*> ch              (n_chans);
  vector<XMLElement*> fit             (n_chans);
  vector<XMLElement*> noise           (n_chans);
  vector<XMLElement*> enoise          (n_chans);
  vector<XMLElement*> pe_level        (n_chans);
  vector<XMLElement*> config          (n_chans);
  vector<XMLElement*> inputDAC        (n_chans);
  vector<XMLElement*> ampDAC          (n_chans);
  vector<XMLElement*> threshold_adjust(n_chans);
  vector<XMLElement*> chanid          (n_chans);
  //col
  vector<array<XMLElement*, MEMDEPTH> > pedestal(n_chans);
  vector<array<XMLElement*, MEMDEPTH> > pedestal_error(n_chans);
  vector<array<XMLElement*, MEMDEPTH> > charge_hit(n_chans);
  vector<array<XMLElement*, MEMDEPTH> > charge_hit_error(n_chans);
  vector<array<XMLElement*, MEMDEPTH> > diff(n_chans);
  vector<array<XMLElement*, MEMDEPTH> > diff_error(n_chans);

  // ********************** //

  data = xml->NewElement("data");
  xml->InsertEndChild(data);

  g_config = xml->NewElement("config");
  data->InsertEndChild(g_config);
  difid = xml->NewElement("difid");
  g_config->InsertEndChild(difid);
  chipid = xml->NewElement("chipid");
  g_config->InsertEndChild(chipid);
  n_channels = xml->NewElement("n_chans");
  g_config->InsertEndChild(n_channels);
  start_time = xml->NewElement("start_time");
  g_config->InsertEndChild(start_time);
  stop_time = xml->NewElement("stop_time");
  g_config->InsertEndChild(stop_time);
  trigger_threshold = xml->NewElement("trigth");
  g_config->InsertEndChild(trigger_threshold);
  gs_threshold = xml->NewElement("gainth");
  g_config->InsertEndChild(gs_threshold);


  for(unsigned ichan = 0; ichan < n_chans; ichan++) {
    unsigned ichan_id = ichan + 1;
    // ***** data > ch ***** //
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "chan_%d", ichan_id );
    ch[ichan] = xml->NewElement(str);    
    data->InsertEndChild(ch[ichan]);

    // ***** data > ch > config ***** //
    config[ichan] = xml->NewElement("config");
    ch[ichan]->InsertEndChild(config[ichan]);

    chanid[ichan] = xml->NewElement("chanid");
    config[ichan]->InsertEndChild(chanid[ichan]);
    inputDAC[ichan] = xml->NewElement("inputDAC");
    config[ichan]->InsertEndChild(inputDAC[ichan]);
    ampDAC[ichan] = xml->NewElement("ampDAC");
    config[ichan]->InsertEndChild(ampDAC[ichan]);
    threshold_adjust[ichan] = xml->NewElement("adjDAC");
    config[ichan]->InsertEndChild(threshold_adjust[ichan]);

    // ***** data > ch > fit***** //
    fit[ichan] = xml->NewElement("fit");
    ch[ichan]->InsertEndChild(fit[ichan]);

    noise[ichan] = xml->NewElement("noise_rate");
    fit[ichan]->InsertEndChild(noise[ichan]);
    enoise[ichan] = xml->NewElement("sigma_rate");
    fit[ichan]->InsertEndChild(enoise[ichan]);
    pe_level[ichan] = xml->NewElement("pe_level");
    fit[ichan]->InsertEndChild(pe_level[ichan]);

    for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
      unsigned icol_id = icol + 1;
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "charge_nohit_%d", icol_id );
      pedestal[ichan][icol] = xml->NewElement(str);
      fit[ichan]->InsertEndChild(pedestal[ichan][icol]);
    }
    for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
      unsigned icol_id = icol + 1;
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "sigma_nohit_%d", icol_id );
      pedestal_error[ichan][icol] = xml->NewElement(str);
      fit[ichan]->InsertEndChild(pedestal_error[ichan][icol]);
    }
    for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
      unsigned icol_id = icol + 1;
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "charge_hit_%d", icol_id );
      charge_hit[ichan][icol] = xml->NewElement(str);
      fit[ichan]->InsertEndChild(charge_hit[ichan][icol]);
    }
    for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
      unsigned icol_id = icol + 1;
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "sigma_hit_%d", icol_id );
      charge_hit_error[ichan][icol] = xml->NewElement(str);
      fit[ichan]->InsertEndChild(charge_hit_error[ichan][icol]);
    }
    for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
      unsigned icol_id = icol + 1;
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "diff_%d", icol_id );
      diff[ichan][icol] = xml->NewElement(str);
      fit[ichan]->InsertEndChild(diff[ichan][icol]);
    }
    for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
      unsigned icol_id = icol + 1;
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "sigma_diff_%d", icol_id );
      diff_error[ichan][icol] = xml->NewElement(str);
      fit[ichan]->InsertEndChild(diff_error[ichan][icol]);
    }
  }
  xml->SaveFile(filename.c_str());
  delete xml;
}

//**********************************************************************
void wgEditXML::SUMMARY_SetGlobalConfigValue(const string& name, const int value, const bool create_new){
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* config = data->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());
  if( target ) {
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "%d", value );
    target->SetText(str);
  }
  else if( create_new == true ) {
    XMLElement* newElement = xml->NewElement(name.c_str());
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "%d", value );
    newElement->SetText(str);
    config->InsertEndChild(newElement);
  }
  else throw wgElementNotFound("Element " + name + " doesn't exist");
}

//**********************************************************************
void wgEditXML::SUMMARY_SetChConfigValue(const string& name, const int value, const int ichan, const bool create_new){
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "chan_%d", ichan );
  XMLElement* chan = data->FirstChildElement(str);
  XMLElement* config = chan->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());
  if( target ) {
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "%d", value );
    target->SetText(str);
  }
  else if( create_new == true ) {
    XMLElement* newElement = xml->NewElement(name.c_str());
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "%d", value );
    newElement->SetText(str);
    config->InsertEndChild(newElement);
  }
  else throw wgElementNotFound("Element " + name + " doesn't exist");
}

//**********************************************************************
void wgEditXML::SUMMARY_SetChFitValue(const string& name, const int value, const int ichan, const bool create_new){
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "chan_%d", ichan );
  XMLElement* chan = data->FirstChildElement(str);
  XMLElement* config = chan->FirstChildElement("fit");
  XMLElement* target = config->FirstChildElement(name.c_str());
  if( target ) {
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "%d", value );
    target->SetText(str);
  }
  else if( create_new == true ) {
    XMLElement* newElement = xml->NewElement(name.c_str());
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "%d", value );
    newElement->SetText(str);
    config->InsertEndChild(newElement);
  }
  else throw wgElementNotFound("Element " + name + " doesn't exist");
}

//**********************************************************************
void wgEditXML::SUMMARY_SetPedFitValue(int value[MEMDEPTH], const int ichan, const bool create_new){
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "chan_%d", ichan );
  XMLElement* chan = data->FirstChildElement(str);
  XMLElement* fit = chan->FirstChildElement("fit");
  for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
    unsigned icol_id = icol + 1;
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "ped_%d", icol_id );
    XMLElement* target = fit->FirstChildElement(str);
    if ( target ) {
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "%d", value[icol] );
      target->SetText(str);
    }else{
      if(create_new == true){
        snprintf( str, XML_ELEMENT_STRING_LENGTH, "ped_%d", icol_id );
        XMLElement* newElement = xml->NewElement(str);
        snprintf( str, XML_ELEMENT_STRING_LENGTH, "%d", value[icol] );
        newElement->SetText(str);
        fit->InsertEndChild(newElement); 
      }else{
        throw wgElementNotFound("Element ped_"+ to_string(icol) +" doesn't exist!");
      }
    }
  }  
}

//**********************************************************************
void wgEditXML::SUMMARY_AddGlobalElement(const string& name){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* newElement = xml->NewElement(name.c_str());
  data->InsertEndChild(newElement);
}

//**********************************************************************
void wgEditXML::SUMMARY_AddChElement(const string& name, const int ich){
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "chan_%d", ich );
  XMLElement* chan = data->FirstChildElement(str);
  XMLElement* newElement = xml->NewElement(name.c_str());
  chan->InsertEndChild(newElement);
}

//**********************************************************************
int wgEditXML::SUMMARY_GetGlobalConfigValue(const string& name){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* config = data->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());
  if ( target ) {
    string value = target->GetText();
    return atof(value.c_str());
  }else{
    throw wgElementNotFound("[SUMMARY_GetGlobalConfigValue] Element:" + name + " doesn't exist!");
    return -1.;
  }
}

//**********************************************************************
int wgEditXML::SUMMARY_GetChConfigValue(const string& name, const int ich){
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "chan_%d", ich );
  XMLElement* chan = data->FirstChildElement(str);
  XMLElement* config = chan->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());
  if ( target ) {
    string value = target->GetText();
    return atoi(value.c_str());
  }else{
    throw wgElementNotFound("[SUMMARY_GetChConfigValue] Element:" + name + " doesn't exist!");
    return -1.;
  }
}

//**********************************************************************
int wgEditXML::SUMMARY_GetChFitValue(const string& name, const int ich){
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "chan_%d", ich );
  XMLElement* chan = data->FirstChildElement(str);
  XMLElement* fit = chan->FirstChildElement("fit");
  XMLElement* target = fit->FirstChildElement(name.c_str());
  if ( target ) {
    string value = target->GetText();
    return atof(value.c_str());
  }else{
    throw wgElementNotFound("[SUMMARY_GetChFitValue] Element:" + name + " doesn't exist!");
    return -1.;
  }
}

//**********************************************************************
void wgEditXML::SUMMARY_GetPedFitValue(int value[MEMDEPTH], const int ich){
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "chan_%d", ich );
  XMLElement* chan = data->FirstChildElement(str);
  XMLElement* config = chan->FirstChildElement("fit");
  for(unsigned icol = 0; icol < MEMDEPTH; icol++){
    unsigned icol_id = icol + 1;
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "ped_%d", icol_id );
    XMLElement* target = config->FirstChildElement(str);
    if ( target ) {
      string temp_value = target->GetText();
      value[icol] = atof(temp_value.c_str());
    }else{
      throw wgElementNotFound("Element: ped" + to_string(icol) + " doesn't exist!");
    }
  }
}

//**********************************************************************
// void wgEditXML::SCURVE_Make(const string& filename){
//   xml = new XMLDocument();
//   XMLDeclaration* decl = xml->NewDeclaration();
//   xml->InsertEndChild(decl);
// 
//   XMLElement* data; 
//   //inputDAC
//   XMLElement* inputDAC[13];
//   //data
//   XMLElement* pe_center1[13];
//   XMLElement* pe_width1[13];
//   XMLElement* pe_center2[13];
//   XMLElement* pe_width2[13];
//   XMLElement* gain1[13];
//   XMLElement* gain2[13];
//   //XMLElement* noise[13];
// 
//   // **********************//
//   data = xml->NewElement("data");
//   xml->InsertEndChild(data);
// 
//   // ***** data > inputDAC ***** //
//   for(unsigned int iDAC=0;iDAC<13;iDAC++){
//     inputDAC[iDAC] = xml->NewElement(Form("inputDAC_%d",1+iDAC*20));    
//     data->InsertEndChild(inputDAC[iDAC]);
// 
//     // ***** data > inputDAC >calib data***** //
//     pe_center1[iDAC]= xml->NewElement("pe_center1");
//     inputDAC[iDAC]->InsertEndChild(pe_center1[iDAC]);
//     pe_width1[iDAC]= xml->NewElement("pe_width1");
//     inputDAC[iDAC]->InsertEndChild(pe_width1[iDAC]);
//     pe_center2[iDAC]= xml->NewElement("pe_center2");
//     inputDAC[iDAC]->InsertEndChild(pe_center2[iDAC]);
//     pe_width2[iDAC]= xml->NewElement("pe_width2");
//     inputDAC[iDAC]->InsertEndChild(pe_width2[iDAC]);
//     gain1[iDAC]= xml->NewElement("gain1");
//     inputDAC[iDAC]->InsertEndChild(gain1[iDAC]);
//     gain2[iDAC]= xml->NewElement("gain2");
//     inputDAC[iDAC]->InsertEndChild(gain2[iDAC]);
//   }
//   xml->SaveFile(filename.c_str());
//   delete xml;
// }

//**********************************************************************
// void wgEditXML::SCURVE_SetValue(const string& name,int iDAC,int value, bool create_new) {
//   XMLElement* data = xml->FirstChildElement("data");
//   XMLElement* inputDAC = data->FirstChildElement(Form("inputDAC_%d",iDAC));
//   XMLElement* target = inputDAC->FirstChildElement(name.c_str());
//   if ( target ) {
//     target->SetText(Form("%d",value));
//   }else{
//     if(create_new == true){
//       XMLElement* newElement = xml->NewElement(name.c_str());
//       newElement->SetText(Form("%d",value));
//       inputDAC->InsertEndChild(newElement);
//     }else{
//       throw wgElementNotFound("Element " + name + " doesn't exist");
//     }
//   }
// }

//**********************************************************************
// int wgEditXML::SCURVE_GetValue(const string& name,int iDAC){
//   XMLElement* data = xml->FirstChildElement("data");
//   XMLElement* inputDAC = data->FirstChildElement(Form("inputDAC_%d",iDAC));
//   XMLElement* target = inputDAC->FirstChildElement(name.c_str());
//   if ( target ) {
//     string value = target->GetText();
//     return atof(value.c_str());
//   }else{
//     throw wgElementNotFound("Element " + name + " doesn't exist");
//     return -1.;
//   }
// }

//**********************************************************************
void wgEditXML::OPT_Make(	const string& 						filename, 
													vector<unsigned> 					inputDACs;
													unsigned 									n_difs,
													vector<unsigned> 					n_chips,
													vector<vector<unsigned>> 	n_chans){
  
	xml = new XMLDocument();
  XMLDeclaration* decl = xml->NewDeclaration();
  xml->InsertEndChild(decl);

  XMLElement* data; 
  XMLElement* dif;
  XMLElement* chip;
  XMLElement* s_th[2];
  XMLElement* i_th[2];
  XMLElement* inputDAC;
  XMLElement* threshold;
  //XMLElement* noise[13];

  // **********************//
  data = xml->NewElement("data");
  xml->InsertEndChild(data);

  for(unsigned int idif=0;idif<n_difs;idif++){
    // ***** data > dif ***** //
		string dif_name = "dif_" + to_string(idif+1);
    dif = xml->NewElement(dif_name);    
    data->InsertEndChild(dif);
    for(unsigned int ichip=0;ichip<n_chips[idif];ichip++){
    	// ***** data > dif > chip ***** //
			string chip_name = "chip_" + to_string(ichip+1);
      chip = xml->NewElement(chip_name);    
      dif->InsertEndChild(chip);
    	for(unsigned int ichan=0;ichan<n_chans[idif][ichip];ichan++){
    		// ***** data > dif > chip > channel ***** //
				string chan_name = "chan_" + to_string(ichan);
    	  chan = xml->NewElement(chan_name);    
    	  dif->InsertEndChild(chan);
    	  s_th[0] = xml->NewElement("s_th1");    
    	  i_th[0] = xml->NewElement("i_th1");    
    	  s_th[1] = xml->NewElement("s_th2");    
    	  i_th[1] = xml->NewElement("i_th2");    
    	  chan->InsertEndChild(s_th[0]);
    	  chan->InsertEndChild(i_th[0]);
    	  chan->InsertEndChild(s_th[1]);
    	  chan->InsertEndChild(i_th[1]);
    	  for(unsigned int iDAC=0;iDAC<inputDACs.size();iDAC++){
    	    // ***** data > dif > chip > channel > inputDAC ***** //
					string inputDAC_name = "inputDAC_" + to_string(inputDACs[iDAC]);
    	    inputDAC = xml->NewElement(inputDAC_name);    
    	    chip->InsertEndChild(inputDAC);
    	    for(unsigned int pe=0;pe<2;pe++){
						string threshold_name = "threshold_" + to_string(pe+1);
    	      threshold = xml->NewElement(threshold_name);    
    	      inputDAC->InsertEndChild(threshold);
    	    }
    	  }
    	}
		}
  }
  xml->SaveFile(filename.c_str());
  delete xml;
}

//**********************************************************************
void wgEditXML::OPT_SetValue(	const string& name,
															int idif, 
															int ichip, 
															int ichan, 
															int iDAC, 
															double value, 
															bool create_new) {
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* chan = chip->FirstChildElement(Form("chan_%d",ichan));
  XMLElement* inputDAC = chan->FirstChildElement(Form("inputDAC_%d",iDAC));
  XMLElement* target = inputDAC->FirstChildElement(name.c_str());

  if ( target ) {
    target->SetText(Form("%f",value));
  }else{
    if(create_new == true){
      XMLElement* newElement = xml->NewElement(name.c_str());
      newElement->SetText(Form("%f",value));
      inputDAC->InsertEndChild(newElement);
    }else{
      throw wgElementNotFound("Element " + name + " doesn't exist");
    }
  }
}

//**********************************************************************
double wgEditXML::OPT_GetValue(const string& name,int idif, int ichip, int ichan, int iDAC) {
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* chan = chip->FirstChildElement(Form("chan_%d",ichan));
  XMLElement* inputDAC = chan->FirstChildElement(Form("inputDAC_%d",iDAC));
  XMLElement* target = inputDAC->FirstChildElement(name.c_str());
  if ( target ) {
    string value = target->GetText();
    return atof(value.c_str());
  }else{
    throw wgElementNotFound("Element " + name + " doesn't exist");
    return -1.;
  }
}

//**********************************************************************
void wgEditXML::OPT_SetChanValue(const string& name,int idif, int ichip, ,int ichan, double value, bool create_new){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* chan = chip->FirstChildElement(Form("chan_%d",ichan));
  XMLElement* target = chan->FirstChildElement(name.c_str());

  if ( target ) {
    target->SetText(Form("%f",value));
  }else{
    if(create_new == true){
      XMLElement* newElement = xml->NewElement(name.c_str());
      newElement->SetText(Form("%f",value));
      chip->InsertEndChild(newElement);
    }else{
      throw wgElementNotFound("Element " + name + " doesn't exist");
    }
  }
}

//**********************************************************************
double wgEditXML::OPT_GetChanValue(const string& name,int idif, int ichip, int ichan){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* chan = chip->FirstChildElement(Form("chan_%d",ichan));
  XMLElement* target = chan->FirstChildElement(name.c_str());
  if ( target ) {
    string value = target->GetText();
    return atof(value.c_str());
  }else{
    throw wgElementNotFound("Element " + name + " doesn't exist");
    return -1.;
  }
}

//**********************************************************************
void wgEditXML::PreCalib_Make(const string& filename){
  xml = new XMLDocument();
  XMLDeclaration* decl = xml->NewDeclaration();
  xml->InsertEndChild(decl);

  XMLElement* data; 
  XMLElement* dif;
  XMLElement* chip;
  XMLElement* chan;
  XMLElement* s_Gain;
  XMLElement* i_Gain;

  // **********************//
  data = xml->NewElement("data");
  xml->InsertEndChild(data);

  for(unsigned int idif=0;idif<2;idif++){
    // ***** data > dif ***** //
    dif = xml->NewElement(Form("dif_%d",idif+1));    
    data->InsertEndChild(dif);
    // ***** data > dif > chip ***** //
    for(unsigned ichip = 0; ichip < NCHIPS; ichip++) {
      chip = xml->NewElement(Form("chip_%d",ichip));    
      dif->InsertEndChild(chip);
      // ***** data > dif > chip > ch ***** //
      for(unsigned ichan = 0; ichan < NCHANNELS; ichan++) {
        chan = xml->NewElement(Form("chan_%d", ichan));    
        chip->InsertEndChild(chan);
        s_Gain = xml->NewElement("s_Gain");    
        i_Gain = xml->NewElement("i_Gain");    
        chan->InsertEndChild(s_Gain);
        chan->InsertEndChild(i_Gain);
      }
    }
  }
  xml->SaveFile(filename.c_str());
  delete xml;
}

//**********************************************************************
void wgEditXML::PreCalib_SetValue(const string& name, int idif, int ichip, int ich, int value, bool create_new) {
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* chan = chip->FirstChildElement(Form("chan_%d",ich));
  XMLElement* target = chan->FirstChildElement(name.c_str());

  if ( target ) {
    target->SetText(Form("%d",value));
  }else{
    if(create_new == true){
      XMLElement* newElement = xml->NewElement(name.c_str());
      newElement->SetText(Form("%d",value));
      chan->InsertEndChild(newElement);
    }else{
      throw wgElementNotFound("Element " + name + " doesn't exist");
    }
  }
}

//**********************************************************************
int wgEditXML::PreCalib_GetValue(const string& name,int idif, int ichip, int ich) {
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* chan   = chip->FirstChildElement(Form("chan_%d",ich));
  XMLElement* target = chan->FirstChildElement(name.c_str());
  if ( target ) {
    string value = target->GetText();
    return atof(value.c_str());
  }else{
    throw wgElementNotFound("Element " + name + " doesn't exist");
    return -1.;
  }
}

//**********************************************************************
void wgEditXML::Calib_Make(const string& filename, Topology& topol){
  xml = new XMLDocument();
  XMLDeclaration* decl = xml->NewDeclaration();
  xml->InsertEndChild(decl);

  XMLElement *data, *dif, *chip, *chan, *difs, *chips, *chans, *pe1, *pe2, *gain, *sigma_gain, *ped, *sigma_ped, *meas_ped, *sigma_meas_ped;
  char str[XML_ELEMENT_STRING_LENGTH];

  // **********************//
  data = xml->NewElement("data");
  xml->InsertEndChild(data);
  difs = xml->NewElement("n_difs");
  difs->SetText(to_string(topol.n_difs).c_str());
  data->InsertEndChild(difs);

  for(unsigned idif = 0; idif < topol.n_difs; idif++) {
    unsigned idif_id = idif + 1;
    // ***** data > dif ***** //
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "dif_%d", idif_id );
    dif = xml->NewElement(str);    
    data->InsertEndChild(dif);
    chips = xml->NewElement("n_chips");

    unsigned n_chips = topol.dif_map[idif].size();
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "%u", n_chips);
    chips->SetText(str);
    data->InsertEndChild(chips);
    // ***** data > dif > chip ***** //
    for(unsigned ichip = 0; ichip < n_chips; ichip++) {
      unsigned ichip_id = ichip + 1;
      chip = xml->NewElement(Form("chip_%d", ichip_id));    
      dif->InsertEndChild(chip);

      unsigned n_chans = topol.dif_map[idif][ichip];
      chans = xml->NewElement("n_chans");
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "%u", n_chans);
      chans->SetText(str);
      dif->InsertEndChild(chans);
      // ***** data > dif > chip > ch ***** //
      for(unsigned ichan = 0; ichan < n_chans; ichan++) {
        unsigned ichan_id = ichan + 1;
        chan = xml->NewElement(Form("chan_%d", ichan_id));    
        chip->InsertEndChild(chan);
        for(unsigned icol_id = 1; icol_id <= MEMDEPTH; icol_id++) {
          snprintf( str, XML_ELEMENT_STRING_LENGTH, "pe1_%d", icol_id );
          pe1 = xml->NewElement(str);
          chan->InsertEndChild(pe1);
        }
        for(unsigned icol_id = 1; icol_id <= MEMDEPTH; icol_id++) {
          snprintf( str, XML_ELEMENT_STRING_LENGTH, "pe2_%d", icol_id );
          pe2 = xml->NewElement(str);
          chan->InsertEndChild(pe2);
        }
        for(unsigned icol_id = 1; icol_id <= MEMDEPTH; icol_id++) {
          snprintf( str, XML_ELEMENT_STRING_LENGTH, "gain_%d", icol_id );
          gain = xml->NewElement(str);  
          chan->InsertEndChild(gain);
        }
        for(unsigned icol_id = 1; icol_id <= MEMDEPTH; icol_id++) {
          snprintf( str, XML_ELEMENT_STRING_LENGTH, "sigma_gain_%d", icol_id );
          sigma_gain = xml->NewElement(str);  
          chan->InsertEndChild(sigma_gain);
        }
        for(unsigned icol_id = 1; icol_id <= MEMDEPTH; icol_id++) {
          snprintf( str, XML_ELEMENT_STRING_LENGTH, "ped_%d", icol_id );
          ped = xml->NewElement(str);  
          chan->InsertEndChild(ped);
        }
        for(unsigned icol_id = 1; icol_id <= MEMDEPTH; icol_id++) {
          snprintf( str, XML_ELEMENT_STRING_LENGTH, "sigma_ped_%d", icol_id );
          sigma_ped = xml->NewElement(str);  
          chan->InsertEndChild(sigma_ped);
        }
        for(unsigned icol_id = 1; icol_id <= MEMDEPTH; icol_id++) {
          snprintf( str, XML_ELEMENT_STRING_LENGTH, "meas_ped_%d", icol_id );
          meas_ped = xml->NewElement(str);  
          chan->InsertEndChild(meas_ped);
        }
        for(unsigned icol_id = 1; icol_id <= MEMDEPTH; icol_id++) {
          snprintf( str, XML_ELEMENT_STRING_LENGTH, "sigma_meas_ped_%d", icol_id );
          sigma_meas_ped = xml->NewElement(str);  
          chan->InsertEndChild(sigma_meas_ped);
        }
      }
    }
  }
  xml->SaveFile(filename.c_str());
  delete xml;
}

//**********************************************************************
void wgEditXML::Calib_SetValue(const string& name, const int idif, const int ichip, const int ich, const int value, const bool create_new){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* chan = chip->FirstChildElement(Form("chan_%d",ich));
  XMLElement* target = chan->FirstChildElement(name.c_str());

  if ( target ) {
    target->SetText(Form("%d", value));
  }else{
    if(create_new == true){
      XMLElement* newElement = xml->NewElement(name.c_str());
      newElement->SetText(Form("%d", value));
      chan->InsertEndChild(newElement);
    }else{
      throw wgElementNotFound("Element " + name + " doesn't exist");
    }
  }
}

//**********************************************************************
int wgEditXML::Calib_GetValue(const string& name,int idif, int ichip, int ich){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* chan   = chip->FirstChildElement(Form("chan_%d",ich));
  XMLElement* target = chan->FirstChildElement(name.c_str());
  string value;
  if (target) {
    value = target->GetText();
  } else {
    throw wgElementNotFound("Element: " + name + " doesn't exist!");
  }
  return atof(value.c_str());
}



