#include "AudioFile.h"
#include "delay.h"
#include <filesystem>

enum State
{
    ST_IDLE,
    ST_READ,
    ST_PROCESS,
    ST_PLAY,
    ST_EXIT
};

bool *hasPlayed = new bool(false);
bool *hasProcessed = new bool(false);

void idleState(State *ptr)
{
    std::string *cmd = new std::string;
    std::cout << "A simple stereo delay effect." << std::endl;
    std::cout << "Enter 'c' to continue or enter 'e' to exit: ";
    std::cin >> *cmd;

    if (*cmd == "c")
    {
        *ptr = ST_READ;
    }
    else if (*cmd == "e")
    {
        std::cout << "Exitting..." << std::endl;
        *ptr = ST_EXIT;
    }
    else
    {
        std::cout << "Invalid Letter!" << std::endl;
    }

    delete cmd;
}

void readState(State *ptr, std::string *fileName)
{
    std::string *cmd = new std::string;
    std::string current_path = std::filesystem::current_path();
    std::cout << "Enter filename (in current working dir): ";
    std::cin >> *cmd;
    *fileName = current_path + "/" + *cmd;

    std::cout << "Enter 'ply' to play, enter 'prc' to start process, enter 'r' to read another file, enter 'e' to exit: ";
    std::cin >> *cmd;

    *hasProcessed = false;

    if (*cmd == "r")
    {
        *ptr = ST_READ;
    }
    else if (*cmd == "ply")
    {
        *ptr = ST_PLAY;
    }
    else if (*cmd == "prc")
    {
        *ptr = ST_PROCESS;
    }
    else if (*cmd == "e")
    {
        std::cout << "Exitting..." << std::endl;
        *ptr = ST_EXIT;
    }

    delete cmd;
}

void playState(State *ptr, std::string *fileName)
{
    std::string command = "afplay " + *fileName;
    if (!*hasPlayed)
    {
        system(command.c_str());
        *hasPlayed = true;
    }
    std::string *cmd = new std::string;
    std::cout << "Enter 'ply' to play again, enter 'prc' to process, enter 'r' to read another file, enter 'e' to exit: ";
    std::cin >> *cmd;

    if (*cmd == "ply")
    {
        system(command.c_str());
    }
    else if (*cmd == "prc")
    {
        *ptr = ST_PROCESS;
    }
    else if (*cmd == "r")
    {
        *ptr = ST_READ;
        *hasPlayed = false;
    }
    else if (*cmd == "e")
    {
        std::cout << "Exitting..." << std::endl;
        *ptr = ST_EXIT;
        *hasPlayed = false;
    }

    delete cmd;
}

void processState(State *ptr)
{
    *hasProcessed = true;
    *hasPlayed = false;

    std::string *cmd = new std::string;
    std::cout << "Enter 'ply' to play delayed file, enter 'prc' to process, enter e to exit: ";
    std::cin >> *cmd;

    if (*cmd == "ply")
    {
        *ptr = ST_PLAY;
    }
    else if (*cmd == "prc")
    {
        *ptr = ST_PROCESS;
    }
    else if (*cmd == "e")
    {
        *ptr = ST_EXIT;
    }

    delete cmd;
}

int main()
{
    std::string input_fileName = " ";
    Delay delay;

    State currentState = ST_IDLE;
    State *state_ptr = &currentState;

    while (currentState != ST_EXIT)
    {
        switch (currentState)
        {
        case ST_IDLE:
            idleState(state_ptr);
            break;

        case ST_READ:
            readState(state_ptr, &input_fileName);
            delay.initialize(&input_fileName);
            break;

        case ST_PROCESS:
            delay.setParams();
            delay.process();
            processState(state_ptr);
            break;

        case ST_PLAY:
            if (!*hasProcessed)
            {
                playState(state_ptr, &delay.in_fileName);
            }
            else if (hasProcessed)
            {
                playState(state_ptr, &delay.out_fileName);
            }
            break;

        case ST_EXIT:
            break;
        }
    }

    return 0;
}