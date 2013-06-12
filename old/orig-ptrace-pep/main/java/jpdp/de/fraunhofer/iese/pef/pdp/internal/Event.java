package de.fraunhofer.iese.pef.pdp.internal;

import de.fraunhofer.iese.pef.pdp.internal.xml.EventParser;
import de.fraunhofer.iese.pef.pdp.internal.xml.XmlUtils;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Text;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.XMLReader;
import org.xml.sax.helpers.XMLReaderFactory;

import java.io.IOException;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.Hashtable;

import javax.xml.transform.dom.DOMSource;

public class Event
{
  private String                      eventAction;
  private boolean                     tryEvent;
  private long                        timestamp;
  private Hashtable<String, Param<?>> params=new Hashtable<String, Param<?>>();

  public Event()
  {
    this.eventAction="noName";
    this.tryEvent=true;
    this.timestamp=System.currentTimeMillis();
  }

  public Event(String action, boolean isTry)
  {
    this.eventAction=action;
    this.tryEvent=isTry;
    this.timestamp=System.currentTimeMillis();
  }
  
  public Event(String action, boolean isTry, long time)
  {
    this.eventAction=action;
    this.tryEvent=isTry;
    this.timestamp=time;
  }

  public Event(String eventXml) throws SAXException, IllegalArgumentException, IOException
  {
    EventParser handler=new EventParser(this);
    XMLReader parser=XMLReaderFactory.createXMLReader();
    parser.setContentHandler(handler);
    parser.parse(new InputSource(new StringReader(eventXml)));
  }

  public void addParam(Param<?> param)
  {
    params.put(param.getName(), param);
  }

  public void removeParam(Param<?> param)
  {
    params.remove(param);
  }

  /**
   * @return the eventName
   */
  public String getEventAction()
  {
    return eventAction;
  }

  /**
   * @param eventAction
   *          the eventAction to set
   */
  public void setEventAction(String eventAction)
  {
    this.eventAction=eventAction;
  }

  /**
   * @return the isTry
   */
  public boolean isTryEvent()
  {
    return tryEvent;
  }

  /**
   * @param isTry
   *          the isTry to set
   */
  public void setTryEvent(boolean isTry)
  {
    this.tryEvent=isTry;
  }

  /**
   * @return the timestamp
   */
  public long getTimestamp()
  {
    return timestamp;
  }

  /**
   * @param timestamp
   *          the timestamp to set
   */
  public void setTimestamp(long timestamp)
  {
    this.timestamp=timestamp;
  }

  /**
   * @return the params
   */
  public ArrayList<Param<?>> getParams()
  {
    return new ArrayList<Param<?>>(params.values());
  }

  /**
   * @param params
   *          the params to set
   */
  public void setParams(Hashtable<String, Param<?>> params)
  {
    this.params=params;
  }
  
  public Param<?> getParameterForName(String name) 
  {
    return params.get(name);
  }

  public Object getParameterValue(String name)
  {
    Param<?> param=params.get(name);
    if(param != null) return params.get(name).getValue();
    return null;
  }

  public void clear()
  {
    params.clear();
  }

  public void addStringParameter(String name, String value)
  {
    if(value != null) addParam(new Param<String>(name, value, Constants.PARAMETER_TYPE_STRING));
  }

  public void addIntParameter(String name, int value)
  {
    addParam(new Param<Integer>(name, value, Constants.PARAMETER_TYPE_INT));
  }

  public void addBooleanParameter(String name, boolean value)
  {
    addParam(new Param<Boolean>(name, value, Constants.PARAMETER_TYPE_BOOL));
  }

  public void addLongParameter(String name, long value)
  {
    addParam(new Param<Long>(name, value, Constants.PARAMETER_TYPE_LONG));
  }

  public void addStringArrayParameter(String name, String[] value)
  {
    if(value != null) addParam(new Param<String[]>(name, value, Constants.PARAMETER_TYPE_STRING_ARRAY));
  }

  public void addByteArrayParameter(String name, byte[] value)
  {
    if(value != null)
    {
      Param<byte[]> param=new Param<byte[]>(name, value, Constants.PARAMETER_TYPE_BINARY);
      addParam(param);
    }
  }

  public String getStringParameter(String name)
  {
    try
    {
      Object o=getParameterForName(name).getValue();
      if(o instanceof String) return (String)o;
    }
    catch(Exception e)
    {
      return null;
    }
    return null;
  }

  public int getIntParameter(String name, int defaultValue)
  {
    try
    {
      Object o=getParameterForName(name).getValue();
      if(o instanceof Integer) return (Integer)o;
    }
    catch(Exception e)
    {
      return defaultValue;
    }
    return defaultValue;
  }

  public boolean getBooleanParameter(String name, boolean defaultValue)
  {
    try
    {
      Object o=getParameterForName(name).getValue();
      if(o instanceof Boolean) { return (Boolean)o; }
    }
    catch(Exception e)
    {
      return defaultValue;
    }
    return defaultValue;
  }

  public long getLongParameter(String name, long defaultValue)
  {
    try
    {
      Object o=getParameterForName(name).getValue();
      if(o instanceof Long) return (Long)o;
    }
    catch(Exception e)
    {
      return defaultValue;
    }
    return defaultValue;
  }

  public byte[] getByteArrayParameter(String name)
  {
    try
    {
      Object o=getParameterForName(name).getValue();
      if(o instanceof byte[]) return (byte[])o;
    }
    catch(Exception e)
    {
      return null;
    }
    return null;
  }

  public String[] getStringArrayParameter(String name)
  {
    try
    {
      Object o=getParameterForName(name).getValue();
      if(o instanceof String[]) return (String[])o;
    }
    catch(Exception e)
    {
      return null;
    }
    return null;
  }

  public String toString()
  {
    String str="Event: " + eventAction + " (" + timestamp + ")\n";
    for (Param<?> param : params.values())
      str += param.toString() + "\n";
    return str;
  }

  public String toXml()
  {
    Document document=XmlUtils.newDocument();
    if(document == null) return null;

    // add basic information
    Element rootElement=document.createElement("event");
    rootElement.setAttribute("isTry", String.valueOf(tryEvent));
    rootElement.setAttribute("action", eventAction);
    rootElement.setAttribute("timestamp", String.valueOf(timestamp));
    document.appendChild(rootElement);

    // add parameters
    for(Param<?> pdpParameter : params.values())
    {
      try
      {
        Element parameter=document.createElement("parameter");
        parameter.setAttribute("name", pdpParameter.getName());
        parameter.setAttribute("type", Constants.PARAMETER_TYPE_NAMES[pdpParameter.getType()]);

        if(pdpParameter.getType() == Constants.PARAMETER_TYPE_BINARY)
        {
          Text cdata=document.createTextNode("CDATA_BEGIN" + pdpParameter.getValue() + "CDATA_END");
          parameter.appendChild(cdata);
        }
        else parameter.setAttribute("value", (String)pdpParameter.getValue());

        rootElement.appendChild(parameter);
      }
      catch(Exception e)
      {
        e.printStackTrace();
        return null;
      }
    }

    String s=XmlUtils.domSourceToString(new DOMSource(document));
    s=s.replace("CDATA_BEGIN", "<![CDATA[");
    s=s.replace("CDATA_END", "]]>");

    return s;
  }

}
