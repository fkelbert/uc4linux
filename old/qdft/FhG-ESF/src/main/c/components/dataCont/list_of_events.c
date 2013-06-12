/*
 * list_of_events.c
 *  Created on: 19/lug/2010
 *      Author: cornelius moucha, lovat
 */

#include "list_of_events.h"
#include "log_list_of_events_c.h"

#define IMPLEMENTED_SYSCALLS 309
GHashTable *syscallTable;
// comparison for 1.000.000 lookups: 2.099s (old way) vs. 0.177s (including preparation of hashtable)

int syscall_to_int (char* syscall)
{
  if (syscall==NULL) return -1;
  int i=0;
  while (DFM_eventnames[i]!=NULL) {
    if(strcmp(DFM_eventnames[i], syscall)==0) {
      return i;
    }
    i++;
  }
  return -1;  //not found
}

unsigned int initSyscallTable()
{
  syscallTable=g_hash_table_new(g_str_hash, g_str_equal);
  if(!syscallTable) {log_error("Error initializing syscall hashtable\n"); return R_ERROR;}

  int a;
  for(a=0;a<IMPLEMENTED_SYSCALLS; a++)
  {
    unsigned int *b=(unsigned int*)mem_alloc(sizeof(int));
    *b=a;
    g_hash_table_insert(syscallTable, DFM_eventnames[a], b);
  }
}

int syscallToInt(const unsigned char *syscall)
{
  if(syscall==NULL) return -1;
  int *x=(int*)g_hash_table_lookup(syscallTable, syscall);
  return x!=NULL?*x:-1;
}

