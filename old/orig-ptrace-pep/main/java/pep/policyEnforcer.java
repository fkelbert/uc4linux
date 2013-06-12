package de.fhg.iese.pef.pep;

import java.util.Hashtable;
import java.util.List;

import de.fhg.iese.broker.component.InformationBrokerComponent;
import de.fhg.iese.osgi.util.logging.Log;

import de.fhg.iese.pef.pdp.ExecuteAction;
import de.fhg.iese.pef.pdp.pdpResponse;

public abstract class policyEnforcer
{
  private final Log log=InformationBrokerComponent.getLog();

  /**
   * Public interface for the {@link PolicyEnforcer}. 
   * This method manages the enforcement procedure
   * 
   * @param event
   *          - the event that has been occurred
   * @param response
   *          - the response of the PDP, containing information of the necessary actions
   * @return the modified event
   */
  public final Object enforce(Object event, pdpResponse response)
  {
    if(response==null || response.getAuthorizationAction()==null)
    {
      log.error("pdpResponse is null or malformed!");
      throw new IllegalArgumentException("pdpResponse is null or malformed!");
    }

    log.debug("Received pdpResponse: " + response.toString());
    log.debug("To be performed on: " + event.toString());

    for(ExecuteAction executeAction : response.getExecuteActions())
    {
      log.debug("ExecuteAction: " + executeAction.toString());
      log.debug("name="+executeAction.getName());
      
      Executor.execute(event, executeAction);
    }

    if(response.getAuthorizationAction().getType() == pdpResponse.Type.INHIBIT)
    {
      log.error("RD: i have to inhibit????");
      event=inhibit(event);
    }
    else if(response.getAuthorizationAction().getType() == pdpResponse.Type.ALLOW)
    {
      Hashtable<String, List<String>> modifiers=response.getAuthorizationAction().getModifiers();
      int delay=response.getAuthorizationAction().getDelay();
      
      // standard delay=-1
      if(modifiers.size()==0 && delay==-1) {return event;}

      for(String key : modifiers.keySet())
      {
        for(String value : modifiers.get(key))
          event=modify(event, key, value);
      }
      event=delay(event, delay, response.getAuthorizationAction().getDelayUnit());
    }

    return event;
  }

  /**
   * Inhibits the event
   * 
   * @param event
   * @return
   */
  protected abstract Object inhibit(Object event);

  /**
   * Modifies the event
   * 
   * @param event
   *          - the event
   * @param name
   *          - the name of parameter to be modified
   * @param value
   *          - the new value of the parameter
   * @return
   */
  protected abstract Object modify(Object event, String name, String value);

  /**
   * Delays the event
   * 
   * @param event
   *          - the event
   * @param amount
   *          - the amount of timesteps to be delayed
   * @param unit
   *          - the unit of the delay
   * @return
   */
  protected abstract Object delay(Object event, int amount, String unit);

}
