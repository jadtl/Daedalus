#import "ViewController.h"
#import <QuartzCore/CAMetalLayer.h>

#include "Daedalus.h"
#include "ExplorerDarwin.h"

#include <string>
#include <vector>

#pragma mark -
#pragma mark ViewController

@implementation ViewController {
    CVDisplayLinkRef displayLink;
    Daedalus* engine;
    ExplorerDarwin* explorer;
}

- (void) dealloc { 
    delete engine;
    CVDisplayLinkRelease(displayLink);
    
    [super dealloc];
}

- (void) viewDidLoad {
    [super viewDidLoad];
    
    self.view.wantsLayer = YES;
    
    std::vector<std::string> args;
    args.push_back("-verbose");
    engine = new Daedalus("Daedalus [Vulkan]", args, (__bridge void*)self.view.layer);
    explorer = new ExplorerDarwin("../Daedalus");
    engine->explorer = explorer;
    engine -> initialize();
    
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink, &DisplayLinkCallback, (__bridge void *)engine);
    CVDisplayLinkStart(displayLink);

}

#pragma mark Display loop callback function

/** Rendering loop callback function for use with a CVDisplayLink. */
static CVReturn DisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* engine) {
    ((Daedalus*) engine)->update();
    
    return kCVReturnSuccess;
}

- (void) viewDidAppear {
    self.view.window.initialFirstResponder = self.view;
    
    self.view.window.title = [NSString stringWithUTF8String:engine->settings().applicationName];
}


-(void) keyDown:(NSEvent*) theEvent {
    Daedalus::Key key;
    switch (theEvent.keyCode) {
        case 0:
            key = Daedalus::KEY_A;
            break;
        case 1:
            key = Daedalus::KEY_S;
            break;
        case 2:
            key = Daedalus::KEY_D;
            break;
        case 12:
            key = Daedalus::KEY_Q;
            break;
        case 13:
            key = Daedalus::KEY_W;
            break;
        case 14:
            key = Daedalus::KEY_E;
            break;
        case 49:
            key = Daedalus::KEY_SPACE;
            break;
    }
    engine -> onKey(key);
}

@end

#pragma mark -
#pragma mark View

@implementation View

/** Indicates that the view wants to draw using the backing layer instead of using drawRect:.  */
-(BOOL) wantsUpdateLayer { return YES; }

/** Returns a Metal-compatible layer. */
+(Class) layerClass { return [CAMetalLayer class]; }

/** If the wantsLayer property is set to YES, this method will be invoked to return a layer instance. */
-(CALayer*) makeBackingLayer {
    CALayer* layer = [self.class.layerClass layer];
    CGSize viewScale = [self convertSizeToBacking: CGSizeMake(1.0, 1.0)];
    layer.contentsScale = MIN(viewScale.width, viewScale.height);
    return layer;
}

-(BOOL) acceptsFirstResponder { return YES; }

@end
