// system C++ includes
#include <iostream>
#include <string>
#include <vector>

// tinyxml2 includes
#include "tinyxml2.hpp"

// user includes
#include "wgConst.hpp"
#include "wgEditConfig.hpp"
#include "wgExceptions.hpp"
#include "wgFileSystemTools.hpp"
#include "wgTopology.hpp"
#include "wgLogger.hpp"
#include "wgEditXML.hpp"

//**********************************************************************
void wgEditXML::Write(){
  xml->SaveFile(wgEditXML::filename.c_str());
}

//**********************************************************************
void wgEditXML::Open(const std::string& filename){
  
  if(!wagasci_tools::check_exist::XmlFile(filename))
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
void wgEditXML::Make(const std::string& filename, const unsigned idif, const unsigned ichip, const unsigned ichan){
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

  for(unsigned k = 0; k < MEMDEPTH; k++){
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "col_%d", k );
    XMLElement* col = xml->NewElement(str);
    chan->InsertEndChild(col);
  }
  xml->SaveFile(filename.c_str());
  delete xml;
}

//**********************************************************************
bool wgEditXML::GetConfig(const std::string& configxml,
                          const unsigned igdcc,
                          const unsigned idif,
                          const unsigned ichip,
                          const unsigned n_chans,
                          i2vector& v) {
  try {
    bool found=false;
    std::string bitstream("");
    
    if(!wagasci_tools::check_exist::XmlFile(configxml))
      throw wgInvalidFile(configxml + " wasn't found or is not valid");
    if(ichip > NCHIPS)
      throw std::invalid_argument("ichip is greater than " + std::to_string(NCHIPS));

    XMLDocument configfile;
    configfile.LoadFile(configxml.c_str()); 
    XMLElement* ecal = configfile.FirstChildElement("ecal");
    XMLElement* domain = ecal->FirstChildElement("domain");
    XMLElement* acqpc = domain->FirstChildElement("acqpc");
    // GDCCs loop
    for(XMLElement* gdcc = acqpc->FirstChildElement("gdcc"); gdcc != NULL; gdcc = gdcc->NextSiblingElement("gdcc")) {
      if( std::string(gdcc->Attribute("name")) == "gdcc_0_" + std::to_string(igdcc) ) {
        // DIFs loop
        for(XMLElement* dif = gdcc->FirstChildElement("dif"); dif != NULL; dif = dif->NextSiblingElement("dif")) {
          if( std::string(dif->Attribute("name")) == "dif_0_" + std::to_string(igdcc) + "_" + std::to_string(idif) ) {
            // ASUs loop
            for(XMLElement* asu = dif->FirstChildElement("asu"); asu != NULL; asu = asu->NextSiblingElement("asu")) {
              if( std::string(asu->Attribute("name")) == "asu_0_" + std::to_string(igdcc) + "_" + std::to_string(idif) + "_" + std::to_string(ichip) ) {
                XMLElement* spiroc2d = asu->FirstChildElement("spiroc2d");
                // loop to find the spiroc2d_bitstream parameter
                for(XMLElement* param = spiroc2d->FirstChildElement("param"); param != NULL; param = param->NextSiblingElement("param")) {
                  if( std::string(param->Attribute("name")) == "spiroc2d_bitstream" ) {
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
  catch (const std::exception& e) {
    Log.eWrite("[" + configxml + "][GetConfig] failed to get spiroc2d_bitstream (DIF = " + std::to_string(idif) + ", chip = " + std::to_string(ichip) + " : " + std::string(e.what()));
    return false;
  }
  return true;
}

//**********************************************************************
void wgEditXML::GetLog(const std::string& filename, std::vector<std::string>& v){
  std::string target("");
  v.clear();
  v.resize(4);
  XMLDocument *xml = new XMLDocument();
  xml->LoadFile(filename.c_str()); 
  XMLElement* log = xml->FirstChildElement("log");
  XMLElement* acq  = log->FirstChildElement("acq");
  for (XMLElement* param = acq->FirstChildElement("param");
       param != NULL;
       param = param->NextSiblingElement("param")) {
    target = param->Attribute("name");
    if (target == "start_time") {
      v[0] = param->GetText();
    } else if (target == "stop_time") {
      v[1] = param->GetText();
    }
    else if (target == "nb_data_pkts") { 
      v[2] = param->GetText();
    }
    else if (target == "nb_lost_pkts") {
      v[3] = param->GetText();
    }
  }
  delete xml;
}

//**********************************************************************
void wgEditXML::SetConfigValue(const std::string& name, const int value, const bool create_new) {
  XMLElement* data   = xml->FirstChildElement("data");
  XMLElement* config = data->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());
  if (target) {
    target->SetText(std::to_string(value).c_str());
  }
  else if(create_new == true) {
    XMLElement* newElement = xml->NewElement(name.c_str());
    newElement->SetText(value);
    config->InsertEndChild(newElement);      
  }
  else throw wgElementNotFound("Element " + name + " doesn't exist");
}

//**********************************************************************
void wgEditXML::SetColValue(const std::string& name, const int icol, const int value, const bool create_new) {
  if (icol < 0 || (unsigned) icol > MEMDEPTH) return;
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
void wgEditXML::SetChValue(const std::string& name, const int value, const bool create_new){
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
void wgEditXML::AddColElement(const std::string& name, const int icol) {
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* chan = data->FirstChildElement("chan");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "col_%d", icol );
  XMLElement* col = chan->FirstChildElement(str);
  XMLElement* newElement = xml->NewElement(name.c_str());
  col->InsertEndChild(newElement);
}

//**********************************************************************
void wgEditXML::AddChElement(const std::string& name) {
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* chan = data->FirstChildElement("chan");
  XMLElement* newElement = xml->NewElement(name.c_str());
  chan->InsertEndChild(newElement);
}

//**********************************************************************
int wgEditXML::GetColValue(const std::string& name, const int icol){
  if(icol < 0 || (unsigned) icol > MEMDEPTH) return 0.0;
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* chan = data->FirstChildElement("chan");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "col_%d", icol );
  XMLElement* col = chan->FirstChildElement(str);
  XMLElement* target = col->FirstChildElement(name.c_str());
  if ( target ) {
    std::string value = target->GetText();
    return stoi(value);
  }else{
    throw wgElementNotFound("Element " + name + " doesn't exist");
    return -1.;
  }
}

//**********************************************************************
int wgEditXML::GetChValue(const std::string& name){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* chan = data->FirstChildElement("chan");
  XMLElement* target = chan->FirstChildElement(name.c_str());
  if ( target ) {
    std::string value = target->GetText();
    return stoi(value);
  }else{
    throw wgElementNotFound("Element " + name + " doesn't exist");
    return -1.;
  }
}

//**********************************************************************
int wgEditXML::GetConfigValue(const std::string& name){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* config = data->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());
  if ( target ) {
    std::string value = target->GetText();
    return atoi(value.c_str());
  }else{
    throw wgElementNotFound("Element " + name + " doesn't exist");
    return -1;
  }
}

//**********************************************************************
void wgEditXML::SUMMARY_Make(const std::string& filename, const unsigned n_chans) {
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
  std::vector<XMLElement*> ch              (n_chans);
  std::vector<XMLElement*> fit             (n_chans);
  std::vector<XMLElement*> noise           (n_chans);
  std::vector<XMLElement*> enoise          (n_chans);
  std::vector<XMLElement*> pe_level        (n_chans);
  std::vector<XMLElement*> config          (n_chans);
  std::vector<XMLElement*> inputDAC        (n_chans);
  std::vector<XMLElement*> ampDAC          (n_chans);
  std::vector<XMLElement*> threshold_adjust(n_chans);
  std::vector<XMLElement*> chanid          (n_chans);
  //col
  std::vector<std::array<XMLElement*, MEMDEPTH> > pedestal(n_chans);
  std::vector<std::array<XMLElement*, MEMDEPTH> > pedestal_error(n_chans);
  std::vector<std::array<XMLElement*, MEMDEPTH> > charge_hit(n_chans);
  std::vector<std::array<XMLElement*, MEMDEPTH> > charge_hit_error(n_chans);
  std::vector<std::array<XMLElement*, MEMDEPTH> > diff(n_chans);
  std::vector<std::array<XMLElement*, MEMDEPTH> > diff_error(n_chans);

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


  for(unsigned ichan = 0; ichan < n_chans; ++ichan) {
    // ***** data > ch ***** //
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "chan_%d", ichan );
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

    for(unsigned icol = 0; icol < MEMDEPTH; ++icol) {
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "charge_nohit_%d", icol );
      pedestal[ichan][icol] = xml->NewElement(str);
      fit[ichan]->InsertEndChild(pedestal[ichan][icol]);
    }
    for(unsigned icol = 0; icol < MEMDEPTH; ++icol) {
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "sigma_nohit_%d", icol );
      pedestal_error[ichan][icol] = xml->NewElement(str);
      fit[ichan]->InsertEndChild(pedestal_error[ichan][icol]);
    }
    for(unsigned icol = 0; icol < MEMDEPTH; ++icol) {
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "charge_hit_%d", icol );
      charge_hit[ichan][icol] = xml->NewElement(str);
      fit[ichan]->InsertEndChild(charge_hit[ichan][icol]);
    }
    for(unsigned icol = 0; icol < MEMDEPTH; ++icol) {
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "sigma_hit_%d", icol );
      charge_hit_error[ichan][icol] = xml->NewElement(str);
      fit[ichan]->InsertEndChild(charge_hit_error[ichan][icol]);
    }
    for(unsigned icol = 0; icol < MEMDEPTH; ++icol) {
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "diff_%d", icol );
      diff[ichan][icol] = xml->NewElement(str);
      fit[ichan]->InsertEndChild(diff[ichan][icol]);
    }
    for(unsigned icol = 0; icol < MEMDEPTH; ++icol) {
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "sigma_diff_%d", icol );
      diff_error[ichan][icol] = xml->NewElement(str);
      fit[ichan]->InsertEndChild(diff_error[ichan][icol]);
    }
  }
  xml->SaveFile(filename.c_str());
  delete xml;
}

