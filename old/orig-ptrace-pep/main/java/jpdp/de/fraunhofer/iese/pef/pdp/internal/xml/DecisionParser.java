
package de.fraunhofer.iese.pef.pdp.internal.xml;

import de.fraunhofer.iese.pef.pdp.internal.AuthorizationAction;
import de.fraunhofer.iese.pef.pdp.internal.Constants;
import de.fraunhofer.iese.pef.pdp.internal.Decision;
import de.fraunhofer.iese.pef.pdp.internal.ExecuteAction;
import de.fraunhofer.iese.pef.pdp.internal.Param;

import org.xml.sax.Attributes;
import org.xml.sax.ContentHandler;
import org.xml.sax.Locator;
import org.xml.sax.SAXException;

import java.util.Hashtable;

/**
 * DocumentHandler to parse the response of the PDP. Creates a PdpResponse
 * object
 */
public class DecisionParser implements ContentHandler {

    private Decision response = null;

    String fallbackDefault = null;

    private Hashtable<String, AuthorizationAction> authorizationActions = new Hashtable<String, AuthorizationAction>();

    private Hashtable<String, String> fallbacks = new Hashtable<String, String>();

    AuthorizationAction authorizationActionTmp = null;

    private String fallbackTmp = null;

    private String tmp = "";

    private ExecuteAction executeActionTmp = null;

    private boolean isDefaultTmp = false;

    public DecisionParser(Decision response) 
    {
        super();
        this.response = response;
    }

    public Decision getResponse() 
    {
        return response;
    }

    /**
     * called at the beginning of each element
     */
    public void startElement(String uri, String localName, String qName, Attributes atts) throws SAXException 
    {
        if (localName.equals("authorizationAction")) 
        {
            parseAuthorizationAction(uri, localName, qName, atts);
        } 
        else if (localName.equals("allow")) {
            authorizationActionTmp.setType(Constants.AUTHORIZATION_ALLOW);
        } 
        else if (localName.equals("inhibit")) {
            authorizationActionTmp.setType(Constants.AUTHORIZATION_INHIBIT);
        }
        else if (localName.equals("modify")) {
            tmp = "modify";
        }
        else if (localName.equals("delay")) {
            parseDelay(uri, localName, qName, atts);
        }
        else if (localName.equals("action")) {
            parseExecuteAction(uri, localName, qName, atts);
        }
        else if (localName.equals("parameter")) {
            parseParameter(uri, localName, qName, atts);
        }

    }

    private void parseParameter(String uri, String localName, String qName, Attributes atts) {
        try {
            String name = atts.getValue("name");
            String value = atts.getValue("value");
            int intType = Param.getIdForName(atts.getValue("type"));

            @SuppressWarnings({
                    "unchecked", "rawtypes"
            })
            Param<?> param = new Param(name,
                    value, intType);
            if (tmp.equals("modify")) {
                authorizationActionTmp.addModifier(param);
            } else if (tmp.equals("action") && executeActionTmp != null) {
                executeActionTmp.addParameter(param);
            }
        } catch (Exception e) {
            throw new IllegalArgumentException("Execute action malformed");
        }
    }

    private void parseExecuteAction(String uri, String localName, String qName, Attributes atts) {
        tmp = "action";
        executeActionTmp = new ExecuteAction();
        try {
            executeActionTmp.setName(atts.getValue("name"));
        } catch (Exception e) {
            throw new IllegalArgumentException("Execute action malformed");
        }

    }