char *DFM_eventnames[]={
    "syscall",      /* 0 = syscall */
    "exit",     /* 1 = exit */
    "fork",     /* 2 = fork */
    "read",     /* 3 = read */
    "write",      /* 4 = write */
    "open",     /* 5 = open */
    "close",      /* 6 = close */
    "wait4",      /* 7 = wait4 */
    "compat_43_ocreat", /* 8 = compat_43 ocreat */
    "link",     /* 9 = link */
    "unlink",     /* 10 = unlink */
    "#11 (obsolete execv)",   /* 11 = obsolete execv */
    "chdir",      /* 12 = chdir */
    "fchdir",     /* 13 = fchdir */
    "mknod",      /* 14 = mknod */
    "chmod",      /* 15 = chmod */
    "chown",      /* 16 = chown */
    "break",      /* 17 = break */
    "compat_25_ogetfsstat", /* 18 = compat_25 ogetfsstat */
    "compat_43_olseek", /* 19 = compat_43 olseek */
    "getpid",     /* 20 = getpid */
    "mount",      /* 21 = mount */
    "unmount",      /* 22 = unmount */
    "setuid",     /* 23 = setuid */
    "getuid",     /* 24 = getuid */
    "geteuid",      /* 25 = geteuid */
  #ifdef PTRACE
    "ptrace",     /* 26 = ptrace */
  #else
    "#26 (unimplemented ptrace)",   /* 26 = unimplemented ptrace */
  #endif
    "recvmsg",      /* 27 = recvmsg */
    "sendmsg",      /* 28 = sendmsg */
    "recvfrom",     /* 29 = recvfrom */
    "accept",     /* 30 = accept */
    "getpeername",      /* 31 = getpeername */
    "getsockname",      /* 32 = getsockname */
    "access",     /* 33 = access */
    "chflags",      /* 34 = chflags */
    "fchflags",     /* 35 = fchflags */
    "sync",     /* 36 = sync */
    "kill",     /* 37 = kill */
    "compat_43_stat43", /* 38 = compat_43 stat43 */
    "getppid",      /* 39 = getppid */
    "compat_43_lstat43",  /* 40 = compat_43 lstat43 */
    "dup",      /* 41 = dup */
    "opipe",      /* 42 = opipe */
    "getegid",      /* 43 = getegid */
    "profil",     /* 44 = profil */
  #ifdef KTRACE
    "ktrace",     /* 45 = ktrace */
  #else
    "#45 (unimplemented ktrace)",   /* 45 = unimplemented ktrace */
  #endif
    "sigaction",      /* 46 = sigaction */
    "getgid",     /* 47 = getgid */
    "sigprocmask",      /* 48 = sigprocmask */
    "getlogin",     /* 49 = getlogin */
    "setlogin",     /* 50 = setlogin */
  #ifdef ACCOUNTING
    "acct",     /* 51 = acct */
  #else
    "#51 (unimplemented acct)",   /* 51 = unimplemented acct */
  #endif
    "sigpending",     /* 52 = sigpending */
    "osigaltstack",     /* 53 = osigaltstack */
    "ioctl",      /* 54 = ioctl */
    "reboot",     /* 55 = reboot */
    "revoke",     /* 56 = revoke */
    "symlink",      /* 57 = symlink */
    "readlink",     /* 58 = readlink */
    "execve",     /* 59 = execve */
    "umask",      /* 60 = umask */
    "chroot",     /* 61 = chroot */
    "compat_43_fstat43",  /* 62 = compat_43 fstat43 */
    "compat_43_ogetkerninfo", /* 63 = compat_43 ogetkerninfo */
    "compat_43_ogetpagesize", /* 64 = compat_43 ogetpagesize */
    "compat_25_omsync", /* 65 = compat_25 omsync */
    "vfork",      /* 66 = vfork */
    "#67 (obsolete vread)",   /* 67 = obsolete vread */
    "#68 (obsolete vwrite)",    /* 68 = obsolete vwrite */
    "sbrk",     /* 69 = sbrk */
    "sstk",     /* 70 = sstk */
    "compat_43_ommap",  /* 71 = compat_43 ommap */
    "#72 (obsolete vadvise)",   /* 72 = obsolete vadvise */
    "munmap",     /* 73 = munmap */
    "mprotect",     /* 74 = mprotect */
    "madvise",      /* 75 = madvise */
    "#76 (obsolete vhangup)",   /* 76 = obsolete vhangup */
    "#77 (obsolete vlimit)",    /* 77 = obsolete vlimit */
    "mincore",      /* 78 = mincore */
    "getgroups",      /* 79 = getgroups */
    "setgroups",      /* 80 = setgroups */
    "getpgrp",      /* 81 = getpgrp */
    "setpgid",      /* 82 = setpgid */
    "setitimer",      /* 83 = setitimer */
    "compat_43_owait",  /* 84 = compat_43 owait */
    "compat_25_swapon", /* 85 = compat_25 swapon */
    "getitimer",      /* 86 = getitimer */
    "compat_43_ogethostname", /* 87 = compat_43 ogethostname */
    "compat_43_osethostname", /* 88 = compat_43 osethostname */
    "compat_43_ogetdtablesize", /* 89 = compat_43 ogetdtablesize */
    "dup2",     /* 90 = dup2 */
    "#91 (unimplemented getdopt)",    /* 91 = unimplemented getdopt */
    "fcntl",      /* 92 = fcntl */
    "select",     /* 93 = select */
    "#94 (unimplemented setdopt)",    /* 94 = unimplemented setdopt */
    "fsync",      /* 95 = fsync */
    "setpriority",      /* 96 = setpriority */
    "socket",     /* 97 = socket */
    "connect",      /* 98 = connect */
    "compat_43_oaccept",  /* 99 = compat_43 oaccept */
    "getpriority",      /* 100 = getpriority */
    "compat_43_osend",  /* 101 = compat_43 osend */
    "compat_43_orecv",  /* 102 = compat_43 orecv */
    "sigreturn",      /* 103 = sigreturn */
    "bind",     /* 104 = bind */
    "setsockopt",     /* 105 = setsockopt */
    "listen",     /* 106 = listen */
    "#107 (obsolete vtimes)",   /* 107 = obsolete vtimes */
    "compat_43_osigvec",  /* 108 = compat_43 osigvec */
    "compat_43_osigblock",  /* 109 = compat_43 osigblock */
    "compat_43_osigsetmask",  /* 110 = compat_43 osigsetmask */
    "sigsuspend",     /* 111 = sigsuspend */
    "compat_43_osigstack",  /* 112 = compat_43 osigstack */
    "compat_43_orecvmsg", /* 113 = compat_43 orecvmsg */
    "compat_43_osendmsg", /* 114 = compat_43 osendmsg */
    "#115 (obsolete vtrace)",   /* 115 = obsolete vtrace */
    "gettimeofday",     /* 116 = gettimeofday */
    "getrusage",      /* 117 = getrusage */
    "getsockopt",     /* 118 = getsockopt */
    "#119 (obsolete resuba)",   /* 119 = obsolete resuba */
    "readv",      /* 120 = readv */
    "writev",     /* 121 = writev */
    "settimeofday",     /* 122 = settimeofday */
    "fchown",     /* 123 = fchown */
    "fchmod",     /* 124 = fchmod */
    "compat_43_orecvfrom",  /* 125 = compat_43 orecvfrom */
    "setreuid",     /* 126 = setreuid */
    "setregid",     /* 127 = setregid */
    "rename",     /* 128 = rename */
    "compat_43_otruncate",  /* 129 = compat_43 otruncate */
    "compat_43_oftruncate", /* 130 = compat_43 oftruncate */
    "flock",      /* 131 = flock */
    "mkfifo",     /* 132 = mkfifo */
    "sendto",     /* 133 = sendto */
    "shutdown",     /* 134 = shutdown */
    "socketpair",     /* 135 = socketpair */
    "mkdir",      /* 136 = mkdir */
    "rmdir",      /* 137 = rmdir */
    "utimes",     /* 138 = utimes */
    "#139 (obsolete 4.2 sigreturn)",    /* 139 = obsolete 4.2 sigreturn */
    "adjtime",      /* 140 = adjtime */
    "compat_43_ogetpeername", /* 141 = compat_43 ogetpeername */
    "compat_43_ogethostid", /* 142 = compat_43 ogethostid */
    "compat_43_osethostid", /* 143 = compat_43 osethostid */
    "compat_43_ogetrlimit", /* 144 = compat_43 ogetrlimit */
    "compat_43_osetrlimit", /* 145 = compat_43 osetrlimit */
    "compat_43_okillpg",  /* 146 = compat_43 okillpg */
    "setsid",     /* 147 = setsid */
    "quotactl",     /* 148 = quotactl */
    "compat_43_oquota", /* 149 = compat_43 oquota */
    "compat_43_ogetsockname", /* 150 = compat_43 ogetsockname */
    "#151 (unimplemented)",   /* 151 = unimplemented */
    "#152 (unimplemented)",   /* 152 = unimplemented */
    "#153 (unimplemented)",   /* 153 = unimplemented */
    "#154 (unimplemented)",   /* 154 = unimplemented */
  #if defined(NFSCLIENT) || defined(NFSSERVER)
    "nfssvc",     /* 155 = nfssvc */
  #else
    "#155 (unimplemented)",   /* 155 = unimplemented */
  #endif
    "compat_43_ogetdirentries", /* 156 = compat_43 ogetdirentries */
    "compat_25_ostatfs",  /* 157 = compat_25 ostatfs */
    "compat_25_ostatfs",  /* 158 = compat_25 ostatfs */
    "#159 (unimplemented)",   /* 159 = unimplemented */
    "#160 (unimplemented)",   /* 160 = unimplemented */
    "getfh",      /* 161 = getfh */
    "compat_09_ogetdomainname", /* 162 = compat_09 ogetdomainname */
    "compat_09_osetdomainname", /* 163 = compat_09 osetdomainname */
    "#164 (unimplemented ouname)",    /* 164 = unimplemented ouname */
    "sysarch",      /* 165 = sysarch */
    "#166 (unimplemented)",   /* 166 = unimplemented */
    "#167 (unimplemented)",   /* 167 = unimplemented */
    "#168 (unimplemented)",   /* 168 = unimplemented */
  #if defined(SYSVSEM) && !defined(__LP64__)
    "compat_10_osemsys",  /* 169 = compat_10 osemsys */
  #else
    "#169 (unimplemented 1.0 semsys)",    /* 169 = unimplemented 1.0 semsys */
  #endif
  #if defined(SYSVMSG) && !defined(__LP64__)
    "compat_10_omsgsys",  /* 170 = compat_10 omsgsys */
  #else
    "#170 (unimplemented 1.0 msgsys)",    /* 170 = unimplemented 1.0 msgsys */
  #endif
  #if defined(SYSVSHM) && !defined(__LP64__)
    "compat_10_oshmsys",  /* 171 = compat_10 oshmsys */
  #else
    "#171 (unimplemented 1.0 shmsys)",    /* 171 = unimplemented 1.0 shmsys */
  #endif
    "#172 (unimplemented)",   /* 172 = unimplemented */
    "pread",      /* 173 = pread */
    "pwrite",     /* 174 = pwrite */
    "#175 (unimplemented ntp_gettime)",   /* 175 = unimplemented ntp_gettime */
    "#176 (unimplemented ntp_adjtime)",   /* 176 = unimplemented ntp_adjtime */
    "#177 (unimplemented)",   /* 177 = unimplemented */
    "#178 (unimplemented)",   /* 178 = unimplemented */
    "#179 (unimplemented)",   /* 179 = unimplemented */
    "#180 (unimplemented)",   /* 180 = unimplemented */
    "setgid",     /* 181 = setgid */
    "setegid",      /* 182 = setegid */
    "seteuid",      /* 183 = seteuid */
  #ifdef LFS
    "lfs_bmapv",      /* 184 = lfs_bmapv */
    "lfs_markv",      /* 185 = lfs_markv */
    "lfs_segclean",     /* 186 = lfs_segclean */
    "lfs_segwait",      /* 187 = lfs_segwait */
  #else
    "#184 (unimplemented)",   /* 184 = unimplemented */
    "#185 (unimplemented)",   /* 185 = unimplemented */
    "#186 (unimplemented)",   /* 186 = unimplemented */
    "#187 (unimplemented)",   /* 187 = unimplemented */
  #endif
    "compat_35_stat35", /* 188 = compat_35 stat35 */
    "compat_35_fstat35",  /* 189 = compat_35 fstat35 */
    "compat_35_lstat35",  /* 190 = compat_35 lstat35 */
    "pathconf",     /* 191 = pathconf */
    "fpathconf",      /* 192 = fpathconf */
    "swapctl",      /* 193 = swapctl */
    "getrlimit",      /* 194 = getrlimit */
    "setrlimit",      /* 195 = setrlimit */
    "getdirentries",      /* 196 = getdirentries */
    "mmap",     /* 197 = mmap */
    "__syscall",      /* 198 = __syscall */
    "lseek",      /* 199 = lseek */
    "truncate",     /* 200 = truncate */
    "ftruncate",      /* 201 = ftruncate */
    "__sysctl",     /* 202 = __sysctl */
    "mlock",      /* 203 = mlock */
    "munlock",      /* 204 = munlock */
    "#205 (unimplemented sys_undelete)",    /* 205 = unimplemented sys_undelete */
    "futimes",      /* 206 = futimes */
    "getpgid",      /* 207 = getpgid */
    "nnpfspioctl",      /* 208 = nnpfspioctl */
    "#209 (unimplemented)",   /* 209 = unimplemented */
  #ifdef LKM
    "lkmnosys",     /* 210 = lkmnosys */
    "lkmnosys",     /* 211 = lkmnosys */
    "lkmnosys",     /* 212 = lkmnosys */
    "lkmnosys",     /* 213 = lkmnosys */
    "lkmnosys",     /* 214 = lkmnosys */
    "lkmnosys",     /* 215 = lkmnosys */
    "lkmnosys",     /* 216 = lkmnosys */
    "lkmnosys",     /* 217 = lkmnosys */
    "lkmnosys",     /* 218 = lkmnosys */
    "lkmnosys",     /* 219 = lkmnosys */
  #else /* !LKM */
    "#210 (unimplemented)",   /* 210 = unimplemented */
    "#211 (unimplemented)",   /* 211 = unimplemented */
    "#212 (unimplemented)",   /* 212 = unimplemented */
    "#213 (unimplemented)",   /* 213 = unimplemented */
    "#214 (unimplemented)",   /* 214 = unimplemented */
    "#215 (unimplemented)",   /* 215 = unimplemented */
    "#216 (unimplemented)",   /* 216 = unimplemented */
    "#217 (unimplemented)",   /* 217 = unimplemented */
    "#218 (unimplemented)",   /* 218 = unimplemented */
    "#219 (unimplemented)",   /* 219 = unimplemented */
  #endif  /* !LKM */
  #ifdef SYSVSEM
    "compat_23_semctl23", /* 220 = compat_23 semctl23 */
    "semget",     /* 221 = semget */
    "compat_35_semop",  /* 222 = compat_35 semop */
    "#223 (obsolete sys_semconfig)",    /* 223 = obsolete sys_semconfig */
  #else
    "#220 (unimplemented semctl)",    /* 220 = unimplemented semctl */
    "#221 (unimplemented semget)",    /* 221 = unimplemented semget */
    "#222 (unimplemented semop)",   /* 222 = unimplemented semop */
    "#223 (unimplemented semconfig)",   /* 223 = unimplemented semconfig */
  #endif
  #ifdef SYSVMSG
    "compat_23_msgctl23", /* 224 = compat_23 msgctl23 */
    "msgget",     /* 225 = msgget */
    "msgsnd",     /* 226 = msgsnd */
    "msgrcv",     /* 227 = msgrcv */
  #else
    "#224 (unimplemented msgctl)",    /* 224 = unimplemented msgctl */
    "#225 (unimplemented msgget)",    /* 225 = unimplemented msgget */
    "#226 (unimplemented msgsnd)",    /* 226 = unimplemented msgsnd */
    "#227 (unimplemented msgrcv)",    /* 227 = unimplemented msgrcv */
  #endif
  #ifdef SYSVSHM
    "shmat",      /* 228 = shmat */
    "compat_23_shmctl23", /* 229 = compat_23 shmctl23 */
    "shmdt",      /* 230 = shmdt */
    "compat_35_shmget", /* 231 = compat_35 shmget */
  #else
    "#228 (unimplemented shmat)",   /* 228 = unimplemented shmat */
    "#229 (unimplemented shmctl)",    /* 229 = unimplemented shmctl */
    "#230 (unimplemented shmdt)",   /* 230 = unimplemented shmdt */
    "#231 (unimplemented shmget)",    /* 231 = unimplemented shmget */
  #endif
    "clock_gettime",      /* 232 = clock_gettime */
    "clock_settime",      /* 233 = clock_settime */
    "clock_getres",     /* 234 = clock_getres */
    "#235 (unimplemented timer_create)",    /* 235 = unimplemented timer_create */
    "#236 (unimplemented timer_delete)",    /* 236 = unimplemented timer_delete */
    "#237 (unimplemented timer_settime)",   /* 237 = unimplemented timer_settime */
    "#238 (unimplemented timer_gettime)",   /* 238 = unimplemented timer_gettime */
    "#239 (unimplemented timer_getoverrun)",    /* 239 = unimplemented timer_getoverrun */
    "nanosleep",      /* 240 = nanosleep */
    "#241 (unimplemented)",   /* 241 = unimplemented */
    "#242 (unimplemented)",   /* 242 = unimplemented */
    "#243 (unimplemented)",   /* 243 = unimplemented */
    "#244 (unimplemented)",   /* 244 = unimplemented */
    "#245 (unimplemented)",   /* 245 = unimplemented */
    "#246 (unimplemented)",   /* 246 = unimplemented */
    "#247 (unimplemented)",   /* 247 = unimplemented */
    "#248 (unimplemented)",   /* 248 = unimplemented */
    "#249 (unimplemented)",   /* 249 = unimplemented */
    "minherit",     /* 250 = minherit */
    "rfork",      /* 251 = rfork */
    "poll",     /* 252 = poll */
    "issetugid",      /* 253 = issetugid */
    "lchown",     /* 254 = lchown */
    "getsid",     /* 255 = getsid */
    "msync",      /* 256 = msync */
  #ifdef SYSVSEM
    "compat_35_semctl35", /* 257 = compat_35 semctl35 */
  #else
    "#257 (unimplemented)",   /* 257 = unimplemented */
  #endif
  #ifdef SYSVSHM
    "compat_35_shmctl35", /* 258 = compat_35 shmctl35 */
  #else
    "#258 (unimplemented)",   /* 258 = unimplemented */
  #endif
  #ifdef SYSVMSG
    "compat_35_msgctl35", /* 259 = compat_35 msgctl35 */
  #else
    "#259 (unimplemented)",   /* 259 = unimplemented */
  #endif
    "compat_o43_getfsstat", /* 260 = compat_o43 getfsstat */
    "compat_o43_statfs",  /* 261 = compat_o43 statfs */
    "compat_o43_fstatfs", /* 262 = compat_o43 fstatfs */
    "pipe",     /* 263 = pipe */
    "fhopen",     /* 264 = fhopen */
    "compat_35_fhstat", /* 265 = compat_35 fhstat */
    "compat_o43_fhstatfs",  /* 266 = compat_o43 fhstatfs */
    "preadv",     /* 267 = preadv */
    "pwritev",      /* 268 = pwritev */
    "kqueue",     /* 269 = kqueue */
    "kevent",     /* 270 = kevent */
    "mlockall",     /* 271 = mlockall */
    "munlockall",     /* 272 = munlockall */
    "getpeereid",     /* 273 = getpeereid */
    "#274 (unimplemented sys_extattrctl)",    /* 274 = unimplemented sys_extattrctl */
    "#275 (unimplemented sys_extattr_set_file)",    /* 275 = unimplemented sys_extattr_set_file */
    "#276 (unimplemented sys_extattr_get_file)",    /* 276 = unimplemented sys_extattr_get_file */
    "#277 (unimplemented sys_extattr_delete_file)",   /* 277 = unimplemented sys_extattr_delete_file */
    "#278 (unimplemented sys_extattr_set_fd)",    /* 278 = unimplemented sys_extattr_set_fd */
    "#279 (unimplemented sys_extattr_get_fd)",    /* 279 = unimplemented sys_extattr_get_fd */
    "#280 (unimplemented sys_extattr_delete_fd)",   /* 280 = unimplemented sys_extattr_delete_fd */
    "getresuid",      /* 281 = getresuid */
    "setresuid",      /* 282 = setresuid */
    "getresgid",      /* 283 = getresgid */
    "setresgid",      /* 284 = setresgid */
    "#285 (obsolete sys_omquery)",    /* 285 = obsolete sys_omquery */
    "mquery",     /* 286 = mquery */
    "closefrom",      /* 287 = closefrom */
    "sigaltstack",      /* 288 = sigaltstack */
  #ifdef SYSVSHM
    "shmget",     /* 289 = shmget */
  #else
    "#289 (unimplemented shmget)",    /* 289 = unimplemented shmget */
  #endif
  #ifdef SYSVSEM
    "semop",      /* 290 = semop */
  #else
    "#290 (unimplemented semop)",   /* 290 = unimplemented semop */
  #endif
    "stat",     /* 291 = stat */
    "fstat",      /* 292 = fstat */
    "lstat",      /* 293 = lstat */
    "fhstat",     /* 294 = fhstat */
  #ifdef SYSVSEM
    "__semctl",     /* 295 = __semctl */
  #else
    "#295 (unimplemented)",   /* 295 = unimplemented */
  #endif
  #ifdef SYSVSHM
    "shmctl",     /* 296 = shmctl */
  #else
    "#296 (unimplemented)",   /* 296 = unimplemented */
  #endif
  #ifdef SYSVMSG
    "msgctl",     /* 297 = msgctl */
  #else
    "#297 (unimplemented)",   /* 297 = unimplemented */
  #endif
    "sched_yield",      /* 298 = sched_yield */
  #ifdef RTHREADS
    "getthrid",     /* 299 = getthrid */
    "thrsleep",     /* 300 = thrsleep */
    "thrwakeup",      /* 301 = thrwakeup */
    "threxit",      /* 302 = threxit */
    "thrsigdivert",     /* 303 = thrsigdivert */
  #else
    "#299 (unimplemented)",   /* 299 = unimplemented */
    "#300 (unimplemented)",   /* 300 = unimplemented */
    "#301 (unimplemented)",   /* 301 = unimplemented */
    "#302 (unimplemented)",   /* 302 = unimplemented */
    "#303 (unimplemented)",   /* 303 = unimplemented */
  #endif
    "__getcwd",     /* 304 = __getcwd */
    "adjfreq",      /* 305 = adjfreq */
    "getfsstat",      /* 306 = getfsstat */
    "statfs",     /* 307 = statfs */
    "fstatfs",      /* 308 = fstatfs */
    "fhstatfs",     /* 309 = fhstatfs */
  };