//**********************************************************************
void wgEditXML::SUMMARY_SetGlobalConfigValue(const std::string& name, const int value, const bool create_new){
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
void wgEditXML::SUMMARY_SetChConfigValue(const std::string& name, const int value, const int ichan, const bool create_new){
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
void wgEditXML::SUMMARY_SetChFitValue(const std::string& name, const int value, const int ichan, const bool create_new){
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
  for(unsigned icol = 0; icol < MEMDEPTH; ++icol) {
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "ped_%d", icol );
    XMLElement* target = fit->FirstChildElement(str);
    if ( target ) {
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "%d", value[icol] );
      target->SetText(str);
    }else{
      if(create_new == true){
        snprintf( str, XML_ELEMENT_STRING_LENGTH, "ped_%d", icol );
        XMLElement* newElement = xml->NewElement(str);
        snprintf( str, XML_ELEMENT_STRING_LENGTH, "%d", value[icol] );
        newElement->SetText(str);
        fit->InsertEndChild(newElement); 
      }else{
        throw wgElementNotFound("Element ped_"+ std::to_string(icol) +" doesn't exist!");
      }
    }
  }  
}

//**********************************************************************
void wgEditXML::SUMMARY_AddGlobalElement(const std::string& name){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* newElement = xml->NewElement(name.c_str());
  data->InsertEndChild(newElement);
}

