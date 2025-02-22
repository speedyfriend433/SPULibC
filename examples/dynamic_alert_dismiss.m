// example of auto dismissing UIAlertViewController 
#import <UIKit/UIKit.h>
#import <objc/runtime.h>
#import <Foundation/Foundation.h>

__attribute__((visibility("default")))
static void swizzleMethod(Class class, SEL originalSelector, SEL swizzledSelector) {
    Method originalMethod = class_getInstanceMethod(class, originalSelector);
    Method swizzledMethod = class_getInstanceMethod(class, swizzledSelector);

    BOOL didAddMethod = class_addMethod(class, originalSelector,
        method_getImplementation(swizzledMethod),
        method_getTypeEncoding(swizzledMethod));

    if (didAddMethod) {
        class_replaceMethod(class, swizzledSelector,
            method_getImplementation(originalMethod),
            method_getTypeEncoding(originalMethod));
    } else {
        method_exchangeImplementations(originalMethod, swizzledMethod);
    }
}

__attribute__((visibility("default")))
@implementation UIViewController (DynamicAlertDismiss)

+ (void)load {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        Class class = [self class];
        SEL originalSelector = @selector(presentViewController:animated:completion:);
        SEL swizzledSelector = @selector(swizzled_presentViewController:animated:completion:);
        swizzleMethod(class, originalSelector, swizzledSelector);
    });
}

- (void)swizzled_presentViewController:(UIViewController *)viewControllerToPresent
                            animated:(BOOL)flag
                          completion:(void (^)(void))completion {
    if ([viewControllerToPresent isKindOfClass:[UIAlertController class]]) {
        NSLog(@"Dismissing alert automatically");
        return;
    }
    [self swizzled_presentViewController:viewControllerToPresent animated:flag completion:completion];
}

@end