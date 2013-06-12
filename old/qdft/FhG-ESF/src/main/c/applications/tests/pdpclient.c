#include <stdio.h>
#include <axiom.h>
#include <axis2_util.h>
#include <axiom_soap.h>
#include <axis2_client.h>
#include "utilities/axiom_utils.h"
#include "logger/logger.h"
#include "log_pdpclient_c.h"

axiom_node_t *build_om_request(const axutil_env_t *env);
axiom_node_t *readPolicyFile(const axutil_env_t *env, char *filename);
const axis2_char_t *process_om_response(const axutil_env_t *env, axiom_node_t *node);

int main(int argc, char** argv)
{
  printf("starting pdp client test\n");
  const axutil_env_t *env = NULL;
  const axis2_char_t *address = NULL;
  axis2_endpoint_ref_t* endpoint_ref = NULL;
  axis2_options_t *options = NULL;
  const axis2_char_t *client_home = NULL;
  axis2_svc_client_t* svc_client = NULL;
  axiom_node_t *payload = NULL;
  axiom_node_t *ret_node = NULL;

  env = axutil_env_create_all("pdptest.log", AXIS2_LOG_LEVEL_TRACE);

  options = axis2_options_create(env);

  //address = "http://localhost/axis2/services/pdp";
  address = "http://localhost/axis2/services/pdp";
  //address = "http://172.16.44.128/axis2/services/pdp";
  if (argc > 1)
      address = argv[1];
  if (axutil_strcmp(address, "-h") == 0)
  {
      printf("Usage : %s [endpoint_url]\n", argv[0]);
      printf("use -h for help\n");
      return 0;
  }
  printf("Using endpoint : %s\n", address);
  endpoint_ref = axis2_endpoint_ref_create(env, address);
  axis2_options_set_to(options, env, endpoint_ref);

  client_home = AXIS2_GETENV("AXIS2C_HOME");
  if (!client_home && !strcmp(client_home, "")) client_home = "../..";

  svc_client = axis2_svc_client_create(env, client_home);
  if (!svc_client)
  {
      printf("Error creating service client\n");
      AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "Stub invoke FAILED: Error code:"
              " %d :: %s", env->error->error_number,
              AXIS2_ERROR_GET_MESSAGE(env->error));
      return -1;
  }

  axis2_svc_client_set_options(svc_client, env, options);

  payload = build_om_request(env);
  //char *filename="/home/raindrop/hiwi/prog/FhG-ESF/src/main/c/build/applications/tests/myPolicyX.xml";
  char *filename="/home/raindrop/hiwi/prog/FhG-ESF/src/main/c/build/applications/tests/myPolicy.xml";
  axiom_node_t *myMsg = NULL;
  myMsg = readPolicyFile(env, filename);
  if(myMsg==NULL) printf("error reading policy File\n");
  else printf("policyFile successfully read\n");
  //dumpAxiomNode(env, myMsg);

  printf("sending soap message\n");
  //printf("continue with usual service\n");
  //ret_node = axis2_svc_client_send_receive(svc_client, env, payload);
  ret_node = axis2_svc_client_send_receive(svc_client, env, myMsg);
  printf("soap message sent: %p\n", ret_node);

  if (ret_node)
  {
      const axis2_char_t *greeting = process_om_response(env, ret_node);
      if (greeting)
          printf("\nReceived answer from service: \"%s\"\n", greeting);

      axiom_node_free_tree(ret_node, env);
      ret_node = NULL;
  }
  else
  {
    AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI, "Stub invoke FAILED: Error code:"
            " %d :: %s", env->error->error_number,
            AXIS2_ERROR_GET_MESSAGE(env->error));
    printf("hello client invoke FAILED!\n");
  }

  if (svc_client)
  {
    axis2_svc_client_free(svc_client, env);
    svc_client = NULL;
  }

  if (env)
  {
    axutil_env_free((axutil_env_t *) env);
    env = NULL;
  }

  return 0;
}

