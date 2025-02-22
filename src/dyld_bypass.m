#import <Foundation/Foundation.h>
#include "../include/spulibc.h"
#include "../include/dyld_validation_check.m"

void SPUInitializeDyldBypass(void) {
    init_bypassDyldLibValidation();
}