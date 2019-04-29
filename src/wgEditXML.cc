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

using namespace std;
using namespace tinyxml2;

string wgEditXML::filename;

//**********************************************************************
void wgEditXML::Write(){
  xml->SaveFile(Form("%s",wgEditXML::filename.c_str()));
}

//**********************************************************************
void wgEditXML::Open(const string& filename){
  CheckExist * Check = new CheckExist;
  if(!Check->XmlFile(filename)){ cout << "Warning! "<< filename <<" doesn't match requirement!"<<endl; return; }
  delete Check;
  wgEditXML::filename=filename; 
  xml = new XMLDocument();
  xml->LoadFile(filename.c_str());
}

//**********************************************************************
void wgEditXML::Close(){ 
  delete xml;
}


//**********************************************************************
void wgEditXML::Make(string& filename,const int ichip,const int ichan){
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

  /*
  XMLElement* LG = xml->NewElement("LG");
  config->InsertEndChild(LG);
  LG->InsertEndChild(LG->GetDocument()->NewText(Form("%d",-1)));

  XMLElement* trig_adj = xml->NewElement("trig_adj");
  config->InsertEndChild(trig_adj);
  trig_adj->InsertEndChild(trig_adj->GetDocument()->NewText(Form("%d",-1)));
  */

  XMLElement* ch = xml->NewElement("ch");
  data->InsertEndChild(ch);

  for(unsigned int k=0; k<MEMDEPTH+1; k++){
    XMLElement* col = xml->NewElement(Form("col_%d",k));
    ch->InsertEndChild(col);
  }
  xml->SaveFile(filename.c_str());
  delete xml;
}

//**********************************************************************
void wgEditXML::GetConfig(string& configxml,unsigned int n_dif,unsigned int ichip,vector<int>& v){
  bool target=false;
  string str("");
  string bitstream("");
  CheckExist *Check = new CheckExist;
  if(!Check->XmlFile(configxml)){ cout << "Warning! "<< filename <<" doesn't match requirement!"<<endl; return; }
  if(ichip > NCHIPS){ cout << "Error! ichip is too large!" <<endl; return; }
  delete Check;

  XMLDocument *configfile = new XMLDocument();
  configfile->LoadFile(configxml.c_str()); 
  XMLElement* ecal = configfile->FirstChildElement("ecal");
  XMLElement* domain = ecal->FirstChildElement("domain");
  XMLElement* acqpc = domain->FirstChildElement("acqpc");
  XMLElement* gdcc = acqpc->FirstChildElement("gdcc");
  for(XMLElement* dif = gdcc->FirstChildElement("dif"); dif!=NULL; dif= dif->NextSiblingElement("dif")){
    str = dif->Attribute("name");
    if(str==Form("dif_1_1_%d",n_dif)){
      for(XMLElement* asu = dif->FirstChildElement("asu"); asu!=NULL; asu= asu->NextSiblingElement("asu")){
        str = asu->Attribute("name");
        if(str==Form("asu_1_1_%d_%d",n_dif,ichip)){   
          XMLElement* spiroc2d = asu->FirstChildElement("spiroc2d");
          for(XMLElement* param = spiroc2d->FirstChildElement("param"); param!=NULL; param= param->NextSiblingElement("param")){
            str = param->Attribute("name");
            if(str=="spiroc2d_bitstream"){
              bitstream = param->GetText();
              target=true;
              break;
            }
          }
        }else if(target){
          break;
        }
      }
    }
  }
  delete configfile; 

  wgEditConfig *EditCon = new wgEditConfig();
  v.clear();
  EditCon->SetBitstream(bitstream);
  for(int i=0;i<32;i++){
    v.push_back(EditCon->Get_trigth());  
    v.push_back(EditCon->Get_gainth());
    v.push_back(EditCon->Get_inputDAC(i));
    v.push_back(EditCon->Get_ampDAC(i));
    v.push_back(EditCon->Get_ampDAC(i));
    v.push_back(EditCon->Get_trigadj(i));
  }
  delete EditCon;
}

