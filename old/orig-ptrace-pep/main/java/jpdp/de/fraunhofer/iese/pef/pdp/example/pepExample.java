package de.fraunhofer.iese.pef.pdp.example;

import de.fraunhofer.iese.pef.pep.IPolicyEnforcementPoint;

public class pepExample implements IPolicyEnforcementPoint
{
  
  public static IPolicyEnforcementPoint curInstance=null;
  
  private pepExample()
  {
  }
  
  public static IPolicyEnforcementPoint getInstance()
  {
    if(curInstance==null) curInstance=new pepExample();
    return curInstance;
  }
  
  @Override
  public int handlePEPsubscribe(String request, int unsubscribe)
  {
    System.out.println("[pepExample] Received PEP subscribe request: [" + request + ", "+ unsubscribe +"]");
    return 0;
  }
}
