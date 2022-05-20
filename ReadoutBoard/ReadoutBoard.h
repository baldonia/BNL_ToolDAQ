#ifndef ReadoutBoard_H
#define ReadoutBoard_H

#include <string>
#include <iostream>

#include "Tool.h"
#include "CAENDigitizer.h"
#include "CAENDigitizerType.h"
#include <sys/time.h>

/**
 * \class ReadoutBoard
 *
 * This is a balnk template for a Tool used by the script to generate a new custom tool. Please fill out the descripton and author information.
*
* $Author: B.Richards $
* $Date: 2019/05/28 10:44:00 $
*/

class ReadoutBoard: public Tool {


 public:

  ReadoutBoard(); ///< Simple constructor
  bool Initialise(std::string configfile,DataModel &data); ///< Initialise Function for setting up Tool resorces. @param configfile The path and name of the dynamic configuration file to read in. @param data A reference to the transient data class used to pass information between Tools.
  bool Execute(); ///< Executre function used to perform Tool perpose. 
  bool Finalise(); ///< Finalise funciton used to clean up resorces.

  static long get_time();

 private:





};


#endif