//**********************************************************************
void wgEditXML::GetLog(string& filename,vector<int>& v){
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
void wgEditXML::SetConfigValue(string& name,int value,int mode=0){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* config = data->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());
  if(target){
    target->SetText(Form("%d",value));
  }else{
    if(mode==1){
      XMLElement* newElement = xml->NewElement(name.c_str());
      newElement->SetText(value);
      config->InsertEndChild(newElement);      
    }else{
      cout <<"Warning! Element "<< name <<" doesn't exist!"<<endl;
    }
  }
}

//**********************************************************************
void wgEditXML::SetColValue(string& name,const int icol,double value,int mode=0){
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
      cout <<"Warning! Element "<< name <<" doesn't exist!"<<endl;
    }
  }
}

//**********************************************************************
void wgEditXML::SetChValue(string& name,double value,int mode=0){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement("ch");
  XMLElement* target = ch->FirstChildElement(name.c_str());
  if(target){
    target->SetText(Form("%.2f",value));
  }else{
    if(mode==1){
      XMLElement* newElement = xml->NewElement(name.c_str());
      newElement->SetText(value);
      ch->InsertEndChild(newElement); 
    }else{
      cout <<"Warning! Element "<< name <<" doesn't exist!"<<endl;
    }
  }  
}


//**********************************************************************
void wgEditXML::AddColElement(string& name,const int icol){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement("ch");
  XMLElement* col = ch->FirstChildElement(Form("col_%d",icol));
  XMLElement* newElement = xml->NewElement(name.c_str());
  col->InsertEndChild(newElement);
}

//**********************************************************************
void wgEditXML::AddChElement(string& name){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement("ch");
  XMLElement* newElement = xml->NewElement(name.c_str());
  ch->InsertEndChild(newElement);
}

//**********************************************************************
double wgEditXML::GetColValue(string& name,const int icol){
  if(icol<0 || icol>16) return 0.0;
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement("ch");
  XMLElement* col = ch->FirstChildElement(Form("col_%d",icol));
  XMLElement* target = col->FirstChildElement(name.c_str());
  if(target){
    string value = target->GetText();
    return atof(value.c_str());
  }else{
    cout << "Error! Element:" << name << " doesn't exist!" << endl;
    return -1.;
  }
}

//**********************************************************************
double wgEditXML::GetChValue(string& name){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement("ch");
  XMLElement* target = ch->FirstChildElement(name.c_str());
  if(target){
    string value = target->GetText();
    return atof(value.c_str());
  }else{
    cout << "Error! Element:" << name << " doesn't exist!" << endl;
    return -1.;
  }
}

//**********************************************************************
int wgEditXML::GetConfigValue(string& name){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* config = data->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());
  if(target){
    string value = target->GetText();
    return atoi(value.c_str());
  }else{
    cout << "Error! Element:" << name << " doesn't exist!" << endl;
    return -1;
  }
}

//**********************************************************************
void wgEditXML::SUMMARY_Make(string& filename,int ichip){
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
  //ch
  XMLElement* ch[32];
  XMLElement* fit[32];
  XMLElement* gain[32];
  XMLElement* noise[32];

  XMLElement* config[32];
  XMLElement* inputDAC[32];
  XMLElement* ampDAC[32];
  XMLElement* threshold_adjust[32];
  //col 
  ////XMLElement* pedestal[32][16];

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

  for(unsigned int i=0;i<32;i++){
    int ich = i; 
    // ***** data > ch ***** //
    ch[ich] = xml->NewElement(Form("ch_%d",ich));    
    data->InsertEndChild(ch[ich]);

    // ***** data > ch > fit***** //
    fit[ich] = xml->NewElement("fit");
    ch[ich]->InsertEndChild(fit[ich]);

    gain[ich] = xml->NewElement("Gain");
    fit[ich]->InsertEndChild(gain[ich]);
    noise[ich] = xml->NewElement("Noise");
    fit[ich]->InsertEndChild(noise[ich]);
    /*
    for(unsigned int j=0;j<16;j++){
      int icol=j;
      pedestal[ich][icol] = xml->NewElement(Form("ped_%d",icol));
      fit[ich]->InsertEndChild(pedestal[ich][icol]);
    }
    */

    // ***** data > ch > config ***** //
    config[ich] = xml->NewElement("config");
    ch[ich]->InsertEndChild(config[ich]);

    inputDAC[ich] = xml->NewElement("inputDAC");
    config[ich]->InsertEndChild(inputDAC[ich]);
    ampDAC[ich] = xml->NewElement("ampDAC");
    config[ich]->InsertEndChild(ampDAC[ich]);
    threshold_adjust[ich] = xml->NewElement("adjDAC");
    config[ich]->InsertEndChild(threshold_adjust[ich]);

    // ***** data > ch > col ***** //
  }
  xml->SaveFile(filename.c_str());
  delete xml;
}

