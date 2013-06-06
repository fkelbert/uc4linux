///*
// * systrace_utils.c
// *
// *      Authors: Carolina Lorini, Ricardo Neisse
// */
//
//#include "systrace_utils.h"
//
//// TODO: optimize the table with hash or linked list
//// TODO: sanitize parameter PID in all the functions
//char* fd_table[AMOUNT_OF_PIDS][AMOUNT_OF_FDS];
//
//
//void initializeFDTable(void) {
//  // sets all bytes to 0
//  plog(LOG_TRACE,"Initializing file descriptors table");
//  memset( fd_table, 0, AMOUNT_OF_FDS * AMOUNT_OF_PIDS );
//}
//
///*
// * This method stores the mapping between the
// * filedescriptor and the filename
// *
// * @PARAM:	fd		the filepointer
// * @PARAM:	filename	the corresponding filename
// *
// * @RETURN	0	if everything is fine
// * 		-1	if there was an error
// */
//int store_fd_filename_mapping(int pid, int fd, char *filename ) {
//  // check if fd is in range
//  if( fd < 0 || fd > ( AMOUNT_OF_FDS - 1 ))
//    return -1;
//  // check if filename is != NULL
//  if( filename == NULL )
//    return -1;
//  //plog(LOG_TRACE,"Associating file descriptor [%d] with [%s]",fd, filename);
//  // allocate memory, +1 due to \0 */r
//  size_t size = strlen( filename );
//  fd_table[pid][fd] = (char *)malloc( size+1 );
//  if( fd_table[pid][ fd ] == NULL ) {
//    // no memory could be allocated
//    return -1;
//  }
//  // copy filename to allocated memory, +1 due to \0 */
//  memcpy( fd_table[pid][fd], filename, size+1 );
//  return 0;
//}
//
//
//
//int get_fd(int pid, char *filename ) {
//  int res=0;
//  // check if filename is != NULL
//  if( filename == NULL )
//    return -1;
//  //plog(LOG_TRACE,"Associating file descriptor [%d] with [%s]",fd, filename);
//  // allocate memory, +1 due to \0 */r
//
//  while ((strcmp(fd_table[pid][res],filename)!=0)&&(res<AMOUNT_OF_FDS)) {
//    res++;
//  }
//  if( res== AMOUNT_OF_FDS ) {
//    // no memory could be allocated
//    return -1;
//  } else {
//    return res;
//  }
//}
///*
// * Deletes the mapping between the
// * file descriptor and the filename
// *
// * @PARAM:	fd	the filepointer
// * @RETURN	0	if everything went well
// * 		-1	if there was an error
// */
//int delete_fd_filename_mapping(int pid,  int fd ) {
//  // check if fd is in range
//  if( fd < 0 || fd > ( AMOUNT_OF_FDS - 1 ) ) {
//    return -1;
//  }
//  // plog(LOG_TRACE,"Removing association between file descriptor [%d] and [%s]",fd, fd_table[fd]);
//  free( fd_table[pid][fd]);		/* free memory */
//  fd_table[pid][ fd ] = NULL;			/* set char * to NULL */
//  // everything went well
//  return 0;
//}
//
//char* get_file_name(int pid, int fd) {
//  // check if fd is in range
//  if( fd < 0 || fd > ( AMOUNT_OF_FDS - 1 ) )
//    return NULL;
//  // plog(LOG_TRACE,"Querying fd association [%d][%s]",fd, fd_table[fd]);
//  return (fd_table[pid][fd]);
//}
//
///* Method used to copy bytes from the address
// * space of the traced program to the address
// * space of the handler
// *
// * @PARAM:	fd		the filepointer of the DEVICE
// * @PARAM:	pid		the PID of the traces process
// * @PARAM:	op		do we want to read or write?
// * @PARAM:	addr		from where do we read (addr in userspace)
// * @PARAM:	buf		where do we store the read bytes
// * @PARAM:	size		how many bytes shoud be read
// *
// * @RETURN how many are indeed read
// */
//int copy_io(int fd, pid_t pid, int op, void *addr, void *buf, size_t size) {
//  struct systrace_io io;
//  memset(&io, 0, sizeof(io));
//  io.strio_pid = pid;
//  io.strio_addr = buf;
//  io.strio_len = size;
//  io.strio_offs = addr;
//  io.strio_op = op;
//  if (ioctl(fd, STRIOCIO, &io) == -1) {
//    perror( "ioctl(STRIOCIO):" );
//    return (-1);
//  }
//  return (0);
//}
//
///*
// * This method reads the string where source points to
// *
// * @PARAM:	fd		the filepointer of the DEVICE
// * @PARAM:	pid		the PID of the traces process
// * @PARAM:	op		do we want to read or write?
// * @PARAM:	source		from where do we read
// * @PARAM:	target		where do we store the read bytes
// *
// * @RETURN:	how many bytes are read
// */
//int get_string_buffer( int fd, pid_t pid, int op, void *source, void *target ) {
//  source -= sizeof(char);
//  target -= sizeof(char);
//  int read = -1;
//  do {
//    source += sizeof(char);
//    target += sizeof(char);
//    copy_io( fd, pid, op, source, target, 1 );
//    read++;
//  } while( (*(char *)target != '\0') );
//
//  return read;
//}
//
///*
// * This method reads SIZE bytes from where SOURCE points to
// * and stores the bytes where TARGET points to
// *
// * @PARAM:	fd		the filepointer of the DEVICE
// * @PARAM:	pid		the PID of the traces process
// * @PARAM:	source		from where do we read
// * @PARAM:	target		where do we store the read bytes
// * @PARAM:	size		the amount of bytes we process
// *
// * @RETURN: how many bytes are indeed processed
// */
//int get_fixed_size_buffer( int fd, pid_t pid, void *source, void *target, size_t size ) {
//  int i, ret;
//
//  // backup pointers
//  void *source_org = source;
//  void *target_org = target;
//
//  source -= sizeof(char);
//  target -= sizeof(char);
//
//  for( i=0; i < size; i++ )
//  {
//    source += sizeof(char);
//    target += sizeof(char);
//    ret = copy_io( fd, pid, SYSTR_READ, source, target, 1 );
//  }
//
//  // restore pointers
//  source = source_org;
//  target = target_org;
//
//  return ret;
//}
//
///*
// * This method writes SIZE bytes from where SOURCE points to
// * and stores the bytes where TARGET points to
// *
// * @PARAM:	fd		the filepointer of the DEVICE
// * @PARAM:	pid		the PID of the traces process
// * @PARAM:	source		from where do we read
// * @PARAM:	target		where do we store the read bytes
// * @PARAM:	size		the amount of bytes we process
// *
// * @RETURN: how many bytes are indeed processed
// */
//int set_fixed_size_buffer( int fd, pid_t pid, void *source, void *target, size_t size ) {
//  int i;
//
//  // backup pointers
//  void *source_org = source;
//  void *target_org = target;
//
//  source -= sizeof(char);
//  target -= sizeof(char);
//
//  for( i=0; i < size; i++ )
//  {
//    source += sizeof(char);
//    target += sizeof(char);
//    if( copy_io( fd, pid, SYSTR_WRITE, target, source, 1 ) != 0 )
//      return -1;	/* an error occured */
//  }
//
//  // restore pointers
//  source = source_org;
//  target = target_org;
//
//  // everything is fine
//  return 0;
//}
//
//int obsd_replace(int fd, pid_t pid, u_int16_t seqnr, struct intercept_replace *repl) {
//  struct systrace_replace replace;
//  size_t len, off;
//  int i, ret;
//
//  memset(&replace, 0, sizeof(replace));
//
//  for (i = 0, len = 0; i < repl->num; i++) {
//    len += repl->len[i];
//  }
//
//  replace.strr_pid = pid;
//  replace.strr_seqnr = seqnr;
//  replace.strr_nrepl = repl->num;
//  replace.strr_base = malloc(len);
//  replace.strr_len = len;
//  if (replace.strr_base == NULL)
//    err(1, "%s: malloc", __func__);
//
//  for (i = 0, off = 0; i < repl->num; i++) {
//    replace.strr_argind[i] = repl->ind[i];
//    replace.strr_offlen[i] = repl->len[i];
//    if (repl->len[i] == 0)
//    {		/* address[i] contains an integer and not an address */
//      replace.strr_off[i] = (size_t)repl->address[i];
//      continue;	/* go to the next for-loop */
//    }
//
//    replace.strr_off[i] = off;	/* strr_off[i] points to the malloc-area where the value is stored */
//    memcpy(replace.strr_base + off, repl->address[i], repl->len[i]);	/* copy value into handler-userspace malloc area */
//
//    if (repl->flags[i] & ICTRANS_NOLINKS)	/* check if flags == ICTRANS_NOLINKS(=1) */
//    {
//      replace.strr_flags[i] = SYSTR_NOLINKS;
//    } else
//    {
//      replace.strr_flags[i] = 0;
//    }
//
//    off += repl->len[i];
//  }
//
//  ret = ioctl(fd, STRIOCREPLACE, &replace);
//  if (ret == -1 && errno != EBUSY) {
//    warn("%s: ioctl", __func__);
//  }
//
//  free(replace.strr_base);
//
//  return (ret);
//}
//
///* This method will replace the filename of
// * the open syscall
// *
// * @PARAM:	cfd		filepointer to /dev/systrace
// * @PARAM:	pid		the pid of the traced process
// * @PARAM:	seqnr		the sequence number of the open syscall
// * 				where you want to change the filename
// * @PARAM: 	newFilename	the new filename
// *
// */
//void open_filename_replace( int cfd, pid_t pid, u_int16_t seqnr, char* newFilename ) {
//  /* change the filename */
//  struct intercept_replace repl;
//  repl.num = 1;	/* we want to change 1 argument in total */
//  repl.ind[ 0 ] = 0;	/* we want to change the first argument */
//  repl.address[ 0 ] = newFilename;	/* address of the new filename */
//  repl.len[ 0 ] = strlen( newFilename ) + 1;	/* how many bytes should be read from address, +1 due to the '\0' */
//  repl.flags[ 0 ] = 0;
//
//  obsd_replace( cfd, pid, seqnr, &repl );
//}
//
//void open_flag_replace( int cfd, pid_t pid, u_int16_t seqnr, int flag ) {
//  // change the filename
//  struct intercept_replace repl;
//  repl.num = 1;   // we want to change 1 argument in total
//  repl.ind[ 0 ] = 1;  // we want to change the second argument
//  repl.address[ 0 ] = flag;
//  repl.len[ 0 ] = 0;  // how many bytes should be read from address, +1 due to the '\0'
//  repl.flags[ 0 ] = 0;
//
//  obsd_replace( cfd, pid, seqnr, &repl );
//}
//
//