esfsyscall_t NotImpl    = {"esfNOTimplemented",  0, { }};
esfsyscall_t sysSyscall = {"syscall",            0, { } };
esfsyscall_t sysExit    = {"exit",               1, { {PARAM_INT, "status"} }};
esfsyscall_t sysFork    = {"fork",               0, { } };
esfsyscall_t sysRead    = {"read",               2, { {PARAM_INT, "fd"}, {PARAM_STRING, "buf"} /* {PARAM_INT, "nbytes"} */ }};
esfsyscall_t sysWrite   = {"write",              1, { {PARAM_INT, "fd"} /* {PARAM_STRING, "buf"}, {PARAM_INT "nbytes"}*/ }};
esfsyscall_t sysOpen    = {"open",               2, { {PARAM_STRING, "filename"}, {PARAM_INTSTR, "flags"} }};
esfsyscall_t sysClose   = {"close",              1, { {PARAM_INT, "fd"} }};
esfsyscall_t sysUnlink  = {"unlink",             1, { {PARAM_STRING, "filename"} }};
esfsyscall_t sysAccept  = {"accept",             1, { {PARAM_INT, "sockfd"} }};
esfsyscall_t sysKill    = {"kill",               2, { {PARAM_INT, "target"}, {PARAM_INT, "signal"} }};
esfsyscall_t sysDup     = {"dup",                1, { {PARAM_INT, "fd"} }};
esfsyscall_t sysExecve  = {"execve",             1, { {PARAM_STRING, "filename"} }}; //args: "const char *" "char *const *" "char *const *"
esfsyscall_t sysVfork   = {"vfork",              0, { } };
esfsyscall_t sysDup2    = {"dup2",               1, { {PARAM_INT, "fd"} }};          //args: "int" "int"
esfsyscall_t sysFcntl   = {"fcntl",              1, { {PARAM_INT, "fd"} }};          //args: "int" "int" "..."
esfsyscall_t sysSocket  = {"socket",             2, { {PARAM_INT, "domain"}, {PARAM_INT, "type"} }}; //args: "int" "int" "int"
esfsyscall_t sysReadv   = {"readv",              2, { {PARAM_INT, "fd"}, {PARAM_STRING, "buf"} /* {PARAM_INT, "nbytes"} */ }};
esfsyscall_t sysWritev  = {"writev",             1, { {PARAM_INT, "fd"} /* {PARAM_STRING, "buf"}, {PARAM_INT "nbytes"}*/ }};
esfsyscall_t sysSendTo  = {"sendto",             1, { {PARAM_INT, "fd"} /* {PARAM_STRING, "buf"}, {PARAM_INT "nbytes"}*/ }};
esfsyscall_t sysRename  = {"rename",             2, { {PARAM_STRING, "oldfilename"}, {PARAM_STRING, "newfilename"} }}; //args: "const char *" "const char *"
esfsyscall_t sysPread   = {"pread",              2, { {PARAM_INT, "fd"}, {PARAM_STRING, "buf"} /* {PARAM_INT, "nbytes"} */ }};
esfsyscall_t sysPwrite  = {"pwrite",             1, { {PARAM_INT, "fd"} /* {PARAM_STRING, "buf"}, {PARAM_INT "nbytes"}*/ }};
esfsyscall_t sysTruncate= {"truncate",           1, { {PARAM_STRING, "filename"} }}; //args: "const char *" "int" "off_t"
esfsyscall_t sysFtruncate={"ftruncate",          1, { {PARAM_INT, "fd"} }}; //args: "int" "int" "off_t"
esfsyscall_t sysPipe    = {"pipe",               1, { {PARAM_INT, "fds"} }}; // args: "int *" // NOT YET SUPPORTED!

