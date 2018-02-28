//
//  main.cpp
//  OpenGL_DSP
//
//  Created by GEORGE QUENTIN on 27/02/2018.
//  Copyright © 2018 Geo Games. All rights reserved.
//

#include "Game.h"

int main(int argc, const char * argv[]) {

    srand ( time(nullptr) );
    //srand(glfwGetTime()); // initialize random seed

    // https://en.sfml-dev.org/forums/index.php?topic=1547.0
    // https://medium.com/@VIRAL/mac-xcode-and-c-file-handling-78ba999032df
    char path[1024];
    getcwd(path, sizeof(path) );
    puts(path);
    
    //path from project working direction, which is set in Edit Scheme -> Run -> Option -> Working Directory
    string filepath = string(path) + "/OpenGL_DSP/resources";

    //std::cout << (filepath) << std::endl;
    //std::cout << (path) << std::endl;

    //start game
    Game game;
    game.Execute(filepath);


    return 0;
}
