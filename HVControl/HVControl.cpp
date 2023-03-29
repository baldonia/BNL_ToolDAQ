#include "HVControl.h"
#include <typeinfo>

HVControl::HVControl():Tool(){}


bool HVControl::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  std::string jsonfile;
  m_variables.Get("JSON_file", jsonfile);
  RunDB db;
  db.addFile(jsonfile);
  RunTable hv = db.getTable("Connection", "HV0");  //fix to not have to specify "HV0"

  HVControl::sys_index = hv.getIndex();

  CAENHV_SYSTEM_TYPE_t sysType;
  std::string system = hv["system"].cast<std::string>();
  if (system=="SY5527") sysType = SY5527;

  int link = hv["LinkType"].cast<int>();
  std::string IP = hv["IP"].cast<std::string>().data();
  std::string username = hv["Username"].cast<std::string>();
  std::string passwd = hv["Password"].cast<std::string>();

  char arg[30];
  strcpy(arg, IP.data());

  CAENHVRESULT ret;
  int handle;

  ret = CAENHV_InitSystem(sysType, link, arg, username.data(), passwd.data(), &handle);
  if (ret) {
    std::cerr<<"Error connecting to HV system: "<<CAENHV_GetError(handle)<<" Error no. "<<ret<<std::endl;
    return false;
  } else {std::cout<<"Successfully connected to: "<<hv.getIndex()<<std::endl;}

  HVControl::handle = handle;
  return true;
}


bool HVControl::Execute(){

  int handle = HVControl::handle;

  CAENHVRESULT ret;
  std::string prop = "SwRelease";
  char result[30];

  ret = CAENHV_GetSysProp(handle, prop.data(), result);

  if (ret) {
    std::cerr<<"Error in Keep Alive Loop: "<<CAENHV_GetError(handle)<<" Error no. "<<ret<<std::endl;
    return false;
  }

  return true;
}


bool HVControl::Finalise(){

  int handle = HVControl::handle;
  std::string sys_index = HVControl::sys_index;

  CAENHVRESULT ret;

  ret = CAENHV_DeinitSystem(handle);
  if (ret) {
    std::cerr<<"Error disconnecting from HV system: "<<CAENHV_GetError(handle)<<" Error no. "<<ret<<std::endl;
    return false;
  }else {std::cout<<"Disconnected from "<<sys_index<<std::endl;}

  return true;
}
