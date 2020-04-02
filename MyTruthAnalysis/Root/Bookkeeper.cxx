#include "MyTruthAnalysis/Bookkeeper.h"

void Bookkeeper::addCut(int i, const std::string& cutname, const float& mcweights) 
{
    if (!(this->_cutflow.count(cutname))) {
        std::pair<std::string, float> cut = {std::to_string(i) + "-" + cutname, 0};
        this->_cutflow.insert(cut);
    }
    this->_cutflow[std::to_string(i) + "-" + cutname] += mcweights;
}

void Bookkeeper::print () const
{   
    std::string outputs;
    outputs += "+-------------------------------+\n";
    for (const auto& cut: this->_cutflow) {
        outputs += "| " + cut.first + "\t| " + std::to_string(cut.second) + "\t|\n";
    }
    outputs += "+-------------------------------+\n";
    std::cout << outputs << "---- Applied " << std::to_string(this->_cutflow.size()) << " cuts!";
    std::cout << std::endl;
}