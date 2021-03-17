#import "ViewController.h"
#import <QuartzCore/CAMetalLayer.h>

#include "Engine.h"
#include <string>
#include <vector>

#pragma mark -
#pragma mark ViewController

@implementation ViewController {
    CVDisplayLinkRef displayLink;
    Engine* engine;
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
    args.push_back("-validate");
    engine = new Engine(args, self.view.layer);
    
    engine -> run();
    
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink, &DisplayLinkCallback, engine);
    CVDisplayLinkStart(displayLink);
    
}

#pragma mark Display loop callback function

/** Rendering loop callback function for use with a CVDisplayLink. */
static CVReturn DisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* engine) {
    ((Engine*) engine) -> update();
    ((Engine*) engine) -> render();
    
    return kCVReturnSuccess;
}

- (void) viewDidAppear {
    [super viewDidAppear];
    
    self.view.window.initialFirstResponder = self.view;
    
    self.view.window.title = [NSString stringWithUTF8String:engine->settings.applicationName.c_str()];
}


-(void) keyDown:(NSEvent*) theEvent {
    Engine::Key key;
    switch (theEvent.keyCode) {
        case 0:
            key = Engine::KEY_A;
            break;
        case 1:
            key = Engine::KEY_S;
            break;
        case 2:
            key = Engine::KEY_D;
            break;
        case 12:
            key = Engine::KEY_Q;
            break;
        case 13:
            key = Engine::KEY_W;
            break;
        case 14:
            key = Engine::KEY_E;
            break;
        case 49:
            key = Engine::KEY_SPACE;
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
