#import <AppKit/AppKit.h>

#include "Engine.h"

#pragma mark -
#pragma mark ViewController

@interface ViewController : NSViewController {
    CVDisplayLinkRef displayLink;
    Engine* engine;
}
@end

#pragma mark -
#pragma mark View

/** metal compatible view for storyboard */
@interface View : NSView
@end
