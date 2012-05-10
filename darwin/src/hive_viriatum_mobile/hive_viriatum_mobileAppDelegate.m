//
//  hive_viriatum_mobileAppDelegate.m
//  hive_viriatum_mobile
//
//  Created by João Magalhães on 12/17/11.
//  Copyright 2011 Hive Solutions Lda. All rights reserved.
//

#include "stdafx.h"

#import "hive_viriatum_mobileAppDelegate.h"

@implementation hive_viriatum_mobileAppDelegate

@synthesize window = _window;
@synthesize navigationController = _navigationController;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // allocates space for the command line arguments
    // these are going to be used in the starting of the service
    struct HashMap_t *arguments;
    
    // creates the hashmap used to represent the arguments
    // from the command line (started as empty)
    createHashMap(&arguments, 0);
    
    // Override point for customization after application launch.
    // Add the navigation controller's view to the window and display.
    self.window.rootViewController = self.navigationController;

    // start running the service (blocking call)
    runService(arguments);
    
    // deletes the arguments hash map (release memory)
    deleteHashMap(arguments);

    [self.window makeKeyAndVisible];
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
    /*
     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
     */
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    /*
     Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
     If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
     */
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    /*
     Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
     */
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    /*
     Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
     */
}

- (void)applicationWillTerminate:(UIApplication *)application {
    /*
     Called when the application is about to terminate.
     Save data if appropriate.
     See also applicationDidEnterBackground:.
     */
}

- (void)dealloc {
    [_window release];
    [_navigationController release];
    [super dealloc];
}

@end
