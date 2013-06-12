
package de.fraunhofer.iese.pef.pdp.internal.xml;

import de.fraunhofer.iese.pef.pdp.internal.Constants;
import de.fraunhofer.iese.pef.pdp.internal.Event;
import de.fraunhofer.iese.pef.pdp.internal.Param;

import org.xml.sax.Attributes;
import org.xml.sax.ContentHandler;
import org.xml.sax.Locator;
import org.xml.sax.SAXException;

import java.util.Arrays;

/**
 * DocumentHandler to parse the response of the PDP. Creates a PdpResponse
 * object
 */
public class EventParser implements ContentHandler {

    private Event event = null;

    Param<?> parameterTmp = null;

    public EventParser(Event event) {
        super();
        this.event = event;
    }

    public void startElement(String uri, String localName, String qName, Attributes atts)
            throws SAXException, IllegalArgumentException {
        if (localName.equals("event")) {
            parseEvent(uri, localName, qName, atts);
        } else if (localName.equals("parameter")) {
            parseEventParameter(uri, localName, qName, atts);
        } else {
            throw new IllegalArgumentException("Event contains unsupported nodes");
        }
    }

    @SuppressWarnings({
            "unchecked", "rawtypes"
    })
    private void parseEventParameter(String uri, String localName, String qName, Attributes atts) {
        try {
            String name = atts.getValue("name");
            int intType = Param.getIdForName(atts.getValue("type"));
            String value = null;
            if (intType == Constants.PARAMETER_TYPE_BINARY) {
                parameterTmp = new Param(name, "", intType);
            } else {
                value = atts.getValue("value");
                Param<?> parameter = new Param(
                        name, value, intType);
                event.addParam(parameter);
            }

        } catch (Exception e) {
            throw new IllegalArgumentException("Event parameter malformed");
        }

    }

    private void parseEvent(String uri, String localName, String qName, Attributes atts) {
        try {
            event.setEventAction(atts.getValue("action"));
        } catch (Exception e) {
            throw new IllegalArgumentException("Event action name is required");
        }

        try {
            event.setTryEvent(Boolean.parseBoolean(atts.getValue("isTry")));
        } catch (Exception e) {
            event.setTryEvent(false);
        }

        try {
            event.setTimestamp(Long.parseLong(atts.getValue("timestamp")));
        } catch (Exception e) {
            event.setTimestamp(System.currentTimeMillis());
        }
    }

    public void endElement(String uri, String localName, String qName) throws SAXException {
    }

    public void endDocument() throws SAXException {
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

    @SuppressWarnings("unchecked")
    public void characters(char[] ch, int start, int length) throws SAXException {
        if (parameterTmp != null && parameterTmp.getType()<=Constants.PARAMETER_TYPE_BINARY) {
            ((Param<String>)parameterTmp).setValue(new String(Arrays.copyOfRange(ch, start, start+length)));
            event.addParam(parameterTmp);
        }
        parameterTmp = null;
    }

}
