/*
 * processSyscall.c
 *
 *  Created on: Jul 26, 2011
 *      Author: cornelius moucha
 */

#include "processSyscall.h"
#include "log_processSyscall_c.h"

/*
event_ptr processSyscall(bool isTry, struct str_message *sys_msg)
{
  action_desc_ptr actionDesc=find_action_desc_by_name_or_insert(pdp_action_desc_store, DFM_eventnames[sys_msg->msg_data.msg_ask.code]);
  event_ptr event=event_new(actionDesc, IDX_ONGOING, isTry);

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
  //event_add_paramInt(event, paramDesc, PARAM_CONTUSAGE, sys_msg->msg_pid);
  event_add_paramString(event, "pid", PARAM_CONTUSAGE, cflags);
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

event_ptr processSyscall(bool isTry, struct str_message *sys_msg)
{
  event_ptr event;

  switch(sys_msg->msg_data.msg_ask.code)
  {
    case SYS_exit:   event=process_exit(isTry, sys_msg);   break;
    case SYS_socket: event=process_socket(isTry, sys_msg); break;
    case SYS_accept: event=process_accept(isTry, sys_msg); break;



    case SYS_recvfrom:
    case SYS_recvmsg:
    case SYS_readv:
    case SYS_pread:
    case SYS_read:      // Event parameters: command user filename fd (+retval) [+bu dest addr]
      if(IGNORE_SYS_FILES && (may_ignore(p_filename)))
      {
        action_desc_ptr actionDesc=find_action_desc_by_name_or_insert(pdp_action_desc_store, DFM_eventnames[sys_msg->msg_data.msg_ask.code]);
        event=event_new(actionDesc, IDX_ONGOING, isTry);
      }
      else
      {
        fd = sys_msg->msg_data.msg_ask.args[0];
        p_filename = get_file_name(sys_msg->msg_pid, fd);

        event=processSyscall(isTry, sys_msg);

        // Filename
        if(p_filename) event_add_paramString(event, "filename", PARAM_CONTUSAGE, p_filename);
        else           event_add_paramString(event, "filename", PARAM_CONTUSAGE, (fd>2 ? fdStr[3] : fdStr[fd]));

        // FileDescriptor
        sprintf(cflags,"%d",fd);
        event_add_paramString(event, "fd", PARAM_CONTUSAGE, cflags);

        //buf dest addr
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
      {
        sprintf (cflags, "%s(fd=%d)",DFM_eventnames[sys_msg->msg_data.msg_ask.code],fd);
        action_desc_ptr actionDesc=find_action_desc_by_name_or_insert(pdp_action_desc_store, cflags);
        event=event_new(actionDesc, IDX_ONGOING, isTry);
      }
      else
      {
        event=processSyscall(isTry, sys_msg);

        // Filename
        if(p_filename) event_add_paramString(event, "filename", PARAM_CONTUSAGE, p_filename);
        else           event_add_paramString(event, "filename", PARAM_CONTUSAGE, (fd>2 ? fdStr[3] : fdStr[fd]));

        // FileDescriptor
        sprintf(cflags,"%d",fd);
        event_add_paramString(event, "fd", PARAM_CONTUSAGE, cflags);
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
        event=processSyscall(isTry, sys_msg);

        // Filename
        if(p_filename) event_add_paramString(event, "filename", PARAM_CONTUSAGE, p_filename);
        else           event_add_paramString(event, "filename", PARAM_CONTUSAGE, (fd>2 ? fdStr[3] : fdStr[fd]));

        // FileDescriptor
        sprintf(cflags,"%d",fd);
        event_add_paramString(event, "fd", PARAM_CONTUSAGE, cflags);
      }
      break;

    case SYS_unlink:
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
        event=processSyscall(isTry, sys_msg);

        // Filename
        if(p_filename) event_add_paramString(event, "filename", PARAM_CONTUSAGE, p_filename);
        else           event_add_paramString(event, "filename", PARAM_CONTUSAGE, (fd>2 ? fdStr[3] : fdStr[fd]));
      }
      break;
    case SYS_kill:      // Event parameters: command user target signal (+retval)
      event=processSyscall(isTry, sys_msg);

      // Pid2
      sprintf(cflags,"[%06d]%s", sys_msg->msg_data.msg_ask.args[0], getProcCommand(sys_msg->msg_pid));
      event_add_paramString(event, "target", PARAM_CONTUSAGE, cflags);

      //signal
      sprintf(cflags,"%d", sys_msg->msg_data.msg_ask.args[1]);
      event_add_paramString(event, "signal", PARAM_CONTUSAGE, cflags);
      break;
    case SYS_rename:    // Event parameters: command, user, old filename, new filename (+retval)
      memcpy( &p, &sys_msg->msg_data.msg_ask.args[0], sizeof(p) );
      get_string_buffer( cfd, sys_msg->msg_pid, SYSTR_READ, p, &filename[0] );

      memcpy( &p2, &sys_msg->msg_data.msg_ask.args[1], sizeof(p2) );
      get_string_buffer( cfd, sys_msg->msg_pid, SYSTR_READ, p2, &filename2[0] );

      event=processSyscall(isTry, sys_msg);
      event_add_paramString(event, "oldfilename", PARAM_CONTUSAGE, filename);
      event_add_paramString(event, "newfilename", PARAM_CONTUSAGE, filename2);
      break;

    case SYS_fork:
    case SYS_vfork:     // Event parameters: command, user, (+retval)
      event=processSyscall(isTry, sys_msg);
      break;

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
        event=processSyscall(isTry, sys_msg);

        // File Descriptor
        sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.args[0]);
        event_add_paramString(event, "fd", PARAM_CONTUSAGE, cflags);
      }
      break;
    case SYS_mmap:      // Event parameters: command user start length prot flags fd offset (+retval)
      event=processSyscall(isTry, sys_msg);

      // start
      sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid)); // is it really correct to use >command< (stored in cflags) as 'start'???
      event_add_paramString(event, "start", PARAM_CONTUSAGE, cflags);

      // length
      sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.args[1]);
      event_add_paramString(event, "length", PARAM_CONTUSAGE, cflags);

      // prot
      event_add_paramString(event, "prot", PARAM_CONTUSAGE, byteToBinary(sys_msg->msg_data.msg_ask.args[2]));

      // flags
      sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.args[3]);
      event_add_paramString(event, "flags", PARAM_CONTUSAGE, cflags);

      // fd
      sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.args[4]);
      paramDesc=find_param_desc_by_name_or_insert(event->action_desc, "fd");
      event_add_paramString(event, "fd", PARAM_CONTUSAGE, cflags);

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
}

event_ptr process_exit(bool isTry, struct str_message *sys_msg)
{ // Event parameters: command, user, status (+retval)
  char cflags[500];
  event_ptr event=processSyscall(isTry, sys_msg);
  sprintf(cflags, "%d", sys_msg->msg_data.msg_ask.args[0]);
  event_add_paramString(event, "status", PARAM_CONTUSAGE, cflags);
  return event;
}

event_ptr process_socket(bool isTry, struct str_message *sys_msg)
{ // Event parameters: command, user, domain (+retval)
  char cflags[500];
  event_ptr event=processSyscall(isTry, sys_msg);
  sprintf(cflags, "%d,%d", sys_msg->msg_data.msg_ask.args[0], sys_msg->msg_data.msg_ask.args[1]);
  event_add_paramString(event, "domain/type", PARAM_CONTUSAGE, cflags);
  return event;
}

event_ptr process_accept(bool isTry, struct str_message *sys_msg)
{ // Event parameters: command, user, sockfd (+retval)
  char cflags[500];
  event_ptr event=processSyscall(isTry, sys_msg);
  sprintf(cflags, "%d", sys_msg->msg_data.msg_ask.args[0]);
  event_add_paramString(event, "sockfd", PARAM_CONTUSAGE, cflags);
  return event;
}

event_ptr process_pipe(bool isTry, struct str_message *sys_msg)
{ // Event parameters: command, user, fdsource, fddests  (+retval)
  void *p;
  int arrfd[2];
  char cflags[500];

  event=processSyscall(isTry, sys_msg);
  memcpy( &p, &sys_msg->msg_data.msg_ask.args[0], sizeof(p) );
  copy_io( cfd,sys_msg->msg_pid, SYSTR_READ, p, &arrfd[0], sizeof(int) );
  copy_io( cfd,sys_msg->msg_pid, SYSTR_READ, p+sizeof(int), &arrfd[1], sizeof(int) );

  // File descriptor source
  sprintf(cflags,"%d",arrfd[0]);
  event_add_paramString(event, "source", PARAM_CONTUSAGE, cflags);

  // File descriptor dest
  sprintf(cflags,"%d",arrfd[1]);
  event_add_paramString(event, "dest", PARAM_CONTUSAGE, cflags);
  break;
}

event_ptr process_open(bool isTry, struct str_message *sys_msg)
{
  //case SYS_creat:
    case SYS_open:      // Event parameters: command, user, filename, and flags  (+return value)
      memcpy( &p, &sys_msg->msg_data.msg_ask.args[0], sizeof(p) );
      get_string_buffer( cfd, sys_msg->msg_pid, SYSTR_READ, p, &filename[0] );

      if((IGNORE_SYS_FILES)&&(may_ignore(&filename[0])))
      {
        action_desc_ptr actionDesc=find_action_desc_by_name_or_insert(pdp_action_desc_store, DFM_eventnames[sys_msg->msg_data.msg_ask.code]);
        event=event_new(actionDesc, IDX_ONGOING, isTry);
      }
      else
      {
        event=processSyscall(isTry, sys_msg);
        event_add_paramString(event, "filename", PARAM_CONTUSAGE, filename);
        event_add_paramString(event, "flags", PARAM_CONTUSAGE, byteToBinary(sys_msg->msg_data.msg_ask.args[1]));
      }
      break;
}






*/








