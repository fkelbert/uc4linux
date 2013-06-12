# logconf.py
import sys, re, os, fileinput
from time import localtime, strftime

# Logging levels for PEF
TRACE  =2
DEBUG  =3
INFO   =4
WARN   =5
ERROR  =6
FATAL  =7
DISABLE=8

# multiple replace function
def mtplReplace(repllist, str):
  regex=re.compile("(%s)" % "|".join(map(re.escape, repllist.keys())))
  return regex.sub(lambda mo: repllist[mo.string[mo.start():mo.end()]], str)

pathSeparator='/'
if sys.platform == "win32":
	pathSeparator="\\"
	
def prepLogHeader(out, src, target, env):
  LOGBASENAME=src[src.rfind(pathSeparator)+1:]
  if env['VERBOSE']==1:
    print "   checking " + LOGBASENAME + ":",
      
  LOGCONFFILE="log_"+LOGBASENAME.replace('.','_')+".h"
  LOGUPPER   =("log_"+LOGBASENAME.replace('.','_')).upper()
  LOGLOWER   =("log_"+LOGBASENAME.replace('.','_')).lower()

  LOGMODULENAME=LOGBASENAME[:LOGBASENAME.find('.')]
  if env['VERBOSE']==1:
    print "generate configuration header... ",    
  # changed for external build directory
  filetemplate=open(env['PEFBASE'] + "/libraries/logger/logtemplate",'r')
  for line in filetemplate:
    repl={"${LOGUPPER}":LOGUPPER,"${LOGLOWER}":LOGLOWER,"${LOGCONFFILE}":LOGCONFFILE.replace('.','_'), "${LOGMODULENAME}":LOGMODULENAME }
    out.write(mtplReplace(repl,line))
  filetemplate.close()

  logIncludeDirectiveFile=str(target[0])
  logIncludeDirectiveFile=logIncludeDirectiveFile[logIncludeDirectiveFile.rfind(pathSeparator)+1:]
  lfile=open(src)
  found=0
  for line in lfile:
    m=re.search('#include "'+logIncludeDirectiveFile+'"', line)
    if m != None:
      if env['VERBOSE']==1:
        print "all seems to be ok"
      found=1
      break
  if found==0:
    if env['VERBOSE']==1:
      print "include directive for logger configuration is missing ",
    lfile.close()

    inclpos=False
    for line in fileinput.input(src, inplace=1):
      if line.startswith('#include'):
        inclpos=True
      else:
        if inclpos:
          print '#include \"'+logIncludeDirectiveFile+'\"'
        inclpos=False
      print line,
    if env['VERBOSE']==1:
      print "=> inserted"
  
