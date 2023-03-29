#include "ReadClock.h"

ReadClock::ReadClock():Tool(){}


bool ReadClock::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  int handle, bID;
  CAEN_DGTZ_ErrorCode ret;

  handle = ReadClock::OpenBoard(m_variables);
  ReadClock::handle = handle;

  m_variables.Get("bID", bID);
  ReadClock::bID = bID;

  return true;
}


bool ReadClock::Execute(){

  int handle=ReadClock::handle, bID=ReadClock::bID;
  CAEN_DGTZ_ErrorCode ret;

  ret = CAEN_DGTZ_WriteRegister(handle, 0x811C, 0x00050000);
  if (!ret) std::cout<<"Board "<<bID<<" CLKOUT propagated to TRGOUT"<<std::endl;
  else std::cout<<ret<<std::endl;

  return true;
}


bool ReadClock::Finalise(){

  int handle=ReadClock::handle, bID=ReadClock::bID;
  CAEN_DGTZ_ErrorCode ret;

  ret = CAEN_DGTZ_Reset(handle);
  if (!ret) std::cout<<"Board "<<bID<<" reset"<<std::endl;
  else std::cout<<ret<<std::endl;

  ret = CAEN_DGTZ_CloseDigitizer(handle);
  if (!ret) std::cout<<"Closed Board "<<bID<<std::endl;

  return true;
}

// Opens board and returns board handle
int ReadClock::OpenBoard(Store m_variables){

  uint32_t address;
  int handle, bID, VME_bridge, LinkNum, ConetNode, verbose;
  CAEN_DGTZ_ErrorCode ret;
  CAEN_DGTZ_BoardInfo_t BoardInfo;

  m_variables.Get("VME_bridge", VME_bridge);
  m_variables.Get("LinkNum", LinkNum);
  m_variables.Get("ConetNode", ConetNode);
  m_variables.Get("verbose", verbose);
  m_variables.Get("bID", bID);

  if (VME_bridge) {

    std::string tmp;

    m_variables.Get("address", tmp);
    address = std::stoi(tmp, 0, 16);
  }
  else address = 0;

  ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink, LinkNum, ConetNode, address, &handle);
  if (ret) std::cout<<"Error opening digitizer. CAEN Error Code: "<<ret<<std::endl;

  ret = CAEN_DGTZ_GetInfo(handle, &BoardInfo);
  if (ret) std::cout <<"Error getting board info. CAEN Error Code: "<<ret<<std::endl;
  else if (!ret && verbose) {
    std::cout<<"Connected to Board "<<bID<<" Model: "<<BoardInfo.ModelName<<std::endl;
  }

  return handle;
}
