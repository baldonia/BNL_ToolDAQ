#include "ReadBoard.h"
#include <math.h>

ReadBoard::ReadBoard():Tool(){}


bool ReadBoard::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);

  m_data= &data;
  m_log= m_data->Log;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  int handle, bID;
  uint32_t AllocatedSize;
  char *buffer = NULL;
  CAEN_DGTZ_ErrorCode ret;

  handle = ReadBoard::OpenBoard(m_variables);
  ReadBoard::handle = handle;

  m_variables.Get("bID", bID);
  ReadBoard::bID = bID;

  bool test = ReadBoard::ConfigureBoard(handle, m_variables);
  if(!test) {
    std::cout<<"Board "<<bID<<" configuration failed!!!"<<std::endl;
    return true;
  }
  else {
    std::cout<<"Board "<<bID<<" configured!"<<std::endl;
  }

  ret = CAEN_DGTZ_MallocReadoutBuffer(handle,&buffer,&AllocatedSize);
  if (ret) {
    std::cout<<"Error allocating readout buffer"<<std::endl;
    return true;
  }

  ReadBoard::buffer = buffer;

  std::string ofile;
  m_variables.Get("ofile", ofile);

  std::string timestamp = ReadBoard::Get_TimeStamp();

  ofile = ofile + "_" + timestamp + ".bin";

  if (!m_data->outfile.is_open()) {
    m_data->outfile.open(ofile, std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);
  }

  usleep(1000000);

  return true;
}


bool ReadBoard::Execute(){

  int handle=ReadBoard::handle, bID=ReadBoard::bID, Nb=ReadBoard::Nb, Ne=ReadBoard::Ne;
  uint32_t BufferSize, NumEvents;
  uint64_t CurrentTime, ElapsedTime;
  CAEN_DGTZ_ErrorCode ret;

  if (!(ReadBoard::acq_started)) {
    ret = CAEN_DGTZ_SWStartAcquisition(handle);
    if (ret)  {
      std::cout<<"Error starting acquisition"<<std::endl;
      return true;
    }
    std::cout<<"Acquisition started!"<<std::endl;
    ReadBoard::acq_started = 1;
    Nb = 0;
    Ne = 0;
    ReadBoard::PrevRateTime = ReadBoard::get_time();
  }

  ret = CAEN_DGTZ_ReadData(handle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, ReadBoard::buffer, &BufferSize);
  if (ret) {
    std::cout<<"ReadData Error"<<std::endl;
    return true;
  }

  if (BufferSize) m_data->outfile.write(ReadBoard::buffer, BufferSize);

  NumEvents = 0;
  if (BufferSize != 0) {
    ret = CAEN_DGTZ_GetNumEvents(handle, ReadBoard::buffer, BufferSize, &NumEvents);
    if (ret) {
      std::cout<<"Readout Error"<<std::endl;
      return true;
    }
  }

  Nb += BufferSize;
  Ne += NumEvents;
  CurrentTime = ReadBoard::get_time();
  ElapsedTime = CurrentTime - ReadBoard::PrevRateTime;

  int show_data_rate;
  m_variables.Get("show_data_rate", show_data_rate);
  if ((ElapsedTime > 2000) && show_data_rate) {
    if (Nb==0) std::cout<<"Board "<<bID<<": No data..."<<std::endl;
    else {
      std::cout<<"Data rate Board "<<bID<<": "<<(float)Nb/((float)ElapsedTime*1048.576f)<<" MB/s   Trigger Rate Board "<<bID<<": "<<((float)Ne*1000.0f)/(float)ElapsedTime<<" Hz"<<std::endl;
    }

    Nb = 0;
    Ne = 0;
    ReadBoard::PrevRateTime = CurrentTime;
  }

  ReadBoard::Ne = Ne;
  ReadBoard::Nb = Nb;


  return true;
}


bool ReadBoard::Finalise(){

  int handle=ReadBoard::handle, bID=ReadBoard::bID, verbose=ReadBoard::verbose;
  CAEN_DGTZ_ErrorCode ret;
  char* buffer = ReadBoard::buffer;

  ret = CAEN_DGTZ_SWStopAcquisition(handle);
  if (!ret && verbose) {
    std::cout<<"Acquisition Stopped"<<std::endl;
    ReadBoard::acq_started = 0;
  }
  ret = CAEN_DGTZ_FreeReadoutBuffer(&buffer);

  if (m_data->outfile.is_open()) {
    m_data->outfile.close();
  }

  ret = CAEN_DGTZ_CloseDigitizer(handle);
  if (!ret && verbose) std::cout<<"Closed Board "<<bID<<std::endl;

  return true;
}