def prepLogConfigHeader(out, src, target, env):
  if env['VERBOSE']==1:
    print "   checking logger configuration (and creating loggerconf.h)"

  found=0
  out.write("#ifndef _LOGGER_BASE_H\n")
  out.write("#define _LOGGER_BASE_H\n")
  logoutputs=[]
  
  # preparing loggerconf.h
  loggerConfOut=open(os.path.join(env['PEFBUILD'], "include", "loggerconf.h"), "w")
  loggerConfStartStr="/*\n * loggerconf.h\n *\n *  WARNING: This file is generated automatically from the configuration settings (scons)!\n *  DO NOT EDIT!\n *  Generated: "+ strftime("%a, %d %b %Y %H:%M:%S", localtime()) +"\n *  Author: cornelius moucha\n */\n\n#ifndef _loggerconf_h\n#define _loggerconf_h\n\n"
  loggerConfOut.write(loggerConfStartStr)
  for opt in env['PEFLOGCONFIG']:
    if "_OUTPUT" in opt:  
      m=re.search('std(out|err)', env['PEFLOGCONFIG'][opt])
      if ( (m != None) and not opt == 'GLOBAL_LOG_OUTPUT' ):
        print "\nFAILURE: stdout or stderr is specified for option = ["+opt+"] => this will destroy usual stdout/stderr file pointer! aborting..."
        Exit(1)
      
    if opt == "ABSOLUTE_PATH":
      if len(env['PEFLOGCONFIG'][opt])==0: 
        absPath="\"\""
      else:
        absPath=env['PEFLOGCONFIG'][opt]
      loggerConfOut.write("static char *const log_absPath = " + absPath + ";\n")
    else:
      if opt=="RUNTIME_LOGLEVEL_EVAL" or opt=="RUNTIME_LOGLEVEL" or opt=="GLOBAL_LOG_LEVEL" or opt=="GLOBAL_LOG_OUTPUT": 
        loggerConfOut.write("#define " + opt + " " + str(env['PEFLOGCONFIG'][opt]) + "\n")
      else:
        newOpt="LOG_"+opt.replace('.','_')
        newOpt=newOpt.upper()
        loggerConfOut.write("#define " + newOpt + " " + str(env['PEFLOGCONFIG'][opt]) + "\n")
        
  loggerConfOut.write("\n\n\n")
  loggerConfOut.write("#endif")      
  loggerConfOut.close()

  # TODO: check for '' in OUTPUT-options!
  lfile = open(os.path.join(env['PEFBUILD'], "include", "loggerconf.h"), "r")
  for line in lfile:
    x=re.search('_OUTPUT ', line)
    if (x != None) and (line.startswith("//")==False and line.startswith(" *")==False):
      clogoutput=(line[line.find("_OUTPUT ")+8:-1]).strip()
      if (clogoutput != "stdout" and clogoutput != "stderr") and not (clogoutput in logoutputs):
        logoutputs.append(clogoutput);
        out.write("static FILE *"+clogoutput+"=NULL;\n");
      else:
        # linux will complain about this redirection of file pointer as it is not constant
        # in linux stdout is defined as "struct _IO_FILE *" whereas in openbsd as "(&__sF[1])" so usual way produces compiler error
        # due to address operator &
        # 111202 - Android is also complaining about &-opterator (&__sF[1]) like openbsd although linux-platform
        if ( (clogoutput == "stdout" or clogoutput == "stderr") and (sys.platform=="openbsd4" or env['PEFANDROID'] == 1)): #or sys.platform=="win32"
          out.write("static FILE *stdout1 = GLOBAL_LOG_OUTPUT;\n");
          out.write("#undef GLOBAL_LOG_OUTPUT\n");
          out.write("#define GLOBAL_LOG_OUTPUT stdout1\n");
        else:
		# win32 similar to openbsd (&_iob[1]) -> gcc fails due to not constant assignment
		if ( (clogoutput == "stdout" or clogoutput == "stderr") and (sys.platform=="win32")):
			out.write("#define STDOUTFLAG 1\n");
			out.write("static FILE *stdout1 = NULL;\n");
			out.write("#undef GLOBAL_LOG_OUTPUT\n");
			out.write("#define GLOBAL_LOG_OUTPUT stdout1\n");
	

    if 'static char *log_absPath' in line:
      fabspath=(line[line.find("log_absPath = ")+15:-3]).strip()
      if not fabspath.endswith('/'):
        print "\nFailure: Specified logging directory doesn't end with closing '/'"
        Exit(1)
      if not os.path.exists(fabspath):
        print "\nFailure: Specified logging directory (" + fabspath + ") doesn't exist"
        Exit(1)

      perm = os.stat(fabspath).st_mode
      permk = stat.S_IFDIR | stat.S_IRUSR | stat.S_IWUSR | stat.S_IXUSR | stat.S_IRGRP | stat.S_IWGRP | stat.S_IXGRP | stat.S_IROTH | stat.S_IWOTH | stat.S_IXOTH
      if not (perm == permk):
        print "\nFailure: Specified logging directory (" + fabspath + ") has wrong permissions (" + str(perm ) + " instead of " + str(permk) + ")"
        Exit(1)

  if found==0:
    if env['VERBOSE']==1:
      print "all sems to be ok"
  
  out.write("#endif\n")
  lfile.close()
  
## PEF logger preparation
def pefLoggerBuildFunction(target, source, env):
  targetFilename=str(target[0])
  out=open(targetFilename, "w")
  prepLogHeader(out, str(source[0]), target, env)
  out.close()
  
  env.Append(CPPPATH = [os.path.dirname(str(target[0]))])
  return None
  
def pefLoggerConfigBuildFunction(target, source, env):
  targetFilename=str(target[0])
  out=open(targetFilename, "w")
  prepLogConfigHeader(out, str(source[0]), target, env)
  out.close()  
  return None




