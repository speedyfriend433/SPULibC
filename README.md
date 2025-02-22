# SPULibC

SPULibC is a powerful iOS framework that provides dynamic runtime manipulation capabilities, primarily focusing on dynamic library loading utilities. The framework uniquely supports both dynamic and static library formats, maintaining full functionality regardless of the library extension used.

## Features

- Dynamic library loading and manipulation
- Runtime method swizzling utilities
- Platform validation checks
- Flexible library format support (both dynamic and static)
- No JIT or jailbreak requirements


## Requirements

- iOS 11.0+
- Xcode 12.0+

## Installation

### Using CMake

1. Clone the repository:
```bash
git clone https://github.com/speedyfriend433/spulibc.git
cd spulibc
```

2. Build the framework:
```bash
mkdir build && cd build
cmake ..
make
```

3. The framework will be available in the `build/lib` directory as both dynamic and static libraries

## Usage

### Library Integration

SPULibC can be integrated into your project either as a dynamic or static library. The framework maintains full functionality regardless of the integration method chosen:

```objective-c
// for dynamic framework
#import <spulibc/spulibc.h>

// for static library
#import "spulibc.h"
```

### Example: Implementing Automatic Alert Dismissal

Here's an example of how you can use SPULibC's runtime manipulation capabilities to implement automatic UIAlertController dismissal:

```objective-c
#import <UIKit/UIKit.h>
#import <spulibc/spulibc.h>

// example implementation of automatic alert dismissal using runtime manipulation
UIAlertController *alert = [UIAlertController alertControllerWithTitle:@"Title"
                                                             message:@"Message"
                                                      preferredStyle:UIAlertControllerStyleAlert];
[self presentViewController:alert animated:YES completion:nil];
```

### Dynamic Library Loading

SPULibC includes utilities for dynamic library manipulation:

```objective-c
#import <spulibc/dyld.h>

// load dynamic library
void* handle = SPUDyldLoadLibrary("/path/to/library.dylib");
if (handle) {
    // Get a symbol from the loaded library
    void* symbol = SPUDyldGetSymbol(handle, "function_name");
    if (symbol) {
        // Cast and use the symbol
        void (*function)() = (void (*)())symbol;
        function();
    } else {
        const char* error = SPUDyldGetLastError();
        NSLog(@"Failed to get symbol: %s", error);
    }
    
    // Unload the library when done
    SPUDyldUnloadLibrary(handle);
} else {
    const char* error = SPUDyldGetLastError();
    NSLog(@"Failed to load library: %s", error);
}
```

## Configuration

### Entitlements

SPULibC is designed to work within Apple's security boundaries and does not require JIT or jailbreak-specific entitlements. The basic entitlements needed are:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>application-identifier</key>
    <string>$(AppIdentifierPrefix)com.spulibc</string>
</dict>
</plist>
```

## Credits

SPULibC incorporates code from the following contributors:

- **opa334**: Dynamic library loading and symbol resolution utilities (dyld.m)
- **Duy Tran**: Mach-O platform patching and validation check bypass functionality (dyld_patch_platform.m, dyld_validation_check.m)

## License

This project is licensed under the MIT License - see the LICENSE file for details.

```
## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.