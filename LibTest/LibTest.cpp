#include "../maxcsolib/Compressor.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>


std::string getPathFromFile(){
    std::ifstream t("path.txt");
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str(); 
}

int main() {
    std::string path = getPathFromFile(); 

    maxcsolib::Arguments args;
    maxcsolib::default_args(args);
    std::cout << "Converting " << path << ".iso to " << path << ".cso" << std::endl; 
    args.inputs.push_back(path + ".iso");
    args.outputs.push_back(path + ".cso");
    maxcsolib::Compressor *compressor = new maxcsolib::Compressor();
    compressor->Compress(args);
    delete compressor;
    std::cout << "Hello World!\n";
    return 0;
}
