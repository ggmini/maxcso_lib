#include "../maxcsolib/Compressor.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <iomanip>

const std::string ANSI_RESET_LINE = "\033[2K\033[0G";

std::string getPathFromFile(){
    std::ifstream t("path.txt");
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str(); 
}

void printState(maxcsolib::Compressor* c, bool* running) {
    return;
    while (*running) {
        if (c->DataAvailable() == true) {
            std::cout << ANSI_RESET_LINE << "Percent: " << std::fixed << std::setprecision(0) << c->Percent() << "%, Ratio: " << c->Ratio() << "%, Speed: " << std::setprecision(2) << c->Speed() << " MB/s";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void runCompressor(maxcsolib::Compressor* c, maxcsolib::Arguments args) {
    c->Compress(args);
}

int main() {
    //std::string path = getPathFromFile(); 
    std::string path = "H:\\pulse";
    maxcsolib::Arguments args;
    maxcsolib::default_args(args);
    std::cout << "Converting " << path << ".iso to " << path << ".cso" << std::endl;
    args.inputs.push_back(path + ".iso");
    args.outputs.push_back(path + ".cso");
    //args.quiet = true;
    maxcsolib::Compressor* compressor = new maxcsolib::Compressor();

    bool running = true;
    std::thread compressor_t(runCompressor, compressor, args);
    std::thread state_t(printState, compressor, &running);
    compressor_t.join();
    running = false;
    state_t.join();

    delete compressor;
    return 0;
}