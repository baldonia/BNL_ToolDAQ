# BNL_ToolDAQ

 A supplement based on the [ToolDAQ Framework](https://docs.google.com/document/d/18rgYMOAGt3XiW9i0qN9kfUK1ssbQgLV1gQgG3hyVUoA/edit).
 BNL_ToolDAQ is developed for the 1-ton and 30-ton WbLS experiments at Brookhaven National Lab. BNL_ToolDAQ configures and reads CAEN V1730S digitizers to output
 binary data files.
 
 ### Prerequisite
 You must have installed ToolDAQ to use the tools in BNL_ToolDAQ. Installation instructions can be found using the above link. You must also copy the folders of the tools you would like to use (ReadBoard, OpenDigitizer, etc) into ToolApplication/UserTools/InactiveTools. Then run the ToolSelect.sh script to activate those tools. Now you should be able to use these supplementary tools.
 
 ### Before running
 Navigate to the configfiles directory and adjust the [digitizer configuration files](docs/digitizer_config_files.md) (`config_b1` for example) and [individual channel setting files](docs/channel_settings_config_files.md) (`b1_chan_set.txt` for example) for the digitizers you want to run. If this is the first time running ToolDAQ in a session, navigate to the ToolApplication directory and source the shell script `source Setup.sh`.
 
 ### To run
 Navigate to the ToolApplication directory then run `./main <your Tool Chain config file>`. If you are running in Interactive Mode, type "Start" to begin the data acquisition. When you would like to stop the acquisition, type "Stop", then "Quit".
 
 ### To add a digitizer
 Create a digitizer configuration file in the configfiles directory using `config_b1` as a template and change the settings to your desired configuration (see [here](docs/digitizer_config_files.md)). Create an individual channel settings configuration file in the configfiles directory using `b1_chan_settings.txt` as a template and change the 
 settings to your desired configuration (see [here](docs/channel_settings_config_files.md)). Add a line to the ToolsConfig file containing `<unique name of Tool instance> <ReadBoard> 
 <path to digitizer config file>`. 
