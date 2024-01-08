//// #include "player.h"
//#include <GLFW\glfw3.h>
//#include <iostream>
//#include <unordered_map>
//#include "player.h"
//
//class Inputhandler
//{
//public:
//    
//    Inputhandler(int& playerID_, std::unordered_map<uint32_t, Player>& players_)
//        : playerID(playerID_), players(players_)
//    {
//
//    }
//    
//    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
//    {
//        if (key == GLFW_KEY_W && action == GLFW_PRESS)
//        {
//            std::cout << "Hej";
//        }
//        if (key == GLFW_KEY_A && action == GLFW_PRESS)
//        {
//            players[playerID].SetDirection(Direction::Left);
//        }
//        if (key == GLFW_KEY_S && action == GLFW_PRESS)
//        {
//
//        }
//        if (key == GLFW_KEY_D && action == GLFW_PRESS)
//        {
//
//        }
//    }
//
//    int& playerID;
//    std::unordered_map<uint32_t, Player>& players;
//};
