/*
 * syscallHandler.c
 *
 *  Created on: Jul 26, 2011
 *      Author: cornelius moucha
 */

#include "syscallHandler.h"
#include "log_syscallHandler_c.h"

extern int cfd;
extern int opid;
extern esfsyscall_t* esfsyscalls[];

int may_ignore(char *filename)
{ //return FALSE;
  if((filename!=NULL) && (
      strcmp(filename,"/dev/tty")==0 ||
      strcmp(filename,"/etc/pwd.db")==0 ||
      strcmp(filename,"/etc/group")==0 ||
      strcmp(filename,"/etc/spwd.db")==0 ||
      strcmp(filename,"/etc/localtime")==0 ||
      substr(filename,"/home/user/.bash_rc") ||
      substr(filename,".bash_profile") ||
      substr(filename,".bash_login") ||
      substr(filename,".bash_logout") ||
      substr(filename,".bash_history") ||
      substr(filename,"/.Xauthority") ||
      substr(filename,".profile") ||
      substr(filename,"/etc/profile") ||
      substr(filename,"terminfo") ||
      substr(filename,"/usr/local/lib/") ||
      substr(filename,"/usr/lib/") ||
      substr(filename,"/usr/share/") ||
      substr(filename,"/usr/local/firefox35/") ||
      substr(filename,"/usr/X11R6/") ||
      substr(filename,"/home/user/.icon") ||
      substr(filename,"/var/run/ld.so.hints") ||
      //substr(filename,"/var/tmp/") ||
      //substr(filename,"/var/cache/fontconfig/") ||
      substr(filename,"/etc/fonts/") ||
      //substr(filename,"/var/run/") ||
      //substr(filename,"/.") //no hidden files/folders monitored, but relatives paths like ../../ are not allowed...
      strcmp(filename,".")==0 ||
      strcmp(filename,"..")==0
  ))
  {
    log_debug("Ignoring file access [%s]", filename);
    return TRUE;
  }
  return FALSE;
}

void processResponse(notifyResponse_ptr eventResponse, struct str_message *sys_msg, struct systrace_answer *sys_ans)
{
  if(eventResponse == NULL) {log_trace("- Event allowed because there is no triggered mechanism"); return;}
  event_log("Mechanisms triggered due to event: ", eventResponse->event);

  if((eventResponse->authorizationAction->response == ACTION_ALLOW))
  {
    log_info("  Syscall allowed");
    sys_ans->stra_policy = SYSTR_POLICY_PERMIT;
  }
  else
  {
    log_info("  Syscall denied");
    sys_ans->stra_policy = SYSTR_POLICY_NEVER;
  }

  // delay syscall for X microseconds (NOT seconds!)
  if(eventResponse->authorizationAction->delay != 0)
  {
    log_info("  Syscall delayed (for [%llu] us)", eventResponse->authorizationAction->delay);
    usleep(eventResponse->authorizationAction->delay);
  }

  // modify
  unsigned int i;
  if(eventResponse->authorizationAction->n_params_to_modify >0)
  {
    log_info("  Syscall modified");
    switch (sys_msg->msg_data.msg_ask.code)
    {
      case SYS_open:
        // Modify open filename or flags
        for(i = 0; i < eventResponse->authorizationAction->n_params_to_modify; i++)
        {
          if(strcmp(eventResponse->authorizationAction->params_to_modify[i]->param_desc->name,"filename")==0)
          {
            log_info("  Changing filename to [%s]", eventResponse->authorizationAction->params_to_modify[i]->value);
            open_filename_replace(sys_msg->msg_pid, sys_msg->msg_seqnr, eventResponse->authorizationAction->params_to_modify[i]->value);
          }
          else if(strcmp(eventResponse->authorizationAction->params_to_modify[i]->param_desc->name,"flags")==0)
          {
            if(eventResponse->authorizationAction->params_to_modify[i]->value)
            {
              log_info("  Changing flags to [%s]", eventResponse->authorizationAction->params_to_modify[i]->value);
              int flags = atoi(eventResponse->authorizationAction->params_to_modify[i]->value);
              open_flag_replace(sys_msg->msg_pid, sys_msg->msg_seqnr, flags); ///TODO: IMPLEMENTATION TO BE CHECKED
            }
          }
        }
        break;

      case SYS_unlink:
        for(i = 0; i < eventResponse->authorizationAction->n_params_to_modify; i++)
        {
          if(strcmp(eventResponse->authorizationAction->params_to_modify[i]->param_desc->name,"filename")==0)
          {
            log_info("  Changing filename to [%s]", eventResponse->authorizationAction->params_to_modify[i]->value);
            open_filename_replace(sys_msg->msg_pid, sys_msg->msg_seqnr, eventResponse->authorizationAction->params_to_modify[i]->value);
          }
        }
        break;
    }
  }

  // execute
  for(i = 0; i < eventResponse->n_actions_to_execute; i++)
  {
    printf ("\n\n\n\n\n\nr_actions->n_actions=%d r_actions->execute[i].id=%s\n\n\n\n\n\n",
        eventResponse->n_actions_to_execute, eventResponse->actions_to_execute[i]->action->action_name);
    if( strcmp(eventResponse->actions_to_execute[i]->action->action_name,"resetMonitor")==0)
    {
      log_info(" Resetting monitor");
      //reset_flag=1;
    }
    else if( strcmp(eventResponse->actions_to_execute[i]->action->action_name,"encryptWrite")==0)
    {
      printf("<Encrypt Write>\n");
    }
    else if( strcmp(eventResponse->actions_to_execute[i]->action->action_name,"encryptRead")==0)
    {
      printf("<Encrypt Read>\n");
    }
    else if( strcmp(eventResponse->actions_to_execute[i]->action->action_name,"encryptOpen")==0)
    {
      printf("<Encrypt Open>\n");
    }
    else if( strcmp(eventResponse->actions_to_execute[i]->action->action_name,"encryptClose")==0)
    {
      printf("<Encrypt Close>\n");
    }
  } // for(i = 0; i < r_actions->n_actions_to_execute; i++)
}

