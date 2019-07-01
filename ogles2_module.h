#ifndef OGLES2_MODULE_H
#define OGLES2_MODULE_H

void ogles2_install_patches(void);
void ogles2_remove_patches(void);
void ogles2_free(void);

const char* ogles2_version_string(void);
const char* ogles2_errors_string(void);

#endif
