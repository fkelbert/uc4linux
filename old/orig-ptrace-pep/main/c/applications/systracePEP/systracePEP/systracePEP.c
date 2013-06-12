/**
 * @file  systracePEP.c
 * @brief Implementation of a Policy Enforcement Point for OpenBSD using systrace
 *
 * @author cornelius moucha, Enrico Lovat
 **/

#include "systracePEP.h"

extern int opid;

int main(int argc, char **argv) {
	// Systrace structures
	struct str_message sys_msg;
	struct systrace_answer sys_ans;
	struct pollfd pfd[1];

	// Starting PDP
	if (pdpStart() != R_SUCCESS) {
		log_error("Error initializing PDP");
		return 1;
	}

	// Deploying policy to PDP
	char *mechanismsFilename = "mechanismsRDsystrace.xml";
	if (pdpDeployMechanism(mechanismsFilename) == R_ERROR) {
		log_error("[%s] could not be loaded!", mechanismsFilename);
		return 1;
	} else
		log_debug("[%s] successfully loaded.", mechanismsFilename);

	// Initialize Systrace
	int cfd = init_systrace();
	initializeFDTable();

	// Initialize Syscall-Table
	if (initSyscallTable() == R_ERROR) {
		log_error("Syscall-Hashtable could not be initialized!");
		return 1;
	} else
		log_debug("Syscall-Table successfully initialized.");

	// Initialize socket for attaching process
	pefSocket_ptr lsocket = NULL;
	if ((lsocket = initSocket("/tmp/systracePEP", &do_attach_process)) == NULL) {
		log_error("Socket could not be initialized!");
		return 1;
	}

	// Code for intercepting system calls
	pfd[0].fd = cfd;
	pfd[0].events = POLLIN;

	// Variables to instantiate a mechanism: id + ECA (event, condition, and actions)
	event_ptr event;
	notifyResponse_ptr eventResponse;
	char filename[512];
	void *p;
	int reset_flag = 0;

	while (1) {
		/* not yet supported in PDP!
		 if(reset_flag == 1)
		 {
		 system("./reload_monitor"); // this just echo "reloading monitor"?!?!
		 // execl("/home/workspace/OSLFramework/syscall_handler/syscall_handler","",0);
		 // delete_all_mechanisms();
		 reset_monitor_table();
		 reset_control_table();

		 //IF_reset();
		 if(pdpDeployMechanism(mechanismsFilename)==R_ERROR)
		 {
		 log_error("[%s] could not be loaded!",mechanismsFilename);
		 return 0;
		 }
		 else log_debug("[%s] successfully loaded.",mechanismsFilename);
		 reset_flag =0;
		 }
		 */

		if (poll(pfd, 1, 0) > 0) {
			if (read(cfd, &sys_msg, sizeof(sys_msg)) == sizeof(sys_msg)) { // Checks if the received systrace event is a request or a response
				switch (sys_msg.msg_type) {
				// This message is sent whenever the emulation of a process changes
				// SYSTR_MSG_EMUL:

				// This message is sent whenever the effective UID or GID has changed during the execution of a system call.
				// SYSTR_MSG_UGID:

				// This is sent whenever the kernel frees the policy identified by msg_policy.
				// SYSTR_MSG_POLICYFREE:

				// This message is sent whenever, before a call to execve(2) a process is privileged (technically,
				// the process has the P_SUGID or P_SUGIDEXEC flag set), but after the call these privileges have been
				// dropped. The new image name is specified in the path argument.
				// SYSTR_MSG_EXECVE:

				// This message is sent whenever a process gains or loses a child
				case SYSTR_MSG_CHILD:
					// log_trace("- Father %d ",sys_msg.msg_pid);
					// log_trace("- Child created ", sys_msg.msg_data.msg_child.new_pid);
					// we keep track of the childs to control user information related to the child
					break;

				case SYSTR_MSG_ASK: // We handle the system call before it is executed, TRY event
					memset(&sys_ans, 0, sizeof(sys_ans));
					sys_ans.stra_pid = sys_msg.msg_pid;
					sys_ans.stra_seqnr = sys_msg.msg_seqnr;
					sys_ans.stra_flags = SYSTR_FLAGS_RESULT;

					event = parseSyscall(cfd, &sys_msg);

					/* EXECVE must be handled as SYSTR_MSG_ASK, not SYSTR_MSG_RES (otherwise wrong parameter)
					 * EXIT doesn't always have a SYSTR_MSG_RES
					 * WRITE has to be forbidden before being executed
					 */
					if (event != NULL) {
						if (MONITORING_ACTIVE == 1 && PRINT_TRACE > 0) {
							if ((PRINT_TRACE > 1)
									|| g_hash_table_size(event->params) > 0)
								if (sys_msg.msg_data.msg_ask.code != SYS_mmap) { //TODO: REMOVE this condition. this is only to get a less verbose output.
									eventLog("++ New ask ", event);
								}
						}

						if ((sys_msg.msg_data.msg_ask.code == SYS_execve
								|| sys_msg.msg_data.msg_ask.code == SYS_exit
								|| sys_msg.msg_data.msg_ask.code == SYS_write
						//								|| sys_msg.msg_data.msg_ask.code
						//									== SYS_truncate
						//								|| sys_msg.msg_data.msg_ask.code
						//									== SYS_ftruncate
						) && g_hash_table_size(event->params) > 0
								&& DATA_FLOW_ENABLED == 1) {
							//log_info("aif");
							IF_update(event);
							//log_info("aiff");
						}
						// Look for triggered mechanisms
						eventResponse = pdpNotifyEvent(event);
						processResponse(eventResponse, &sys_msg, &sys_ans);
					}

					if (DATA_FLOW_ENABLED == 1) {
						if (get_deny_because_of_if() != 0) {
							printf(
									"Syscall denied because of IF's requirements violation\n");

							reset_deny_because_of_if();
							sys_ans.stra_policy = SYSTR_POLICY_NEVER;
						}
					}

					//log_info("aast");
					//int x=kill(opid, 0);
					//if(x==-1)
					//  log_error("error sending check signal!!! [%d]", errno);
					//else log_info("a+");

					answerSystrace(&sys_msg, &sys_ans);
					break;

				case SYSTR_MSG_RES: // Syscall response
					switch (sys_msg.msg_data.msg_ask.code) {
					case SYS_open:
						memcpy(&p, &sys_msg.msg_data.msg_ask.args[0],
								sizeof(p));
						get_string_buffer(cfd, sys_msg.msg_pid, SYSTR_READ, p,
								&filename[0]);
						store_fd_filename_mapping(sys_msg.msg_pid,
								sys_msg.msg_data.msg_ask.rval[0], filename);
						break;
					}

					event = parseSyscall(cfd, &sys_msg);
					if (event != NULL) {
						//              printf ("-%d-",MONITORING_ACTIVE);
						if (MONITORING_ACTIVE == 1 && PRINT_TRACE > 0) {
							if (sys_msg.msg_data.msg_ask.code != SYS_mmap) { //TODO: REMOVE this condition. this is only to get a less verbose output.
								eventLog("++ New res ", event);
							}
							if (((PRINT_TRACE > 1)
									|| (sys_msg.msg_data.msg_ask.code
											!= SYS_execve
											&& sys_msg.msg_data.msg_ask.code
													!= SYS_exit
											&& sys_msg.msg_data.msg_ask.code
													!= SYS_write
											//											&& sys_msg.msg_data.msg_ask.code
											//													!= SYS_ftruncate
											//											&& sys_msg.msg_data.msg_ask.code
											//												!= SYS_truncate
											&& g_hash_table_size(event->params)
													> 0))

							&& (DATA_FLOW_ENABLED == 1)) {
								//log_info("rif");
								IF_update(event);
								//log_info("riff");
							}
						}
					}
					if (DATA_FLOW_ENABLED == 1)
						reset_deny_because_of_if();
					//log_info("rast");
					//int y=kill(opid,0);
					//if(y==-1)
					//  log_error("error sending pid check signal!!! [%d]",errno);
					//else log_info("r+");
					answerSystrace(&sys_msg, &sys_ans);
					break;
				} //switch (sys_msg.msg_type)

				if (event != NULL) { // TODO free temp stuff!
					//event_free(event);
					//event=NULL;
				}
				eventResponse = NULL;
			} //if(read(cfd, &sys_msg, sizeof(sys_msg)) == sizeof(sys_msg))
			usleep(1); // sleeping 1us really necessary?? -> result in high cpu usage
		} //if(poll(pfd,1,0) > 0)
		else
			usleep(100);
	}
	return 0;
}