//===========================================================================================

// Gets current time in milliseconds
long ReadBoard::get_time(){

  long time_ms;

  struct timeval tv;
  struct timezone tz;

  gettimeofday(&tv, &tz);

  time_ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

  return time_ms;
}

// Gets timestamp as a string (DDMMYYYYTHHMM)
std::string ReadBoard::Get_TimeStamp(){

  char timestamp[13];

  time_t rawtime;
  struct tm* timeinfo;

  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(timestamp, 13, "%y%m%dT%H%M", timeinfo);

  return timestamp;
}

// Opens board and returns board handle
int ReadBoard::OpenBoard(Store m_variables){

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

// Sets board settings like trigger threshold, record length, DC offset, etc.
bool ReadBoard::ConfigureBoard(int handle, Store m_variables) {

  uint32_t recLen, ChanEnableMask, postTrig, thresh, DCOff, ChanSelfTrigMask;
  uint32_t length, mask, percent, reg;
  int bID, verbose, use_ETTT;
  float dynRange;
  std::string polarity, tmp, tmp2, ChanSelfTrigMode, TrigInMode, SWTrigMode;
  CAEN_DGTZ_TriggerPolarity_t pol;
  CAEN_DGTZ_TriggerMode_t selftrigmode;
  CAEN_DGTZ_ErrorCode ret;

  m_variables.Get("RecLen", recLen);
  m_variables.Get("ChanEnableMask", tmp);
  m_variables.Get("PostTrig", postTrig);
  m_variables.Get("DynRange", dynRange);
  m_variables.Get("thresh", thresh);
  m_variables.Get("polarity", polarity);
  m_variables.Get("ChanSelfTrigMask", tmp2);
  m_variables.Get("ChanSelfTrigMode", ChanSelfTrigMode);
  m_variables.Get("TrigInMode", TrigInMode);
  m_variables.Get("SWTrigMode", SWTrigMode);
  m_variables.Get("DCOff", DCOff);
  m_variables.Get("bID", bID);
  m_variables.Get("verbose", verbose);
  m_variables.Get("use_ETTT", use_ETTT);

  ChanEnableMask = std::stoi(tmp, 0, 16);
  ChanSelfTrigMask = std::stoi(tmp2, 0, 16);

  ret = CAEN_DGTZ_Reset(handle);
  if (!ret && verbose) std::cout<<"Board reset"<<std::endl;
  else if (ret) {
    std::cout<<"Board reset failed!!!"<<std::endl;
    return false;
  }

  ret = CAEN_DGTZ_WriteRegister(handle,0xEF08,bID);
  if (!ret && verbose) std::cout<<"Set Board ID!"<<std::endl;
  else if (ret) {
    std::cout<<"Error setting board ID"<<std::endl;
    return false;
  }

  ret = CAEN_DGTZ_SetRecordLength (handle,recLen);
  ret = CAEN_DGTZ_GetRecordLength(handle,&length);
  if (!ret && verbose) std::cout<<"Record Length set to: "<<length<<std::endl;
  else if (ret) {
    std::cout<<"Error setting record length"<<std::endl;
    return false;
  }

  ret = CAEN_DGTZ_SetPostTriggerSize(handle, postTrig);
  ret = CAEN_DGTZ_GetPostTriggerSize(handle, &percent);
  if (!ret && verbose) std::cout<<"Post Trigger set to: "<<percent<<"%"<<std::endl;
  else if (ret) {
    std::cout<<"Error setting post trigger size"<<std::endl;
    return false;
  }

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
  else if (ret) {
    std::cout<<"Error setting trigger polarity"<<std::endl;
    return false;
  }

  ret = CAEN_DGTZ_SetChannelEnableMask(handle, ChanEnableMask);
  ret = CAEN_DGTZ_GetChannelEnableMask(handle, &mask);
  if (!ret && verbose) std::cout<<"Channels Enabled: "<<std::hex<<mask<<std::dec<<std::endl;
  else if (ret) {
    std::cout<<"Error enabling channels"<<std::endl;
    return false;
  }

  if (TrigInMode=="DISABLED") {
    ret = CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_DISABLED);
  }
  else if (TrigInMode=="EXTOUT_ONLY") {
    ret = CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_EXTOUT_ONLY);
  }
  else if (TrigInMode=="ACQ_ONLY") {
    ret = CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);
  }
  else if (TrigInMode=="ACQ_AND_EXTOUT") {
    ret = CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT);
  }
  else {
    std::cout<<"TrigInMode must be DISABLED/EXTOUT_ONLY/ACQ_ONLY/ACQ_AND_EXTOUT"<<std::endl;
    ret = CAEN_DGTZ_GenericError;
  }

  if (SWTrigMode=="DISABLED") {
    ret = CAEN_DGTZ_SetSWTriggerMode(handle, CAEN_DGTZ_TRGMODE_DISABLED);
  }
  else if (SWTrigMode=="EXTOUT_ONLY") {
    ret = CAEN_DGTZ_SetSWTriggerMode(handle, CAEN_DGTZ_TRGMODE_EXTOUT_ONLY);
  }
  else if (SWTrigMode=="ACQ_ONLY") {
    ret = CAEN_DGTZ_SetSWTriggerMode(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);
  }
  else if (SWTrigMode=="ACQ_AND_EXTOUT") {
    ret = CAEN_DGTZ_SetSWTriggerMode(handle, CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT);
  }
  else {
    std::cout<<"SWTrigMode must be DISABLED/EXTOUT_ONLY/ACQ_ONLY/ACQ_AND_EXTOUT"<<std::endl;
    ret = CAEN_DGTZ_GenericError;
  }


  ret = CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED);
  if (ret) {
    std::cout<<"Error setting acquisition mode"<<std::endl;
    return false;
  }

  if (use_ETTT) {
    ret = CAEN_DGTZ_ReadRegister(handle, 0x811C, &reg);
    reg = (reg & 0xFF0FFFFF) | 0x00400000;
    ret = CAEN_DGTZ_WriteRegister(handle, 0x811C, reg);
    if (ret) {
      std::cout<<"Error setting ETTT"<<std::endl;
      return false;
    }
  }

  ret = CAEN_DGTZ_SetMaxNumEventsBLT(handle, 10);
  if (ret) {
    std::cout<<"Error setting Max Num Events BLT"<<std::endl;
    return false;
  }

  int use_global;
  m_variables.Get("use_global", use_global);

