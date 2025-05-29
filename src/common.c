#include "common.h"

#include <proto/dos.h>

#include <stdio.h>
#include <string.h>

void find_process_name2(struct Node * node, char * destination)
{
    char buffer[NAME_LEN / 2] = { 0 };

    if (node->ln_Type == NT_PROCESS) {
        IDOS->GetCliProgramName(buffer, sizeof(buffer));
    }

    if (strlen(buffer) > 0) {
        snprintf(destination, NAME_LEN, "%s '%s'", node->ln_Name, buffer);
    } else {
        snprintf(destination, NAME_LEN, "%s", node->ln_Name);
    }
}
