#include "definitions.h"
#include "variables.h"

/*
 * array to map the public filedescripter
 * to the private filedescripter and store
 * the filename
 */
struct fd_table_entry fd_table[ AMOUNT_OF_FDS ];

/*
 * define files that are container-files
 * REMARK: this list can be assumed as given
 */
const int AMOUNT_OF_CONTAINERS = 3;
char *container_file_list[ 3 ] =
{
	"./container",
	"./container_new",
	"/tmp/container"
};

