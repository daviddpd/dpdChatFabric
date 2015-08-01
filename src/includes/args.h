#include <getopt.h>
#include "cfstructs.h"
#include "dpdChatFabric.h"

#ifndef CF_ARGS_H
#define CF_ARGS_H

void chatFabric_usage(char *p);

void chatFabric_args(int argc, char**argv, chatFabricConfig *config, chatFabricAction *a);


#endif