//args: "void *" "size_t" "int" "int" "int" "long" "off_t" */
//    startaddr  length, prot, flags, fd,  offset,
esfsyscall_t sysMmap    = {"mmap",               6, { {PARAM_INT, "start"}, {PARAM_INT, "length"}, {PARAM_INTSTR, "prot"}, {PARAM_INT, "flags"},
                                                      {PARAM_INT, "fd"}, {PARAM_INT, "offset"} }};

esfsyscall_t* esfsyscalls[]=
{
  &sysSyscall,
  &sysExit,
  &sysFork,
  &sysRead,
  &sysWrite,
  &sysOpen,
  &sysClose,
  &NotImpl,       /* 7 = wait4 */
  &NotImpl,       /* 8 = compat_43 ocreat */
  &NotImpl,       /* 9 = link */
  &sysUnlink,     /* 10 = unlink */
  &NotImpl,       /* 11 = obsolete execv */
  &NotImpl,       /* 12 = chdir */
  &NotImpl,       /* 13 = fchdir */
  &NotImpl,       /* 14 = mknod */
  &NotImpl,       /* 15 = chmod */
  &NotImpl,       /* 16 = chown */
  &NotImpl,       /* 17 = break */
  &NotImpl,       /* 18 = compat_25 ogetfsstat */
  &NotImpl,       /* 19 = compat_43 olseek */
  &NotImpl,       /* 20 = getpid */
  &NotImpl,       /* 21 = mount */
  &NotImpl,       /* 22 = unmount */
  &NotImpl,       /* 23 = setuid */
  &NotImpl,       /* 24 = getuid */
  &NotImpl,       /* 25 = geteuid */
  &NotImpl,       /* 26 = ptrace or 26 = unimplemented ptrace */
  &sysRead,       /* 27 = recvmsg */
  &sysWrite,      /* 28 = sendmsg */
  &sysRead,       /* 29 = recvfrom */
  &sysAccept,     /* 30 = accept */
  &NotImpl,       /* 31 = getpeername */
  &NotImpl,       /* 32 = getsockname */
  &NotImpl,       /* 33 = access */
  &NotImpl,       /* 34 = chflags */
  &NotImpl,       /* 35 = fchflags */
  &NotImpl,       /* 36 = sync */
  &sysKill,       /* 37 = kill */
  &NotImpl,       /* 38 = compat_43 stat43 */
  &NotImpl,       /* 39 = getppid */
  &NotImpl,       /* 40 = compat_43 lstat43 */
  &sysDup,        /* 41 = dup */
  &NotImpl,       /* 42 = opipe */
  &NotImpl,       /* 43 = getegid */
  &NotImpl,       /* 44 = profil */
  &NotImpl,       /* 45 = ktrace or 45 = unimplemented ktrace */
  &NotImpl,       /* 46 = sigaction */
  &NotImpl,       /* 47 = getgid */
  &NotImpl,       /* 48 = sigprocmask */
  &NotImpl,       /* 49 = getlogin */
  &NotImpl,       /* 50 = setlogin */
  &NotImpl,       /* 51 = acct or 51 = unimplemented acct */
  &NotImpl,       /* 52 = sigpending */
  &NotImpl,       /* 53 = osigaltstack */
  &NotImpl,       /* 54 = ioctl */
  &NotImpl,       /* 55 = reboot */
  &NotImpl,       /* 56 = revoke */
  &NotImpl,       /* 57 = symlink */
  &NotImpl,       /* 58 = readlink */
  &sysExecve,     /* 59 = execve */
  &NotImpl,       /* 60 = umask */
  &NotImpl,       /* 61 = chroot */
  &NotImpl,       /* 62 = compat_43 fstat43 */
  &NotImpl,       /* 63 = compat_43 ogetkerninfo */
  &NotImpl,       /* 64 = compat_43 ogetpagesize */
  &NotImpl,       /* 65 = compat_25 omsync */
  &sysVfork,      /* 66 = vfork */
  &NotImpl,       /* 67 = obsolete vread */
  &NotImpl,       /* 68 = obsolete vwrite */
  &NotImpl,       /* 69 = sbrk */
  &NotImpl,       /* 70 = sstk */
  &NotImpl,       /* 71 = compat_43 ommap */
  &NotImpl,       /* 72 = obsolete vadvise */
  &NotImpl,       /* 73 = munmap */
  &NotImpl,       /* 74 = mprotect */
  &NotImpl,       /* 75 = madvise */
  &NotImpl,       /* 76 = obsolete vhangup */
  &NotImpl,       /* 77 = obsolete vlimit */
  &NotImpl,       /* 78 = mincore */
  &NotImpl,       /* 79 = getgroups */
  &NotImpl,       /* 80 = setgroups */
  &NotImpl,       /* 81 = getpgrp */
  &NotImpl,       /* 82 = setpgid */
  &NotImpl,       /* 83 = setitimer */
  &NotImpl,       /* 84 = compat_43 owait */
  &NotImpl,       /* 85 = compat_25 swapon */
  &NotImpl,       /* 86 = getitimer */
  &NotImpl,       /* 87 = compat_43 ogethostname */
  &NotImpl,       /* 88 = compat_43 osethostname */
  &NotImpl,       /* 89 = compat_43 ogetdtablesize */
  &sysDup2,       /* 90 = dup2 */
  &NotImpl,       /* 91 = unimplemented getdopt */
  &sysFcntl,      /* 92 = fcntl */
  &NotImpl,        /* 93 = select */
  &NotImpl,       /* 94 = unimplemented setdopt */
  &NotImpl,       /* 95 = fsync */
  &NotImpl,       /* 96 = setpriority */
  &sysSocket,     /* 97 = socket */
  &NotImpl,       /* 98 = connect */
  &NotImpl,       /* 99 = compat_43 oaccept */
  &NotImpl,       /* 100 = getpriority */
  &NotImpl,       /* 101 = compat_43 osend */
  &NotImpl,       /* 102 = compat_43 orecv */
  &NotImpl,       /* 103 = sigreturn */
  &NotImpl,       /* 104 = bind */
  &NotImpl,       /* 105 = setsockopt */
  &NotImpl,       /* 106 = listen */
  &NotImpl,       /* 107 = obsolete vtimes */
  &NotImpl,       /* 108 = compat_43 osigvec */
  &NotImpl,       /* 109 = compat_43 osigblock */
  &NotImpl,       /* 110 = compat_43 osigsetmask */
  &NotImpl,       /* 111 = sigsuspend */
  &NotImpl,       /* 112 = compat_43 osigstack */
  &NotImpl,       /* 113 = compat_43 orecvmsg */
  &NotImpl,       /* 114 = compat_43 osendmsg */
  &NotImpl,       /* 115 = obsolete vtrace */
  &NotImpl,       /* 116 = gettimeofday */
  &NotImpl,       /* 117 = getrusage */
  &NotImpl,       /* 118 = getsockopt */
  &NotImpl,       /* 119 = obsolete resuba */
  &sysReadv,      /* 120 = readv */
  &sysWritev,     /* 121 = writev */
  &NotImpl,       /* 122 = settimeofday */
  &NotImpl,       /* 123 = fchown */
  &NotImpl,       /* 124 = fchmod */
  &NotImpl,       /* 125 = compat_43 orecvfrom */
  &NotImpl,       /* 126 = setreuid */
  &NotImpl,       /* 127 = setregid */
  &sysRename,     /* 128 = rename */
  &NotImpl,       /* 129 = compat_43 otruncate */
  &NotImpl,       /* 130 = compat_43 oftruncate */
  &NotImpl,       /* 131 = flock */
  &NotImpl,       /* 132 = mkfifo */
  &sysSendTo,       /* 133 = sendto */
  &NotImpl,       /* 134 = shutdown */
  &NotImpl,       /* 135 = socketpair */
  &NotImpl,       /* 136 = mkdir */
  &NotImpl,      /* 137 = rmdir */
  &NotImpl,     /* 138 = utimes */
  &NotImpl,    /* 139 = obsolete 4.2 sigreturn */
  &NotImpl,      /* 140 = adjtime */
  &NotImpl, /* 141 = compat_43 ogetpeername */
  &NotImpl, /* 142 = compat_43 ogethostid */
  &NotImpl, /* 143 = compat_43 osethostid */
  &NotImpl, /* 144 = compat_43 ogetrlimit */
  &NotImpl, /* 145 = compat_43 osetrlimit */
  &NotImpl,  /* 146 = compat_43 okillpg */
  &NotImpl,     /* 147 = setsid */
  &NotImpl,     /* 148 = quotactl */
  &NotImpl, /* 149 = compat_43 oquota */
  &NotImpl, /* 150 = compat_43 ogetsockname */
  &NotImpl,   /* 151 = unimplemented */
  &NotImpl,   /* 152 = unimplemented */
  &NotImpl,   /* 153 = unimplemented */
  &NotImpl,   /* 154 = unimplemented */
  &NotImpl,     /* 155 = nfssvc */
  &NotImpl, /* 156 = compat_43 ogetdirentries */
  &NotImpl,  /* 157 = compat_25 ostatfs */
  &NotImpl,  /* 158 = compat_25 ostatfs */
  &NotImpl,   /* 159 = unimplemented */
  &NotImpl,   /* 160 = unimplemented */
  &NotImpl,      /* 161 = getfh */
  &NotImpl, /* 162 = compat_09 ogetdomainname */
  &NotImpl, /* 163 = compat_09 osetdomainname */
  &NotImpl,    /* 164 = unimplemented ouname */
  &NotImpl,      /* 165 = sysarch */
  &NotImpl,   /* 166 = unimplemented */
  &NotImpl,   /* 167 = unimplemented */
  &NotImpl,   /* 168 = unimplemented */
  &NotImpl,  /* 169 = compat_10 osemsys */
  &NotImpl,  /* 170 = compat_10 omsgsys */
  &NotImpl,  /* 171 = compat_10 oshmsys */
  &NotImpl,   /* 172 = unimplemented */
  &sysPread,      /* 173 = pread */
  &sysPwrite,     /* 174 = pwrite */
  &NotImpl,   /* 175 = unimplemented ntp_gettime */
  &NotImpl,   /* 176 = unimplemented ntp_adjtime */
  &NotImpl,   /* 177 = unimplemented */
  &NotImpl,   /* 178 = unimplemented */
  &NotImpl,   /* 179 = unimplemented */
  &NotImpl,   /* 180 = unimplemented */
  &NotImpl,     /* 181 = setgid */
  &NotImpl,      /* 182 = setegid */
  &NotImpl,      /* 183 = seteuid */
  &NotImpl,      /* 184 = lfs_bmapv */
  &NotImpl,      /* 185 = lfs_markv */
  &NotImpl,     /* 186 = lfs_segclean */
  &NotImpl,      /* 187 = lfs_segwait */
  &NotImpl, /* 188 = compat_35 stat35 */
  &NotImpl,  /* 189 = compat_35 fstat35 */
  &NotImpl,  /* 190 = compat_35 lstat35 */
  &NotImpl,     /* 191 = pathconf */
  &NotImpl,      /* 192 = fpathconf */
  &NotImpl,      /* 193 = swapctl */
  &NotImpl,      /* 194 = getrlimit */
  &NotImpl,      /* 195 = setrlimit */
  &NotImpl,      /* 196 = getdirentries */
  &sysMmap,     /* 197 = mmap */
  &NotImpl,      /* 198 = __syscall */
  &NotImpl,      /* 199 = lseek */
  &sysTruncate,     /* 200 = truncate */
  &sysFtruncate,      /* 201 = ftruncate */
  &NotImpl,     /* 202 = __sysctl */
  &NotImpl,      /* 203 = mlock */
  &NotImpl,      /* 204 = munlock */
  &NotImpl,    /* 205 = unimplemented sys_undelete */
  &NotImpl,      /* 206 = futimes */
  &NotImpl,      /* 207 = getpgid */
  &NotImpl,      /* 208 = nnpfspioctl */
  &NotImpl,     /* 209 = unimplemented */
  &NotImpl,     /* 210 = lkmnosys */
  &NotImpl,     /* 211 = lkmnosys */
  &NotImpl,     /* 212 = lkmnosys */
  &NotImpl,     /* 213 = lkmnosys */
  &NotImpl,     /* 214 = lkmnosys */
  &NotImpl,     /* 215 = lkmnosys */
  &NotImpl,     /* 216 = lkmnosys */
  &NotImpl,     /* 217 = lkmnosys */
  &NotImpl,     /* 218 = lkmnosys */
  &NotImpl,     /* 219 = lkmnosys */
  &NotImpl, /* 220 = compat_23 semctl23 */
  &NotImpl,     /* 221 = semget */
  &NotImpl,  /* 222 = compat_35 semop */
  &NotImpl,    /* 223 = obsolete sys_semconfig */
  &NotImpl, /* 224 = compat_23 msgctl23 */
  &NotImpl,     /* 225 = msgget */
  &NotImpl,     /* 226 = msgsnd */
  &NotImpl,     /* 227 = msgrcv */
  &NotImpl,      /* 228 = shmat */
  &NotImpl, /* 229 = compat_23 shmctl23 */
  &NotImpl,      /* 230 = shmdt */
  &NotImpl, /* 231 = compat_35 shmget */
  &NotImpl,      /* 232 = clock_gettime */
  &NotImpl,      /* 233 = clock_settime */
  &NotImpl,     /* 234 = clock_getres */
  &NotImpl,    /* 235 = unimplemented timer_create */
  &NotImpl,    /* 236 = unimplemented timer_delete */
  &NotImpl,   /* 237 = unimplemented timer_settime */
  &NotImpl,   /* 238 = unimplemented timer_gettime */
  &NotImpl,    /* 239 = unimplemented timer_getoverrun */
  &NotImpl,      /* 240 = nanosleep */
  &NotImpl,   /* 241 = unimplemented */
  &NotImpl,   /* 242 = unimplemented */
  &NotImpl,   /* 243 = unimplemented */
  &NotImpl,   /* 244 = unimplemented */
  &NotImpl,   /* 245 = unimplemented */
  &NotImpl,   /* 246 = unimplemented */
  &NotImpl,   /* 247 = unimplemented */
  &NotImpl,   /* 248 = unimplemented */
  &NotImpl,   /* 249 = unimplemented */
  &NotImpl,     /* 250 = minherit */
  &NotImpl,      /* 251 = rfork */
  &NotImpl,     /* 252 = poll */
  &NotImpl,      /* 253 = issetugid */
  &NotImpl,     /* 254 = lchown */
  &NotImpl,     /* 255 = getsid */
  &NotImpl,      /* 256 = msync */
  &NotImpl, /* 257 = compat_35 semctl35 */
  &NotImpl, /* 258 = compat_35 shmctl35 */
  &NotImpl, /* 259 = compat_35 msgctl35 */
  &NotImpl, /* 260 = compat_o43 getfsstat */
  &NotImpl,  /* 261 = compat_o43 statfs */
  &NotImpl, /* 262 = compat_o43 fstatfs */
  &sysPipe,     /* 263 = pipe */
  &NotImpl,     /* 264 = fhopen */
  &NotImpl, /* 265 = compat_35 fhstat */
  &NotImpl,  /* 266 = compat_o43 fhstatfs */
  &NotImpl,     /* 267 = preadv */
  &NotImpl,      /* 268 = pwritev */
  &NotImpl,     /* 269 = kqueue */
  &NotImpl,     /* 270 = kevent */
  &NotImpl,     /* 271 = mlockall */
  &NotImpl,     /* 272 = munlockall */
  &NotImpl,     /* 273 = getpeereid */
  &NotImpl,    /* 274 = unimplemented sys_extattrctl */
  &NotImpl,    /* 275 = unimplemented sys_extattr_set_file */
  &NotImpl,    /* 276 = unimplemented sys_extattr_get_file */
  &NotImpl,   /* 277 = unimplemented sys_extattr_delete_file */
  &NotImpl,    /* 278 = unimplemented sys_extattr_set_fd */
  &NotImpl,    /* 279 = unimplemented sys_extattr_get_fd */
  &NotImpl,   /* 280 = unimplemented sys_extattr_delete_fd */
  &NotImpl,      /* 281 = getresuid */
  &NotImpl,      /* 282 = setresuid */
  &NotImpl,      /* 283 = getresgid */
  &NotImpl,      /* 284 = setresgid */
  &NotImpl,    /* 285 = obsolete sys_omquery */
  &NotImpl,     /* 286 = mquery */
  &NotImpl,      /* 287 = closefrom */
  &NotImpl,      /* 288 = sigaltstack */
  &NotImpl,     /* 289 = shmget */
  &NotImpl,      /* 290 = semop */
  &NotImpl,     /* 291 = stat */
  &NotImpl,      /* 292 = fstat */
  &NotImpl,      /* 293 = lstat */
  &NotImpl,     /* 294 = fhstat */
  &NotImpl,     /* 295 = __semctl */
  &NotImpl,     /* 296 = shmctl */
  &NotImpl,     /* 297 = msgctl */
  &NotImpl,      /* 298 = sched_yield */
  &NotImpl,     /* 299 = getthrid */
  &NotImpl,     /* 300 = thrsleep */
  &NotImpl,      /* 301 = thrwakeup */
  &NotImpl,      /* 302 = threxit */
  &NotImpl,     /* 303 = thrsigdivert */
  &NotImpl,     /* 304 = __getcwd */
  &NotImpl,      /* 305 = adjfreq */
  &NotImpl,      /* 306 = getfsstat */
  &NotImpl,     /* 307 = statfs */
  &NotImpl,      /* 308 = fstatfs */
  &NotImpl,     /* 309 = fhstatfs */
};











