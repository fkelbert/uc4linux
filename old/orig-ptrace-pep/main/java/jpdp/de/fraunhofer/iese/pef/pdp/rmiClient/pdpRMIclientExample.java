package de.fraunhofer.iese.pef.pdp.rmiClient;

import java.rmi.Naming;
import java.rmi.RMISecurityManager;

import de.fraunhofer.iese.pef.pdp.internal.IPolicyDecisionPoint;

public class pdpRMIclientExample 
{

  public static void main(String[] args) 
  {
    String host="localhost";
    int port=9984;
    
    try
    {
      if(System.getSecurityManager()==null)
      {
        System.setSecurityManager(
          new RMISecurityManager()
          {
            public void checkConnect(String host, int port) {}
            public void checkConnect(String host, int port, Object context) {}
          }
        );
      }
      IPolicyDecisionPoint obj=(IPolicyDecisionPoint) Naming.lookup("rmi://" + host + ":" + port + "/pdpRMI");
      
      System.out.println("List deployed mechanisms: ");
      System.out.println(obj.listDeployedMechanismsJNI());
      System.out.println("Deploy example policy: " + obj.pdpDeployPolicy("/home/uc/workspace/pef/src/main/xml/examples/test.xml"));
      System.out.println("List deployed mechanisms: ");
      System.out.println(obj.listDeployedMechanismsJNI());
      System.out.println("Terminating rmi client example.");
    }
    catch(Exception e)
    {
      System.out.println("Exception occured: " + e.getMessage());
      e.printStackTrace();
    }
  }

}
