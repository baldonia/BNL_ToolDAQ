#include "ReadBoard.h"

ReadBoard::ReadBoard():Tool(){}


bool ReadBoard::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;


  uint32_t address, bID;
  int verbose, handle;
  std::string tmp1, ofile;
  CAEN_DGTZ_ErrorCode ret;
  CAEN_DGTZ_BoardInfo_t BoardInfo;
//  std::ofstream outfile;
//  std::cout<<outfile.is_open()<<std::endl;

  m_variables.Get("address", tmp1);
  m_variables.Get("bID", bID);
  m_variables.Get("verbose", verbose);
  m_variables.Get("ofile", ofile);

  ReadBoard::bID = bID;
  ReadBoard::verbose = verbose;

  address = std::stoi(tmp1, 0, 16);

  if (!(bID>0) || bID>m_data->num_boards) {
    std::cout<<"Error: Board ID must be integer between 1 and the number of boards"<<std::endl;
    return true;
  }

//  ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink,0,0,address,&handle);
  ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink,0,0,0,&handle);
  ReadBoard::handle = handle;

  ret = CAEN_DGTZ_GetInfo(handle,&BoardInfo);
  if (!ret && verbose) std::cout<<"Connected to Board "<<bID<<" Model: "<<BoardInfo.ModelName<<std::endl;

  uint32_t recLen, chanMask, postTrig, thresh, DCOff;
  uint32_t length, mask, percent;
  float dynRange;
  std::string polarity, tmp2;
  CAEN_DGTZ_TriggerPolarity_t pol = CAEN_DGTZ_TriggerOnRisingEdge;

  m_variables.Get("RecLen", recLen);
  m_variables.Get("ChanEnableMask", tmp2);
  m_variables.Get("PostTrig", postTrig);
  m_variables.Get("DynRange", dynRange);
  m_variables.Get("thresh", thresh);
  m_variables.Get("polarity", polarity);
  m_variables.Get("DCOff", DCOff);

  chanMask = std::stoi(tmp2, 0, 16);

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

  uint32_t AllocatedSize;
  char *buffer = NULL;

//  FILE *outfile = fopen("output.bin", "wb");

  if (!m_data->outfile.is_open()) {
    m_data->outfile.open(ofile, std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);
  }

  ret = CAEN_DGTZ_MallocReadoutBuffer(handle,&buffer,&AllocatedSize);
  if (ret) {
    std::cout<<"Error allocating readout buffer"<<std::endl;
    return true;
  }

  ReadBoard::buffer = buffer;

  usleep(1000000);

  return true;
}


bool ReadBoard::Execute(){

  int handle=ReadBoard::handle, Nb=ReadBoard::Nb, Ne=ReadBoard::Ne;
  uint32_t BufferSize, NumEvents;
  uint64_t CurrentTime, ElapsedTime;
  CAEN_DGTZ_ErrorCode ret;
  char *buffer = ReadBoard::buffer;
//  char *buffer = NULL;
//  uint32_t AllocatedSize;

  if (!(ReadBoard::acq_started)) {
//    ret = CAEN_DGTZ_MallocReadoutBuffer(handle,&buffer,&AllocatedSize);
//    usleep(1000000);
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

//  usleep(2000000);
  ret = CAEN_DGTZ_ReadData(handle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &BufferSize);
  if (ret) {
    std::cout<<"ReadData Error"<<std::endl;
    return true;
  }
//  fwrite(ReadBoard::buffer, 1, BufferSize, outfile);

  if (BufferSize) m_data->outfile.write(buffer, BufferSize);

  NumEvents = 0;
  if (BufferSize != 0) {
    ret = CAEN_DGTZ_GetNumEvents(handle, buffer, BufferSize, &NumEvents);
    if (ret) {
      std::cout<<"Readout Error"<<std::endl;
      return true;
    }
  }

  Nb += BufferSize;
  Ne += NumEvents;
  CurrentTime = ReadBoard::get_time();
  ElapsedTime = CurrentTime - ReadBoard::PrevRateTime;

  if (ElapsedTime > 1000) {
    std::cout<<Ne<<std::endl;
    if (Nb==0) std::cout<<"No data..."<<std::endl;
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

//  fclose(outfile);

  if (m_data->outfile.is_open()) {
    m_data->outfile.close();
  }

  ret = CAEN_DGTZ_CloseDigitizer(handle);
  if (!ret && verbose) std::cout<<"Closed Board "<<bID<<std::endl;

  return true;
}


long ReadBoard::get_time(){

  long time_ms;

  struct timeval tv;
  struct timezone tz;

  gettimeofday(&tv, &tz);

  time_ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

  return time_ms;
}
