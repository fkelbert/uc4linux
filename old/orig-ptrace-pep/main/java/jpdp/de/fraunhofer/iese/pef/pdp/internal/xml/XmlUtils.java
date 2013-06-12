package de.fraunhofer.iese.pef.pdp.internal.xml;

import java.io.StringWriter;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.w3c.dom.Document;
import org.w3c.dom.Element;

public class XmlUtils {

    /**
     * Helper function that adds a parameter to the the given element in the
     * given document
     * 
     * @param document - The document
     * @param element - The element the parameter will be added to
     * @param name - The name of the parameter
     * @param value - The value of the parameter
     */
    public static void addParameter(Document document, Element element, String name, String value) {
        if (document == null || element == null || name == null)
            throw new IllegalArgumentException("Element and document must not be null!");
        if (value == null)
            return;
        Element parameter = document.createElement("parameter");
        parameter.setAttribute("name", name);
        parameter.setAttribute("value", value);
        element.appendChild(parameter);
    }

    /**
     * Helper function that converts a DOMSource to a String
     * 
     * @param source The {@link DOMSource}
     * @return the giben {@link DOMSource} as a string
     */
    public static String domSourceToString(DOMSource source) {
        try {
            StringWriter writer = new StringWriter();
            StreamResult result = new StreamResult(writer);
            TransformerFactory tf = TransformerFactory.newInstance();
            Transformer transformer = tf.newTransformer();
            transformer.transform(source, result);
            return writer.toString();
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    /**
     * Helper function to create a new Document
     * 
     * @return a new {@link Document}
     */
    public static Document newDocument() {
        try {
            DocumentBuilderFactory documentBuilderFactory = DocumentBuilderFactory.newInstance();
            documentBuilderFactory.setNamespaceAware(true);
            DocumentBuilder documentBuilder;
            documentBuilder = documentBuilderFactory.newDocumentBuilder();
            Document document = documentBuilder.newDocument();
            document.setXmlStandalone(true);
            return document;
        } catch (ParserConfigurationException e) {
            return null;
        }
    }
    
}
