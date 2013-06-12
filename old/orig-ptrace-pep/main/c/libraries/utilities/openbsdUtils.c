/**
 * @file openbsdUtils.c
 * @brief Implementation of auxiliary utilities for OpenBSD (systrace)
 *
 * @author  Ricardo Neisse
**/

#include "openbsdUtils.h"

char *getProcCommand(int pid)
{
  kvm_t *kd;
  char *command;
  struct kinfo_proc2 *proc;
  int n_proc;
  struct passwd *user_info=NULL;
  kd=kvm_open(NULL, NULL, NULL, KVM_NO_FILES, "error");
  if(kd)
  {
    proc=kvm_getproc2(kd, KERN_PROC_PID, pid, sizeof(struct kinfo_proc2), &n_proc);
    if(n_proc == 1)
      if(proc) command=proc->p_comm;
    kvm_close(kd);
  }
  if(user_info == NULL) log_debug("Error retrieving user info for process pid=[%d]", pid);
  return strdup(command);
}

struct passwd *getUserInfo(int pid, struct passwd *user_infoA)
{
  kvm_t *kd;
  struct kinfo_proc2 *proc;
  int n_proc;
  struct passwd *user_info=NULL;
  kd=kvm_open(NULL, NULL, NULL, KVM_NO_FILES, "error");
  if(kd)
  {
    proc=kvm_getproc2(kd, KERN_PROC_PID, pid, sizeof(struct kinfo_proc2), &n_proc);
    if(n_proc == 1)
      if(proc) user_info=getpwuid(proc->p_uid);
    kvm_close(kd);
  }
  if(user_info == NULL)
    log_trace("Error retrieving user info for process pid=[%d]", pid);
  return user_info;
}
