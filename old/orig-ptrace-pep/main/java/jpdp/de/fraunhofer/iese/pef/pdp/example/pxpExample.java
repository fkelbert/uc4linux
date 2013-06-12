package de.fraunhofer.iese.pef.pdp.example;

import java.util.ArrayList;

import de.fraunhofer.iese.pef.pdp.internal.Param;

public class pxpExample
{
  public static int handlePXPexecute(String name, ArrayList<Param<?>> params)
  {
    System.out.println("[pxpExample] received PXP execution request: [" + name + ", "+ params +"]");
    return 0;
  }
}