/* not used */
/*
case SYS_munmap:
  user_info = getUserInfo(sys_msg->msg_pid);
  event = (eventOLD_t *)malloc(sizeof(eventOLD_t));
  event->event_name = strdup(DFM_eventnames[sys_msg->msg_data.msg_ask.code]);
  event->mode = M_EALL;

  // Parameters: command user
  if (sys_msg->msg_type==SYSTR_MSG_RES){
    event->n_params = 6;
  }else {
    event->n_params = 5;
  }
  c_param = 0;
  event->params = (param_t*)malloc(sizeof(param_t) * event->n_params ) ;
  // Command
  sprintf(cflags,"[%06d]", sys_msg->msg_pid);
  event->params[c_param].param_name = strdup("pid");
  if (cflags) {
    event->params[c_param].param_value = strdup(cflags);
  } else {
    event->params[c_param].param_value = strdup("< >");
  }

  sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
  command = strdup(cflags);
  event->params[c_param].param_name = strdup("command");
  if (command) {
    event->params[c_param].param_value = command;
  } else {
    event->params[c_param].param_value = strdup("< >");
  }

  // User
  c_param++;
  event->params[c_param].param_name = strdup("user");
  event->params[c_param].param_value = strdup(user_info->pw_name);

  // start
  c_param++;
  event->params[c_param].param_name = strdup("start");
  //        sprintf(cflags,"0x%x(%s)",sys_msg->msg_data.msg_ask.args[0],byte_to_binary(sys_msg->msg_data.msg_ask.args[0]));
  sprintf(cflags,"%x",sys_msg->msg_data.msg_ask.args[0]);
  event->params[c_param].param_value = strdup(cflags);

  // length
  c_param++;
  event->params[c_param].param_name = strdup("length");
  sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.args[1]);
  event->params[c_param].param_value = strdup(cflags);

  if (sys_msg->msg_type==SYSTR_MSG_RES){
    c_param++;
    event->params[c_param].param_name = strdup("retval");
    //        sprintf(cflags,"0x%x(%s)",sys_msg->msg_data.msg_ask.rval[0],byte_to_binary(sys_msg->msg_data.msg_ask.rval[0]));
    sprintf(cflags,"%x",sys_msg->msg_data.msg_ask.rval[0]);
    event->params[c_param].param_value = strdup(cflags);
  }
  break;

case SYS_ioctl:
  fd = sys_msg->msg_data.msg_ask.args[0];
  p_filename = get_file_name(fd);
  user_info = getUserInfo(sys_msg->msg_pid);

  // Generate event
  event = (eventOLD_t *)malloc(sizeof(eventOLD_t));
  event->event_name=strdup("ioctl");
  event->mode = M_EALL;
  // Parameters: command user filename fd request(+retval)
  //retvalue
  if (sys_msg->msg_type==SYSTR_MSG_RES){
    event->n_params = 7;
  }else {
    event->n_params = 6;
  }

  event->params = (param_t*)malloc(sizeof(param_t) * event->n_params);
  c_param = 0;
  // Command
  sprintf(cflags,"[%06d]", sys_msg->msg_pid);
  event->params[c_param].param_name = strdup("pid");
  if (cflags) {
    event->params[c_param].param_value = strdup(cflags);
  } else {
    event->params[c_param].param_value = strdup("< >");
  }

  sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
  command = strdup(cflags);
  event->params[c_param].param_name = strdup("command");
  if (command) {
    event->params[c_param].param_value = command;
  } else {
    event->params[c_param].param_value = strdup("< >");
  }

  // User
  c_param++;
  event->params[c_param].param_name = strdup("user");
  event->params[c_param].param_value = strdup(user_info->pw_name);

  // Filename
  c_param++;
  event->params[c_param].param_name = strdup("filename");
  if (p_filename) {
    event->params[c_param].param_value = strdup(p_filename);
  } else {
    event->params[c_param].param_value = strdup((fd==2)?"stderr":((fd==1)?"stdout":(fd==0)?"stdin":"unknown"));
  }

  // FileDescriptor
  c_param++;
  event->params[c_param].param_name = strdup("filedescriptor");
  sprintf(cflags,"%d",fd);
  event->params[c_param].param_value = strdup(cflags);

  // request
  c_param++;
  event->params[c_param].param_name = strdup("request");
  sprintf(cflags,"0x%x(%s)",sys_msg->msg_data.msg_ask.args[1],byte_to_binary(sys_msg->msg_data.msg_ask.args[1]));
  event->params[c_param].param_value = strdup(cflags);

  //retvalue
  if (sys_msg->msg_type==SYSTR_MSG_RES){
    c_param++;
    event->params[c_param].param_name = strdup("retval");
    sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.rval[0]);
    event->params[c_param].param_value = strdup(cflags);
  }
  break;  */









