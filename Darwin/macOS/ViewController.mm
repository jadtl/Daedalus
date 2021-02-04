#import "ViewController.h"
#import <QuartzCore/CAMetalLayer.h>

#include "ShellDarwin.h"
#include "Daedalus.h"

#pragma mark -
#pragma mark ViewController

@implementation ViewController {
    
    CVDisplayLinkRef _displayLink;
    ShellDarwin* _shell;
    Engine* _engine;
    
}

- (void) dealloc {
    
    delete _shell;
    delete _engine;
    CVDisplayLinkRelease(_displayLink);
    
    [super dealloc];
    
}

- (void) viewDidLoad {
    
    [super viewDidLoad];
    
    self.view.wantsLayer = YES;
    
    std::vector<std::string> args;
    args.push_back("-v");
    _engine = new Daedalus(args);
    
    _shell = new ShellDarwin(*_engine);
    _shell -> run(self.view.layer);
    
    CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
    CVDisplayLinkSetOutputCallback(_displayLink, &DisplayLinkCallback, _shell);
    CVDisplayLinkStart(_displayLink);
    
}

#pragma mark Display loop callback function

/** Rendering loop callback function for use with a CVDisplayLink. */
static CVReturn DisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now,
                                    const CVTimeStamp* outputTime, CVOptionFlags flagsIn,
                                    CVOptionFlags* flagsOut, void* target) {
    
   ((ShellDarwin*) target) -> update_and_draw();
    
    return kCVReturnSuccess;
    
}

- (void) viewDidAppear {
    
    [super viewDidAppear];
    
    self.view.window.initialFirstResponder = self.view;
    
    self.view.window.title = @"Daedalus";
    
}


-(void) keyDown:(NSEvent*) theEvent {
    Engine::Key key;
    switch (theEvent.keyCode) {
        case 53:
            key = Engine::KEY_ESC;
            break;
        case 126:
            key = Engine::KEY_UP;
            break;
        case 125:
            key = Engine::KEY_DOWN;
            break;
        case 49:
            key = Engine::KEY_SPACE;
            break;
        case 3:
            key = Engine::KEY_F;
            break;
        default:
            key = Engine::KEY_UNKNOWN;
            break;
    }

    _engine->on_key(key);
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