//**********************************************************************
void wgEditXML::SUMMARY_AddChElement(const std::string& name, const int ich){
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "chan_%d", ich );
  XMLElement* chan = data->FirstChildElement(str);
  XMLElement* newElement = xml->NewElement(name.c_str());
  chan->InsertEndChild(newElement);
}

//**********************************************************************
int wgEditXML::SUMMARY_GetGlobalConfigValue(const std::string& name){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* config = data->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());
  if ( target ) {
    std::string value = target->GetText();
    return stoi(value);
  }else{
    throw wgElementNotFound("[SUMMARY_GetGlobalConfigValue] Element:" + name + " doesn't exist!");
    return -1.;
  }
}

//**********************************************************************
int wgEditXML::SUMMARY_GetChConfigValue(const std::string& name, const int ich){
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "chan_%d", ich );
  XMLElement* chan = data->FirstChildElement(str);
  XMLElement* config = chan->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());
  if ( target ) {
    std::string value = target->GetText();
    return atoi(value.c_str());
  }else{
    throw wgElementNotFound("[SUMMARY_GetChConfigValue] Element:" + name + " doesn't exist!");
    return -1.;
  }
}

//**********************************************************************
int wgEditXML::SUMMARY_GetChFitValue(const std::string& name, const int ich){
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "chan_%d", ich );
  XMLElement* chan = data->FirstChildElement(str);
  XMLElement* fit = chan->FirstChildElement("fit");
  XMLElement* target = fit->FirstChildElement(name.c_str());
  if ( target ) {
    std::string value = target->GetText();
    return stoi(value);
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
  for(unsigned icol = 0; icol < MEMDEPTH; ++icol){
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "ped_%d", icol );
    XMLElement* target = config->FirstChildElement(str);
    if ( target ) {
      std::string temp_value = target->GetText();
      value[icol] = atof(temp_value.c_str());
    }else{
      throw wgElementNotFound("Element: ped" + std::to_string(icol) + " doesn't exist!");
    }
  }
}

