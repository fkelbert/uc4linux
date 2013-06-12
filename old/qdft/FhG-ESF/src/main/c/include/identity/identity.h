/**
 * @file
 * @brief
 *
 * @author Ricardo Neisse
 **/

// see identity EMF model

#include "entity.h"

typedef struct digital_identity_s {
  entity_ptr *owner;
  identity_attribute_ptr *attributes;
} digital_identity_t;
typedef digital_identity_t *digital_identity_ptr;

typedef struct identity_attribute_s {
  unsigned char *name;
  unsigned char *value;
} identity_attribute_t;
typedef identity_attribute_t *identity_attribute_ptr;