event_ptr processSyscall(struct str_message *sys_msg)
{
  action_desc_ptr actionDesc=find_action_desc_by_name_or_insert(pdp_action_desc_store, DFM_eventnames[sys_msg->msg_data.msg_ask.code]);
  event_ptr event=event_new(actionDesc, IDX_ONGOING, sys_msg->msg_type==SYSTR_MSG_ASK);

  processStandardParameters(event, sys_msg);
  processResponseParameter(event, sys_msg);
  return event;
}

unsigned int processStandardParameters(event_ptr event, struct str_message *sys_msg)
{
  struct passwd *user_info=NULL;
  char cflags[128];
  action_param_desc_ptr paramDesc = NULL;

  snprintf(cflags, 128, "[%06d]", sys_msg->msg_pid);
  event_add_paramInt(event, "pid", PARAM_CONTUSAGE, sys_msg->msg_pid);
  event_add_paramString(event, "command", PARAM_CONTUSAGE, getProcCommand(sys_msg->msg_pid));

  // User
  user_info=getUserInfo(sys_msg->msg_pid, user_info);
  event_add_paramString(event, "user", PARAM_CONTUSAGE, user_info->pw_name);
  return R_SUCCESS;
}

unsigned int processResponseParameter(event_ptr event, struct str_message* sys_msg)
{
  char cflags[16];
  action_param_desc_ptr paramDesc = NULL;
  if(sys_msg->msg_type==SYSTR_MSG_RES)
  {
    if(sys_msg->msg_data.msg_ask.code==SYS_exit)
      snprintf(cflags, 16, "%d", sys_msg->msg_data.msg_ask.args[0]);
    else
      snprintf(cflags, 16, "%d", sys_msg->msg_data.msg_ask.rval[0]);
    event_add_paramString(event, "retval", PARAM_CONTUSAGE, cflags);

    if(sys_msg->msg_data.msg_ask.code==SYS_close)
      delete_fd_filename_mapping(sys_msg->msg_pid, sys_msg->msg_data.msg_ask.args[0]);

  }
  return R_SUCCESS;
}

unsigned int readStringParam(struct str_message *sys_msg, unsigned int a, char *buf)
{
  log_error("reading String Param(a=%d)",a);
  void *p;
  memcpy( &p, &sys_msg->msg_data.msg_ask.args[a], sizeof(p) );
  //get_string_buffer( cfd, sys_msg->msg_pid, SYSTR_READ, p, &filename );
  get_string_buffer( cfd, sys_msg->msg_pid, SYSTR_READ, p, buf );
  log_error("ok, finished");
  log_error("ok, finished with result=[%s]", buf);
}

