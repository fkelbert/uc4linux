package de.fraunhofer.iese.pef.pdp.example;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;

import de.fraunhofer.iese.pef.pdp.PolicyDecisionPoint;
import de.fraunhofer.iese.pef.pdp.internal.Constants;
import de.fraunhofer.iese.pef.pdp.internal.Decision;
import de.fraunhofer.iese.pef.pdp.internal.Event;
import de.fraunhofer.iese.pef.pdp.internal.IPolicyDecisionPoint;
import de.fraunhofer.iese.pef.pdp.internal.Param;

import de.fraunhofer.iese.pef.pep.IPolicyEnforcementPoint;

public class pdpNativeExample
{
  private static IPolicyDecisionPoint lpdp;
  
  public static StringBuffer readFile(String filename) throws IOException
  {
    File file=new File(filename);
    StringBuffer contents=new StringBuffer();
    BufferedReader reader=null;
    reader=new BufferedReader(new FileReader(file));
    String text=null;
    while((text=reader.readLine()) != null)
      contents.append(text).append(System.getProperty("line.separator"));
    reader.close();
    return contents;
  }  

  @SuppressWarnings("unused")
  public static void main(String args[]) throws IOException, InterruptedException 
  {
    IPolicyEnforcementPoint pep = pepExample.getInstance();
    //pxpExample pxp = pxpExample();
    //pipExample pip = new pipExample();
        
    System.out.println("Initiating PDP instance...");
    lpdp=PolicyDecisionPoint.getInstance();
    if(lpdp==null) {System.out.println("Could not retrieve PDP instance!!"); return;} 
    System.out.println("PDP instance retrieved");
    
    System.out.println("[jniPDP]: Start PDP");
    int startret=-1;
    try
    {
      startret=lpdp.pdpStart();
      System.out.println("jniPDP: PDP started with result=["+startret+"]");
    }
    catch(Exception e)
    {
      System.out.println("Error starting PDP...");
      System.out.println(e.getMessage());
      e.printStackTrace();
    }
    
    if(startret==1) 
    {
      System.out.println("Error starting PDP!");
      return;
    }
    
    int ret=0;

    // Registering this PEP
    ret=lpdp.registerPEP("jpdpPEP", "de/fraunhofer/iese/pef/pdp/example/pepExample", "handlePEPsubscribe", "(Ljava/lang/String;I)I");
    System.out.println("Registering PEP="+ret);
    ret=lpdp.registerAction("action2", "jpdpPEP");
    System.out.println("Registering Action="+ret);

    // Registering this PXP
    ret=lpdp.registerPXP("jpdpPXP", "de/fraunhofer/iese/pef/pdp/example/pxpExample", "handlePXPexecute", "(Ljava/lang/String;Ljava/util/ArrayList;)I");
    System.out.println("Registering PXP="+ret);
    ret=lpdp.registerExecutor("notify", "jpdpPXP");
    System.out.println("Registering Executor="+ret);

    
    //String policyFileName="c:\\local\\rd\\usr\\home\\moucha\\workspace\\pef\\src\\main\\xml\\examples\\test.xml";
    String policyFileName="/home/rd/workspace/pef/src/main/xml/examples/test.xml";
    System.out.println("jniPDP: deployPolicy returned: \"" + lpdp.pdpDeployPolicy(policyFileName) + "\"");
    System.out.println("jniPDP: deployed mechanisms:\n##" + lpdp.listDeployedMechanismsJNI() + "##");

    Event event1=new Event("action2", true, System.currentTimeMillis());
    event1.addParam(new Param<String>("val2", "value2", Constants.PARAMETER_TYPE_STRING));
    
    System.in.read();
    System.out.println("Sending event " + event1);
    Decision pdpResponse=lpdp.pdpNotifyEventJNI(event1);
    System.out.println(pdpResponse.toString());
    
    System.in.read();
    ret=lpdp.pdpStop();
    System.out.println("Native PDP stopping finished ["+ret+"]");

    //System.out.println("trying to get list after stopping PDP");
    //System.out.println("jniPDP: deployed mechanisms:\n" + lpdp.listDeployedMechanisms());
    //System.in.read();
    System.exit(0);
  }
}




