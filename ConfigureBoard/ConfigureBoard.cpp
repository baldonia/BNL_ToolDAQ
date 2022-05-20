#include "ConfigureBoard.h"

ConfigureBoard::ConfigureBoard():Tool(){}


bool ConfigureBoard::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log;

//  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  return true;
}


bool ConfigureBoard::Execute(){

  uint32_t recLen, chanMask, bID, postTrig, thresh, DCOff;
  uint32_t length, mask, percent;
  int handle, verbose;
  float dynRange;
  std::string polarity, tmp;
  CAEN_DGTZ_ErrorCode ret;
  CAEN_DGTZ_TriggerPolarity_t pol = CAEN_DGTZ_TriggerOnRisingEdge;

  m_variables.Get("RecLen", recLen);
  m_variables.Get("ChanEnableMask", tmp);
  m_variables.Get("bID", bID);
  m_variables.Get("PostTrig", postTrig);
  m_variables.Get("DynRange", dynRange);
  m_variables.Get("thresh", thresh);
  m_variables.Get("polarity", polarity);
  m_variables.Get("DCOff", DCOff);
  m_variables.Get("verbose", verbose);

  chanMask = std::stoi(tmp, 0, 16);

  handle = m_data->handles[bID-1];

  ret = CAEN_DGTZ_Reset(handle);
  if (!ret && verbose) std::cout<<"Board reset"<<std::endl;

  ret = CAEN_DGTZ_WriteRegister(handle,0xEF08,bID);
  if (!ret && verbose) std::cout<<"Set Board ID!"<<std::endl;

  ret = CAEN_DGTZ_SetRecordLength (handle,recLen);
  ret = CAEN_DGTZ_GetRecordLength(handle,&length);
  if (!ret && verbose) std::cout<<"Record Length set to: "<<length<<std::endl;

  ret = CAEN_DGTZ_SetPostTriggerSize(handle, postTrig);
  ret = CAEN_DGTZ_GetPostTriggerSize(handle, &percent);
  if (!ret && verbose) std::cout<<"Post Trigger set to: "<<percent<<"%"<<std::endl;

  if(dynRange==2.0) {
    ret = CAEN_DGTZ_WriteRegister(handle, 0x8028, 0);
    if (!ret && verbose) std::cout<<"Dynamic Range set to 2 Vpp"<<std::endl;
  }
  else if (dynRange==0.5) {
    ret = CAEN_DGTZ_WriteRegister(handle, 0x8028, 1);
    if (!ret && verbose) std::cout<<"Dynamic Range set to 0.5 Vpp"<<std::endl;
  }
  else {
    std::cout<<"Invalid Dynamic Range Input"<<std::endl;
    ret = CAEN_DGTZ_GenericError;
  }

  ret = CAEN_DGTZ_WriteRegister(handle, 0x8080, thresh);
  if (!ret && verbose) std::cout<<"Trigger threshold set"<<std::endl;

  if (polarity!="positive" && polarity!="negative") {
    std::cout<<"Invalid trigger polarity. Must be 'positive' or 'negative'"<<std::endl;
    ret = CAEN_DGTZ_GenericError;
  }
  else if (polarity=="positive") {
    ret = CAEN_DGTZ_SetTriggerPolarity(handle,0,CAEN_DGTZ_TriggerOnRisingEdge);
    ret = CAEN_DGTZ_GetTriggerPolarity(handle,0,&pol);
  }
  else if (polarity=="negative") {
    ret = CAEN_DGTZ_SetTriggerPolarity(handle,0,CAEN_DGTZ_TriggerOnFallingEdge);
    ret = CAEN_DGTZ_GetTriggerPolarity(handle,0,&pol);
  }
  if (!ret && verbose && pol==0) std::cout<<"Trigger polarity set to: positive"<<std::endl;
  else if (!ret && verbose && pol==1) std::cout<<"Trigger polarity set to: negative"<<std::endl;

  ret = CAEN_DGTZ_WriteRegister(handle, 0x8098, DCOff);
  if (!ret && verbose) std::cout<<"DC Offset set"<<std::endl;

  ret = CAEN_DGTZ_SetChannelEnableMask(handle, chanMask);
  ret = CAEN_DGTZ_GetChannelEnableMask(handle, &mask);
  if (!ret && verbose) std::cout<<"Channels Enabled: "<<std::hex<<mask<<std::dec<<std::endl;

  ret = CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED);

  ret = CAEN_DGTZ_SetChannelSelfTrigger(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY, chanMask);

  return true;
}


bool ConfigureBoard::Finalise(){

  return true;
}
