package de.fraunhofer.iese.pef.pdp;

import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;
import java.util.ArrayList;

import de.fraunhofer.iese.pef.pdp.internal.Decision;
import de.fraunhofer.iese.pef.pdp.internal.Event;
import de.fraunhofer.iese.pef.pdp.internal.IPolicyDecisionPoint;

//public class PolicyDecisionPoint implements IPolicyDecisionPoint
public class PolicyDecisionPoint extends UnicastRemoteObject implements IPolicyDecisionPoint
{
  private static final long          serialVersionUID = 1L;
  
  public static IPolicyDecisionPoint curInstance=null;
  public static boolean              pdpRunning =false;

  private PolicyDecisionPoint() throws RemoteException
  {
    System.out.println("Loading native PDP library");
    try
    {
      System.loadLibrary("pdp");
      pdpRunning=true;
      System.out.println("Native PDP library loaded...");
    }
    catch(Exception e)
    {
      System.out.println("Could not load native PDP library!");
      System.out.println(e.getMessage());
    }
  }
  
  public static IPolicyDecisionPoint getInstance() throws RemoteException
  {
    if(curInstance==null) curInstance=new PolicyDecisionPoint();
    return curInstance;
  }
  
  // Native method declaration
  public native int               pdpStart();
  public native int               pdpStop();
  
  public native int               registerPEP(String pepName, String className, String methodName, String methodSignature);
  public native int               registerAction(String actionName, String pepName);
  public native int               registerPXP(String pepName, String className, String methodName, String methodSignature);
  public native int               registerExecutor(String actionName, String pepName);
  
  public native String            pdpNotifyEventXML(String event);
  public native Decision          pdpNotifyEventJNI(Event event);
  
  public native int               pdpDeployPolicy(String mechanism_doc_path);
  public native int               pdpDeployPolicyString(String policy);
  public native int               pdpDeployMechanism(String mechanism_doc_path, String mechName);
  public native int               pdpDeployMechanismString(String policy, String mechName);
  public native int               pdpRevokeMechanism(String mechName);

  public native String            listDeployedMechanisms();
  public native ArrayList<String> listDeployedMechanismsJNI();

  public native int               setRuntimeLogLevel(int newLevel);

}
