/**
 * @file
 * @brief
 *
 * @author Ricardo Neisse
 **/

// see identity EMF model

#include "identity.h"

typedef struct entity_s {
  unsigned char *name;
  digital_identity_ptr *identities;
} entity_t;
typedef entity_t *entity_ptr;



// see action EMF model

#include "entity.h"
#include <glib/ghash.h>


// The classes of actions indicate the type of resource
// ++ Usage actions are operations in data, they change the state of the data in the system.
// ++ Signaling actions are data transfer operations, they represent communication outside the
// entity boundaries.
// ++ Other actions? well, they are other actions.
// How do we exactly model the system??




//
// An action intent is a description of
// an operation to be performed. It references
// an action description and the parameters of
// the intent.
//
typedef struct action_intent_s {
    action_desc_ptr *action_desc;
    action_intent_param_ptr* params;
    unsigned int n_params;
    entity_ptr subject;
} action_intent_t;
typedef action_intent_t *action_intent_ptr;

//
// The parameter of an action intent. It references
// an action parameter and assigns a value to the
// parameter for the specific intent. The parameter
// value might be an xpath expression that should be
// resolved when intent is carried on (resolved by "type");
//
typedef struct action_intent_param_s {
  action_param_desc_ptr *action_param_desc;
  unsigned char *value;
  unsigned char *type; // type might be "xpath"
} param_desc_t;
typedef action_intent_param_t *action_intent_param_ptr;

// functions are related to
// action_desc_store
action_desc_store_new(unsigned int nsize); // zero for default initial size
action_desc_store_add(action_desc_ptr action_desc);
action_desc_store_lookup(char *name);


// action_intent
action_intent_new(action_desc_ptr);
// this function checks if param name and value are in the action description,
// if not the param is added
action_intent_add_param(char *name, char *value, bool is_xpath);

// no functions related to param param_desc, etc.





