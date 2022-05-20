#include "ReadoutBoard.h"

ReadoutBoard::ReadoutBoard():Tool(){}


bool ReadoutBoard::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  return true;
}


bool ReadoutBoard::Execute(){

  int handle, bID, verbose, Nb=0, Ne=0;
  uint32_t AllocatedSize, BufferSize, NumEvents;
  uint64_t StartTime, CurrentTime, ElapsedTime, PrevRateTime, AcqDuration;
  char *buffer = NULL;
  CAEN_DGTZ_ErrorCode ret;

  m_variables.Get("bID", bID);
  m_variables.Get("verbose", verbose);

  AcqDuration = m_data->AcqDuration * 1000;
  handle = m_data->handles[bID-1];

  ret = CAEN_DGTZ_MallocReadoutBuffer(handle,&buffer,&AllocatedSize);

  ret = CAEN_DGTZ_SWStartAcquisition(handle);
  if (ret) {
    std::cout<<"Error starting acquisition"<<std::endl;
    return true;
  }
  StartTime = ReadoutBoard::get_time();
  PrevRateTime = ReadoutBoard::get_time();
  std::cout<<"Acquisition started!"<<std::endl;

  while(ReadoutBoard::get_time()<(AcqDuration + StartTime)) {

    ret = CAEN_DGTZ_ReadData(handle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &BufferSize);
    if (ret) {
      std::cout<<"Readout Error"<<std::endl;
      return true;
    }

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
    CurrentTime = ReadoutBoard::get_time();
    ElapsedTime = CurrentTime - PrevRateTime;

    if (ElapsedTime > 1000) {
      if (Nb==0) std::cout<<"No data..."<<std::endl;
      else {
        std::cout<<"Data rate Board "<<bID<<": "<<(float)Nb/((float)ElapsedTime*1048.576f)<<" MB/s   Trigger Rate Board "<<bID<<": "<<((float)Ne*1000.0f)/(float)ElapsedTime<<" Hz"<<std::endl;
      }

      Nb = 0;
      Ne = 0;
      PrevRateTime = CurrentTime;
    }
  }

  ret = CAEN_DGTZ_SWStopAcquisition(handle);
  if (!ret && verbose) std::cout<<"Acquisition Stopped"<<std::endl;

  return true;
}


bool ReadoutBoard::Finalise(){

  return true;
}


 long ReadoutBoard::get_time(){

  long time_ms;

  struct timeval tv;
  struct timezone tz;

  gettimeofday(&tv, &tz);

  time_ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

  return time_ms;
}
