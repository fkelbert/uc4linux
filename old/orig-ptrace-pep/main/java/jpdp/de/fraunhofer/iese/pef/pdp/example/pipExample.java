package de.fraunhofer.iese.pef.pdp.example;

import de.fraunhofer.iese.pef.pip.IPolicyInformationPoint;

public class pipExample implements IPolicyInformationPoint
{
  public static IPolicyInformationPoint curInstance=null;
  
  private pipExample()
  {
  }
  
  public static IPolicyInformationPoint getInstance()
  {
    if(curInstance==null) curInstance=new pipExample();
    return curInstance;
  }
  
  @Override
  public int eval(String value1, String value2)
  {
    System.out.println("received PIP request for evaluation: " + value1);
    return 1;
  }

  @Override
  public String init(String method, String param)
  {
    System.out.println("received PIP request for initialization: " + method + "; param="+param);
    return "bliblablub";
  }
  
  public String init(String method, String param, String initDataID)
  {
    System.out.println("received PIP request for initialization: " + method + "; param="+param+"; initialDataID="+initDataID);
    return "bliblablub";
  }


}