axiom_node_t *readPolicyFile(const axutil_env_t *env, char *filename)
{
  axiom_node_t* polNode = NULL;
  axiom_xml_reader_t *xml_reader = NULL;
  axiom_stax_builder_t *om_builder = NULL;
  axiom_document_t *document = NULL;
  axiom_node_t* root_node = NULL;
  axiom_element_t *root_ele = NULL;
  axiom_xml_writer_t *xml_writer = NULL;
  axiom_output_t *om_output = NULL;
  axis2_char_t *buffer = NULL;

  //xml_reader = axiom_xml_reader_create_for_io(env, read_input_callback,close_input_callback, NULL, NULL);
  xml_reader = axiom_xml_reader_create_for_file(env, filename, NULL);
  if(!xml_reader) {printf("error creating xml_reader\n"); return NULL;}
  //else printf("xml reader successfully created\n");

  om_builder = axiom_stax_builder_create(env, xml_reader);
  if(!om_builder)
  {
      axiom_xml_reader_free(xml_reader, env);
      return AXIS2_FAILURE;
  }
  //printf("om_builder successfully created\n");
  document = axiom_stax_builder_get_document(om_builder, env);
  if(!document)
  {
       axiom_stax_builder_free(om_builder, env);
       return AXIS2_FAILURE;
  }
  //printf("document successfully extracted\n");
  root_node = axiom_document_get_root_element(document, env);
  if(!root_node)
  {
      axiom_stax_builder_free(om_builder, env);
       return AXIS2_FAILURE;
  }
  //else printf("root node extraced!\n");

  if(root_node)
  {
    if(axiom_node_get_node_type(root_node, env) == AXIOM_ELEMENT)
    {
      root_ele = (axiom_element_t*)axiom_node_get_data_element(root_node, env);
      if(root_ele)
      {
        printf(" %s" ,axiom_element_get_localname(root_ele, env));
      }
    }
  }
  printf("\n");
  //else printf("root_node=NULL?!\n");

  axiom_document_build_all(document, env);
  axiom_document_build_all(document, env);

  xml_writer = axiom_xml_writer_create_for_memory(env, NULL, AXIS2_TRUE, 0, AXIS2_XML_PARSER_TYPE_BUFFER);

  om_output = axiom_output_create(env, xml_writer);

  axiom_node_serialize(root_node, env, om_output);

  buffer = (axis2_char_t*)axiom_xml_writer_get_xml(xml_writer, env);

  polNode=root_node;
  return polNode;
}

axiom_node_t *build_om_request(const axutil_env_t *env)
{
  axiom_node_t* greet_om_node = NULL;
  axiom_element_t * greet_om_ele = NULL;

  greet_om_ele = axiom_element_create(env, NULL, "greet", NULL, &greet_om_node);
  axiom_element_set_text(greet_om_ele, env, "Hello Server!", greet_om_node);

  return greet_om_node;
}

const axis2_char_t *process_om_response(const axutil_env_t *env, axiom_node_t *node)
{
  printf("processing response!\n\n\n");
  axiom_node_t *service_greeting_node = NULL;
  axiom_node_t *return_node = NULL;

  if(node)
  {
      axiom_stax_builder_t *om_builder = NULL;
  axiom_xml_writer_t *xml_writer = NULL;
  axiom_output_t *om_output = NULL;
  axis2_char_t *buffer = NULL;

  xml_writer = axiom_xml_writer_create_for_memory(env, NULL, AXIS2_TRUE, 0, AXIS2_XML_PARSER_TYPE_BUFFER);
  om_output = axiom_output_create(env, xml_writer);
  axiom_node_serialize(node, env, om_output);
  buffer = (axis2_char_t*)axiom_xml_writer_get_xml(xml_writer, env);


  printf("%s ", buffer);

    service_greeting_node = axiom_node_get_first_child(node, env);
    if (service_greeting_node && axiom_node_get_node_type(service_greeting_node, env) == AXIOM_TEXT)
    {
      axiom_text_t *greeting = (axiom_text_t *)axiom_node_get_data_element(service_greeting_node, env);
      if (greeting && axiom_text_get_value(greeting , env))
      {
          return axiom_text_get_value(greeting, env);
      }
    }
  }
  return NULL;
}
