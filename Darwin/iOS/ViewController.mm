#import "ViewController.h"

#include "ShellDarwin.h"
#include "Daedalus.h"

#pragma mark -
#pragma mark ViewController

@implementation ViewController {
    
    CADisplayLink* _displayLink;
    ShellDarwin* _shell;
    Engine* _engine;
    
}

-(void) dealloc {
    
    delete _shell;
    delete _engine;
    
    [_displayLink release];
    [super dealloc];
    
}

/** Since this is a single-view app, init Vulkan when the view is loaded. */
-(void) viewDidLoad {
    
    [super viewDidLoad];

    self.view.contentScaleFactor = UIScreen.mainScreen.nativeScale;

    std::vector<std::string> args;
    args.push_back("-v");
    _engine = new Daedalus(args);

    _shell = new ShellDarwin(*_engine);
    _shell->run(self.view.layer);

    uint32_t fps = 60;
    _displayLink = [CADisplayLink displayLinkWithTarget: self selector: @selector(renderLoop)];
    [_displayLink setFrameInterval: 60 / fps];
    [_displayLink addToRunLoop: NSRunLoop.currentRunLoop forMode: NSDefaultRunLoopMode];
    
}

-(void) renderLoop {
    
    _shell->updateAndDraw();
    
}

@end

#pragma mark -
#pragma mark View

@implementation View

/** Returns a Metal-compatible layer. */
+(Class) layerClass { return [CAMetalLayer class]; }

@end
