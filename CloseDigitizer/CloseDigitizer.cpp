#include "CloseDigitizer.h"

CloseDigitizer::CloseDigitizer():Tool(){}


bool CloseDigitizer::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  return true;
}


bool CloseDigitizer::Execute(){

  int bID, handle, verbose;
  CAEN_DGTZ_ErrorCode ret;

  m_variables.Get("bID", bID);
  m_variables.Get("verbose", verbose);

  handle = m_data->handles[bID-1];

  ret = CAEN_DGTZ_CloseDigitizer(handle);
  if (!ret && verbose) std::cout<<"Closed Board "<<bID<<std::endl;

  return true;
}


bool CloseDigitizer::Finalise(){

  return true;
}
