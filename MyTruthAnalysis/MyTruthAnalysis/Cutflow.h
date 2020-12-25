#ifndef MyTruthAnalysis_Cutflow_H
#define MyTruthAnalysis_Cutflow_H

#include <map>
#include <vector>
#include <string>

class Cutflow
{
private:
    std::vector<std::pair<std::string, float>> m_vCutflow;
    std::map<std::string, std::size_t> m_mapIndex;
public:
    Cutflow() : m_vCutflow({}){};
    void addCut(const std::string &sName, const float &fWeights);
    void print() const;
};

#endif