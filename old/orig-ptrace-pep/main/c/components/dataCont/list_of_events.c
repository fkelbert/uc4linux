/*
 * list_of_events.c
 *  Created on: 19/lug/2010
 *      Author: cornelius moucha, lovat
 */

#include "list_of_events.h"
#include "pdpTypes.h"
#include <mxml.h>

#define IMPLEMENTED_SYSCALLS 364
GHashTable *syscallTable = NULL;
GHashTable *defaultSyscallTable = NULL;
// comparison for 1.000.000 lookups: 2.099s (old way) vs. 0.177s (including preparation of hashtable)

/**
 * Creates the hash table that gathers the relation between system call names and ids
 *
 * @return Integer indicating the success of the funciton
 */
unsigned int initSyscallTable()
{
  syscallTable=g_hash_table_new(g_str_hash, g_str_equal);
  // inside log_error("Error initializing syscall hashtable\n");
  if(!syscallTable) { return R_ERROR;}

  int a;
  /* Allocate every entry for every system call */
  for(a=0;a<IMPLEMENTED_SYSCALLS; a++)
  {
    unsigned int *b=(unsigned int*)malloc(sizeof(int));
    *b=a;
    g_hash_table_insert(syscallTable, DFM_eventnames[a], b);
  }
  return R_SUCCESS;
}
/**
 * Creates the hash table that gathers the relation between system call codes and their default response
 *
 * @param file The name of the file we are loading the default responses for every system call
 *
 * @return Integer indicating the success of the funciton
 */
unsigned int initDefaultAnswerTable(char *file)
{
  /* Check if it is the first time to call this function */
  if(defaultSyscallTable != NULL){
	  g_hash_table_destroy(defaultSyscallTable);
  }
  defaultSyscallTable=g_hash_table_new(g_int_hash, g_int_equal);
  // inside log_error("Error initializing syscall hashtable\n");
  if(!defaultSyscallTable) { return R_ERROR;}
  int a;
  FILE *fp;
  mxml_node_t *tree;
  mxml_node_t *node;
  mxml_node_t *number;
  mxml_node_t *defAction;
  /* Open the file and load the <data> element */
  fp = fopen(file, "r");
  tree = mxmlLoadFile(NULL, fp, MXML_INTEGER_CALLBACK);
  node = mxmlFindElement(tree, tree, "data", NULL, NULL, MXML_DESCEND);
  /* Get the first system call */
  node = mxmlGetFirstChild(node);
  do{
	/* Get the code of the system call and the default policy for it */
  	number = mxmlFindElement(node, tree, "syscallNumber", NULL, NULL, MXML_DESCEND);
  	defAction = mxmlFindElement(node, tree, "defaultAction", NULL, NULL, MXML_DESCEND);
  	unsigned int *b=(unsigned int*)malloc(sizeof(int));
  	*b=mxmlGetInteger(defAction);
  	unsigned int *c=(unsigned int*)malloc(sizeof(int));
  	*c=mxmlGetInteger(number);
  	/* Insert the gathered data into our hashtable */
  	g_hash_table_insert(defaultSyscallTable, c, b);
    node = mxmlGetNextSibling(node);
  }while(node != NULL);
  /* Clean up */
  fclose(fp);
  mxmlDelete(tree);
  return R_SUCCESS;
}
/**
 * Selects the default action of a system call from the hash table that contains this relation.
 *
 * @param syscallCode The code of the system call from which we are looking the default action
 *
 * @return Integer with the default action
 */
int syscallAction(int syscallCode){
	  if((syscallCode<0)||(syscallCode >= IMPLEMENTED_SYSCALLS )) return -1;
	  int *x=(int*)g_hash_table_lookup(defaultSyscallTable, &syscallCode);
	  return x!=NULL?*x:-1;
}

/*
 * Gets the system call code given the system call name
 *
 * @param syscall The name of the system call
 *
 * @return Integer containing the system call code
 */
int syscallToInt(const unsigned char *syscall)
{
  if(syscall==NULL) return -1;
  int *x=(int*)g_hash_table_lookup(syscallTable, syscall);
  return x!=NULL?*x:-1;
}

/*
 * Code for a default Action with respect to a syscall.
 *
 * 0 -> ACTION_INHIBIT
 * 1 -> ACTION_ALLOW
 * 2 -> ACTION_ASK
 *
 */

