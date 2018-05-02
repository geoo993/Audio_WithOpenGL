//
//  main.cpp
//  OpenGL_DSP
//
//  Created by GEORGE QUENTIN on 27/02/2018.
//  Copyright © 2018 Geo Games. All rights reserved.
//

#include "Game.h"

int main(int argc, const char * argv[]) {

    // https://en.sfml-dev.org/forums/index.php?topic=1547.0
    // https://medium.com/@VIRAL/mac-xcode-and-c-file-handling-78ba999032df
    //    char path[1024];
    //    getcwd(path, sizeof(path) );
    //    puts(path);

    char * path = getcwd(NULL, 0); // Platform-dependent, see reference link below

    //path from project working direction, which is set in Edit Scheme -> Run -> Option -> Working Directory
    string filepath = string(path) + "/OpenGL_DSP/resources";

    std::cout << "Current dir: " << (path) << std::endl;
    std::cout << "full file path: " << (filepath) << std::endl;

    //start game
    Game game;
    game.Execute(filepath);



    return 0;
}
