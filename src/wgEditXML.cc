#include <iostream>
#include <string>
#include <vector>
#include <TROOT.h>
#include <cstdlib>
#include <time.h>

#include "tinyxml2.h"
#include "Const.h"
#include "wgEditXML.h"
#include "wgEditConfig.h"
#include "wgErrorCode.h"
#include "wgExceptions.h"
#include "wgTools.h"

using namespace std;
using namespace tinyxml2;

string wgEditXML::filename;

//**********************************************************************
void wgEditXML::Write(){
  xml->SaveFile(Form("%s",wgEditXML::filename.c_str()));
}

//**********************************************************************
void wgEditXML::Open(const string& filename){
  CheckExist Check;
  if(!Check.XmlFile(filename)) throw wgInvalidFile("[" + filename +"][wgEditXML::Open] error in opening XML file");
  wgEditXML::filename=filename; 
  wgEditXML::xml = new XMLDocument();
  XMLError eResult = wgEditXML::xml->LoadFile(filename.c_str());
  if (eResult != tinyxml2::XML_SUCCESS) throw wgInvalidFile("[" + filename +"][wgEditXML::Open] error in opening XML file");
}

//**********************************************************************
void wgEditXML::Close(){ 
  delete xml;
}

//**********************************************************************
void wgEditXML::Make(const string& filename, const unsigned ichip, const unsigned ichan){
  xml = new XMLDocument();
  XMLDeclaration* decl = xml->NewDeclaration();
  xml->InsertEndChild(decl);

  XMLElement* data = xml->NewElement("data");
  xml->InsertEndChild(data);

  XMLElement* config = data->GetDocument()->NewElement("config");
  data->InsertEndChild(config);

  XMLElement* chipid = xml->NewElement("chipid");
  config->InsertEndChild(chipid);
  chipid->InsertEndChild(chipid->GetDocument()->NewText(Form("%d",ichip)));

  XMLElement* chanid = xml->NewElement("chanid");
  config->InsertEndChild(chanid);
  chanid->InsertEndChild(chanid->GetDocument()->NewText(Form("%d",ichan)));

  XMLElement* start_time = xml->NewElement("start_time");
  config->InsertEndChild(start_time);
  start_time->InsertEndChild(start_time->GetDocument()->NewText(Form("%d",0)));
  
  XMLElement* stop_time = xml->NewElement("stop_time");
  config->InsertEndChild(stop_time);
  stop_time->InsertEndChild(stop_time->GetDocument()->NewText(Form("%d",0)));
  
  XMLElement* trigth = xml->NewElement("trigth");
  config->InsertEndChild(trigth);
  trigth->InsertEndChild(trigth->GetDocument()->NewText(Form("%d",-1)));

  XMLElement* gainth = xml->NewElement("gainth");
  config->InsertEndChild(gainth);
  gainth->InsertEndChild(gainth->GetDocument()->NewText(Form("%d",-1)));

  XMLElement* inputDAC = xml->NewElement("inputDAC");
  config->InsertEndChild(inputDAC);
  inputDAC->InsertEndChild(inputDAC->GetDocument()->NewText(Form("%d",-1)));

  XMLElement* HG = xml->NewElement("HG");
  config->InsertEndChild(HG);
  HG->InsertEndChild(HG->GetDocument()->NewText(Form("%d",-1)));

  XMLElement* LG = xml->NewElement("LG");
  config->InsertEndChild(LG);
  LG->InsertEndChild(LG->GetDocument()->NewText(Form("%d",-1)));

  XMLElement* trig_adj = xml->NewElement("trig_adj");
  config->InsertEndChild(trig_adj);
  trig_adj->InsertEndChild(trig_adj->GetDocument()->NewText(Form("%d",-1)));

  XMLElement* ch = xml->NewElement("ch");
  data->InsertEndChild(ch);

  for(unsigned int k=0; k<MEMDEPTH; k++){
    XMLElement* col = xml->NewElement(Form("col_%d",k));
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
	  throw wgInvalidFile(filename + " wasn't found or is not valid");
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
			  if( string(asu->Attribute("name")) == "spiroc2d_bitstream" ) {
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

	wgEditConfig EditCon;
	v.clear();
	EditCon.SetBitstream(bitstream);
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
void wgEditXML::SetConfigValue(const string& name, const int value, const int mode) {
  XMLElement* data   = xml->FirstChildElement("data");
  XMLElement* config = data->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());
  if (target) {
    target->SetText(to_string(value).c_str());
  }
  else if(mode == CREATE_NEW_MODE) {
	XMLElement* newElement = xml->NewElement(name.c_str());
	newElement->SetText(value);
	config->InsertEndChild(newElement);      
  }
  else throw wgElementNotFound("Element " + name + " doesn't exist");
}

//**********************************************************************
void wgEditXML::SetColValue(const string& name,const int icol,double value,int mode=0){
  if(icol<0 || icol>16) return;
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement("ch");
  XMLElement* col = ch->FirstChildElement(Form("col_%d",icol));
  XMLElement* target = col->FirstChildElement(name.c_str());
  if(target){
    target->SetText(Form("%.2f",value));
  }else{
    if(mode==1){
      XMLElement* newElement = xml->NewElement(name.c_str());
      newElement->SetText(value);
      col->InsertEndChild(newElement);
    }else{
      Log.eWrite("Warning! Element "+ name +" doesn't exist!");
    }
  }
}

//**********************************************************************
void wgEditXML::SetChValue(const string& name, const double value, const int mode){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement("ch");
  XMLElement* target = ch->FirstChildElement(name.c_str());
  if(target){
    target->SetText(Form("%.2f",value));
  }else{
    if(mode == CREATE_NEW_MODE){
      XMLElement* newElement = xml->NewElement(name.c_str());
      newElement->SetText(value);
      ch->InsertEndChild(newElement); 
    }else{
      Log.eWrite("[wgEditXML::SetChValue] Warning! Element " + name + " doesn't exist!");
    }
  }  
}


//**********************************************************************
void wgEditXML::AddColElement(const string& name,const int icol){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement("ch");
  XMLElement* col = ch->FirstChildElement(Form("col_%d",icol));
  XMLElement* newElement = xml->NewElement(name.c_str());
  col->InsertEndChild(newElement);
}

//**********************************************************************
void wgEditXML::AddChElement(const string& name){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement("ch");
  XMLElement* newElement = xml->NewElement(name.c_str());
  ch->InsertEndChild(newElement);
}

//**********************************************************************
double wgEditXML::GetColValue(const string& name,const int icol){
  if(icol<0 || icol>16) return 0.0;
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement("ch");
  XMLElement* col = ch->FirstChildElement(Form("col_%d",icol));
  XMLElement* target = col->FirstChildElement(name.c_str());
  if(target){
    string value = target->GetText();
    return atof(value.c_str());
  }else{
    Log.eWrite("Error! Element:" + name + " doesn't exist!");
    return -1.;
  }
}

//**********************************************************************
double wgEditXML::GetChValue(const string& name){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement("ch");
  XMLElement* target = ch->FirstChildElement(name.c_str());
  if(target){
    string value = target->GetText();
    return atof(value.c_str());
  }else{
    Log.eWrite("Error! Element:" + name + " doesn't exist!");
    return -1.;
  }
}

//**********************************************************************
int wgEditXML::GetConfigValue(const string& name){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* config = data->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());
  if(target){
    string value = target->GetText();
    return atoi(value.c_str());
  }else{
    Log.eWrite("Error! Element:" + name + " doesn't exist!");
    return -1;
  }
}

//**********************************************************************
void wgEditXML::SUMMARY_Make(const string& filename, const unsigned n_chans) {
  xml = new XMLDocument();
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
  XMLElement* ch   [n_chans];
  XMLElement* fit  [n_chans];
  XMLElement* gain [n_chans];
  XMLElement* noise[n_chans];
  XMLElement* config          [n_chans];
  XMLElement* inputDAC        [n_chans];
  XMLElement* ampDAC          [n_chans];
  XMLElement* threshold_adjust[n_chans];
  //col 
  XMLElement* pedestal[n_chans][MEMDEPTH];

  // **********************//

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
    ch[ichan] = xml->NewElement(Form("ch_%d",ichan));    
    data->InsertEndChild(ch[ichan]);

    // ***** data > ch > fit***** //
    fit[ichan] = xml->NewElement("fit");
    ch[ichan]->InsertEndChild(fit[ichan]);

    gain[ichan] = xml->NewElement("Gain");
    fit[ichan]->InsertEndChild(gain[ichan]);
    noise[ichan] = xml->NewElement("Noise");
    fit[ichan]->InsertEndChild(noise[ichan]);

    for(unsigned icol = 0; icol < MEMDEPTH; icol++) {
      pedestal[ichan][icol] = xml->NewElement(Form("ped_%d",icol));
      fit[ichan]->InsertEndChild(pedestal[ichan][icol]);
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
void wgEditXML::SUMMARY_SetGlobalConfigValue(const string& name, const int value, const int mode){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* config = data->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());
  if( target )
	target->SetText(Form("%d",value));
  else if( mode == CREATE_NEW_MODE ) {
	XMLElement* newElement = xml->NewElement(name.c_str());
	newElement->SetText(Form("%d",value));
	config->InsertEndChild(newElement);
  }
  else throw wgElementNotFound("Element " + name + " doesn't exist");
}

//**********************************************************************
void wgEditXML::SUMMARY_SetChConfigValue(const string& name, const int value, const int ichan, const int mode){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement(Form("ch_%d",ichan));
  XMLElement* config = ch->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());
  if( target )
    target->SetText(Form("%d",value));
  else if( mode == CREATE_NEW_MODE ) {
      XMLElement* newElement = xml->NewElement(name.c_str());
      newElement->SetText(Form("%d",value));
      config->InsertEndChild(newElement);
  }
  else throw wgElementNotFound("Element " + name + " doesn't exist");
}

//**********************************************************************
void wgEditXML::SUMMARY_SetChFitValue(const string& name, const int value, const int ichan, const int mode){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement(Form("ch_%d",ichan));
  XMLElement* config = ch->FirstChildElement("fit");
  XMLElement* target = config->FirstChildElement(name.c_str());
  if( target )
    target->SetText(Form("%d",value));
  else if( mode == CREATE_NEW_MODE ) {
      XMLElement* newElement = xml->NewElement(name.c_str());
      newElement->SetText(Form("%d",value));
      config->InsertEndChild(newElement);
  }
  else throw wgElementNotFound("Element " + name + " doesn't exist");
}

//**********************************************************************
void wgEditXML::SUMMARY_SetPedFitValue(double* value,int ich,int mode=0){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement(Form("ch_%d",ich));
  XMLElement* fit = ch->FirstChildElement("fit");
  for(unsigned int i=0;i<MEMDEPTH;i++){
    int icol=i;
    XMLElement* target = fit->FirstChildElement(Form("ped_%d",icol));
    if(target){
      target->SetText(Form("%.2f",value[icol]));
    }else{
      if(mode==1){
        XMLElement* newElement = xml->NewElement(Form("ped_%d",icol));
        newElement->SetText(Form("%.2f",value[icol]));
        fit->InsertEndChild(newElement); 
      }else{
        Log.eWrite("Warning! Element ped_"+ to_string(icol) +" doesn't exist!");
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
void wgEditXML::SUMMARY_AddChElement(const string& name,int ich){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement(Form("ch_%d",ich));
  XMLElement* newElement = xml->NewElement(name.c_str());
  ch->InsertEndChild(newElement);
}

//**********************************************************************
int wgEditXML::SUMMARY_GetGlobalConfigValue(const string& name){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* config = data->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());
  if(target){
    string value = target->GetText();
    return atof(value.c_str());
  }else{
    Log.eWrite("[SUMMARY_GetGlobalConfigValue] Element:" + name + " doesn't exist!");
    return -1.;
  }
}

//**********************************************************************
int wgEditXML::SUMMARY_GetChConfigValue(const string& name,int ich){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement(Form("ch_%d",ich));
  XMLElement* config = ch->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());
  if(target){
    string value = target->GetText();
    return atoi(value.c_str());
  }else{
    Log.eWrite("[SUMMARY_GetChConfigValue] Element:" + name + " doesn't exist!");
    return -1.;
  }
}

//**********************************************************************
double wgEditXML::SUMMARY_GetChFitValue(const string& name,int ich){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement(Form("ch_%d",ich));
  XMLElement* fit = ch->FirstChildElement("fit");
  XMLElement* target = fit->FirstChildElement(name.c_str());
  if(target){
    string value = target->GetText();
    return atof(value.c_str());
  }else{
    Log.eWrite("[SUMMARY_GetChFitValue] Element:" + name + " doesn't exist!");
    return -1.;
  }
}

//**********************************************************************
void wgEditXML::SUMMARY_GetPedFitValue(double* value,int ich){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement(Form("ch_%d",ich));
  XMLElement* config = ch->FirstChildElement("fit");
  for(unsigned int i=0;i<MEMDEPTH;i++){
    int icol=i;
    XMLElement* target = config->FirstChildElement(Form("ped_%d",icol));
    if(target){
      string temp_value = target->GetText();
      value[icol] = atof(temp_value.c_str());
    }else{
      Log.eWrite("Error! Element: ped" + to_string(icol) + " doesn't exist!");
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
    pe_center1[iDAC]= xml->NewElement(Form("pe_center1"));
    inputDAC[iDAC]->InsertEndChild(pe_center1[iDAC]);
    pe_width1[iDAC]= xml->NewElement(Form("pe_width1"));
    inputDAC[iDAC]->InsertEndChild(pe_width1[iDAC]);
    pe_center2[iDAC]= xml->NewElement(Form("pe_center2"));
    inputDAC[iDAC]->InsertEndChild(pe_center2[iDAC]);
    pe_width2[iDAC]= xml->NewElement(Form("pe_width2"));
    inputDAC[iDAC]->InsertEndChild(pe_width2[iDAC]);
    gain1[iDAC]= xml->NewElement(Form("gain1"));
    inputDAC[iDAC]->InsertEndChild(gain1[iDAC]);
    gain2[iDAC]= xml->NewElement(Form("gain2"));
    inputDAC[iDAC]->InsertEndChild(gain2[iDAC]);
  }
  xml->SaveFile(filename.c_str());
  delete xml;
}

//**********************************************************************
void wgEditXML::SCURVE_SetValue(const string& name,int iDAC,double value,int mode=0){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* inputDAC = data->FirstChildElement(Form("inputDAC_%d",iDAC));
  XMLElement* target = inputDAC->FirstChildElement(name.c_str());
  if(target){
    target->SetText(Form("%f",value));
  }else{
    if(mode==1){
      XMLElement* newElement = xml->NewElement(name.c_str());
      newElement->SetText(Form("%f",value));
      inputDAC->InsertEndChild(newElement);
    }else{
      Log.eWrite("Warning! Element "+ name +" doesn't exist!");
    }
  }
}

//**********************************************************************
double wgEditXML::SCURVE_GetValue(const string& name,int iDAC){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* inputDAC = data->FirstChildElement(Form("inputDAC_%d",iDAC));
  XMLElement* target = inputDAC->FirstChildElement(name.c_str());
  if(target){
    string value = target->GetText();
    return atof(value.c_str());
  }else{
    Log.eWrite("Error! Element:" + name + " doesn't exist!");
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
      s_th[0] = xml->NewElement(Form("s_th1"));    
      i_th[0] = xml->NewElement(Form("i_th1"));    
      s_th[1] = xml->NewElement(Form("s_th2"));    
      i_th[1] = xml->NewElement(Form("i_th2"));    
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
void wgEditXML::OPT_SetValue(const string& name,int idif, int ichip, int iDAC,double value,int mode=0){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* inputDAC = chip->FirstChildElement(Form("inputDAC_%d",iDAC));
  XMLElement* target = inputDAC->FirstChildElement(name.c_str());

  if(target){
    target->SetText(Form("%f",value));
  }else{
    if(mode==1){
      XMLElement* newElement = xml->NewElement(name.c_str());
      newElement->SetText(Form("%f",value));
      inputDAC->InsertEndChild(newElement);
    }else{
      Log.eWrite("Warning! Element "+ name +" doesn't exist!");
    }
  }
}

//**********************************************************************
double wgEditXML::OPT_GetValue(const string& name,int idif, int ichip, int iDAC){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* inputDAC = chip->FirstChildElement(Form("inputDAC_%d",iDAC));
  XMLElement* target = inputDAC->FirstChildElement(name.c_str());
  if(target){
    string value = target->GetText();
    return atof(value.c_str());
  }else{
    Log.eWrite("Error! Element:" + name + " doesn't exist!");
    return -1.;
  }
}

//**********************************************************************
void wgEditXML::OPT_SetChipValue(const string& name,int idif, int ichip,double value,int mode=0){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* target = chip->FirstChildElement(name.c_str());

  if(target){
    target->SetText(Form("%f",value));
  }else{
    if(mode==1){
      XMLElement* newElement = xml->NewElement(name.c_str());
      newElement->SetText(Form("%f",value));
      chip->InsertEndChild(newElement);
    }else{
      Log.eWrite("Warning! Element "+ name +" doesn't exist!");
    }
  }
}

//**********************************************************************
double wgEditXML::OPT_GetChipValue(const string& name,int idif, int ichip){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* target = chip->FirstChildElement(name.c_str());
  if(target){
    string value = target->GetText();
    return atof(value.c_str());
  }else{
    Log.eWrite("Error! Element:" + name + " doesn't exist!");
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
    for(unsigned int i=0;i<NCHIPS;i++){
      int ichip=i;
      chip = xml->NewElement(Form("chip_%d",ichip));    
      dif->InsertEndChild(chip);
      // ***** data > dif > chip > ch ***** //
      for(unsigned int j=0;j<32;j++){
        int ich=j;
        ch = xml->NewElement(Form("ch_%d",ich));    
        chip->InsertEndChild(ch);
        s_Gain = xml->NewElement(Form("s_Gain"));    
        i_Gain = xml->NewElement(Form("i_Gain"));    
        ch->InsertEndChild(s_Gain);
        ch->InsertEndChild(i_Gain);
      }
    }
  }
  xml->SaveFile(filename.c_str());
  delete xml;
}

//**********************************************************************
void wgEditXML::PreCalib_SetValue(const string& name,int idif, int ichip, int ich,double value,int mode=0){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* ch = chip->FirstChildElement(Form("ch_%d",ich));
  XMLElement* target = ch->FirstChildElement(name.c_str());

  if(target){
    target->SetText(Form("%f",value));
  }else{
    if(mode==1){
      XMLElement* newElement = xml->NewElement(name.c_str());
      newElement->SetText(Form("%f",value));
      ch->InsertEndChild(newElement);
    }else{
      Log.eWrite("Warning! Element "+ name +" doesn't exist!");
    }
  }
}

//**********************************************************************
double wgEditXML::PreCalib_GetValue(const string& name,int idif, int ichip, int ich){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* ch   = chip->FirstChildElement(Form("ch_%d",ich));
  XMLElement* target = ch->FirstChildElement(name.c_str());
  if(target){
    string value = target->GetText();
    return atof(value.c_str());
  }else{
    Log.eWrite("Error! Element:" + name + " doesn't exist!");
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

  // **********************//
  data = xml->NewElement("data");
  xml->InsertEndChild(data);
  XMLElement* difs = xml->NewElement("n_difs");
  difs->SetText(to_string(n_difs).c_str());
  data->InsertEndChild(difs);
  XMLElement* chips = xml->NewElement("n_chips");
  difs->SetText(to_string(n_chips).c_str());
  data->InsertEndChild(chips);
  XMLElement* chans = xml->NewElement("n_chans");
  difs->SetText(to_string(n_chans).c_str());
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
        pe1 = xml->NewElement(Form("pe1"));    
        pe2 = xml->NewElement(Form("pe2"));    
        gain = xml->NewElement(Form("Gain"));    
        ch->InsertEndChild(pe1);
        ch->InsertEndChild(pe2);
        ch->InsertEndChild(gain);
      }
    }
  }
  xml->SaveFile(filename.c_str());
  delete xml;
}

//**********************************************************************
void wgEditXML::Calib_SetValue(const string& name,int idif, int ichip, int ich,double value,int mode=0){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* ch = chip->FirstChildElement(Form("ch_%d",ich));
  XMLElement* target = ch->FirstChildElement(name.c_str());

  if(target){
    target->SetText(Form("%f",value));
  }else{
    if(mode==1){
      XMLElement* newElement = xml->NewElement(name.c_str());
      newElement->SetText(Form("%f",value));
      ch->InsertEndChild(newElement);
    }else{
      Log.eWrite("Warning! Element "+ name +" doesn't exist!");
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
  }
  else {
    throw wgElementNotFound(Form("Element: %s doesn't exist!", name.c_str()));
  }
  return atof(value.c_str());
}