int DFM_defaultActions[]={
						ACTION_ALLOW, //"restart_syscall",   /*   0*/
						ACTION_ALLOW, //"exit",		 /*  1*/
						ACTION_ALLOW, //"fork",		 /*  2*/
						ACTION_ALLOW, //"read",		 /*  3*/
						ACTION_ALLOW, //"write",		 /*  4*/
						ACTION_ASK,   //"open",		 /*  5*/
						ACTION_ALLOW, //"close",		 /*  6*/
						ACTION_ALLOW, //"waitpid",		 /*  7*/
						ACTION_ALLOW, //"creat",		 /*  8*/
						ACTION_ALLOW, //"link",		 /*  9*/
						ACTION_ALLOW, //"unlink",		 /* 10*/
						ACTION_ALLOW, //"execve",		 /* 11*/
						ACTION_ALLOW, //"chdir",		 /* 12*/
						ACTION_ALLOW, //"time",		 /* 13*/
						ACTION_ALLOW, //"mknod",		 /* 14*/
						ACTION_ALLOW, //"chmod",		 /* 15*/
						ACTION_ALLOW, //"lchown",		 /* 16*/
						ACTION_ALLOW, //"break",		 /* 17*/
						ACTION_ALLOW, //"oldstat",		 /* 18*/
						ACTION_ALLOW, //"lseek",		 /* 19*/
						ACTION_ALLOW, //"getpid",		 /* 20*/
						ACTION_ALLOW, //"mount",		 /* 21*/
						ACTION_ALLOW, //"umount",		 /* 22*/
						ACTION_ALLOW, //"setuid",		 /* 23*/
						ACTION_ALLOW, //"getuid",		 /* 24*/
						ACTION_ALLOW, //"stime",		 /* 25*/
						ACTION_ALLOW, //"ptrace",		 /* 26*/
						ACTION_ALLOW, //"alarm",		 /* 27*/
						ACTION_ALLOW, //"oldfstat",		 /* 28*/
						ACTION_ALLOW, //"pause",		 /* 29*/
						ACTION_ALLOW, //"utime",		 /* 30*/
						ACTION_ALLOW, //"stty",		 /* 31*/
						ACTION_ALLOW, //"gtty",		 /* 32*/
						ACTION_ALLOW, //"access",		 /* 33*/
						ACTION_ALLOW, //"nice",		 /* 34*/
						ACTION_ALLOW, //"ftime",		 /* 35*/
						ACTION_ALLOW, //"sync",		 /* 36*/
						ACTION_ALLOW, //"kill",		 /* 37*/
						ACTION_ALLOW, //"rename",		 /* 38*/
						ACTION_ALLOW, //"mkdir",		 /* 39*/
						ACTION_ALLOW, //"rmdir",		 /* 40*/
						ACTION_ALLOW, //"dup",		 /* 41*/
						ACTION_ALLOW, //"pipe",		 /* 42*/
						ACTION_ALLOW, //"times",		 /* 43*/
						ACTION_ALLOW, //"prof",		 /* 44*/
						ACTION_ALLOW, //"brk",		 /* 45*/
						ACTION_ALLOW, //"setgid",		 /* 46*/
						ACTION_ALLOW, //"getgid",		 /* 47*/
						ACTION_ALLOW, //"signal",		 /* 48*/
						ACTION_ALLOW, //"geteuid",		 /* 49*/
						ACTION_ALLOW, //"getegid",		 /* 50*/
						ACTION_ALLOW, //"acct",		 /* 51*/
						ACTION_ALLOW, //"umount2",		 /* 52*/
						ACTION_ALLOW, //"lock",		 /* 53*/
						ACTION_ALLOW, //"ioctl",		 /* 54*/
						ACTION_ALLOW, //"fcntl",		 /* 55*/
						ACTION_ALLOW, //"mpx",		 /* 56*/
						ACTION_ALLOW, //"setpgid",		 /* 57*/
						ACTION_ALLOW, //"ulimit",		 /* 58*/
						ACTION_ALLOW, //"oldolduname",	/* 59*/
						ACTION_ALLOW, //"umask",		 /* 60*/
						ACTION_ALLOW, //"chroot",		 /* 61*/
						ACTION_ALLOW, //"ustat",		 /* 62*/
						ACTION_ALLOW, //"dup2",		 /* 63*/
						ACTION_ALLOW, //"getppid",		 /* 64*/
						ACTION_ALLOW, //"getpgrp",		 /* 65*/
						ACTION_ALLOW, //"setsid",		 /* 66*/
						ACTION_ALLOW, //"sigaction",		 /* 67*/
						ACTION_ALLOW, //"sgetmask",		 /* 68*/
						ACTION_ALLOW, //"ssetmask",		 /* 69*/
						ACTION_ALLOW, //"setreuid",		 /* 70*/
						ACTION_ALLOW, //"setregid",		 /* 71*/
						ACTION_ALLOW, //"sigsuspend",		 /* 72*/
						ACTION_ALLOW, //"sigpending",		 /* 73*/
						ACTION_ALLOW, //"sethostname",	 /* 74*/
						ACTION_ALLOW, //"setrlimit",		 /* 75*/
						ACTION_ALLOW, //"getrlimit",		 /* 76 */
						ACTION_ALLOW, //"getrusage",		 /* 77*/
						ACTION_ALLOW, //"gettimeofday",	 /* 78*/
						ACTION_ALLOW, //"settimeofday",	 /* 79*/
						ACTION_ALLOW, //"getgroups",		 /* 80*/
						ACTION_ALLOW, //"setgroups",		 /* 81*/
						ACTION_ALLOW, //"select",		 /* 82*/
						ACTION_ALLOW, //"symlink",		 /* 83*/
						ACTION_ALLOW, //"oldlstat",		 /* 84*/
						ACTION_ALLOW, //"readlink",		 /* 85*/
						ACTION_ALLOW, //"uselib",		 /* 86*/
						ACTION_ALLOW, //"swapon",		 /* 87*/
						ACTION_ALLOW, //"reboot",		 /* 88*/
						ACTION_ALLOW, //"readdir",		 /* 89*/
						ACTION_ALLOW, //"mmap",		 /* 90*/
						ACTION_ALLOW, //"munmap",		 /* 91*/
						ACTION_ALLOW, //"truncate",		 /* 92*/
						ACTION_ALLOW, //"ftruncate",		 /* 93*/
						ACTION_ALLOW, //"fchmod",		 /* 94*/
						ACTION_ALLOW, //"fchown",		 /* 95*/
						ACTION_ALLOW, //"getpriority",	 /*96*/
						ACTION_ALLOW, //"setpriority",	 /*97*/
						ACTION_ALLOW, //"profil",		 /* 98*/
						ACTION_ALLOW, //"statfs",		 /* 99*/
						ACTION_ALLOW, //"fstatfs",		 /*100*/
						ACTION_ALLOW, //"ioperm",		 /*101*/
						ACTION_ALLOW, //"socketcall",		 /*102*/
						ACTION_ALLOW, //"syslog",		 /*103*/
						ACTION_ALLOW, //"setitimer",		 /*104*/
						ACTION_ALLOW, //"getitimer",		 /*105*/
						ACTION_ALLOW, //"stat",		 /*106*/
						ACTION_ALLOW, //"lstat",		 /*107*/
						ACTION_ALLOW, //"fstat",		 /*108*/
						ACTION_ALLOW, //"olduname",		 /*109*/
						ACTION_ALLOW, //"iopl",		 /*110*/
						ACTION_ALLOW, //"vhangup",		 /*111*/
						ACTION_ALLOW, //"idle",		 /*112*/
						ACTION_ALLOW, //"vm86old",		 /*113*/
						ACTION_ALLOW, //"wait4",		 /*114*/
						ACTION_ALLOW, //"swapoff",		 /*115*/
						ACTION_ALLOW, //"sysinfo",		 /*116*/
						ACTION_ALLOW, //"ipc",		 /*117*/
						ACTION_ALLOW, //"fsync",		 /*118*/
						ACTION_ALLOW, //"sigreturn",		 /*119*/
						ACTION_ALLOW, //"clone",		 /*120*/
						ACTION_ALLOW, //"setdomainname",	/*121*/
						ACTION_ALLOW, //"uname",		 /*122*/
						ACTION_ALLOW, //"modify_ldt",		 /*123*/
						ACTION_ALLOW, //"adjtimex",		 /*124*/
						ACTION_ALLOW, //"mprotect",		 /*125*/
						ACTION_ALLOW, //"sigprocmask",	/* 126*/
						ACTION_ALLOW, //"create_module", /*	127*/
						ACTION_ALLOW, //"init_module",	/* 128*/
						ACTION_ALLOW, //"delete_module",	/* 129*/
						ACTION_ALLOW, //"get_kernel_syms",	/* 130*/
						ACTION_ALLOW, //"quotactl",		 /*131*/
						ACTION_ALLOW, //"getpgid",		 /*132*/
						ACTION_ALLOW, //"fchdir",		 /*133*/
						ACTION_ALLOW, //"bdflush",		 /*134*/
						ACTION_ALLOW, //"sysfs",		 /* 135*/
						ACTION_ALLOW, //"personality",	/* 136*/
						ACTION_ALLOW, //"afs_syscall",	/* 137 */
						ACTION_ALLOW, //"setfsuid",		 /*138*/
						ACTION_ALLOW, //"setfsgid",		 /*139*/
						ACTION_ALLOW, //"_llseek",		 /*140*/
						ACTION_ALLOW, //"getdents",		 /*141*/
						ACTION_ALLOW, //"_newselect",		 /*142*/
						ACTION_ALLOW, //"flock",		 /*143*/
						ACTION_ALLOW, //"msync",		 /*144*/
						ACTION_ALLOW, //"readv",		 /*145*/
						ACTION_ALLOW, //"writev",		 /*146*/
						ACTION_ALLOW, //"getsid",		 /*147*/
						ACTION_ALLOW, //"fdatasync",		 /*148*/
						ACTION_ALLOW, //"_sysctl",		 /*149*/
						ACTION_ALLOW, //"mlock",		 /*150*/
						ACTION_ALLOW, //"munlock",		 /*151*/
						ACTION_ALLOW, //"mlockall",		 /*152*/
						ACTION_ALLOW, //"munlockall",		 /*153*/
						ACTION_ALLOW, //"sched_setparam",		 /*154*/
						ACTION_ALLOW, //"sched_getparam",		 /*155*/
						ACTION_ALLOW, //"sched_setscheduler",		 /*156*/
						ACTION_ALLOW, //"sched_getscheduler",		 /*157*/
						ACTION_ALLOW, //"sched_yield",		 /*158*/
						ACTION_ALLOW, //"sched_get_priority_max",	/* 159*/
						ACTION_ALLOW, //"sched_get_priority_min",	/* 160*/
						ACTION_ALLOW, //"sched_rr_get_interval",	/*161*/
						ACTION_ALLOW, //"nanosleep",		 /*162*/
						ACTION_ALLOW, //"mremap",		 /*163*/
						ACTION_ALLOW, //"setresuid",		 /*164*/
						ACTION_ALLOW, //"getresuid",		 /*165*/
						ACTION_ALLOW, //"vm86",		 /*166*/
						ACTION_ALLOW, //"query_module",	/*167*/
						ACTION_ALLOW, //"poll",		 /*168*/
						ACTION_ALLOW, //"nfsservctl",		 /*169*/
						ACTION_ALLOW, //"setresgid",		 /*170*/
						ACTION_ALLOW, //"getresgid",		 /*171*/
						ACTION_ALLOW, //"prctl",            /*  172*/
						ACTION_ALLOW, //"rt_sigreturn",	/* 173*/
						ACTION_ALLOW, //"rt_sigaction",	 /* 174*/
						ACTION_ALLOW, //"rt_sigprocmask",	/* 175*/
						ACTION_ALLOW, //"rt_sigpending",	/* 176*/
						ACTION_ALLOW, //"rt_sigtimedwait",	/* 177*/
						ACTION_ALLOW, //"rt_sigqueueinfo",	/* 178*/
						ACTION_ALLOW, //"rt_sigsuspend",	/* 179*/
						ACTION_ALLOW, //"pread64",		 /*180*/
						ACTION_ALLOW, //"pwrite64",		 /*181*/
						ACTION_ALLOW, //"chown",		 /*182*/
						ACTION_ALLOW, //"getcwd",		 /*183*/
						ACTION_ALLOW, //"capget",		 /*184*/
						ACTION_ALLOW, //"capset",		 /*185*/
						ACTION_ALLOW, //"sigaltstack",	/*186*/
						ACTION_ALLOW, //"sendfile",		 /*187*/
						ACTION_ALLOW, //"getpmsg",		 /*188*/
						ACTION_ALLOW, //"putpmsg",		 /*189*/
						ACTION_ALLOW, //"vfork",		 /*190*/
						ACTION_ALLOW, //"ugetrlimit",		 /*191*/
						ACTION_ALLOW, //"mmap2",		 /*192*/
						ACTION_ALLOW, //"truncate64",		 /*193*/
						ACTION_ALLOW, //"ftruncate64",	/* 194*/
						ACTION_ALLOW, //"stat64",		 /*195*/
						ACTION_ALLOW, //"lstat64",		 /*196*/
						ACTION_ALLOW, //"fstat64",		 /*197*/
						ACTION_ALLOW, //"lchown32",		 /*198*/
						ACTION_ALLOW, //"getuid32",		 /*199*/
						ACTION_ALLOW, //"getgid32",		 /*200*/
						ACTION_ALLOW, //"geteuid32",		 /*201*/
						ACTION_ALLOW, //"getegid32",		 /*202*/
						ACTION_ALLOW, //"setreuid32",		 /*203*/
						ACTION_ALLOW, //"setregid32",		 /*204*/
						ACTION_ALLOW, //"getgroups32",	/* 205*/
						ACTION_ALLOW, //"setgroups32", 	/* 206*/
						ACTION_ALLOW, //"fchown32",		 /*207*/
						ACTION_ALLOW, //"setresuid32",	/* 208*/
						ACTION_ALLOW, //"getresuid32",	/* 209*/
						ACTION_ALLOW, //"setresgid32",	/* 210*/
						ACTION_ALLOW, //"getresgid32",	/* 211*/
						ACTION_ALLOW, //"chown32",		 /*212*/
						ACTION_ALLOW, //"setuid32",		 /*213*/
						ACTION_ALLOW, //"setgid32",		 /*214*/
						ACTION_ALLOW, //"setfsuid32",		 /*215*/
						ACTION_ALLOW, //"setfsgid32",		 /*216*/
						ACTION_ALLOW, //"pivot_root",		 /*217*/
						ACTION_ALLOW, //"mincore",		 /*218*/
						ACTION_ALLOW, //"madvise",		 /*219*/
						ACTION_ALLOW, //"madvise1",		 /*219 EVERYTHING +1 DOWN HERE	*/
						ACTION_ALLOW, //"getdents64",		 /*220*/
						ACTION_ALLOW, //"fcntl64",		 /*221*/
						ACTION_ALLOW, //"unused",			/*222*/
						ACTION_ALLOW, //"unused",			/*223*/
						ACTION_ALLOW, //"gettid",		 /*224*/
						ACTION_ALLOW, //"readahead",		 /*225*/
						ACTION_ALLOW, //"setxattr",		 /*226*/
						ACTION_ALLOW, //"lsetxattr",		 /*227*/
						ACTION_ALLOW, //"fsetxattr",		 /*228*/
						ACTION_ALLOW, //"getxattr",		 /*229*/
						ACTION_ALLOW, //"lgetxattr",		 /*230*/
						ACTION_ALLOW, //"fgetxattr",		 /*231*/
						ACTION_ALLOW, //"listxattr",		 /*232*/
						ACTION_ALLOW, //"llistxattr",		 /*233*/
						ACTION_ALLOW, //"flistxattr",		 /*234*/
						ACTION_ALLOW, //"removexattr",	/* 235*/
						ACTION_ALLOW, //"lremovexattr",	/* 236*/
						ACTION_ALLOW, //"fremovexattr",	/* 237*/
						ACTION_ALLOW, //"tkill",		 /*238*/
						ACTION_ALLOW, //"sendfile64",		 /*239*/
						ACTION_ALLOW, //"futex",		 /*240*/
						ACTION_ALLOW, //"sched_setaffinity",	/* 241*/
						ACTION_ALLOW, //"sched_getaffinity",	/* 242*/
						ACTION_ALLOW, //"set_thread_area",	/* 243*/
						ACTION_ALLOW, //"get_thread_area",	/* 244*/
						ACTION_ALLOW, //"io_setup",		 /*245*/
						ACTION_ALLOW, //"io_destroy",		 /*246*/
						ACTION_ALLOW, //"io_getevents",	/* 247*/
						ACTION_ALLOW, //"io_submit",		 /*248*/
						ACTION_ALLOW, //"io_cancel",		 /*249*/
						ACTION_ALLOW, //"fadvise64",		 /*250*/
						ACTION_ALLOW, //"available",		/*251*/
						ACTION_ALLOW, //"exit_group",		 /*252*/
						ACTION_ALLOW, //"lookup_dcookie",	/* 253*/
						ACTION_ALLOW, //"epoll_create",	/* 254*/
						ACTION_ALLOW, //"epoll_ctl",		 /*255*/
						ACTION_ALLOW, //"epoll_wait",		 /*256*/
						ACTION_ALLOW, //"remap_file_pages",	/* 257*/
						ACTION_ALLOW, //"set_tid_address",	/* 258*/
						ACTION_ALLOW, //"timer_create",	/* 259*/
						ACTION_ALLOW, //"timer_settime", /* 260	(__NR_timer_create+1)*/
						ACTION_ALLOW, //"timer_gettime",	/* 261 (__NR_timer_create+2)*/
						ACTION_ALLOW, //"timer_getoverrun",	/* 262(__NR_timer_create+3)*/
						ACTION_ALLOW, //"timer_delete",	/* 263(__NR_timer_create+4) */
						ACTION_ALLOW, //"clock_settime",	/* 264(__NR_timer_create+5) */
						ACTION_ALLOW, //"clock_gettime",	/* 265(__NR_timer_create+6) */
						ACTION_ALLOW, //"clock_getres",	/* 266(__NR_timer_create+7) */
						ACTION_ALLOW, //"clock_nanosleep",	/* 267(__NR_timer_create+8) */
						ACTION_ALLOW, //"statfs64",		 /*268*/
						ACTION_ALLOW, //"fstatfs64",		 /*269*/
						ACTION_ALLOW, //"tgkill",		 /*270*/
						ACTION_ALLOW, //"utimes",		 /*271*/
						ACTION_ALLOW, //"fadvise64_64",	/*272*/
						ACTION_ALLOW, //"vserver",		 /*273*/
						ACTION_ALLOW, //"mbind",		 /*274*/
						ACTION_ALLOW, //"get_mempolicy",	/* 275*/
						ACTION_ALLOW, //"set_mempolicy",	/* 276*/
						ACTION_ALLOW, //"mq_open ",		 /*277*/
						ACTION_ALLOW, //"mq_unlink",		 /* 278 (__NR_mq_open+1)*/
						ACTION_ALLOW, //"mq_timedsend",	/* 279 (__NR_mq_open+2)*/
						ACTION_ALLOW, //"mq_timedreceive", 	/*280 (__NR_mq_open+3)*/
						ACTION_ALLOW, //"mq_notify",		 /*281 (__NR_mq_open+4)*/
						ACTION_ALLOW, //"mq_getsetattr",	/*282 (__NR_mq_open+5)*/
						ACTION_ALLOW, //"kexec_load",		 /*283*/
						ACTION_ALLOW, //"waitid",		 /*284*/
					 	ACTION_ALLOW, //"sys_setaltroot",	/* 285 */
						ACTION_ALLOW, //"add_key",		 /*286*/
						ACTION_ALLOW, //"request_key",	/* 287*/
						ACTION_ALLOW, //"keyctl",		 /*288*/
						ACTION_ALLOW, //"ioprio_set",		 /*289*/
						ACTION_ALLOW, //"ioprio_get",		 /*290*/
						ACTION_ALLOW, //"inotify_init",	/*291*/
						ACTION_ALLOW, //"inotify_add_watch",	/*292*/
						ACTION_ALLOW, //"inotify_rm_watch",	/*293*/
						ACTION_ALLOW, //"migrate_pages",	/*294*/
						ACTION_ALLOW, //"openat",		 /*295*/
						ACTION_ALLOW, //"mkdirat",		 /*296*/
						ACTION_ALLOW, //"mknodat",		 /*297*/
						ACTION_ALLOW, //"fchownat",		 /*298*/
						ACTION_ALLOW, //"futimesat",		 /*299*/
						ACTION_ALLOW, //"fstatat64",		 /*300*/
						ACTION_ALLOW, //"unlinkat",		 /*301*/
						ACTION_ALLOW, //"renameat",		 /*302*/
						ACTION_ALLOW, //"linkat",		 /*303*/
						ACTION_ALLOW, //"symlinkat",		 /*304*/
						ACTION_ALLOW, //"readlinkat",		 /*305*/
						ACTION_ALLOW, //"fchmodat",		 /*306*/
						ACTION_ALLOW, //"faccessat",		 /*307*/
						ACTION_ALLOW, //"pselect6",		 /*308*/
						ACTION_ALLOW, //"ppoll",		 /*309*/
						ACTION_ALLOW, //"unshare",		 /*310*/
						ACTION_ALLOW, //"set_robust_list",	/*311*/
						ACTION_ALLOW, //"get_robust_list",	/*312*/
						ACTION_ALLOW, //"splice",		 /*313*/
						ACTION_ALLOW, //"sync_file_range",	/*314*/
						ACTION_ALLOW, //"tee",		 /*315*/
						ACTION_ALLOW, //"vmsplice",		 /*316*/
						ACTION_ALLOW, //"move_pages",		 /*317*/
						ACTION_ALLOW, //"getcpu",		 /*318*/
						ACTION_ALLOW, //"epoll_pwait",	/*319*/
						ACTION_ALLOW, //"utimensat",		 /*320*/
						ACTION_ALLOW, //"signalfd",		 /*321*/
						ACTION_ALLOW, //"timerfd_create",	/*322*/
						ACTION_ALLOW, //"eventfd",		 /*323*/
						ACTION_ALLOW, //"fallocate",		 /*324*/
						ACTION_ALLOW, //"timerfd_settime",	/*325*/
						ACTION_ALLOW, //"timerfd_gettime",	/*326*/
						ACTION_ALLOW, //"signalfd4",		 /*327*/
						ACTION_ALLOW, //"eventfd2",		 /*328*/
						ACTION_ALLOW, //"epoll_create1",	/*329*/
						ACTION_ALLOW, //"dup3",		 /*330*/
						ACTION_ALLOW, //"pipe2",		 /*331*/
						ACTION_ALLOW, //"inotify_init1",	/*332*/
						ACTION_ALLOW, //"preadv",		 /*333*/
						ACTION_ALLOW, //"pwritev",		 /*334*/
						ACTION_ALLOW, //"rt_tgsigqueueinfo",	/*335*/
						ACTION_ALLOW, //"perf_event_open",	/*336*/
						ACTION_ALLOW, //"recvmmsg",		 /*337*/
						ACTION_ALLOW, //"fanotify_init",	/*338*/
						ACTION_ALLOW, //"fanotify_mark",	/*339*/
						ACTION_ALLOW, //"prlimit64",		 /*340*/
						ACTION_ALLOW, //"name_to_handle_at",	/*341*/
						ACTION_ALLOW, //"open_by_handle_at",  /*342*/
						ACTION_ALLOW, //"clock_adjtime",	/*343*/
						ACTION_ALLOW, //"syncfs",           /*  344*/
						ACTION_ALLOW, //"sendmmsg",		 /*345*/
						ACTION_ALLOW, //"setns",		 /*346*/
						ACTION_ALLOW, //"socket",		/*348*/
						ACTION_ALLOW, //"bind",		/*349*/
						ACTION_ALLOW, //"connect",		/*350*/
						ACTION_ALLOW, //"listen",		/*351*/
						ACTION_ALLOW, //"accept",		/*352*/
						ACTION_ALLOW, //"getsockname",		/*353*/
						ACTION_ALLOW, //"getpeername",		/*354*/
						ACTION_ALLOW, //"socketpair",		/*355*/
						ACTION_ALLOW, //"send",		/*356*/
						ACTION_ALLOW, //"recv",		/*357*/
						ACTION_ALLOW, //"sendto",		/*358*/
						ACTION_ALLOW, //"recvfrom",		/*359*/
						ACTION_ALLOW, //"shutdown",		/*360*/
						ACTION_ALLOW, //"setsockopt",		/*361*/
						ACTION_ALLOW, //"getsockopt",		/*362*/
						ACTION_ALLOW, //"sendmsg",		/*363*/
						ACTION_ALLOW, //"recvmsg"		/*364*/

};

