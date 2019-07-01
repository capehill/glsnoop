#ifndef WARP3DNOVA_MODULE_H
#define WARP3DNOVA_MODULE_H

void warp3dnova_install_patches(void);
void warp3dnova_remove_patches(void);
void warp3dnova_free(void);

const char* warp3dnova_version_string(void);
const char* warp3dnova_errors_string(void);

#endif