//**********************************************************************
void wgEditXML::OPT_Make(const std::string& filename, 
                         u1vector inputDACs,
                         unsigned n_difs,
                         u1vector n_chips,
                         u2vector n_chans) {

  xml = new XMLDocument();
  XMLDeclaration* decl = xml->NewDeclaration();
  xml->InsertEndChild(decl);
  char str[XML_ELEMENT_STRING_LENGTH];
  
  XMLElement* data; 
  XMLElement* dif;
  XMLElement* chip;
  XMLElement* chan;
  XMLElement* slope_threshold[2];
  XMLElement* intercept_threshold[2];
  XMLElement* inputDAC;
  XMLElement* threshold;
  //XMLElement* noise[13];

  // **********************//
  data = xml->NewElement("data");
  xml->InsertEndChild(data);

  for(unsigned idif = 0; idif < n_difs; ++idif) {
    // ***** data > dif ***** //
    snprintf(str, XML_ELEMENT_STRING_LENGTH, "dif_%d", idif);
    dif = xml->NewElement(str);
    data->InsertEndChild(dif);
    for(unsigned ichip = 0; ichip < n_chips[idif]; ++ichip) {
      // ***** data > dif > chip ***** //
      snprintf(str, XML_ELEMENT_STRING_LENGTH, "chip_%d", ichip);
      chip = xml->NewElement(str);
      dif->InsertEndChild(chip);
      for(unsigned ichan = 0; ichan < n_chans[idif][ichip]; ++ichan) {
        // ***** data > dif > chip > channel ***** //
        snprintf(str, XML_ELEMENT_STRING_LENGTH, "chan_%d", ichan);
        chan = xml->NewElement(str);    
        dif->InsertEndChild(chan);
        slope_threshold[0] = xml->NewElement("slope_threshold1");
        chan->InsertEndChild(slope_threshold[0]);
        intercept_threshold[0] = xml->NewElement("intercept_threshold1");
        chan->InsertEndChild(intercept_threshold[0]);
        slope_threshold[1] = xml->NewElement("slope_threshold2");
        chan->InsertEndChild(slope_threshold[1]);
        intercept_threshold[1] = xml->NewElement("intercept_threshold2");
        chan->InsertEndChild(intercept_threshold[1]);
        for(unsigned iDAC = 0; iDAC < inputDACs.size(); ++iDAC) {
          // ***** data > dif > chip > channel > inputDAC ***** //
          snprintf(str, XML_ELEMENT_STRING_LENGTH, "inputDAC_%d", inputDACs[iDAC]);
          inputDAC = xml->NewElement(str);
          chip->InsertEndChild(inputDAC);
          for(unsigned pe = 1; pe <= 2; ++pe) {
            snprintf(str, XML_ELEMENT_STRING_LENGTH, "threshold_%d", pe);
            threshold = xml->NewElement(str);
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
void wgEditXML::OPT_SetValue(const std::string& name,
                             unsigned idif, 
                             unsigned ichip, 
                             unsigned ichan, 
                             unsigned iDAC, 
                             double value, 
                             bool create_new) {
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf(str, XML_ELEMENT_STRING_LENGTH, "dif_%d", idif);
  XMLElement* dif  = data->FirstChildElement(str);
  snprintf(str, XML_ELEMENT_STRING_LENGTH, "chip_%d", ichip);
  XMLElement* chip = dif->FirstChildElement(str);
  snprintf(str, XML_ELEMENT_STRING_LENGTH, "chan_%d", ichan);
  XMLElement* chan = chip->FirstChildElement(str);
  snprintf(str, XML_ELEMENT_STRING_LENGTH, "inputDAC_%d", iDAC);
  XMLElement* inputDAC = chan->FirstChildElement(str);
  XMLElement* target = inputDAC->FirstChildElement(name.c_str());

  snprintf(str, XML_ELEMENT_STRING_LENGTH, "%f", value);
  if (target) {
    target->SetText(str);
  } else if (create_new == true){
    XMLElement* newElement = xml->NewElement(name.c_str());
    newElement->SetText(str);
    inputDAC->InsertEndChild(newElement);
  } else {
    throw wgElementNotFound("Element " + name + " doesn't exist");
  }
}

//**********************************************************************
double wgEditXML::OPT_GetValue(const std::string& name,unsigned idif, unsigned ichip, unsigned ichan, unsigned iDAC) {
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf(str, XML_ELEMENT_STRING_LENGTH, "dif_%d", idif);
  XMLElement* dif  = data->FirstChildElement(str);
  snprintf(str, XML_ELEMENT_STRING_LENGTH, "chip_%d", ichip);
  XMLElement* chip = dif->FirstChildElement(str);
  snprintf(str, XML_ELEMENT_STRING_LENGTH, "chan_%d", ichan);
  XMLElement* chan = chip->FirstChildElement(str);
  snprintf(str, XML_ELEMENT_STRING_LENGTH, "inputDAC_%d", iDAC);
  XMLElement* inputDAC = chan->FirstChildElement(str);
  XMLElement* target = inputDAC->FirstChildElement(name.c_str());
  if (target) {
    std::string value = target->GetText();
    return stoi(value);
  } else {
    throw wgElementNotFound("Element " + name + " doesn't exist");
    return -1;
  }
}

//**********************************************************************
void wgEditXML::OPT_SetChanValue(const std::string& name,unsigned idif, unsigned ichip, unsigned ichan, double value, bool create_new) {
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf(str, XML_ELEMENT_STRING_LENGTH, "dif_%d", idif);
  XMLElement* dif  = data->FirstChildElement(str);
  snprintf(str, XML_ELEMENT_STRING_LENGTH, "chip_%d", ichip);
  XMLElement* chip = dif->FirstChildElement(str);
  snprintf(str, XML_ELEMENT_STRING_LENGTH, "chan_%d", ichan);
  XMLElement* chan = chip->FirstChildElement(str);
  XMLElement* target = chan->FirstChildElement(name.c_str());

  snprintf(str, XML_ELEMENT_STRING_LENGTH, "%f", value);
  if (target) {
    target->SetText(str);
  } else if(create_new == true){
      XMLElement* newElement = xml->NewElement(name.c_str());
      newElement->SetText(str);
      chip->InsertEndChild(newElement);
  } else {
    throw wgElementNotFound("Element " + name + " doesn't exist");
  }
}

//**********************************************************************
double wgEditXML::OPT_GetChanValue(const std::string& name,unsigned idif, unsigned ichip, unsigned ichan) {
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf(str, XML_ELEMENT_STRING_LENGTH, "dif_%d", idif);
  XMLElement* dif  = data->FirstChildElement(str);
  snprintf(str, XML_ELEMENT_STRING_LENGTH, "chip_%d", ichip);
  XMLElement* chip = dif->FirstChildElement(str);
  snprintf(str, XML_ELEMENT_STRING_LENGTH, "chan_%d", ichan);
  XMLElement* chan = chip->FirstChildElement(str);
  XMLElement* target = chan->FirstChildElement(name.c_str());
  if (target) {
    std::string value = target->GetText();
    return stoi(value);
  } else {
    throw wgElementNotFound("Element " + name + " doesn't exist");
    return -1;
  }
}

//**********************************************************************
void wgEditXML::GainCalib_Make(const std::string& filename, const Topology& topol) {
  xml = new XMLDocument();
  XMLDeclaration* decl = xml->NewDeclaration();
  xml->InsertEndChild(decl);

  XMLElement* data; 
  XMLElement* dif;
  XMLElement* chip;
  XMLElement* chan;
  XMLElement* col;
  XMLElement* slope;
  XMLElement* intercept;

  // **********************//
  data = xml->NewElement("data");
  xml->InsertEndChild(data);
  char str[XML_ELEMENT_STRING_LENGTH];
  
  for (auto const& dif_map: topol.dif_map) {
    unsigned idif = dif_map.first;
    // ***** data > dif ***** //
    snprintf(str, XML_ELEMENT_STRING_LENGTH, "dif_%d", idif);
    dif = xml->NewElement(str);
    data->InsertEndChild(dif);
    // ***** data > dif > chip ***** //
    for (auto const& chip_map: dif_map.second) {
      unsigned ichip = chip_map.first;
      snprintf(str, XML_ELEMENT_STRING_LENGTH, "chip_%d", ichip);
      chip = xml->NewElement(str);    
      dif->InsertEndChild(chip);
      // ***** data > dif > chip > chan ***** //
      for (unsigned ichan = 0; ichan < chip_map.second; ++ichan) {
        snprintf(str, XML_ELEMENT_STRING_LENGTH, "chan_%d", ichan);
        chan = xml->NewElement(str);
        chip->InsertEndChild(chan);
        // ***** data > dif > chip > chan > col ***** //
        for (unsigned icol = 0; icol < MEMDEPTH; ++icol) {
          snprintf(str, XML_ELEMENT_STRING_LENGTH, "col_%d", icol);
          col = xml->NewElement(str);
          chan->InsertEndChild(col);
          slope = xml->NewElement("slope_gain");
          col->InsertEndChild(slope);
          intercept = xml->NewElement("intercept_gain");
          col->InsertEndChild(intercept);
        }
      }
    }
  }
  xml->SaveFile(filename.c_str());
  delete xml;
}

//**********************************************************************
void wgEditXML::GainCalib_SetValue(const std::string& name, unsigned idif, unsigned ichip, unsigned ichan, unsigned icol, unsigned value, bool create_new) {
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf(str, XML_ELEMENT_STRING_LENGTH, "dif_%d", idif);
  XMLElement* dif  = data->FirstChildElement(str);
  snprintf(str, XML_ELEMENT_STRING_LENGTH, "chip_%d", ichip);
  XMLElement* chip = dif->FirstChildElement(str);
  snprintf(str, XML_ELEMENT_STRING_LENGTH, "chan_%d", ichan);
  XMLElement* chan = chip->FirstChildElement(str);
  snprintf(str, XML_ELEMENT_STRING_LENGTH, "col_%d", icol);
  XMLElement* col = chan->FirstChildElement(str);
  
  XMLElement* target = col->FirstChildElement(name.c_str());
  snprintf(str, XML_ELEMENT_STRING_LENGTH, "%d", value);
  if (target) {
    target->SetText(str);
  } else if (create_new == true) {
    XMLElement* newElement = xml->NewElement(name.c_str());
    newElement->SetText(str);
    col->InsertEndChild(newElement);
  } else {
    throw wgElementNotFound("Element " + name + " doesn't exist");
  }
}

//**********************************************************************
double wgEditXML::GainCalib_GetValue(const std::string& name, unsigned idif, unsigned ichip, unsigned ichan, unsigned icol) {
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf(str, XML_ELEMENT_STRING_LENGTH, "dif_%d", idif);
  XMLElement* dif  = data->FirstChildElement(str);
  snprintf(str, XML_ELEMENT_STRING_LENGTH, "chip_%d", ichip);
  XMLElement* chip = dif->FirstChildElement(str);
  snprintf(str, XML_ELEMENT_STRING_LENGTH, "chan_%d", ichan);
  XMLElement* chan = chip->FirstChildElement(str);
  snprintf(str, XML_ELEMENT_STRING_LENGTH, "col_%d", icol);
  XMLElement* col = chan->FirstChildElement(str);
  XMLElement* target = col->FirstChildElement(name.c_str());
  if (target) {
    std::string value = target->GetText();
    return stod(value);
  } else {
    throw wgElementNotFound("Element " + name + " doesn't exist");
  }
  return -1;
}

//**********************************************************************
void wgEditXML::Pedestal_Make(const std::string& filename, Topology& topol){
  xml = new XMLDocument();
  XMLDeclaration* decl = xml->NewDeclaration();
  xml->InsertEndChild(decl);

  XMLElement *data, *dif, *chip, *chan, *difs, *chips, *chans, *pe1, *pe2,
      *gain, *sigma_gain, *ped, *sigma_ped, *meas_ped, *sigma_meas_ped;
  char str[XML_ELEMENT_STRING_LENGTH];

  // **********************//
  data = xml->NewElement("data");
  xml->InsertEndChild(data);
  difs = xml->NewElement("n_difs");
  difs->SetText(std::to_string(topol.n_difs).c_str());
  data->InsertEndChild(difs);

  for(unsigned idif = 0; idif < topol.n_difs; ++idif) {
    // ***** data > dif ***** //
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "dif_%d", idif );
    dif = xml->NewElement(str);    
    data->InsertEndChild(dif);
    chips = xml->NewElement("n_chips");

    unsigned n_chips = topol.dif_map[idif].size();
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "%u", n_chips);
    chips->SetText(str);
    data->InsertEndChild(chips);
    // ***** data > dif > chip ***** //
    for(unsigned ichip = 0; ichip < n_chips; ++ichip) {
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "chip_%d", ichip );
      chip = xml->NewElement(str);    
      dif->InsertEndChild(chip);

      unsigned n_chans = topol.dif_map[idif][ichip];
      chans = xml->NewElement("n_chans");
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "%u", n_chans);
      chans->SetText(str);
      dif->InsertEndChild(chans);
      // ***** data > dif > chip > ch ***** //
      for(unsigned ichan = 0; ichan < n_chans; ++ichan) {
        snprintf( str, XML_ELEMENT_STRING_LENGTH, "chan_%d", ichan );
        chan = xml->NewElement(str);
        chip->InsertEndChild(chan);
        for (unsigned icol = 0; icol < MEMDEPTH; icol++) {
          snprintf( str, XML_ELEMENT_STRING_LENGTH, "pe1_%d", icol );
          pe1 = xml->NewElement(str);
          chan->InsertEndChild(pe1);
        }
        for (unsigned icol = 0; icol < MEMDEPTH; icol++) {
          snprintf( str, XML_ELEMENT_STRING_LENGTH, "pe2_%d", icol );
          pe2 = xml->NewElement(str);
          chan->InsertEndChild(pe2);
        }
        for (unsigned icol = 0; icol < MEMDEPTH; icol++) {
          snprintf( str, XML_ELEMENT_STRING_LENGTH, "gain_%d", icol );
          gain = xml->NewElement(str);  
          chan->InsertEndChild(gain);
        }
        for (unsigned icol = 0; icol < MEMDEPTH; icol++) {
          snprintf( str, XML_ELEMENT_STRING_LENGTH, "sigma_gain_%d", icol );
          sigma_gain = xml->NewElement(str);  
          chan->InsertEndChild(sigma_gain);
        }
        for (unsigned icol = 0; icol < MEMDEPTH; icol++) {
          snprintf( str, XML_ELEMENT_STRING_LENGTH, "ped_%d", icol );
          ped = xml->NewElement(str);  
          chan->InsertEndChild(ped);
        }
        for (unsigned icol = 0; icol < MEMDEPTH; icol++) {
          snprintf( str, XML_ELEMENT_STRING_LENGTH, "sigma_ped_%d", icol );
          sigma_ped = xml->NewElement(str);  
          chan->InsertEndChild(sigma_ped);
        }
        for (unsigned icol = 0; icol < MEMDEPTH; icol++) {
          snprintf( str, XML_ELEMENT_STRING_LENGTH, "meas_ped_%d", icol );
          meas_ped = xml->NewElement(str);  
          chan->InsertEndChild(meas_ped);
        }
        for (unsigned icol = 0; icol < MEMDEPTH; icol++) {
          snprintf( str, XML_ELEMENT_STRING_LENGTH, "sigma_meas_ped_%d", icol );
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
void wgEditXML::Pedestal_SetChanValue(const std::string& name, const int idif,
                                      const int ichip, const int ichan,
                                      const int value, const bool create_new) {
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "dif_%d", idif );
  XMLElement* dif  = data->FirstChildElement(str);
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "chip_%d", ichip );
  XMLElement* chip = dif->FirstChildElement(str);
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "chan_%d", ichan );
  XMLElement* chan = chip->FirstChildElement(str);
  XMLElement* target = chan->FirstChildElement(name.c_str());

  if (target) {
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "%d", value );
    target->SetText(str);
  } else {
    if(create_new == true) {
      XMLElement* newElement = xml->NewElement(name.c_str());
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "%d", value );
      newElement->SetText(str);
      chan->InsertEndChild(newElement);
    } else {
      throw wgElementNotFound("Element " + name + " doesn't exist");
    }
  }
}

//**********************************************************************
int wgEditXML::Pedestal_GetChanValue(const std::string& name, const int idif,
                                     const int ichip, const int ichan) {
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "dif_%d", idif );
  XMLElement* dif  = data->FirstChildElement(str);
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "chip_%d", ichip );
  XMLElement* chip = dif->FirstChildElement(str);
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "chan_%d", ichan );
  XMLElement* chan   = chip->FirstChildElement(str);
  XMLElement* target = chan->FirstChildElement(name.c_str());
  std::string value;
  if (target) {
    value = target->GetText();
  } else {
    throw wgElementNotFound("Element: " + name + " doesn't exist!");
  }
  return stoi(value);
}

