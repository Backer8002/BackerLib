
# How to use the logger

## Init functions

- loggingInit(size\_t queueSize,bool shouldMemLogm,shouldLogFileLocation)  
	>It will start the worker thread and create the log queue.  
	>Save the pointer returnd from it since it is the logging handle.

- loggingRegFileHandleToLogLevel(FILE\* handle, char\* logLevelsToReg)
	>Registers the file handle to an error level or several. 
	>The worker thread takes ownership if it already does not have it.

- loggingRegLogLevel(char\* logLevel,char\* LogLevelName)
	>Registers a log level to the system
- loggingRegDefualtConf()
	>Registers a defualt setup that is used by backerLib

## Event Registers

- logEvent(logHandle,const event\_t\*)
- logEventFileLocation(logHandle,const event\_t\*)
- free
- malloc
- realloc
- calloc