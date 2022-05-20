#include "TestTool.h"
#include <stdio.h>

TestTool::TestTool():Tool(){}


bool TestTool::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

//  FILE *pfile = fopen("output.bin", "wb");

  return true;
}


bool TestTool::Execute(){

  int handle, handle2, handle3;
  CAEN_DGTZ_ErrorCode ret;
  uint32_t size, bsize, length;
  CAEN_DGTZ_BoardInfo_t BoardInfo, BoardInfo2, BoardInfo3;
  char *buffer = NULL;

  FILE *outfile = fopen("output.bin", "wb");

  // Connect to the Digitizer
  ret = CAEN_DGTZ_OpenDigitizer (
			CAEN_DGTZ_OpticalLink,
			0,
			0,
			0x32130000,            // Use this if optical link is connected to VME bridge
//			0,                     // Use this if optical link is connected to digitizer
			&handle
			);

  ret = CAEN_DGTZ_GetInfo (
			handle,
			&BoardInfo
			);

 if (!ret) {
    std::cout<<"Connected to Board Model: "<<BoardInfo.ModelName<<std::endl;
    std::cout<<"ROC Firmware Release: "<<BoardInfo.ROC_FirmwareRel<<std::endl;
    std::cout<<"AMC Firmware Release: "<<BoardInfo.AMC_FirmwareRel<<std::endl;
  }

  ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink,0,0,0x32100000,&handle2);
  ret = CAEN_DGTZ_GetInfo(handle2,&BoardInfo2);

  if (!ret) { std::cout<<"Connected to Board 1 Model: "<<BoardInfo2.ModelName<<std::endl;}

  ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink,0,0,0x32120000,&handle3);
  ret = CAEN_DGTZ_GetInfo(handle3,&BoardInfo3);

  if (!ret) { std::cout<<"Connected to Board 2 Model: "<<BoardInfo3.ModelName<<std::endl;}

  std::cout<<handle<<std::endl;
  std::cout<<handle2<<std::endl;
  std::cout<<handle3<<std::endl;

  // Reset the digitizer
  ret = CAEN_DGTZ_Reset (handle);
  if (!ret) {std::cout<<"Digitizer Reset"<<std::endl;}

  //==============================================================================
  // Digitizer Configuration


  // Set record length
  ret = CAEN_DGTZ_SetRecordLength (handle, 640);
  ret = CAEN_DGTZ_GetRecordLength (handle, &length);
  if (!ret) {std::cout<<"Record Length set to "<<length<<" samples" <<std::endl;}

  // Enable channels
  ret = CAEN_DGTZ_SetChannelEnableMask (handle, 1);
  if (!ret) { std::cout<<"Channel 0 enabled"<<std::endl; }

  // Acquisition settings
  ret = CAEN_DGTZ_SetAcquisitionMode (handle, CAEN_DGTZ_SW_CONTROLLED);
  ret = CAEN_DGTZ_SetChannelSelfTrigger(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY, 1);
  ret = CAEN_DGTZ_SetChannelTriggerThreshold (handle, 0, 11000);
  ret = CAEN_DGTZ_SetSWTriggerMode (handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);

  // Buffer settings
  ret = CAEN_DGTZ_SetMaxNumEventsBLT (handle, 3);
  if (!ret) {std::cout<<"Max Number of Events for block transfer set to 3"<<std::endl;}
  ret = CAEN_DGTZ_MallocReadoutBuffer (handle, &buffer, &size);

  //=============================================================================
  // Data Acquisition


  ret = CAEN_DGTZ_SWStartAcquisition(handle);
  if (!ret) {std::cout<<"Acquisition started"<<std::endl;}

//  ret = CAEN_DGTZ_SendSWtrigger(handle);
//  if (!ret) {std::cout<<"Software trigger sent"<<std::endl;}

  sleep(5);

  ret = CAEN_DGTZ_ReadData(handle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &bsize);
  if (!ret) {std::cout<<"Data Read!"<<std::endl;}

  ret = CAEN_DGTZ_SWStopAcquisition(handle);
  if (!ret) {std::cout<<"Acquisition stopped"<<std::endl;}


  //==============================================================================

  // Write binary to file
  fwrite (buffer, 1, bsize, outfile);

  // Free Buffer Memory
  ret = CAEN_DGTZ_FreeReadoutBuffer(&buffer);

  // Close Digitizer
  ret = CAEN_DGTZ_CloseDigitizer (handle);
  if (!ret) { std::cout<<"Digitizer closed"<<std::endl;}

  fclose(outfile);

  return true;
}


bool TestTool::Finalise(){

//  fclose(pfile);

  return true;
}
