/**
 * Copyright (c) 2011 Kaiserslautern University of Technology
 *
 * Authors: Denis Feth <d_feth@cs.uni-kl.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package de.fhg.iese.pef.pep;

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Hashtable;

import de.fhg.iese.broker.message.Event;
import de.fhg.iese.osgi.util.logging.Log;
import de.fhg.iese.pef.pdp.ExecuteAction;
import de.fhg.iese.pef.pdp.PDP;
import de.fhg.iese.pef.pep.interceptor.rdInterceptor;

/**
 * The Executor is responsible for exectuing Actions as specified in the policy.
 * It therefore receives an {@link ExecuteAction} object (produced by the PDP)
 * which will be executed by this class.<br/>
 * Currently, the Executor can execute three types of Actions:
 * <ol>
 * <li>log - logging into the log file</li>
 * <li></li>
 * </ol>
 * 
 */
public class Executor
{
  private final static Log log=rdInterceptor.getLog();
  
  /**
   * Executes the given {@link ExecuteAction}
   * 
   * @param action
   */
  public static void execute(Object event, ExecuteAction action)
  {
    log.info("Executing: " + action.getName());

    if(action.getName().equals("deployPolicy")) actionDeployPolicy(event, action);
    else if(action.getName().startsWith("log")) actionLog(action);
    else log.error("execute Action=["+action.getName()+"] not supported!");
  }
  
  private static void actionDeployPolicy(Object event, ExecuteAction action) throws IllegalArgumentException
  {
    Event curEvent=(Event)event;
    System.out.println("executing deployPolicy for HSRrequest");
    
    System.out.println("action parameters: "+action.toString());
    Hashtable<String, String> actionParameters=action.getPairs();
    
    String filename;
    filename=actionParameters.get("filename");
    
    String deployPolicy=null;
    if(actionParameters.containsKey("modify"))
    {
      String substitute=actionParameters.get("modify");
      if(!substitute.equals("sessionID")) 
        throw new IllegalArgumentException("Don't know how to modify param=["+substitute+"] for deployPolicy action");
      
      StringBuffer fileContents=null;
      try
      {
        File file = new File(filename);
        fileContents = new StringBuffer();
        BufferedReader reader = new BufferedReader(new FileReader(file));
        String text = null;
        while ((text = reader.readLine()) != null)
          fileContents.append(text);
          //contents.append(text).append(System.getProperty("line.separator"));
      }
      catch(Exception e)
      {
        System.out.println("exception while reading policy");
      }
      
      deployPolicy=fileContents.toString();
      
      String currentPayload=curEvent.getPayload();
      String sessionID=currentPayload.substring(currentPayload.indexOf("sessionId=")+11, 
                                                currentPayload.indexOf("\"", currentPayload.indexOf("sessionId=")+11));
      log.debug("sessionID="+sessionID);
      
      deployPolicy=deployPolicy.replace("$SESSIONID", sessionID);
      
      // deploying policy as String...
      // TODO: nyi
      BufferedOutputStream bufferedOutput = null;
      try
      {
        bufferedOutput=new BufferedOutputStream(new FileOutputStream("c:\\local\\rd\\usr\\home\\moucha\\tmp1aal.xml"));
        bufferedOutput.write(deployPolicy.getBytes());
      }
      catch(FileNotFoundException ex)
      {
        ex.printStackTrace();
      }
      catch(IOException ex)
      {
        ex.printStackTrace();
      }
      finally
      {
        try
        {
          if(bufferedOutput!=null)
          {
            bufferedOutput.flush();
            bufferedOutput.close();
          }
        }
        catch(IOException ex)
        {
          ex.printStackTrace();
        }
      }
      
      System.out.println("written policy to tmp1aal.xml");
      int ret=-1;
      try
      {
        ret=PDP.getInstance().pdpDeployPolicy("c:\\local\\rd\\usr\\home\\moucha\\tmp1aal.xml");
      }
      catch(Exception e)
      {
        System.out.println("error deploying policy");
        System.out.println(e.getMessage());
      }
      System.out.println("deploying policy returned "+ret);
    }
    else
    {
      // deploy policy via filename
      int ret=-1;
      try
      {
        ret=PDP.getInstance().pdpDeployPolicy(filename);
      }
      catch(Exception e)
      {
        System.out.println("error deploying policy");
        System.out.println(e.getMessage());
      }
      System.out.println("deploying policy returned "+ret);
    }
    // deploying policy for current sessionID finished...
  }

  private static void actionLog(ExecuteAction action)
  {
    try
    {
      FileWriter fstream=new FileWriter("log.txt", true);
      BufferedWriter out=new BufferedWriter(fstream);

      Calendar cal=Calendar.getInstance();
      SimpleDateFormat sdf=new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
      String now=sdf.format(cal.getTime());

      String msg=action.getPairs().get("msg");

      msg=now + " [" + (action.getName().indexOf("_") != -1 ? action.getName().substring(action.getName().indexOf("_") + 1) : "u") + "] " + msg + "\n\n";
      out.write(msg);
      out.close();
      log.info(msg);

    }
    catch(IOException e)
    {
      log.error("Error while writing to log.");
    }
  }
}
