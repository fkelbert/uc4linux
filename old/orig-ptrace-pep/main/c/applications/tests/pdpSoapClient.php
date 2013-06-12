<?php
  echo "Start<br>";
  error_reporting(E_ALL);
  $client = new SoapClient(NULL, 
                           array( 
                           "location" => "http://localhost:2345/pdp",
                           "uri" => "urn:pdp",
                           "style" => SOAP_RPC,   // can omit this
                           "use" => SOAP_LITERAL, // can omit this
                           "trace" => 1)
                          );
  //var_dump($client);
  ob_start();?>
<policySet
	xmlns="http://iese.fhg.de/policy"
	xmlns:e="http://iese.fhg.de/event"
	xmlns:mc="http://www.master-fp7.eu/mcalculus.xsd"
	xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
	xsi:schemaLocation="http://iese.fhg.de/policy enforcementLanguage-FhG.xsd http://www.master-fp7.eu/mcalculus.xsd mcalculus.xsd">

<!-- Observation mechanism evaluates formula, when formula is violated (is false) then actions are executed -->

<!-- Control mechanism checks for trigger, if condition is true, then enforcement actions are executed -->
<preventiveMechanism name="ExampleControl">
  <description>
    Example of control mechanism
  </description>
  <timestepSize interval="2" timeUnit="second"/>
  <trigger matchName="logout">
  </trigger>
  <condition>
    <before interval="5">
      <eAll matchName="login">
        <matchParameter name="user"
                        value="trigger/parameter['user']" 
                        isXPathExpression="true"/>
      </eAll>
    </before>
  </condition>
  <allow/>
  <execute>
    <action name="log" targetComponent="self">
      <parameter name="message" value="Error"/>
    </action>
    <action name="soapCall" targetComponent="self">
      <parameter name="url" value="http://localhost/log"/>
      <parameter name="message" value="Error"/>
    </action>
  </execute>
</preventiveMechanism>  
</policySet>
  <?php
  
  $myPolicy=ob_get_contents();
  ob_end_clean();
  $myPolicy=new SoapVar($myPolicy, XSD_ANYXML);
  $parameters=array($myPolicy);
  try
  {    
      $result = $client->__call("deployPolicy",$parameters,array("uri" => "urn:pdp","soapaction" => "urn:pdp#deployPolicy")); // soapaction not necessary
  }
  catch(Exception $e)
  {
    echo "\nDumping request headers:\n" .$client->__getLastRequestHeaders();
    echo "<br><br>";
    echo("\nDumping request:\n".$client->__getLastRequest());
    echo "<br><br>";
    echo("\nDumping response headers:\n".$client->__getLastResponseHeaders());
    echo "<br><br>";
    echo("\nDumping response:\n".$client->__getLastResponse());
    echo "<br>";
  }
  echo "Finished: $result<br>";

?>
