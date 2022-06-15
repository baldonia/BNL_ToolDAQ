# BNL_ToolDAQ

 An application using the [ToolDAQ Framework](https://docs.google.com/document/d/18rgYMOAGt3XiW9i0qN9kfUK1ssbQgLV1gQgG3hyVUoA/edit).
 BNL_ToolDAQ is developed for the 1-ton and 30-ton WbLS experiments at Brookhaven National Lab. BNL_ToolDAQ configures and reads CAEN V1730S digitizers to output
 binary data files.
 
 ### To run
 Navigate to the ToolApplication directory then run `./main <your Tool Chain config file>`. Before running the first time, you must source the shell script 
 `source Setup.sh` in the ToolApplication directory.
 
 ### To add a digitizer
 Create a digitizer configuration file in the configfiles directory using `config_b1` as a template and change the settings to your desired configuration (see config 
 files readme). Create an individual channel settings configuration file in the configfiles directory using `b1_chan_settings.txt` as a template and change the 
 settings to your desired configuration (see config files readme). Add a line to the ToolsConfig file containing `<unique name of Tool instance> <ReadBoard> 
 <path to digitizer config file>`. 