    private void parseDelay(String uri, String localName, String qName, Attributes atts) {
        authorizationActionTmp.setDelay(Integer.parseInt(atts.getValue("amount")));
        String unit = atts.getValue("unit");
        if (unit == null || unit.equalsIgnoreCase("TIMESTEP")) {
            authorizationActionTmp.setDelayUnit(Constants.TIMESTEP);
        } else if (unit.equalsIgnoreCase("NANOSECOND")) {
            throw new IllegalArgumentException("Nanoseconds currently not supported");
        } else if (unit.equalsIgnoreCase("MICROSECOND")) {
            authorizationActionTmp.setDelayUnit(Constants.MICROSECOND);
        } else if (unit.equalsIgnoreCase("MILLISECOND")) {
            authorizationActionTmp.setDelayUnit(Constants.MILLISECOND);
        } else if (unit.equalsIgnoreCase("SECOND")) {
            authorizationActionTmp.setDelayUnit(Constants.SECOND);
        } else if (unit.equalsIgnoreCase("MINUTE")) {
            authorizationActionTmp.setDelayUnit(Constants.MINUTE);
        } else if (unit.equalsIgnoreCase("HOUR")) {
            authorizationActionTmp.setDelayUnit(Constants.HOUR);
        } else if (unit.equalsIgnoreCase("DAY")) {
            authorizationActionTmp.setDelayUnit(Constants.DAY);
        } else if (unit.equalsIgnoreCase("WEEK")) {
            authorizationActionTmp.setDelayUnit(Constants.WEEK);
        } else if (unit.equalsIgnoreCase("MONTH")) {
            authorizationActionTmp.setDelayUnit(Constants.MONTH);
        } else if (unit.equalsIgnoreCase("YEAR")) {
            authorizationActionTmp.setDelayUnit(Constants.YEAR);
        }
    }

    private void parseAuthorizationAction(String uri, String localName, String qName,
            Attributes atts) {
        authorizationActionTmp = new AuthorizationAction();
        try {
            authorizationActionTmp.setName(atts.getValue("name"));
        } catch (Exception e) {
            throw new IllegalArgumentException("Authorization action requires name");
        }

        String type = atts.getValue("type");
        if (type == null) {
            type = "";
        }
        if (type.equals("default")) {
            isDefaultTmp = true;
        } else {
            isDefaultTmp = false;
        }

        try {
            fallbackTmp = atts.getValue("fallback");
            if (fallbackTmp == null) {
                fallbackTmp = "inhibit";
            }

        } catch (Exception e) {
            throw new IllegalArgumentException("Authorization action malformed");
        }

    }

    /**
     * called at the end of each element
     */
    public void endElement(String uri, String localName, String qName) throws SAXException {
        if (localName.equals("authorizationAction")) {
            if (isDefaultTmp) {
                response.setAuthorizationAction(authorizationActionTmp);
                fallbackDefault = fallbackTmp;
            } else {
                authorizationActions.put(authorizationActionTmp.getName(),
                        authorizationActionTmp);
                fallbacks.put(authorizationActionTmp.getName(), fallbackTmp);
            }
            authorizationActionTmp = null;
        } else if (localName.equals("modify")) {
            tmp = "";
        } else if (localName.equals("action")) {

            if (authorizationActionTmp != null) {
                authorizationActionTmp.addExecuteAction(executeActionTmp);
            } else {
                response.addExecuteAction(executeActionTmp);
            }

            tmp = "";
            executeActionTmp = null;
        }
    }

    public void endDocument() throws SAXException {
        if (response.getAuthorizationAction() != null && !fallbackDefault.equalsIgnoreCase("inhibit")) {
            if (!authorizationActions.containsKey(fallbackDefault)) {
                throw new IllegalArgumentException("Fallback authorization action not found");
            }
            AuthorizationAction def = authorizationActions.get(fallbackDefault);
            response.getAuthorizationAction().setFallback(def);
            setAuthorizationActionFallbackRecursive(def);
        }
    }

    private void setAuthorizationActionFallbackRecursive(AuthorizationAction authorizationAction) {
        String fallbackName = fallbacks.get(authorizationAction.getName());
        if (fallbackName == null || fallbackName.equalsIgnoreCase("inhibit")) {
            return;
        }
        AuthorizationAction fallbackRef = authorizationActions.get(fallbackName);
        authorizationAction.setFallback(fallbackRef);
        setAuthorizationActionFallbackRecursive(fallbackRef);

    }

    public void endPrefixMapping(String prefix) throws SAXException {
    }

    public void ignorableWhitespace(char[] ch, int start, int length) throws SAXException {
    }

    public void processingInstruction(String target, String data) throws SAXException {
    }

    public void setDocumentLocator(Locator locator) {
    }

    public void skippedEntity(String name) throws SAXException {
    }

    public void startDocument() throws SAXException {
    }

    public void startPrefixMapping(String prefix, String uri) throws SAXException {
    }

    public void characters(char[] ch, int start, int length) throws SAXException {
    }

}