// Use the global settings
  if (use_global) {

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
    if (ret) {
      std::cout<<"Error setting dynamic range"<<std::endl;
      return false;
    }

    ret = CAEN_DGTZ_WriteRegister(handle, 0x8080, thresh);
    if (!ret && verbose) std::cout<<"Trigger threshold set"<<std::endl;
    else if (ret) {
      std::cout<<"Error setting trigger threshold"<<std::endl;
      return false;
    }

    ret = CAEN_DGTZ_WriteRegister(handle, 0x8098, DCOff);
    if (!ret && verbose) std::cout<<"DC Offset set"<<std::endl;
    else if (ret) {
      std::cout<<"Error setting DC offset"<<std::endl;
      return false;
    }

    if (ChanSelfTrigMode=="DISABLED") {
      ret = CAEN_DGTZ_SetChannelSelfTrigger(handle, CAEN_DGTZ_TRGMODE_DISABLED, ChanSelfTrigMask);
    }
    else if (ChanSelfTrigMode=="EXTOUT_ONLY") {
      ret = CAEN_DGTZ_SetChannelSelfTrigger(handle, CAEN_DGTZ_TRGMODE_EXTOUT_ONLY, ChanSelfTrigMask);
    }
    else if (ChanSelfTrigMode=="ACQ_ONLY") {
      ret = CAEN_DGTZ_SetChannelSelfTrigger(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY, ChanSelfTrigMask);
    }
    else if (ChanSelfTrigMode=="ACQ_AND_EXTOUT") {
      ret = CAEN_DGTZ_SetChannelSelfTrigger(handle, CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT, ChanSelfTrigMask);
    }
    else {
      std::cout<<"ChanSelfTrigMode must be DISABLED/EXT_ONLY/ACQ_ONLY/ACQ_AND_EXTOUT"<<std::endl;
      ret = CAEN_DGTZ_GenericError;
    }
    if (ret) {
      std::cout<<"Error setting channel self trigger"<<std::endl;
      return false;
    }

  }