event_ptr esfParseSyscall(struct str_message *sys_msg)
{
  char filename[5120];
  event_ptr event=processSyscall(sys_msg);
  unsigned int a;
  esfsyscall_t *curSyscall=esfsyscalls[sys_msg->msg_data.msg_ask.code];

  for(a=0; a<esfsyscalls[sys_msg->msg_data.msg_ask.code]->numParams; a++)
  {
    if(esfsyscalls[sys_msg->msg_data.msg_ask.code]->params[a].paramType==PARAM_STRING)
    { // PARAM_STRING
      readStringParam(sys_msg, a, filename);
      if(sys_msg->msg_data.msg_ask.code==SYS_open && strcmp(esfsyscalls[sys_msg->msg_data.msg_ask.code]->params[a].paramName, "filename")==0)
      {
        if((IGNORE_SYS_FILES)&&(may_ignore(filename)))
          return event;
      }
      event_add_paramString(event, esfsyscalls[sys_msg->msg_data.msg_ask.code]->params[a].paramName, PARAM_CONTUSAGE, filename);
    }
    else if(esfsyscalls[sys_msg->msg_data.msg_ask.code]->params[a].paramType==PARAM_INTSTR)
    { // PARAM_INT with conversion to string
      event_add_paramString(event, esfsyscalls[sys_msg->msg_data.msg_ask.code]->params[a].paramName, PARAM_CONTUSAGE, byteToBinary(sys_msg->msg_data.msg_ask.args[a]));
    }
    else
    { // PARAM_INT
      event_add_paramInt(event, esfsyscalls[sys_msg->msg_data.msg_ask.code]->params[a].paramName, PARAM_CONTUSAGE, sys_msg->msg_data.msg_ask.args[a]);
    }
  }
  event_log("finished processing params: ", event);
  return event;
}


