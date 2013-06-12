/*
 * systrace_utils.c
 *      Authors: Carolina Lorini, Ricardo Neisse
 */

#include "systrace_utils.h"
#include "log_systrace_utils_c.h"

// Systrace file descriptor
int cfd;
int opid;

GHashTable *pidfdtable;
unsigned int initializeFDTable()
{
  log_info("Initializing file descriptors table");
  pidfdtable=g_hash_table_new(g_int_hash, g_int_equal);
  check_null_int(pidfdtable, "Unable to create fd-mapping table");
  return R_SUCCESS;
}

/*
 * This method stores the mapping between the filedescriptor and the filename
 *
 * @PARAM:	fd		the filepointer
 * @PARAM:	filename	the corresponding filename
 * @RETURN	0	if everything is fine
 * 		     -1	if there was an error
 */
unsigned int store_fd_filename_mapping(int pid, int fd, char *filename)
{
  check_null_int(filename, "Failure: Storing empty filename for mapping");
  GHashTable *lfdtable=g_hash_table_lookup(pidfdtable, &pid);
  if(lfdtable==NULL) lfdtable=g_hash_table_new(g_int_hash, g_int_equal);

  int *x=(int*)mem_alloc(sizeof(int));
  int *y=(int*)mem_alloc(sizeof(int));

  *x=fd; *y=pid;
  g_hash_table_insert(lfdtable, x, strdup(filename));
  g_hash_table_insert(pidfdtable, y, lfdtable);
  log_info("%s - Associated file descriptor [%d][%d] with [%s]",__func__, pid, fd, filename);
  return R_SUCCESS;
}

int get_fd(int pid, char *filename)
{ // not implemented for new structure (as mapping pid -> filename is NOT unambiguous and not really used)
  return -1;
}

/*
 * Deletes the mapping between the file descriptor and the filename
 * @PARAM:	fd	the filepointer
 * @RETURN	R_SUCCESS	if everything went well
 * 		      R_ERROR 	if there was an error
 */
unsigned int delete_fd_filename_mapping(int pid, int fd)
{
  GHashTable *lfdtable=g_hash_table_lookup(pidfdtable, &pid);
  if(lfdtable==NULL) return R_ERROR;
  else g_hash_table_remove(lfdtable, &fd);
  return R_SUCCESS;
}


char* get_file_name(int pid, int fd)
{
  log_debug("%s - Querying fd association [%d][%d]",__func__, pid, fd);
  GHashTable *lfdtable=g_hash_table_lookup(pidfdtable, &pid);
  if(lfdtable==NULL) return NULL;
  else
  {
    char *filename=g_hash_table_lookup(lfdtable, &fd);
    if(filename!=NULL) log_debug("%s - returning [%s]\n", __func__, filename);
    else log_debug("%s - returning NULL", __func__);
    return filename;
  }
}

/* Method used to copy bytes from the address
 * space of the traced program to the address
 * space of the handler
 * 
 * @PARAM:	fd		the filepointer of the DEVICE
 * @PARAM:	pid		the PID of the traces process
 * @PARAM:	op		do we want to read or write?
 * @PARAM:	addr		from where do we read (addr in userspace)
 * @PARAM:	buf		where do we store the read bytes
 * @PARAM:	size		how many bytes shoud be read
 *
 * @RETURN how many are indeed read
 */
int copy_io(int fd, pid_t pid, int op, void *addr, void *buf, size_t size)
{
  struct systrace_io io;
  memset(&io, 0, sizeof(io));
  io.strio_pid=pid;
  io.strio_addr=buf;
  io.strio_len=size;
  io.strio_offs=addr;
  io.strio_op=op;
  if(ioctl(fd, STRIOCIO, &io) == -1)
  {
    perror("ioctl(STRIOCIO):");
    return (-1);
  }
  return (0);
}

/*
 * This method reads the string where source points to
 *
 * @PARAM:	fd		the filepointer of the DEVICE
 * @PARAM:	pid		the PID of the traces process
 * @PARAM:	op		do we want to read or write?
 * @PARAM:	source		from where do we read
 * @PARAM:	target		where do we store the read bytes
 *
 * @RETURN:	how many bytes are read
 */
int get_string_buffer(int fd, pid_t pid, int op, void *source, void *target)
{
  source=(unsigned char *)source - sizeof(char);
  target=(unsigned char *)target - sizeof(char);
  int read=-1;
  do
  {
    source=(unsigned char *)source + sizeof(char);
    target=(unsigned char *)target + sizeof(char);
    copy_io(fd, pid, op, source, target, 1);
    read++;
  }
  while((*(char *)target != '\0'));
  return read;
}