//**********************************************************************
void wgEditXML::SUMMARY_SetGlobalConfigValue(string& name,int value,int mode=0){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* config = data->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());
  if(target){
    target->SetText(Form("%d",value));
  }else{
    if(mode==1){
      XMLElement* newElement = xml->NewElement(name.c_str());
      newElement->SetText(Form("%d",value));
      config->InsertEndChild(newElement);
    }else{
      cout <<"Warning! Element "<< name <<" doesn't exist!"<<endl;
    }
  }
}

//**********************************************************************
void wgEditXML::SUMMARY_SetChConfigValue(string& name,int value,int ich,int mode=0){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement(Form("ch_%d",ich));
  XMLElement* config = ch->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());
  if(target){
    target->SetText(Form("%d",value));
  }else{
    if(mode==1){
      XMLElement* newElement = xml->NewElement(name.c_str());
      newElement->SetText(Form("%d",value));
      config->InsertEndChild(newElement); 
    }else{
      cout <<"Warning! Element "<< name <<" doesn't exist!"<<endl;
    }
  }  
}

//**********************************************************************
void wgEditXML::SUMMARY_SetChFitValue(string& name,double value,int ich,int mode=0){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement(Form("ch_%d",ich));
  XMLElement* fit = ch->FirstChildElement("fit");
  XMLElement* target = fit->FirstChildElement(name.c_str());
  if(target){
    target->SetText(Form("%.2f",value));
  }else{
    if(mode==1){
      XMLElement* newElement = xml->NewElement(name.c_str());
      newElement->SetText(Form("%.2f",value));
      fit->InsertEndChild(newElement); 
    }else{
      cout <<"Warning! Element "<< name <<" doesn't exist!"<<endl;
    }
  }  
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
        cout <<"Warning! Element ped_"<< icol <<" doesn't exist!"<<endl;
      }
    }
  }  
}

//**********************************************************************
void wgEditXML::SUMMARY_AddGlobalElement(string& name){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* newElement = xml->NewElement(name.c_str());
  data->InsertEndChild(newElement);
}

//**********************************************************************
void wgEditXML::SUMMARY_AddChElement(string& name,int ich){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement(Form("ch_%d",ich));
  XMLElement* newElement = xml->NewElement(name.c_str());
  ch->InsertEndChild(newElement);
}

//**********************************************************************
int wgEditXML::SUMMARY_GetGlobalConfigValue(string& name){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* config = data->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());
  if(target){
    string value = target->GetText();
    return atof(value.c_str());
  }else{
    cout << "Error! Element:" << name << " doesn't exist!" << endl;
    return -1.;
  }
}

//**********************************************************************
int wgEditXML::SUMMARY_GetChConfigValue(string& name,int ich){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement(Form("ch_%d",ich));
  XMLElement* config = ch->FirstChildElement("config");
  XMLElement* target = config->FirstChildElement(name.c_str());
  if(target){
    string value = target->GetText();
    return atoi(value.c_str());
  }else{
    cout << "Error! Element:" << name << " doesn't exist!" << endl;
    return -1.;
  }
}

