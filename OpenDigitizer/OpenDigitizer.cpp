#include "OpenDigitizer.h"

OpenDigitizer::OpenDigitizer():Tool(){}


bool OpenDigitizer::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  return true;
}


bool OpenDigitizer::Execute(){

  uint32_t address;
  int bID, verbose, handle;
  std::string tmp;
  CAEN_DGTZ_ErrorCode ret;
  CAEN_DGTZ_BoardInfo_t BoardInfo;

  m_variables.Get("address", tmp);
  m_variables.Get("bID", bID);
  m_variables.Get("verbose", verbose);

  address = std::stoi(tmp, 0, 16);

  if (!(bID>0) || bID>m_data->num_boards) {
    std::cout<<"Error: Board ID must be integer between 1 and the number of boards"<<std::endl;
    return true;
  }

  ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink,0,0,address,&handle);
  m_data->handles[bID-1] = handle;

  ret = CAEN_DGTZ_GetInfo(handle,&BoardInfo);
  if (!ret && verbose) std::cout<<"Connected to Board "<<bID<<" Model: "<<BoardInfo.ModelName<<std::endl;

  return true;
}


bool OpenDigitizer::Finalise(){

  return true;
}
