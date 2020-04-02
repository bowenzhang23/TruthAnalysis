#ifndef MyTruthAnalysis_Bookkeeper_H
#define MyTruthAnalysis_Bookkeeper_H

#include <map>
#include <string>
#include <algorithm>
#include <iostream>

class Bookkeeper {
private:
    std::map<std::string, float> _cutflow;
public:
    Bookkeeper(): _cutflow({}) {};
    void addCut(int i, const std::string& cutname, const float& mcweights);
    void print () const;
};

#endif