/*
 * This method reads SIZE bytes from where SOURCE points to
 * and stores the bytes where TARGET points to
 * 
 * @PARAM:	fd		the filepointer of the DEVICE
 * @PARAM:	pid		the PID of the traces process
 * @PARAM:	source		from where do we read
 * @PARAM:	target		where do we store the read bytes
 * @PARAM:	size		the amount of bytes we process
 * 
 * @RETURN: how many bytes are indeed processed
 */
int get_fixed_size_buffer(int fd, pid_t pid, void *source, void *target, size_t size)
{
  int i, ret;

  // backup pointers
  void *source_org=source;
  void *target_org=target;

  source=(unsigned char *)source - sizeof(char);
  target=(unsigned char *)target - sizeof(char);

  for(i=0; i < size; i++)
  {
    source=(unsigned char *)source + sizeof(char);
    target=(unsigned char *)target + sizeof(char);
    ret=copy_io(fd, pid, SYSTR_READ, source, target, 1);
  }

  // restore pointers
  source=source_org;
  target=target_org;

  return ret;
}

/*
 * This method writes SIZE bytes from where SOURCE points to and stores the bytes where TARGET points to
 * 
 * @PARAM:	fd		the filepointer of the DEVICE
 * @PARAM:	pid		the PID of the traces process
 * @PARAM:	source		from where do we read
 * @PARAM:	target		where do we store the read bytes
 * @PARAM:	size		the amount of bytes we process
 * 
 * @RETURN: how many bytes are indeed processed
 */
int set_fixed_size_buffer(int fd, pid_t pid, void *source, void *target, size_t size)
{
  int i;

  // backup pointers
  void *source_org=source;
  void *target_org=target;

  source=(unsigned char *)source - sizeof(char);
  target=(unsigned char *)target - sizeof(char);

  for(i=0; i < size; i++)
  {
    source=(unsigned char *)source + sizeof(char);
    target=(unsigned char *)target + sizeof(char);
    if(copy_io(fd, pid, SYSTR_WRITE, target, source, 1) != 0) return -1; /* an error occured */
  }

  // restore pointers
  source=source_org;
  target=target_org;

  // everything is fine
  return 0;
}

// Systrace functions
int init_systrace()
{
  int fd;
  if((fd = open("/dev/systrace", O_RDONLY)) == -1) err(1, "/dev/systrace");

  /* Get a systrace descriptor. */
  if(ioctl(fd, STRIOCCLONE, &cfd) == -1) err(1, "STRIOCCLONE");
  close(fd);
  return cfd;
}

/**
 * Receives an array char* as  parameter
 * Each char* in the array is the name of one property to be attested
 */
unsigned char *do_attach_process(unsigned char *msg)
{
  int i;
  const char *val;

  pid_t pid=atoi(msg);
  opid = pid;
  log_info("Attaching process [%d]",pid);
  unsigned char *out=(unsigned char*)mem_alloc(8*sizeof(unsigned char));

  if(ioctl(cfd, STRIOCATTACH, &pid) == -1) sprintf(out,"fail");
  else
  {
    if(install_policy(pid)) sprintf(out,"fail");
    else                    sprintf(out,"success");
  }
  return out;
}

int install_policy(pid_t pid)
{
  struct systrace_policy strpol;
  int i;

  // Install systrace-policy that allows all system calls
  strpol.strp_op = SYSTR_POLICY_NEW;
  strpol.strp_maxents = SYS_MAXSYSCALL;
  if(ioctl(cfd, STRIOCPOLICY, &strpol) == -1) return -1;

  strpol.strp_op = SYSTR_POLICY_ASSIGN;
  strpol.strp_pid = pid;
  if(ioctl(cfd, STRIOCPOLICY, &strpol) == -1) return -1;

  // Permit all system calls
  for(i=0; i < SYS_MAXSYSCALL; i++)
  {
    strpol.strp_op=SYSTR_POLICY_MODIFY;
    strpol.strp_code=i;
    strpol.strp_policy=SYSTR_POLICY_PERMIT;
    if(ioctl(cfd, STRIOCPOLICY, &strpol) == -1) return -1;
  }

  // Register to all system calls
  for(i=0; i < SYS_MAXSYSCALL; i++)
    ask_syscalls(cfd,i,strpol);
  log_debug("Systrace-Policy installed");
  return 0;
}


int ask_syscalls(int cfd, int syscall_code, struct systrace_policy strpol)
{
  strpol.strp_op = SYSTR_POLICY_MODIFY;
  strpol.strp_code = syscall_code;
  strpol.strp_policy = SYSTR_POLICY_ASK;
  if(ioctl(cfd, STRIOCPOLICY, &strpol) == -1) return -1;
  else return 0;
}

