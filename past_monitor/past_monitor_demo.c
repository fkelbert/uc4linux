#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "logger.h"
#include "event.h"
#include "past_monitor.h"


int main(int argc, char *argv[]) {
  char *id = "Monitor";
  char *policy;
  // Monitor timestep and update thread usleep
  int64_t thread_usleep = 200000;
  //                      | seconds
  int64_t usec_timestep = 2000000;
  int ret;

  plog (LOG_TRACE, "Running past monitor demo");

  init_past_monitors();

  start_past_monitor_update_thread(thread_usleep);

  // True and false values
  policy = "(false)";
  policy = "(true)";

  // Propositional logic operators
  policy = "(not true)";
  policy = "(true or false)";
  policy = "(true and false)";
  policy = "(true implies false)";
  policy = "(false implies true)";

  // Eall(event) -> {start | ongoing}
  // Efst(event) -> {start}
  // Every time step the monitor checks if the event or
  // a refinement of the event is true or false in the
  // current time step
  policy = "Eall(open)";
  // Parameters of events
  policy = "Eall(open, {(file, /etc/passwd)} )";
  // A is a refinement of B if A has the same event_name/id
  // and B has the same parameter name/values as A or more

  // always(Formula)
  // Formula must be true all the time, if it was false once
  // then always is violated and will be false forever
  policy = "always( Eall(login) )";
  // You must pay at least once before you open
  policy = "Eall(open) implies not(always(not(Eall(pay))))";
  // Eventually is expressed using not(always(not(E)))

  // before(N, Formula)
  // Formula was true N time steps previously
  // Event login must have been true 7 timesteps before
  // Eall(logout) implies before(7, Eall(login))
  policy = "before(4, Eall(login))";

  // since(A, B)
  // Since B was true A must have been
  // true all the time until now
  // since login message must be true all the time
  policy = "since(Eall(message), Eall(login))";

  // within(N, Formula)
  // At least once within the previous N timesteps
  // event must have been true
  policy = "within(5, Eall(login))";

  // during(N, Formula)
  // During all the previous N time steps Formula
  // should have been true
  policy = "during(5 timesteps, Eall(login))";

  // replim(N , Min, Max, Formula)";
  // During the previous N time steps Formula must
  // have been true at least Min and at most Max times
  // login is allowed to happen at least 2 and at most 4 times
  // in the last 4 seconds, time period should be at least
  policy = "replim(1 , 1, 3, Eall(login))";

  // repmax(N, Formula)
  // Formula is allowed to happen at most N times
  // since start of the trace
  // "repmax(1 , Eall(open))"
  // Open should not have happen in more then one time step,
  // independently of how many open events happen in the same
  // time step
  policy = "repmax(1 , Eall(login))";

  // login is allowed to happen at most 1 time
  // until logout happends, or logout should never happen
  // after logout happens we do not care about login anymore
  policy = "repsince(1 , Eall(login), Eall(logout))";

  // XPath expressions to access event parameters
  policy = "XPathEval( /triggerEvent/parameter[@name='source']/@value='evil' )";


  // Other examples
  policy = "Eall(open, {(file, /etc/passwd)} )";
  policy = "Eall(open) implies not(always(not(Eall(pay))))";
  policy = "always(not Eall(open))";
  policy = "always(before(3, Eall(login)) implies within(3, Eall(logout)))";
  policy = "within(3, Eall(logout))";
  policy = "before(3, Eall(login))";
  policy = "always(before(3, Eall(login)) implies during(3, Eall(logout)))";

  // Check during and with if the current timestep is included.?


  // login since send message
  // Event login must have happened at least once
  // and since the login the event send_message
  // must have happened every time step
  policy = "repsince(2, Eall(login_fail), Eall(login_success))";
  policy = "XPathEval( /triggerEvent/parameter[@name='source']/@value='evil' )";
  policy = "since(Eall(message), Eall(login_success))";
  policy = "always(before(3, Eall(login)) implies during(3, Eall(logout)))";

  ret = add_past_monitor(id, policy, usec_timestep);
  if (ret == 0) {
    plog(LOG_ERROR, "Error creating monitor, aborting");
    exit(0);
  }
  activate_past_monitor(id);

  getchar();

  event_t *evt = (event_t*)malloc(sizeof(event_t));
  evt->mode = M_EALL;
  evt->desired = FALSE;
  evt->n_params = 1;
  evt->params = (param_t *)malloc(sizeof(param_t) * 1);
  evt->params[0].param_name = "source";
  evt->params[0].param_value = "evil";
  create_xml_doc(evt);

 /*
  evt->event_name = "login";
  int value = update_past_monitor_by_id(id, evt);
  if (value) {
    plog(LOG_TRACE, "Monitor not violated");
  } else {
    plog(LOG_TRACE, "Monitor violated");
  }
*/

  evt->event_name = "login_fail";
  update_all_past_monitors(evt);
  getchar();

  evt->event_name = "login_success";
  update_all_past_monitors(evt);
  getchar();

  evt->event_name = "message";
  update_all_past_monitors(evt);
  getchar();

  evt->event_name = "message";
  update_all_past_monitors(evt);
  getchar();

  evt->event_name = "login_fail";
  update_all_past_monitors(evt);
  getchar();

  evt->event_name = "login_success";
  update_all_past_monitors(evt);
  getchar();

  evt->event_name = "login_fail";
  update_all_past_monitors(evt);
  getchar();

  evt->event_name = "login_fail";
  update_all_past_monitors(evt);
  getchar();

  evt->event_name = "login_success";
  update_all_past_monitors(evt);
  getchar();

  evt->event_name = "login_fail";
  update_all_past_monitors(evt);
  getchar();

  evt->event_name = "login_fail";
  update_all_past_monitors(evt);
  getchar();

  evt->event_name = "login_fail";
  update_all_past_monitors(evt);
  getchar();

  evt->event_name = "login_success";
  update_all_past_monitors(evt);
  getchar();

  evt->event_name = "send_message";
  update_all_past_monitors(evt);
  getchar();

  evt->event_name = "send_message";
  update_all_past_monitors(evt);
  getchar();

  evt->event_name = "login";
  update_all_past_monitors(evt);

  evt->event_name = "send_message";
  update_all_past_monitors(evt);
  getchar();

  evt->event_name = "logout";
  update_all_past_monitors(evt);
  getchar();

  evt->event_name = "login";
  update_all_past_monitors(evt);

  getchar();

  evt->event_name = "send_message";
  update_all_past_monitors(evt);
  getchar();

  evt->event_name = "logout";
  update_all_past_monitors(evt);
  getchar();

  evt->event_name = "logout";
  update_all_past_monitors(evt);
  getchar();

  evt->event_name = "pay";
  update_all_past_monitors(evt);
  getchar();

  evt->event_name = "open";
  update_all_past_monitors(evt);
  getchar();

  update_all_past_monitors(evt);
  getchar();

  return 0;
}
