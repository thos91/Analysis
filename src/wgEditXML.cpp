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
void wgEditXML::Make(const string& filename, const unsigned ichip, const unsigned ichan){
  char str[XML_ELEMENT_STRING_LENGTH];
  xml = new XMLDocument();
  XMLDeclaration* decl = xml->NewDeclaration();
  xml->InsertEndChild(decl);

  XMLElement* data = xml->NewElement("data");
  xml->InsertEndChild(data);

  XMLElement* config = data->GetDocument()->NewElement("config");
  data->InsertEndChild(config);

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

  XMLElement* ch = xml->NewElement("ch");
  data->InsertEndChild(ch);

  for(unsigned int k=0; k<MEMDEPTH; k++){
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "col_%d", k );
    XMLElement* col = xml->NewElement(str);
    ch->InsertEndChild(col);
  }
  xml->SaveFile(filename.c_str());
  delete xml;
}

//**********************************************************************
bool wgEditXML::GetConfig(const string& configxml, const unsigned idif, const unsigned ichip, const unsigned n_chans, vector<vector<int>>& v) {
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
    XMLElement* gdcc = acqpc->FirstChildElement("gdcc");
    // DIFs loop
    for(XMLElement* dif = gdcc->FirstChildElement("dif"); dif != NULL; dif = dif->NextSiblingElement("dif")) {
      if( string(dif->Attribute("name")) == "dif_1_1_" + to_string(idif) ) {
        // ASUs loop
        for(XMLElement* asu = dif->FirstChildElement("asu"); asu != NULL; asu = asu->NextSiblingElement("asu")) {
          if( string(asu->Attribute("name")) == "asu_1_1_" + to_string(idif) + "_" + to_string(ichip) ) {
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
void wgEditXML::SetColValue(const string& name, const int icol, const double value, const bool create_new) {
  if(icol<0 || icol>MEMDEPTH) return;
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement("ch");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "col_%d", icol );
  XMLElement* col = ch->FirstChildElement(str);
  XMLElement* target = col->FirstChildElement(name.c_str());
  if ( target ) {
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "%.2f", value );
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
void wgEditXML::SetChValue(const string& name, const double value, const bool create_new){
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement("ch");
  XMLElement* target = ch->FirstChildElement(name.c_str());
  if ( target ) {
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "%.2f", value );
    target->SetText(str);
  }else{
    if(create_new == true){
      XMLElement* newElement = xml->NewElement(name.c_str());
      newElement->SetText(value);
      ch->InsertEndChild(newElement); 
    }else{
      Log.eWrite("[wgEditXML::SetChValue] Warning! Element " + name + " doesn't exist!");
    }
  }  
}


//**********************************************************************
void wgEditXML::AddColElement(const string& name, const int icol) {
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement("ch");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "col_%d", icol );
  XMLElement* col = ch->FirstChildElement(str);
  XMLElement* newElement = xml->NewElement(name.c_str());
  col->InsertEndChild(newElement);
}

//**********************************************************************
void wgEditXML::AddChElement(const string& name) {
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement("ch");
  XMLElement* newElement = xml->NewElement(name.c_str());
  ch->InsertEndChild(newElement);
}

//**********************************************************************
double wgEditXML::GetColValue(const string& name,const int icol){
  if(icol<0 || icol>MEMDEPTH) return 0.0;
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement("ch");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "col_%d", icol );
  XMLElement* col = ch->FirstChildElement(str);
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
double wgEditXML::GetChValue(const string& name){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement("ch");
  XMLElement* target = ch->FirstChildElement(name.c_str());
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
  //col
  vector<array<XMLElement*, MEMDEPTH> > pedestal(n_chans);
  vector<array<XMLElement*, MEMDEPTH> > pedestal_error(n_chans);
  vector<array<XMLElement*, MEMDEPTH> > raw_charge(n_chans);
  vector<array<XMLElement*, MEMDEPTH> > raw_charge_error(n_chans);
  vector<array<XMLElement*, MEMDEPTH> > gain(n_chans);
  vector<array<XMLElement*, MEMDEPTH> > gain_error(n_chans);

  // ********************** //

  data = xml->NewElement("data");
  xml->InsertEndChild(data);

  g_config = xml->NewElement("config");
  data->InsertEndChild(g_config);
  start_time = xml->NewElement("start_time");
  g_config->InsertEndChild(start_time);
  stop_time = xml->NewElement("stop_time");
  g_config->InsertEndChild(stop_time);
  trigger_threshold = xml->NewElement("trigth");
  g_config->InsertEndChild(trigger_threshold);
  gs_threshold = xml->NewElement("gainth");
  g_config->InsertEndChild(gs_threshold); 

  for(unsigned ichan = 0; ichan < n_chans; ichan++) {
    // ***** data > ch ***** //
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "ch_%d", ichan );
    ch[ichan] = xml->NewElement(str);    
    data->InsertEndChild(ch[ichan]);

    // ***** data > ch > fit***** //
    fit[ichan] = xml->NewElement("fit");
    ch[ichan]->InsertEndChild(fit[ichan]);

    noise[ichan] = xml->NewElement("noise");
    fit[ichan]->InsertEndChild(noise[ichan]);
    enoise[ichan] = xml->NewElement("enoise");
    fit[ichan]->InsertEndChild(enoise[ichan]);
    pe_level[ichan] = xml->NewElement("pe_level");
    fit[ichan]->InsertEndChild(pe_level[ichan]);

    for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "ped_%d", icol );
      pedestal[ichan][icol] = xml->NewElement(str);
      fit[ichan]->InsertEndChild(pedestal[ichan][icol]);
    }
    for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "eped_%d", icol );
      pedestal_error[ichan][icol] = xml->NewElement(str);
      fit[ichan]->InsertEndChild(pedestal_error[ichan][icol]);
    }
    for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "raw_%d", icol );
      raw_charge[ichan][icol] = xml->NewElement(str);
      fit[ichan]->InsertEndChild(raw_charge[ichan][icol]);
    }
    for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "eraw_%d", icol );
      raw_charge_error[ichan][icol] = xml->NewElement(str);
      fit[ichan]->InsertEndChild(raw_charge_error[ichan][icol]);
    }
    for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "gain_%d", icol );
      gain[ichan][icol] = xml->NewElement(str);
      fit[ichan]->InsertEndChild(gain[ichan][icol]);
    }
    for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "egain_%d", icol );
      gain_error[ichan][icol] = xml->NewElement(str);
      fit[ichan]->InsertEndChild(gain_error[ichan][icol]);
    }

    // ***** data > ch > config ***** //
    config[ichan] = xml->NewElement("config");
    ch[ichan]->InsertEndChild(config[ichan]);

    inputDAC[ichan] = xml->NewElement("inputDAC");
    config[ichan]->InsertEndChild(inputDAC[ichan]);
    ampDAC[ichan] = xml->NewElement("ampDAC");
    config[ichan]->InsertEndChild(ampDAC[ichan]);
    threshold_adjust[ichan] = xml->NewElement("adjDAC");
    config[ichan]->InsertEndChild(threshold_adjust[ichan]);

    // ***** data > ch > col ***** //
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
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "ch_%d", ichan );
  XMLElement* ch = data->FirstChildElement(str);
  XMLElement* config = ch->FirstChildElement("config");
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
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "ch_%d", ichan );
  XMLElement* ch = data->FirstChildElement(str);
  XMLElement* config = ch->FirstChildElement("fit");
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
void wgEditXML::SUMMARY_SetPedFitValue(double value[MEMDEPTH], const int ichan, const bool create_new){
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "ch_%d", ichan );
  XMLElement* ch = data->FirstChildElement(str);
  XMLElement* fit = ch->FirstChildElement("fit");
  for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "ped_%d", icol );
    XMLElement* target = fit->FirstChildElement(str);
    if ( target ) {
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "%.2f", value[icol] );
      target->SetText(str);
    }else{
      if(create_new == true){
        snprintf( str, XML_ELEMENT_STRING_LENGTH, "ped_%d", icol );
        XMLElement* newElement = xml->NewElement(str);
        snprintf( str, XML_ELEMENT_STRING_LENGTH, "%.2f", value[icol] );
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
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "ch_%d", ich );
  XMLElement* ch = data->FirstChildElement(str);
  XMLElement* newElement = xml->NewElement(name.c_str());
  ch->InsertEndChild(newElement);
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
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "ch_%d", ich );
  XMLElement* ch = data->FirstChildElement(str);
  XMLElement* config = ch->FirstChildElement("config");
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
double wgEditXML::SUMMARY_GetChFitValue(const string& name, const int ich){
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "ch_%d", ich );
  XMLElement* ch = data->FirstChildElement(str);
  XMLElement* fit = ch->FirstChildElement("fit");
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
void wgEditXML::SUMMARY_GetPedFitValue(double value[MEMDEPTH], const int ich){
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "ch_%d", ich );
  XMLElement* ch = data->FirstChildElement(str);
  XMLElement* config = ch->FirstChildElement("fit");
  for(unsigned icol = 0; icol < MEMDEPTH; icol++){
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "ped_%d", icol );
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
void wgEditXML::SCURVE_Make(const string& filename){
  xml = new XMLDocument();
  XMLDeclaration* decl = xml->NewDeclaration();
  xml->InsertEndChild(decl);

  XMLElement* data; 
  //inputDAC
  XMLElement* inputDAC[13];
  //data
  XMLElement* pe_center1[13];
  XMLElement* pe_width1[13];
  XMLElement* pe_center2[13];
  XMLElement* pe_width2[13];
  XMLElement* gain1[13];
  XMLElement* gain2[13];
  //XMLElement* noise[13];

  // **********************//
  data = xml->NewElement("data");
  xml->InsertEndChild(data);

  // ***** data > inputDAC ***** //
  for(unsigned int iDAC=0;iDAC<13;iDAC++){
    inputDAC[iDAC] = xml->NewElement(Form("inputDAC_%d",1+iDAC*20));    
    data->InsertEndChild(inputDAC[iDAC]);

    // ***** data > inputDAC >calib data***** //
    pe_center1[iDAC]= xml->NewElement("pe_center1");
    inputDAC[iDAC]->InsertEndChild(pe_center1[iDAC]);
    pe_width1[iDAC]= xml->NewElement("pe_width1");
    inputDAC[iDAC]->InsertEndChild(pe_width1[iDAC]);
    pe_center2[iDAC]= xml->NewElement("pe_center2");
    inputDAC[iDAC]->InsertEndChild(pe_center2[iDAC]);
    pe_width2[iDAC]= xml->NewElement("pe_width2");
    inputDAC[iDAC]->InsertEndChild(pe_width2[iDAC]);
    gain1[iDAC]= xml->NewElement("gain1");
    inputDAC[iDAC]->InsertEndChild(gain1[iDAC]);
    gain2[iDAC]= xml->NewElement("gain2");
    inputDAC[iDAC]->InsertEndChild(gain2[iDAC]);
  }
  xml->SaveFile(filename.c_str());
  delete xml;
}