//**********************************************************************
double wgEditXML::SUMMARY_GetChFitValue(string& name,int ich){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* ch = data->FirstChildElement(Form("ch_%d",ich));
  XMLElement* fit = ch->FirstChildElement("fit");
  XMLElement* target = fit->FirstChildElement(name.c_str());
  if(target){
    string value = target->GetText();
    return atof(value.c_str());
  }else{
    cout << "Error! Element:" << name << " doesn't exist!" << endl;
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
      cout << "Error! Element: ped" << icol << " doesn't exist!" << endl;
    }
  }
}

//**********************************************************************
void wgEditXML::SCURVE_Make(string& filename){
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
void wgEditXML::SCURVE_SetValue(string& name,int iDAC,double value,int mode=0){
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
      cout <<"Warning! Element "<< name <<" doesn't exist!"<<endl;
    }
  }
}

//**********************************************************************
double wgEditXML::SCURVE_GetValue(string& name,int iDAC){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* inputDAC = data->FirstChildElement(Form("inputDAC_%d",iDAC));
  XMLElement* target = inputDAC->FirstChildElement(name.c_str());
  if(target){
    string value = target->GetText();
    return atof(value.c_str());
  }else{
    cout << "Error! Element:" << name << " doesn't exist!" << endl;
    return -1.;
  }
}

//**********************************************************************
void wgEditXML::OPT_Make(string& filename){
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
void wgEditXML::OPT_SetValue(string& name,int idif, int ichip, int iDAC,double value,int mode=0){
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
      cout <<"Warning! Element "<< name <<" doesn't exist!"<<endl;
    }
  }
}

//**********************************************************************
double wgEditXML::OPT_GetValue(string& name,int idif, int ichip, int iDAC){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* inputDAC = chip->FirstChildElement(Form("inputDAC_%d",iDAC));
  XMLElement* target = inputDAC->FirstChildElement(name.c_str());
  if(target){
    string value = target->GetText();
    return atof(value.c_str());
  }else{
    cout << "Error! Element:" << name << " doesn't exist!" << endl;
    return -1.;
  }
}

//**********************************************************************
void wgEditXML::OPT_SetChipValue(string& name,int idif, int ichip,double value,int mode=0){
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
      cout <<"Warning! Element "<< name <<" doesn't exist!"<<endl;
    }
  }
}

//**********************************************************************
double wgEditXML::OPT_GetChipValue(string& name,int idif, int ichip){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* target = chip->FirstChildElement(name.c_str());
  if(target){
    string value = target->GetText();
    return atof(value.c_str());
  }else{
    cout << "Error! Element:" << name << " doesn't exist!" << endl;
    return -1.;
  }
}

//**********************************************************************
void wgEditXML::PreCalib_Make(string& filename){
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
void wgEditXML::PreCalib_SetValue(string& name,int idif, int ichip, int ich,double value,int mode=0){
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
      cout <<"Warning! Element "<< name <<" doesn't exist!"<<endl;
    }
  }
}

//**********************************************************************
double wgEditXML::PreCalib_GetValue(string& name,int idif, int ichip, int ich){
  XMLElement* data = xml->FirstChildElement("data");
  XMLElement* dif  = data->FirstChildElement(Form("dif_%d",idif));
  XMLElement* chip = dif->FirstChildElement(Form("chip_%d",ichip));
  XMLElement* ch   = chip->FirstChildElement(Form("ch_%d",ich));
  XMLElement* target = ch->FirstChildElement(name.c_str());
  if(target){
    string value = target->GetText();
    return atof(value.c_str());
  }else{
    cout << "Error! Element:" << name << " doesn't exist!" << endl;
    return -1.;
  }
}

//**********************************************************************
void wgEditXML::Calib_Make(string& filename){
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
void wgEditXML::Calib_SetValue(string& name,int idif, int ichip, int ich,double value,int mode=0){
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
      cout <<"Warning! Element "<< name <<" doesn't exist!"<<endl;
    }
  }
}

//**********************************************************************
double wgEditXML::Calib_GetValue(string& name,int idif, int ichip, int ich){
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



