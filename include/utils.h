#ifndef UTILS_H
#define UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

// bypass dyld validation check
void init_bypassDyldLibValidation(void);

#ifdef __cplusplus
}
#endif

#endif