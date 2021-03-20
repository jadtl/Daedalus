#import "ViewController.h"

#include "Engine.h"

#pragma mark -
#pragma mark ViewController

@implementation ViewController {
    CADisplayLink* displayLink;
    Engine* engine;
}

-(void) dealloc {
    delete engine;
    
    [displayLink release];
    [super dealloc];
}

/** Since this is a single-view app, init Vulkan when the view is loaded. */
-(void) viewDidLoad {
    [super viewDidLoad];
    
    self.view.contentScaleFactor = UIScreen.mainScreen.nativeScale;

    std::vector<std::string> args;
    //args.push_back("-validate");
    engine = new Engine(args, (__bridge void*)self.view.layer);
    engine -> initialize();

    uint32_t fps = 60;
    displayLink = [CADisplayLink displayLinkWithTarget: self selector: @selector(renderLoop)];
    [displayLink setFrameInterval: 60 / fps];
    [displayLink addToRunLoop: NSRunLoop.currentRunLoop forMode: NSDefaultRunLoopMode];
}

-(void) renderLoop { engine -> update(); engine -> render(); }

@end

#pragma mark -
#pragma mark View

@implementation View

/** Returns a Metal-compatible layer. */
+(Class) layerClass { return [CAMetalLayer class]; }

@end
