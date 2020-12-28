#ifndef MyTruthAnalysis_HelperFunctions_H
#define MyTruthAnalysis_HelperFunctions_H

// xAOD
#include <xAODTruth/TruthParticle.h>
#include <xAODJet/JetContainer.h>
#include <xAODJet/Jet.h>
#include <exception>

namespace TruthAna
{

    class TauIsNotFinalOrDecayNoNeutrino : public std::exception
    {
    };

    constexpr float GeV = 1'000;

    bool hasChild(const xAOD::TruthParticle *parent, const int absPdgId);

    bool hasChild(const xAOD::TruthParticle *parent, const int absPdgId, std::vector<unsigned int> &indices);

    bool isFromHiggs(const xAOD::TruthParticle *particle);

    /// this uses TruthFlavour
    bool isBJet(const xAOD::Jet *jet);

    /// truth flavour is not available, this use dR matching to truth b partons
    bool isDiBJet(const xAOD::Jet *fatjet, const xAOD::TruthParticle* b0, const xAOD::TruthParticle* b1);

    bool isBTruth(const xAOD::TruthParticle *parton);

    bool isTauTruth(const xAOD::TruthParticle *tau);

    bool contains(const std::vector<int> &v, const std::size_t &element);

    std::string printVec(const std::vector<int> &v, const std::string &message);

    const xAOD::TruthParticle *getFinal(const xAOD::TruthParticle *particle);

    void getFinalHelper(const xAOD::TruthParticle *particle, xAOD::TruthParticle *&final);

    TLorentzVector tauVisP4(const xAOD::TruthParticle *tau);

    bool isGoodEvent(); // TODO

    bool isOS(const xAOD::TruthParticle *p0, const xAOD::TruthParticle *p1);

    bool isGoodTau(const xAOD::TruthParticle *tau, double ptCut, double etaCut);

    bool isGoodB(const xAOD::TruthParticle *b, double ptCut, double etaCut);

    bool isNotOverlap(const xAOD::TruthParticle *b0, const xAOD::TruthParticle *b1, const xAOD::TruthParticle *tau0, const xAOD::TruthParticle *tau1, double minDR);

} // namespace TruthAna
#endif