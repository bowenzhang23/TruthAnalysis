#ifndef MyTruthAnalysis_HelperFunctions_H
#define MyTruthAnalysis_HelperFunctions_H

// xAOD
#include <xAODTruth/TruthParticle.h>
#include <xAODJet/JetContainer.h>
#include <xAODJet/Jet.h>

constexpr float GeV = 1'000;

bool hasChild(const xAOD::TruthParticle* parent, const int absPdgId);

bool hasChild(const xAOD::TruthParticle* parent, const int absPdgId, std::vector<unsigned int>& indices);

bool isFromHiggs(const xAOD::TruthParticle* particle);

bool isBJet(const xAOD::Jet* jet);

bool isBTruth(const xAOD::TruthParticle* jet);

bool isTauTruth(const xAOD::TruthParticle* tau);

bool contains(const std::vector<int>& v, const std::size_t& element);

const xAOD::TruthParticle* getFinal(const xAOD::TruthParticle* particle);

void getFinalHelper(const xAOD::TruthParticle* particle, xAOD::TruthParticle*& final);

bool isGoodEvent();  // TODO

bool isOS(const xAOD::TruthParticle* p0, const xAOD::TruthParticle* p1);

bool isGoodTau(const xAOD::TruthParticle* tau, double ptCut, double etaCut);

bool isGoodB(const xAOD::TruthParticle* b, double ptCut, double etaCut);

#endif