char *DFM_eventnames[]={
	"restart_syscall",   /*   0*/
	"exit",		 /*  1*/
	"fork",		 /*  2*/
	"read",		 /*  3*/
	"write",		 /*  4*/
	"open",		 /*  5*/
	"close",		 /*  6*/
	"waitpid",		 /*  7*/
	"creat",		 /*  8*/
	"link",		 /*  9*/
	"unlink",		 /* 10*/
	"execve",		 /* 11*/
	"chdir",		 /* 12*/
	"time",		 /* 13*/
	"mknod",		 /* 14*/
	"chmod",		 /* 15*/
	"lchown",		 /* 16*/
	"break",		 /* 17*/
	"oldstat",		 /* 18*/
	"lseek",		 /* 19*/
	"getpid",		 /* 20*/
	"mount",		 /* 21*/
	"umount",		 /* 22*/
	"setuid",		 /* 23*/
	"getuid",		 /* 24*/
	"stime",		 /* 25*/
	"ptrace",		 /* 26*/
	"alarm",		 /* 27*/
	"oldfstat",		 /* 28*/
	"pause",		 /* 29*/
	"utime",		 /* 30*/
	"stty",		 /* 31*/
	"gtty",		 /* 32*/
	"access",		 /* 33*/
	"nice",		 /* 34*/
	"ftime",		 /* 35*/
	"sync",		 /* 36*/
	"kill",		 /* 37*/
	"rename",		 /* 38*/
	"mkdir",		 /* 39*/
	"rmdir",		 /* 40*/
	"dup",		 /* 41*/
	"pipe",		 /* 42*/
	"times",		 /* 43*/
	"prof",		 /* 44*/
	"brk",		 /* 45*/
	"setgid",		 /* 46*/
	"getgid",		 /* 47*/
	"signal",		 /* 48*/
	"geteuid",		 /* 49*/
	"getegid",		 /* 50*/
	"acct",		 /* 51*/
	"umount2",		 /* 52*/
	"lock",		 /* 53*/
	"ioctl",		 /* 54*/
	"fcntl",		 /* 55*/
	"mpx",		 /* 56*/
	"setpgid",		 /* 57*/
	"ulimit",		 /* 58*/
	"oldolduname",	/* 59*/
	"umask",		 /* 60*/
	"chroot",		 /* 61*/
	"ustat",		 /* 62*/
	"dup2",		 /* 63*/
	"getppid",		 /* 64*/
	"getpgrp",		 /* 65*/
	"setsid",		 /* 66*/
	"sigaction",		 /* 67*/
	"sgetmask",		 /* 68*/
	"ssetmask",		 /* 69*/
	"setreuid",		 /* 70*/
	"setregid",		 /* 71*/
	"sigsuspend",		 /* 72*/
	"sigpending",		 /* 73*/
	"sethostname",	 /* 74*/
	"setrlimit",		 /* 75*/
	"getrlimit",		 /* 76 */
	"getrusage",		 /* 77*/
	"gettimeofday",	 /* 78*/
	"settimeofday",	 /* 79*/
	"getgroups",		 /* 80*/
	"setgroups",		 /* 81*/
	"select",		 /* 82*/
	"symlink",		 /* 83*/
	"oldlstat",		 /* 84*/
	"readlink",		 /* 85*/
	"uselib",		 /* 86*/
	"swapon",		 /* 87*/
	"reboot",		 /* 88*/
	"readdir",		 /* 89*/
	"mmap",		 /* 90*/
	"munmap",		 /* 91*/
	"truncate",		 /* 92*/
	"ftruncate",		 /* 93*/
	"fchmod",		 /* 94*/
	"fchown",		 /* 95*/
	"getpriority",	 /*96*/
	"setpriority",	 /*97*/
	"profil",		 /* 98*/
	"statfs",		 /* 99*/
	"fstatfs",		 /*100*/
	"ioperm",		 /*101*/
	"socketcall",		 /*102*/
	"syslog",		 /*103*/
	"setitimer",		 /*104*/
	"getitimer",		 /*105*/
	"stat",		 /*106*/
	"lstat",		 /*107*/
	"fstat",		 /*108*/
	"olduname",		 /*109*/
	"iopl",		 /*110*/
	"vhangup",		 /*111*/
	"idle",		 /*112*/
	"vm86old",		 /*113*/
	"wait4",		 /*114*/
	"swapoff",		 /*115*/
	"sysinfo",		 /*116*/
	"ipc",		 /*117*/
	"fsync",		 /*118*/
	"sigreturn",		 /*119*/
	"clone",		 /*120*/
	"setdomainname",	/*121*/
	"uname",		 /*122*/
	"modify_ldt",		 /*123*/
	"adjtimex",		 /*124*/
	"mprotect",		 /*125*/
	"sigprocmask",	/* 126*/
	"create_module", /*	127*/
	"init_module",	/* 128*/
	"delete_module",	/* 129*/
	"get_kernel_syms",	/* 130*/
	"quotactl",		 /*131*/
	"getpgid",		 /*132*/
	"fchdir",		 /*133*/
	"bdflush",		 /*134*/
	"sysfs",		 /* 135*/
	"personality",	/* 136*/
	"afs_syscall",	/* 137 */
	"setfsuid",		 /*138*/
	"setfsgid",		 /*139*/
	"_llseek",		 /*140*/
	"getdents",		 /*141*/
	"_newselect",		 /*142*/
	"flock",		 /*143*/
	"msync",		 /*144*/
	"readv",		 /*145*/
	"writev",		 /*146*/
	"getsid",		 /*147*/
	"fdatasync",		 /*148*/
	"_sysctl",		 /*149*/
	"mlock",		 /*150*/
	"munlock",		 /*151*/
	"mlockall",		 /*152*/
	"munlockall",		 /*153*/
	"sched_setparam",		 /*154*/
	"sched_getparam",		 /*155*/
	"sched_setscheduler",		 /*156*/
	"sched_getscheduler",		 /*157*/
	"sched_yield",		 /*158*/
	"sched_get_priority_max",	/* 159*/
	"sched_get_priority_min",	/* 160*/
	"sched_rr_get_interval",	/*161*/
	"nanosleep",		 /*162*/
	"mremap",		 /*163*/
	"setresuid",		 /*164*/
	"getresuid",		 /*165*/
	"vm86",		 /*166*/
	"query_module",	/*167*/
	"poll",		 /*168*/
	"nfsservctl",		 /*169*/
	"setresgid",		 /*170*/
	"getresgid",		 /*171*/
	"prctl",            /*  172*/
	"rt_sigreturn",	/* 173*/
	"rt_sigaction",	 /* 174*/
	"rt_sigprocmask",	/* 175*/
	"rt_sigpending",	/* 176*/
	"rt_sigtimedwait",	/* 177*/
	"rt_sigqueueinfo",	/* 178*/
	"rt_sigsuspend",	/* 179*/
	"pread64",		 /*180*/
	"pwrite64",		 /*181*/
	"chown",		 /*182*/
	"getcwd",		 /*183*/
	"capget",		 /*184*/
	"capset",		 /*185*/
	"sigaltstack",	/*186*/
	"sendfile",		 /*187*/
	"getpmsg",		 /*188*/
	"putpmsg",		 /*189*/
	"vfork",		 /*190*/
	"ugetrlimit",		 /*191*/
	"mmap2",		 /*192*/
	"truncate64",		 /*193*/
	"ftruncate64",	/* 194*/
	"stat64",		 /*195*/
	"lstat64",		 /*196*/
	"fstat64",		 /*197*/
	"lchown32",		 /*198*/
	"getuid32",		 /*199*/
	"getgid32",		 /*200*/
	"geteuid32",		 /*201*/
	"getegid32",		 /*202*/
	"setreuid32",		 /*203*/
	"setregid32",		 /*204*/
	"getgroups32",	/* 205*/
	"setgroups32", 	/* 206*/
	"fchown32",		 /*207*/
	"setresuid32",	/* 208*/
	"getresuid32",	/* 209*/
	"setresgid32",	/* 210*/
	"getresgid32",	/* 211*/
	"chown32",		 /*212*/
	"setuid32",		 /*213*/
	"setgid32",		 /*214*/
	"setfsuid32",		 /*215*/
	"setfsgid32",		 /*216*/
	"pivot_root",		 /*217*/
	"mincore",		 /*218*/
	"madvise",		 /*219*/
	"madvise1",		 /*219 EVERYTHING +1 DOWN HERE	*/
	"getdents64",		 /*220*/
	"fcntl64",		 /*221*/
	"unused",			/*222*/
	"unused",			/*223*/
	"gettid",		 /*224*/
	"readahead",		 /*225*/
	"setxattr",		 /*226*/
	"lsetxattr",		 /*227*/
	"fsetxattr",		 /*228*/
	"getxattr",		 /*229*/
	"lgetxattr",		 /*230*/
	"fgetxattr",		 /*231*/
	"listxattr",		 /*232*/
	"llistxattr",		 /*233*/
	"flistxattr",		 /*234*/
	"removexattr",	/* 235*/
	"lremovexattr",	/* 236*/
	"fremovexattr",	/* 237*/
	"tkill",		 /*238*/
	"sendfile64",		 /*239*/
	"futex",		 /*240*/
	"sched_setaffinity",	/* 241*/
	"sched_getaffinity",	/* 242*/
	"set_thread_area",	/* 243*/
	"get_thread_area",	/* 244*/
	"io_setup",		 /*245*/
	"io_destroy",		 /*246*/
	"io_getevents",	/* 247*/
	"io_submit",		 /*248*/
	"io_cancel",		 /*249*/
	"fadvise64",		 /*250*/
	"available",		/*251*/
	"exit_group",		 /*252*/
	"lookup_dcookie",	/* 253*/
	"epoll_create",	/* 254*/
	"epoll_ctl",		 /*255*/
	"epoll_wait",		 /*256*/
	"remap_file_pages",	/* 257*/
	"set_tid_address",	/* 258*/
	"timer_create",	/* 259*/
	"timer_settime", /* 260	(__NR_timer_create+1)*/
	"timer_gettime",	/* 261 (__NR_timer_create+2)*/
	"timer_getoverrun",	/* 262(__NR_timer_create+3)*/
	"timer_delete",	/* 263(__NR_timer_create+4) */
	"clock_settime",	/* 264(__NR_timer_create+5) */
	"clock_gettime",	/* 265(__NR_timer_create+6) */
	"clock_getres",	/* 266(__NR_timer_create+7) */
	"clock_nanosleep",	/* 267(__NR_timer_create+8) */
	"statfs64",		 /*268*/
	"fstatfs64",		 /*269*/
	"tgkill",		 /*270*/
	"utimes",		 /*271*/
	"fadvise64_64",	/*272*/
	"vserver",		 /*273*/
	"mbind",		 /*274*/
	"get_mempolicy",	/* 275*/
	"set_mempolicy",	/* 276*/
	"mq_open ",		 /*277*/
	"mq_unlink",		 /* 278 (__NR_mq_open+1)*/
	"mq_timedsend",	/* 279 (__NR_mq_open+2)*/
	"mq_timedreceive", 	/*280 (__NR_mq_open+3)*/
	"mq_notify",		 /*281 (__NR_mq_open+4)*/
	"mq_getsetattr",	/*282 (__NR_mq_open+5)*/
	"kexec_load",		 /*283*/
	"waitid",		 /*284*/
	"sys_setaltroot",	/* 285 */
	"add_key",		 /*286*/
	"request_key",	/* 287*/
	"keyctl",		 /*288*/
	"ioprio_set",		 /*289*/
	"ioprio_get",		 /*290*/
	"inotify_init",	/*291*/
	"inotify_add_watch",	/*292*/
	"inotify_rm_watch",	/*293*/
	"migrate_pages",	/*294*/
	"openat",		 /*295*/
	"mkdirat",		 /*296*/
	"mknodat",		 /*297*/
	"fchownat",		 /*298*/
	"futimesat",		 /*299*/
	"fstatat64",		 /*300*/
	"unlinkat",		 /*301*/
	"renameat",		 /*302*/
	"linkat",		 /*303*/
	"symlinkat",		 /*304*/
	"readlinkat",		 /*305*/
	"fchmodat",		 /*306*/
	"faccessat",		 /*307*/
	"pselect6",		 /*308*/
	"ppoll",		 /*309*/
	"unshare",		 /*310*/
	"set_robust_list",	/*311*/
	"get_robust_list",	/*312*/
	"splice",		 /*313*/
	"sync_file_range",	/*314*/
	"tee",		 /*315*/
	"vmsplice",		 /*316*/
	"move_pages",		 /*317*/
	"getcpu",		 /*318*/
	"epoll_pwait",	/*319*/
	"utimensat",		 /*320*/
	"signalfd",		 /*321*/
	"timerfd_create",	/*322*/
	"eventfd",		 /*323*/
	"fallocate",		 /*324*/
	"timerfd_settime",	/*325*/
	"timerfd_gettime",	/*326*/
	"signalfd4",		 /*327*/
	"eventfd2",		 /*328*/
	"epoll_create1",	/*329*/
	"dup3",		 /*330*/
	"pipe2",		 /*331*/
	"inotify_init1",	/*332*/
	"preadv",		 /*333*/
	"pwritev",		 /*334*/
	"rt_tgsigqueueinfo",	/*335*/
	"perf_event_open",	/*336*/
	"recvmmsg",		 /*337*/
	"fanotify_init",	/*338*/
	"fanotify_mark",	/*339*/
	"prlimit64",		 /*340*/
	"name_to_handle_at",	/*341*/
	"open_by_handle_at",  /*342*/
	"clock_adjtime",	/*343*/
	"syncfs",           /*  344*/
	"sendmmsg",		 /*345*/
	"setns",		 /*346*/
	"socket",		/*348*/
	"bind",		/*349*/
	"connect",		/*350*/
	"listen",		/*351*/
	"accept",		/*352*/
	"getsockname",		/*353*/
	"getpeername",		/*354*/
	"socketpair",		/*355*/
	"send",		/*356*/
	"recv",		/*357*/
	"sendto",		/*358*/
	"recvfrom",		/*359*/
	"shutdown",		/*360*/
	"setsockopt",		/*361*/
	"getsockopt",		/*362*/
	"sendmsg",		/*363*/
	"recvmsg"		/*364*/

};