//**********************************************************************
void wgEditXML::Pedestal_SetDifConfigValue(const std::string& name, const int idif,
                                           const int value, const bool create_new) {
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "dif_%d", idif );
  XMLElement* dif  = data->FirstChildElement(str);
  XMLElement* config = dif->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());

  if (target) {
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "%d", value );
    target->SetText(str);
  } else {
    if(create_new == true){
      XMLElement* newElement = xml->NewElement(name.c_str());
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "%d", value );
      newElement->SetText(str);
      config->InsertEndChild(newElement);
    } else {
      throw wgElementNotFound("Element " + name + " doesn't exist");
    }
  }
}

//**********************************************************************
int wgEditXML::Pedestal_GetDifConfigValue(const std::string& name, const int idif) {
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "dif_%d", idif );
  XMLElement* dif  = data->FirstChildElement(str);
  XMLElement* config = dif->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());
  std::string value;
  if (target) {
    value = target->GetText();
  } else {
    throw wgElementNotFound("Element: " + name + " doesn't exist!");
  }
  return stoi(value);
}

//**********************************************************************
void wgEditXML::Pedestal_SetChipConfigValue(const std::string& name, const int idif,
                                            const int ichip, const int value,
                                            const bool create_new) {
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "dif_%d", idif );
  XMLElement* dif  = data->FirstChildElement(str);
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "chip_%d", ichip );
  XMLElement* chip = dif->FirstChildElement(str);
  XMLElement* config = chip->FirstChildElement(str);
  XMLElement* target = config->FirstChildElement(name.c_str());

  if (target) {
    snprintf( str, XML_ELEMENT_STRING_LENGTH, "%d", value );
    target->SetText(str);
  } else {
    if(create_new == true) {
      XMLElement* newElement = xml->NewElement(name.c_str());
      snprintf( str, XML_ELEMENT_STRING_LENGTH, "%d", value );
      newElement->SetText(str);
      config->InsertEndChild(newElement);
    } else {
      throw wgElementNotFound("Element " + name + " doesn't exist");
    }
  }
}

//**********************************************************************
int wgEditXML::Pedestal_GetChipConfigValue(const std::string& name, const int idif,
                                           const int ichip) {
  char str[XML_ELEMENT_STRING_LENGTH];
  XMLElement* data = xml->FirstChildElement("data");
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "dif_%d", idif );
  XMLElement* dif  = data->FirstChildElement(str);
  snprintf( str, XML_ELEMENT_STRING_LENGTH, "chip_%d", ichip );
  XMLElement* chip = dif->FirstChildElement(str);
  XMLElement* config = chip->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());
  std::string value;
  if (target) {
    value = target->GetText();
  } else {
    throw wgElementNotFound("Element: " + name + " doesn't exist!");
  }
  return stoi(value);
}
