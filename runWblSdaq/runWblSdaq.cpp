#include "runWblSdaq.h"

runWblSdaq::runWblSdaq():Tool(){}


bool runWblSdaq::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  std::string jsonfile, WbLSdaq_path;
  m_variables.Get("jsonfile", jsonfile);
  runWblSdaq::jsonfile = jsonfile;

  m_variables.Get("WbLSdaq_path", WbLSdaq_path);
  runWblSdaq::WbLSdaq_path = WbLSdaq_path;

  return true;
}


bool runWblSdaq::Execute(){

  std::string cmd = "cd "+runWblSdaq::WbLSdaq_path+"; ./WbLSdaq "+runWblSdaq::jsonfile;
//  std::string cmd = "cd ~/WbLSdaq; ./WbLSdaq "+runWblSdaq::jsonfile;
//  std::cout<<cmd.c_str()<<std::endl;
//  system("cd ~/WbLSdaq; ./WbLSdaq ~/WbLSdaq/example-v1730-config.json");
  system(cmd.c_str());

  return true;
}


bool runWblSdaq::Finalise(){

  return true;
}
