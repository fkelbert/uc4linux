/*
 * openbsd_utils.c
 *      Author: Ricardo Neisse
 */

#include "openbsd_utils.h"
#include "log_openbsd_utils_c.h"

char *getProcCommand(int pid)
{
  kvm_t *kd;
  char *command;
  struct kinfo_proc *proc;
  int n_proc;
  struct passwd *user_info=NULL;
  kd=kvm_open(NULL, NULL, NULL, KVM_NO_FILES, "error");
  if(kd)
  {
    log_trace("kvm_getproc2 --> kvm_getprocs  (OBSD5)");
    proc=kvm_getprocs(kd, KERN_PROC_PID, pid, sizeof(struct kinfo_proc), &n_proc);
    if(n_proc == 1)
    {
      if(proc) command=proc->p_comm;
    }
    kvm_close(kd);
  }
  if(user_info == NULL)
    log_debug("Error retrieving user info for process pid=[%d]", pid);
  return strdup(command);
}

struct passwd *getUserInfo(int pid, struct passwd *user_infoA)
{
  kvm_t *kd;
  struct kinfo_proc *proc;
  int n_proc;
  struct passwd *user_info=NULL;
  kd=kvm_open(NULL, NULL, NULL, KVM_NO_FILES, "error");
  if(kd)
  {
    log_trace("kvm_getproc2 --> kvm_getprocs  (OBSD5)");
    proc=kvm_getprocs(kd, KERN_PROC_PID, pid, sizeof(struct kinfo_proc), &n_proc);
    if(n_proc == 1)
    {
      if(proc) user_info=getpwuid(proc->p_uid);
    }
    kvm_close(kd);
  }
  if(user_info == NULL)
  {
    log_trace("Error retrieving user info for process pid=[%d]", pid);
    //return R_ERROR;
  }
  return user_info;
  //return R_SUCCESS;
}
