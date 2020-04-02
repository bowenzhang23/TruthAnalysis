// My Class
#include "MyTruthAnalysis/HelperFunctions.h"

// ROOT
#include "TLorentzVector.h"

// std
#include <algorithm>
#include <iostream>

bool hasChild(const xAOD::TruthParticle* parent, const int absPdgId) 
{
    const std::size_t nChildren = parent->nChildren();
    for (std::size_t iChild = 0; iChild != nChildren; ++iChild) {
        const xAOD::TruthParticle* child = parent->child(iChild);
        if (child && (absPdgId == child->absPdgId())) {
            return true;
        }
    }
    return false;
}

bool hasChild(const xAOD::TruthParticle* parent, const int absPdgId, std::vector<unsigned int>& indices)
{
    bool found = false;
    const std::size_t nChildren = parent->nChildren();
    for (std::size_t iChild = 0; iChild < nChildren; iChild++) {
        const xAOD::TruthParticle* child = parent->child(iChild);
        if (child && (absPdgId == child->absPdgId())) {
            found = true;
            indices.push_back(iChild);
        }
    }
    return found;
}

bool isFromHiggs(const xAOD::TruthParticle* particle) 
{
    return (particle->auxdata<unsigned int>("classifierParticleOrigin") == 14);
}

bool isBJet(const xAOD::Jet* jet) 
{
    return (jet->auxdata<int>("TrueFlavor") == 5);
}

bool isBTruth(const xAOD::TruthParticle* jet) 
{
    return (jet->absPdgId() == 5);
}

bool isTauTruth(const xAOD::TruthParticle* tau) 
{
    return (tau->absPdgId() == 15);
}

bool contains(const std::vector<int>& v, const std::size_t& element) 
{
    return std::count(v.begin(), v.end(), element);
}

const xAOD::TruthParticle* getFinal(const xAOD::TruthParticle* particle) 
{
    xAOD::TruthParticle* final = nullptr;
    getFinalHelper(particle, final);
    return final;
}

void getFinalHelper(const xAOD::TruthParticle* particle, xAOD::TruthParticle*& final) 
{
    std::vector<unsigned int> idx{};
    if (hasChild(particle, particle->absPdgId(), idx)) {
        getFinalHelper(particle->child(idx[0]), final);
    } else {
        final = const_cast<xAOD::TruthParticle*>(particle);
    }
}

bool isGoodEvent() {
    return true;
}

bool isOS(const xAOD::TruthParticle* p0, const xAOD::TruthParticle* p1){
    return (p0->pdgId() * p1->pdgId() < 0);
}

bool isGoodTau(const xAOD::TruthParticle* tau, double ptCut, double etaCut) {
    if (!isTauTruth(tau)) return false;
    TLorentzVector tau_vis{};
    std::vector<unsigned int> idx{};
    
    if (hasChild(tau, 16, idx)) {  // 16 ->vt
        tau_vis = tau->p4() - tau->child(idx[0])->p4();
    }
    if (tau_vis.Pt() < ptCut * GeV) return false;
    double eta = tau_vis.Eta();
    if (std::abs(eta) > etaCut) return false;
    if (std::abs(eta) < 1.52 && std::abs(eta) > 1.37) return false;

    return true;
}

bool isGoodB(const xAOD::TruthParticle* b, double ptCut, double etaCut) {
    if (!isBTruth(b)) return false;
    if (b->pt() < ptCut * GeV) return false;
    double eta = b->eta();
    if (std::abs(eta) > etaCut) return false;

    return true;
}