unsigned int answerSystrace(struct str_message *sys_msg, struct systrace_answer *sys_ans)
{ // answer to systrace
  if(sys_msg->msg_type==SYSTR_MSG_RES)
  {
    //log_info("astps");
    //memset(&sys_ans, 0, sizeof(sys_ans));
    memset(sys_ans, 0, sizeof(struct systrace_answer));
    sys_ans->stra_pid = sys_msg->msg_pid;
    sys_ans->stra_seqnr = sys_msg->msg_seqnr;
  }

  //log_info("astsioc");
  if(ioctl(cfd, STRIOCANSWER, sys_ans) == -1) 
  {
    err(1, "STRIOCANSWER"); 
    //log_error("astsioc2");
    //if(ioctl(cfd, STRIOCANSWER, sys_ans) == -1)
    //{
    //  log_error("error sending answer to systrace!");
    //  err(1, "STRIOCANSWER");
      return R_ERROR;
    //}
    //else return R_SUCCESS;
  }
  return R_SUCCESS;
}



// move to somewhere else!
int obsd_replace(pid_t pid, u_int16_t seqnr, struct intercept_replace *repl)
{
  struct systrace_replace replace;
  size_t len, off;
  int i, ret;

  memset(&replace, 0, sizeof(replace));

  for(i=0, len=0; i < repl->num; i++)
  {
    len+=repl->len[i];
  }

  replace.strr_pid=pid;
  replace.strr_seqnr=seqnr;
  replace.strr_nrepl=repl->num;
  replace.strr_base=malloc(len);
  replace.strr_len=len;
  if(replace.strr_base == NULL) err(1, "%s: malloc", __func__);

  for(i=0, off=0; i < repl->num; i++)
  {
    replace.strr_argind[i]=repl->ind[i];
    replace.strr_offlen[i]=repl->len[i];
    if(repl->len[i] == 0)
    { /* address[i] contains an integer and not an address */
      replace.strr_off[i]=(size_t)repl->address[i];
      continue; /* go to the next for-loop */
    }

    replace.strr_off[i]=off; /* strr_off[i] points to the malloc-area where the value is stored */
    memcpy(replace.strr_base + off, repl->address[i], repl->len[i]); /* copy value into handler-userspace malloc area */

    if(repl->flags[i] & ICTRANS_NOLINKS) /* check if flags == ICTRANS_NOLINKS(=1) */
    {
      replace.strr_flags[i]=SYSTR_NOLINKS;
    }
    else
    {
      replace.strr_flags[i]=0;
    }

    off+=repl->len[i];
  }

  ret=ioctl(cfd, STRIOCREPLACE, &replace);
  if(ret == -1 && errno != EBUSY)
  {
    warn("%s: ioctl", __func__);
  }

  free(replace.strr_base);
  return (ret);
}

/* This method will replace the filename of
 * the open syscall
 *
 * @PARAM:	cfd		filepointer to /dev/systrace (globally defined)
 * @PARAM:	pid		the pid of the traced process
 * @PARAM:	seqnr		the sequence number of the open syscall
 * 				where you want to change the filename
 * @PARAM: 	newFilename	the new filename
 *
 */
void open_filename_replace(pid_t pid, u_int16_t seqnr, char* newFilename)
{
  /* change the filename */
  printf("\n\n\nAHHHHHHHHHHHHHHHHHHHH OPEN FILENAME REPLACEn\n\n\n");
  fflush(NULL);
  struct intercept_replace repl;
  repl.num=1; /* we want to change 1 argument in total */
  repl.ind[0]=0; /* we want to change the first argument */
  repl.address[0]=newFilename; /* address of the new filename */
  repl.len[0]=strlen(newFilename) + 1; /* how many bytes should be read from address, +1 due to the '\0' */
  repl.flags[0]=0;

  obsd_replace(pid, seqnr, &repl);
}

void open_flag_replace(pid_t pid, u_int16_t seqnr, int flag)
{
  // change the filename
  printf("\n\n\nAHHHHHHHHHHHHHHHHHHHH OPEN FLAG REPLACEn\n\n\n");
  fflush(NULL);
  struct intercept_replace repl;
  repl.num=1; // we want to change 1 argument in total
  repl.ind[0]=1; // we want to change the second argument

  repl.address[0]=(char*)flag;
  repl.len[0]=0; // how many bytes should be read from address, +1 due to the '\0'
  repl.flags[0]=0;

  obsd_replace(pid, seqnr, &repl);
}

