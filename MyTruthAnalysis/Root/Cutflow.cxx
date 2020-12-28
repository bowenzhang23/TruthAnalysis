#include "MyTruthAnalysis/Cutflow.h"

#include <iostream>
#include <iomanip>

using std::cout;
using std::setw;
using std::size_t;
using std::string;
using std::vector;

void Cutflow::addCut(const std::string &sName, const float &fWeights)
{
    if (m_mapIndex.find(sName) != m_mapIndex.end()) 
    {
        m_vCutflow[m_mapIndex[sName]].second += fWeights;
    }
    else
    {
        m_vCutflow.push_back(std::make_pair(sName, fWeights));
        m_mapIndex[sName] = m_vCutflow.size() - 1;
    }
}

void Cutflow::print() const
{
    cout << "Printing cutflow\n";
    cout << "----------------\n";
    size_t nLongestP2{2};
    for (auto &p : m_vCutflow)
    {
        nLongestP2 = std::max(p.first.length() + 2, nLongestP2);
    }

    cout << std::left << setw(5) << "idx " << std::left << setw(nLongestP2) 
         << "Name" << std::left << setw(10) << "SumW" << "Rel. Eff.\n";
    for (size_t i = 0; i < m_vCutflow.size(); ++i)
    {
        cout << "(" << std::left << setw(2) << i + 1  << ") "
             << std::left << setw(nLongestP2) << m_vCutflow[i].first 
             << std::left << setw(10) << m_vCutflow[i].second;
        if (i > 0)
            cout << m_vCutflow[i].second / m_vCutflow[i - 1].second << '\n';
        else
            cout << '\n';
    }
}