// Use the individual channel settings
  else {

    std::string path;
    m_variables.Get("chan_set_file", path);

    std::ifstream ifile;
    ifile.open(path);

    if (!(ifile.is_open())) {
      std::cout<<"Error opening channel settings file"<<std::endl;
      return false;
    }

    int value, chan, reg;
    std::string line;
    std::getline(ifile, line);

    while(std::getline(ifile, line, ',')) {

      chan = std::stoi(line);

      std::getline(ifile, line, ',');
      value = std::stoi(line);
      ret = CAEN_DGTZ_SetChannelDCOffset(handle, chan, value);
      if (ret) {
        std::cout<<"Error setting channel "<<chan<<" DC offset"<<std::endl;
        return false;
      }

      std::getline(ifile, line, ',');
      value = std::stoi(line);
      ret = CAEN_DGTZ_SetChannelTriggerThreshold(handle, chan, value);
      if (ret) {
        std::cout<<"Error setting channel "<<chan<<" trigger threshold"<<std::endl;
        return false;
      }

      if (chan%2) std::getline(ifile, line);
      else std::getline(ifile, line, ',');
      reg = 0x1028 + (0x100 * chan);
      if (line=="2.0") {
        ret = CAEN_DGTZ_WriteRegister(handle, reg, 0);
      }
      else if (line=="0.5") {
        ret = CAEN_DGTZ_WriteRegister(handle, reg, 1);
      }
      else {
        ret = CAEN_DGTZ_GenericError;
        std::cout<<"Invalid dynamic range, must be 2.0 or 0.5"<<std::endl;
      }
      if (ret) {
        std::cout<<"Error setting channel "<<chan<<" dynamic range"<<std::endl;
        return false;
      }

      if (!(chan%2)) {

        int ch_to_set = int(pow(2, chan));

        std::getline(ifile, line, ',');
        if (line=="EXTOUT_ONLY") {
          ret = CAEN_DGTZ_SetChannelSelfTrigger(handle, CAEN_DGTZ_TRGMODE_EXTOUT_ONLY, ch_to_set);
        }
        else if (line=="ACQ_ONLY") {
          ret = CAEN_DGTZ_SetChannelSelfTrigger(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY, ch_to_set);
        }
        else if (line=="ACQ_AND_EXTOUT") {
          ret = CAEN_DGTZ_SetChannelSelfTrigger(handle, CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT, ch_to_set);
        }
        else if (line=="DISABLED") {
          ret = CAEN_DGTZ_SetChannelSelfTrigger(handle, CAEN_DGTZ_TRGMODE_DISABLED, ch_to_set);
        }
        else {
          std::cout<<"Channel self trigger mode must be DISABLED/ACQ_ONLY/EXTOUT_ONLY/ACQ_AND_EXTOUT"<<std::endl;
          ret = CAEN_DGTZ_GenericError;
        }
        if (ret) {
          std::cout<<"Error setting channel "<<chan<<" self trigger mode"<<std::endl;
          return false;
        }

        reg = 0x1084 + (chan * 0x100);

        std::getline(ifile, line);
        if (line=="OR") {
          ret = CAEN_DGTZ_WriteRegister(handle, reg, 3);
        }
        else if (line=="AND") {
          ret = CAEN_DGTZ_WriteRegister(handle, reg, 0);
        }
        else if (line=="Xn") {
          ret = CAEN_DGTZ_WriteRegister(handle, reg, 1);
        }
        else if (line=="Xn+1") {
          ret = CAEN_DGTZ_WriteRegister(handle, reg, 2);
        }
        else {
          std::cout<<"Channel self trigger logic must be OR/AND/Xn/Xn+1"<<std::endl;
          ret = CAEN_DGTZ_GenericError;
        }
        if (ret) {
          std::cout<<"Error setting channel "<<chan<<" self trigger logic"<<std::endl;
          return false;
        }
      }
    }
    ifile.close();
  }
  return true;
}