event_ptr parseSyscall(int cfd, struct str_message* sys_msg)
{
  void *p;
  void *p2;
  // TODO:
  // - filename is max 512?
  // - filename is not absolute path

  char filename[5120];
  char *p_filename;
  char filename2[5120];
  char *p_filename2;
  char cflags[500];
  int i, fd, x;
  int arrfd[2];
  int flags;
  int* pointertoint;

  event_ptr event = NULL;
  action_param_desc_ptr paramDesc = NULL;
  bool isTry = sys_msg->msg_type==SYSTR_MSG_ASK;

  if(sys_msg->msg_data.msg_ask.code < SYS_MAXSYSCALL)
  { //fuck the portability I won't use MAX_SYSCALL (should work now)
    if(sys_msg->msg_data.msg_ask.code>309) log_error("\n\n[%s] - SYS_MAXSYSCALL syscall [%d]\n\n", __func__, sys_msg->msg_data.msg_ask.code);
//    printf("%d ",sys_msg->msg_data.msg_ask.code);
  }
  else
  {
//    printf ("<..event=null msg_code=%d> \n",sys_msg->msg_data.msg_ask.code);
//    fflush(NULL);
    return NULL;
  }

  switch(sys_msg->msg_data.msg_ask.code)
  {
    case SYS_exit:    event=esfParseSyscall(sys_msg);  
                      //if(event_get_param_value_by_name_int(event, "pid")==opid) 
                      //  exit(EXIT_SUCCESS); 
                      break;
    case SYS_socket:  event=esfParseSyscall(sys_msg);  break;
    case SYS_accept:  event=esfParseSyscall(sys_msg);  break;
    case SYS_pipe:      // Event parameters: command, user, fdsource, fddests  (+retval)
      event=processSyscall(sys_msg);

      memcpy( &p, &sys_msg->msg_data.msg_ask.args[0], sizeof(p) );
      copy_io( cfd,sys_msg->msg_pid, SYSTR_READ, p, &arrfd[0], sizeof(int) );
      copy_io( cfd,sys_msg->msg_pid, SYSTR_READ, p+sizeof(int), &arrfd[1], sizeof(int) );


      // 2nd way?!
      log_error("reading pipe params 2nd way");
      log_error("args[0]=[%p], args[1]=[%p]", sys_msg->msg_data.msg_ask.args[0], sys_msg->msg_data.msg_ask.args[1]);
      log_error("args[0]=[%d], args[1]=[%d]", sys_msg->msg_data.msg_ask.args[0], sys_msg->msg_data.msg_ask.args[1]);

      int xa=-1, xb=-1;
      log_error("xa=[%d], xb=[%d]", xa, xb);
      memcpy( &p, &sys_msg->msg_data.msg_ask.args[0], sizeof(p) );
      copy_io( cfd,sys_msg->msg_pid, SYSTR_READ, p, &xa, sizeof(int) );

      memcpy( &p2, &sys_msg->msg_data.msg_ask.args[1], sizeof(p2) );
      log_error("p+sizeof(int)=[%p], p2=[%p]", p+sizeof(int), p2);
      copy_io( cfd,sys_msg->msg_pid, SYSTR_READ, p2, &xb, sizeof(int) );
      log_error("xa=[%d], xb=[%d]", xa, xb);

      event_add_paramInt(event, "source", PARAM_CONTUSAGE, arrfd[0]);
      event_add_paramInt(event, "dest", PARAM_CONTUSAGE, arrfd[1]);
      break;

  //case SYS_creat:
    case SYS_open:   event=esfParseSyscall(sys_msg);  break;

    case SYS_recvfrom:
    case SYS_recvmsg:
    case SYS_readv:
    case SYS_pread:
    case SYS_read:      // Event parameters: command user filename fd (+retval) [+buf dest addr]
      if(IGNORE_SYS_FILES && (may_ignore(p_filename)))
      {
        action_desc_ptr actionDesc=find_action_desc_by_name_or_insert(pdp_action_desc_store, DFM_eventnames[sys_msg->msg_data.msg_ask.code]);
        event=event_new(actionDesc, IDX_ONGOING, isTry);
      }
      else
      {
        fd = sys_msg->msg_data.msg_ask.args[0];
        p_filename = get_file_name(sys_msg->msg_pid, fd);

        event=processSyscall(sys_msg);

        // Filename
        if(p_filename) event_add_paramString(event, "filename", PARAM_CONTUSAGE, p_filename);
        else           event_add_paramString(event, "filename", PARAM_CONTUSAGE, (fd>2 ? fdStr[3] : fdStr[fd]));

        event_add_paramInt(event, "fd", PARAM_CONTUSAGE, fd);

		if(sys_msg->msg_type==SYSTR_MSG_RES)
			event_add_paramInt(event, "sizea", PARAM_CONTUSAGE, sys_msg->msg_data.msg_ask.rval[0]);
		else
			event_add_paramInt(event, "sizea", PARAM_CONTUSAGE, -1);


        //buf dest addr
        // TODO: should use event_add_paramAddress!
        sprintf(cflags,"%x",sys_msg->msg_data.msg_ask.args[1]);
        event_add_paramString(event, "buf", PARAM_CONTUSAGE, cflags);
      }
      break;

    case SYS_write:
    case SYS_pwrite:
    case SYS_writev:
    case SYS_truncate:
    case SYS_ftruncate:
    //case SYS_connect:
    case SYS_sendto:
    case SYS_sendmsg:   // Event parameters: command user filename filedescriptor  (+retval)
      if(sys_msg->msg_data.msg_ask.code==SYS_truncate)
      {
        memcpy( &p, &sys_msg->msg_data.msg_ask.args[0], sizeof(p) );
        get_string_buffer( cfd, sys_msg->msg_pid, SYSTR_READ, p, &filename[0] );

        fd=get_fd(sys_msg->msg_pid, filename);
        log_info("ok, got fd=[%d] in SYS_truncate for filename[%s]", fd, filename);

        p_filename=strdup(filename);
      }
      else
      {
        fd = sys_msg->msg_data.msg_ask.args[0];
        p_filename = get_file_name(sys_msg->msg_pid, fd);
      }

      if((IGNORE_SYS_FILES)&&(may_ignore(p_filename)))
      { // TODO: this actionDescription will never be found!
        sprintf (cflags, "%s(fd=%d)",DFM_eventnames[sys_msg->msg_data.msg_ask.code],fd);
        action_desc_ptr actionDesc=find_action_desc_by_name_or_insert(pdp_action_desc_store, cflags);
        event=event_new(actionDesc, IDX_ONGOING, isTry);
      }
      else
      {
        event=processSyscall(sys_msg);

        // Filename
        if(p_filename) event_add_paramString(event, "filename", PARAM_CONTUSAGE, p_filename);
        else           event_add_paramString(event, "filename", PARAM_CONTUSAGE, (fd>2 ? fdStr[3] : fdStr[fd]));

        event_add_paramInt(event, "fd", PARAM_CONTUSAGE, fd);

        switch(sys_msg->msg_data.msg_ask.code)
        {
			case SYS_write:
			case SYS_pwrite:
			case SYS_writev:
			case SYS_sendto:
			case SYS_sendmsg:
				if(sys_msg->msg_type==SYSTR_MSG_RES)
					event_add_paramInt(event, "sizea", PARAM_CONTUSAGE, sys_msg->msg_data.msg_ask.rval[0]);
				else
					event_add_paramInt(event, "sizea", PARAM_CONTUSAGE, sys_msg->msg_data.msg_ask.args[2]);
				break;
			case SYS_truncate:
			case SYS_ftruncate:
			{
				struct stat point;
				stat(p_filename, &point);
				log_info("sizea = - %ld + %lld = %lld",(long int) sys_msg->msg_data.msg_ask.args[2], (long long int) point.st_size, (long int)(sys_msg->msg_data.msg_ask.args[2])-(long long)(point.st_size));
				event_add_paramInt(event, "sizea", PARAM_CONTUSAGE, (long int)(sys_msg->msg_data.msg_ask.args[2])-(long long)(point.st_size));
				event_add_paramInt(event, "oldSize", PARAM_CONTUSAGE, (long long)(point.st_size));
				break;
			}
        }



      }
      break;

    case SYS_close:
      fd = sys_msg->msg_data.msg_ask.args[0];
      p_filename = get_file_name(sys_msg->msg_pid,fd);

      if((IGNORE_SYS_FILES)&&(may_ignore(p_filename)))
      {
        action_desc_ptr actionDesc=find_action_desc_by_name_or_insert(pdp_action_desc_store, DFM_eventnames[sys_msg->msg_data.msg_ask.code]);
        event=event_new(actionDesc, IDX_ONGOING, isTry);
      }
      else
      {
        event=processSyscall(sys_msg);

        // Filename
        if(p_filename) event_add_paramString(event, "filename", PARAM_CONTUSAGE, p_filename);
        else           event_add_paramString(event, "filename", PARAM_CONTUSAGE, (fd>2 ? fdStr[3] : fdStr[fd]));

        event_add_paramInt(event, "fd", PARAM_CONTUSAGE, fd);

      }
      break;

    case SYS_unlink: event=esfParseSyscall(sys_msg);  break;
    case SYS_execve:
      if(((sys_msg->msg_data.msg_ask.code)==SYS_execve)&&(sys_msg->msg_type==SYSTR_MSG_RES))
        p_filename = strdup("<empty>");
      else
      {
        memcpy( &p, &sys_msg->msg_data.msg_ask.args[0], sizeof( p ) );
        get_string_buffer( cfd, sys_msg->msg_pid, SYSTR_READ, p, &filename[0] );
        p_filename=strdup(&filename[0]);
      }

      if((IGNORE_SYS_FILES)&&(may_ignore(p_filename)))
      {
        action_desc_ptr actionDesc=find_action_desc_by_name_or_insert(pdp_action_desc_store, DFM_eventnames[sys_msg->msg_data.msg_ask.code]);
        event=event_new(actionDesc, IDX_ONGOING, isTry);
      }
      else
      {
        event=processSyscall(sys_msg);

        // Filename
        if(p_filename) {
        	struct stat point;
        	stat(p_filename, &point);
        	event_add_paramInt(event, "sizea", PARAM_CONTUSAGE, point.st_size);
        	event_add_paramString(event, "filename", PARAM_CONTUSAGE, p_filename);
        }
        else {
        	event_add_paramInt(event, "sizea", PARAM_CONTUSAGE, -1);
        	event_add_paramString(event, "filename", PARAM_CONTUSAGE, (fd>2 ? fdStr[3] : fdStr[fd]));
        }

      }


      break;

    case SYS_kill:      // Event parameters: command user target signal (+retval)
      event=processSyscall(sys_msg);

      // Pid2
      sprintf(cflags,"[%06d]%s", sys_msg->msg_data.msg_ask.args[0], getProcCommand(sys_msg->msg_pid));
      event_add_paramString(event, "target", PARAM_CONTUSAGE, cflags);
      event_add_paramInt(event, "signal", PARAM_CONTUSAGE, sys_msg->msg_data.msg_ask.args[1]);
      event_add_paramInt(event, "sizea", PARAM_CONTUSAGE, sizeof(int));
      break;

    case SYS_rename:    // Event parameters: command, user, old filename, new filename (+retval)
      memcpy( &p, &sys_msg->msg_data.msg_ask.args[0], sizeof(p) );
      get_string_buffer( cfd, sys_msg->msg_pid, SYSTR_READ, p, &filename[0] );

      memcpy( &p2, &sys_msg->msg_data.msg_ask.args[1], sizeof(p2) );
      get_string_buffer( cfd, sys_msg->msg_pid, SYSTR_READ, p2, &filename2[0] );

      event=processSyscall(sys_msg);
      event_add_paramString(event, "oldfilename", PARAM_CONTUSAGE, filename);
      event_add_paramString(event, "newfilename", PARAM_CONTUSAGE, filename2);
      break;

    case SYS_fork:  event=esfParseSyscall(sys_msg);  break;
    case SYS_vfork: event=esfParseSyscall(sys_msg);  break;

    case SYS_dup:
    case SYS_dup2:
    case SYS_fcntl:     // Event parameters: command, user, fd, (+retval)
      if(sys_msg->msg_data.msg_ask.code==SYS_fcntl && (sys_msg->msg_data.msg_ask.args[1])!=F_DUPFD)
      {
        action_desc_ptr actionDesc=find_action_desc_by_name_or_insert(pdp_action_desc_store, DFM_eventnames[sys_msg->msg_data.msg_ask.code]);
        event=event_new(actionDesc, IDX_ONGOING, isTry);
      }
      else
      {
        event=processSyscall(sys_msg);
        event_add_paramInt(event, "fd", PARAM_CONTUSAGE, sys_msg->msg_data.msg_ask.args[0]);
      }
      break;
    case SYS_mmap:      // Event parameters: command user start length prot flags fd offset (+retval)
      event=processSyscall(sys_msg);

      // pid
      event_add_paramInt(event, "pid", PARAM_CONTUSAGE, sys_msg->msg_pid);

      // start
      sprintf(cflags,"%x", sys_msg->msg_data.msg_ask.args[0]); // is it really correct to use >command< (stored in cflags) as 'start'???
      event_add_paramString(event, "start", PARAM_CONTUSAGE, cflags);


      // length
      sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.args[1]);
      event_add_paramString(event, "length", PARAM_CONTUSAGE, cflags);

      //sizea == length
      event_add_paramInt(event, "sizea", PARAM_CONTUSAGE, sys_msg->msg_data.msg_ask.args[1]);

      // prot
      event_add_paramString(event, "prot", PARAM_CONTUSAGE, byteToBinary(sys_msg->msg_data.msg_ask.args[2]));

      // flags
      sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.args[3]);
      event_add_paramString(event, "flags", PARAM_CONTUSAGE, cflags);

      // fd
      event_add_paramInt(event, "fd", PARAM_CONTUSAGE, sys_msg->msg_data.msg_ask.args[4]);

      // offset
      sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.args[5]);
      event_add_paramString(event, "offset", PARAM_CONTUSAGE, cflags);



      if(sys_msg->msg_type==SYSTR_MSG_RES)
      {
        sprintf(cflags,"%x",sys_msg->msg_data.msg_ask.rval[0]);
        event_add_paramString(event, "retval", PARAM_CONTUSAGE, cflags);
      }

      break;

    default:
    {
      action_desc_ptr actionDesc=find_action_desc_by_name_or_insert(pdp_action_desc_store, DFM_eventnames[sys_msg->msg_data.msg_ask.code]);
      event=event_new(actionDesc, IDX_ONGOING, isTry);
      break;
    }
  } //switch(sys_msg->msg_data.msg_ask.code)
  return event;
}