//**********************************************************************
void wgEditXML::SCURVE_SetValue(const string& name,int iDAC,double value, bool create_new) {
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* inputDAC = data->FirstChildElement(Form("inputDAC_%d",iDAC));
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
double wgEditXML::SCURVE_GetValue(const string& name,int iDAC){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* inputDAC = data->FirstChildElement(Form("inputDAC_%d",iDAC));
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
void wgEditXML::OPT_Make(const string& filename){
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

  for(unsigned int idif=0;idif<2;idif++){
    // ***** data > dif ***** //
    dif = xml->NewElement(Form("dif_%d",idif+1));    
    data->InsertEndChild(dif);
    // ***** data > dif > chip ***** //
    for(unsigned int i=0;i<NCHIPS;i++){
      int ichip=i;
      chip = xml->NewElement(Form("chip_%d",ichip));    
      dif->InsertEndChild(chip);
      s_th[0] = xml->NewElement("s_th1");    
      i_th[0] = xml->NewElement("i_th1");    
      s_th[1] = xml->NewElement("s_th2");    
      i_th[1] = xml->NewElement("i_th2");    
      chip->InsertEndChild(s_th[0]);
      chip->InsertEndChild(i_th[0]);
      chip->InsertEndChild(s_th[1]);
      chip->InsertEndChild(i_th[1]);
      for(unsigned int iDAC=0;iDAC<13;iDAC++){
        // ***** data > dif > chip > inputDAC ***** //
        inputDAC = xml->NewElement(Form("inputDAC_%d",iDAC*20+1));    
        chip->InsertEndChild(inputDAC);
        for(unsigned int pe=0;pe<2;pe++){
          threshold = xml->NewElement(Form("threshold_%d",pe+1));    
          inputDAC->InsertEndChild(threshold);
        }
      }
    }
  }
  xml->SaveFile(filename.c_str());
  delete xml;
}

//**********************************************************************
void wgEditXML::OPT_SetValue(const string& name,int idif, int ichip, int iDAC, double value, bool create_new) {
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* inputDAC = chip->FirstChildElement(Form("inputDAC_%d",iDAC));
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
double wgEditXML::OPT_GetValue(const string& name,int idif, int ichip, int iDAC) {
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* inputDAC = chip->FirstChildElement(Form("inputDAC_%d",iDAC));
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
void wgEditXML::OPT_SetChipValue(const string& name,int idif, int ichip, double value, bool create_new){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* target = chip->FirstChildElement(name.c_str());

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
double wgEditXML::OPT_GetChipValue(const string& name,int idif, int ichip){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* target = chip->FirstChildElement(name.c_str());
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
  XMLElement* ch;
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
        ch = xml->NewElement(Form("ch_%d", ichan));    
        chip->InsertEndChild(ch);
        s_Gain = xml->NewElement("s_Gain");    
        i_Gain = xml->NewElement("i_Gain");    
        ch->InsertEndChild(s_Gain);
        ch->InsertEndChild(i_Gain);
      }
    }
  }
  xml->SaveFile(filename.c_str());
  delete xml;
}

//**********************************************************************
void wgEditXML::PreCalib_SetValue(const string& name, int idif, int ichip, int ich, double value, bool create_new) {
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* ch = chip->FirstChildElement(Form("ch_%d",ich));
  XMLElement* target = ch->FirstChildElement(name.c_str());

  if ( target ) {
    target->SetText(Form("%f",value));
  }else{
    if(create_new == true){
      XMLElement* newElement = xml->NewElement(name.c_str());
      newElement->SetText(Form("%f",value));
      ch->InsertEndChild(newElement);
    }else{
      throw wgElementNotFound("Element " + name + " doesn't exist");
    }
  }
}

//**********************************************************************
double wgEditXML::PreCalib_GetValue(const string& name,int idif, int ichip, int ich) {
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* ch   = chip->FirstChildElement(Form("ch_%d",ich));
  XMLElement* target = ch->FirstChildElement(name.c_str());
  if ( target ) {
    string value = target->GetText();
    return atof(value.c_str());
  }else{
    throw wgElementNotFound("Element " + name + " doesn't exist");
    return -1.;
  }
}

//**********************************************************************
void wgEditXML::Calib_Make(const string& filename, const unsigned n_difs, const unsigned n_chips, const unsigned n_chans){
  xml = new XMLDocument();
  XMLDeclaration* decl = xml->NewDeclaration();
  xml->InsertEndChild(decl);

  XMLElement* data; 
  XMLElement* dif;
  XMLElement* chip;
  XMLElement* ch;
  XMLElement* pe1;
  XMLElement* pe2;
  XMLElement* gain;
  XMLElement* ped;
  XMLElement* ped_nohit;
  char str[XML_ELEMENT_STRING_LENGTH];

  // **********************//
  data = xml->NewElement("data");
  xml->InsertEndChild(data);
  XMLElement* difs = xml->NewElement("n_difs");
  difs->SetText(to_string(n_difs).c_str());
  data->InsertEndChild(difs);
  XMLElement* chips = xml->NewElement("n_chips");
  chips->SetText(to_string(n_chips).c_str());
  data->InsertEndChild(chips);
  XMLElement* chans = xml->NewElement("n_chans");
  chans->SetText(to_string(n_chans).c_str());
  data->InsertEndChild(chans);

  for(unsigned idif = 0; idif < n_difs; idif++) {
    // ***** data > dif ***** //
    dif = xml->NewElement(Form("dif_%d",idif+1));    
    data->InsertEndChild(dif);
    // ***** data > dif > chip ***** //
    for(unsigned ichip = 0; ichip < n_chips; ichip++) {
      chip = xml->NewElement(Form("chip_%d",ichip));    
      dif->InsertEndChild(chip);
      // ***** data > dif > chip > ch ***** //
      for(unsigned ichan = 0; ichan < n_chans; ichan++) {
        ch = xml->NewElement(Form("ch_%d",ichan));    
        chip->InsertEndChild(ch);
        for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
          snprintf( str, XML_ELEMENT_STRING_LENGTH, "pe1_%d", icol );
          pe1 = xml->NewElement(str);
          ch->InsertEndChild(pe1);
        }
        for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
          snprintf( str, XML_ELEMENT_STRING_LENGTH, "pe2_%d", icol );
          pe2 = xml->NewElement(str);
          ch->InsertEndChild(pe2);
        }
        for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
          snprintf( str, XML_ELEMENT_STRING_LENGTH, "gain_%d", icol );
          gain = xml->NewElement(str);  
          ch->InsertEndChild(gain);
        }
        for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
          snprintf( str, XML_ELEMENT_STRING_LENGTH, "ped_%d", icol );
          ped = xml->NewElement(str);  
          ch->InsertEndChild(ped);
        }
        for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
          snprintf( str, XML_ELEMENT_STRING_LENGTH, "ped_nohit_%d", icol );
          ped_nohit = xml->NewElement(str);  
          ch->InsertEndChild(ped_nohit);
        }		
      }
    }
  }
  xml->SaveFile(filename.c_str());
  delete xml;
}

//**********************************************************************
void wgEditXML::Calib_SetValue(const string& name, const int idif, const int ichip, const int ich, const double value, const bool create_new){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* ch = chip->FirstChildElement(Form("ch_%d",ich));
  XMLElement* target = ch->FirstChildElement(name.c_str());

  if ( target ) {
    target->SetText(Form("%f",value));
  }else{
    if(create_new == true){
      XMLElement* newElement = xml->NewElement(name.c_str());
      newElement->SetText(Form("%f",value));
      ch->InsertEndChild(newElement);
    }else{
      throw wgElementNotFound("Element " + name + " doesn't exist");
    }
  }
}

//**********************************************************************
double wgEditXML::Calib_GetValue(const string& name,int idif, int ichip, int ich){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* ch   = chip->FirstChildElement(Form("ch_%d",ich));
  XMLElement* target = ch->FirstChildElement(name.c_str());
  string value;
  if (target) {
    value = target->GetText();
  } else {
    throw wgElementNotFound("Element: " + name + " doesn't exist!");
  }
  return atof(value.c_str());
}



