#ifndef ReadBoard_H
#define ReadBoard_H

#include <string>
#include <iostream>

#include "Tool.h"
#include "CAENDigitizer.h"
#include "CAENDigitizerType.h"
#include <sys/time.h>

/**
 * \class ReadBoard
 *
 * This is a balnk template for a Tool used by the script to generate a new custom tool. Please fill out the descripton and author information.
*
* $Author: B.Richards $
* $Date: 2019/05/28 10:44:00 $
*/

class ReadBoard: public Tool {


 public:

  ReadBoard(); ///< Simple constructor
  bool Initialise(std::string configfile,DataModel &data); ///< Initialise Function for setting up Tool resorces. @param configfile The path and name of the dynamic configuration file to read in. @param data A reference to the transient data class used to pass information between Tools.
  bool Execute(); ///< Executre function used to perform Tool perpose. 
  bool Finalise(); ///< Finalise funciton used to clean up resorces.

  static long get_time();

  int OpenBoard(Store m_variables);
  bool ConfigureBoard(int handle, Store m_variables);

  uint32_t bID;
  uint64_t PrevRateTime;
  int handle, verbose, acq_started=0, Nb, Ne;
  char* buffer;

 private:





};


#endif
