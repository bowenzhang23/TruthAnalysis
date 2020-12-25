// AsgTools
#include <AsgTools/MessageCheck.h>

// xAOD
#include <xAODEventInfo/EventInfo.h>
#include <xAODTruth/TruthEvent.h>
#include <xAODTruth/TruthEventContainer.h>
#include <xAODTruth/TruthParticle.h>
#include <xAODTruth/TruthParticleContainer.h>
#include <xAODTruth/TruthVertex.h>
#include <xAODJet/JetContainer.h>
#include <xAODJet/Jet.h>

// ROOT
#include <TLorentzVector.h>
#include <TTree.h>
#include <TH1.h>


// My headers
#include "MyTruthAnalysis/TruthAnaHHbbtautau.h"
#include "MyTruthAnalysis/HelperFunctions.h"

// std
#include <map>
#include <memory>
#include <algorithm>
#include <functional>
#include <numeric>
#include <cassert>

using namespace TruthAna;

TruthAnaHHbbtautau::TruthAnaHHbbtautau(const std::string &name,
                                       ISvcLocator *pSvcLocator)
    : TruthAnaBase(name, pSvcLocator)
{
  setProperty("RootStreamName", "TruthAna");
}

StatusCode TruthAnaHHbbtautau::initialize()
{
  ANA_MSG_INFO("Initializing ...");
  ANA_CHECK( book( TTree("MyTree", "truth analysis tree") ) );
  m_cTree = tree("MyTree");
  m_cTree->SetMaxTreeSize(500'000'000);
  initBranches();

  return StatusCode::SUCCESS;
}

StatusCode TruthAnaHHbbtautau::execute()
{
  resetBranches();

  // retrieve the eventInfo object from the event store
  const xAOD::EventInfo *eventInfo = nullptr;
  ANA_CHECK(evtStore()->retrieve(eventInfo, "EventInfo"));

  const xAOD::TruthEventContainer *truthEventContainer = nullptr;
  ANA_CHECK(evtStore()->retrieve(truthEventContainer, "TruthEvents"));

  // print out run and event number from retrieved object
  ANA_MSG_DEBUG("in execute, runNumber = " << eventInfo->runNumber() << ", eventNumber = " << eventInfo->eventNumber());

  ANA_MSG_DEBUG("in execute, truth event container size = " << truthEventContainer->size());
  const xAOD::TruthEvent *truthEvent = nullptr;
  if (truthEventContainer->size() == 1)
  {
    truthEvent = truthEventContainer->at(0);
  }
  else
  {
    ANA_MSG_WARNING("in execute, no truth event container!");
    return StatusCode::SUCCESS;
  }

  // retrieve truth taus container
  const xAOD::TruthParticleContainer *truthTaus = nullptr;
  ANA_CHECK(evtStore()->retrieve(truthTaus, "TruthTaus"));
  ANA_MSG_DEBUG("Truth taus container size: " << truthTaus->size());

  /*
   * TODO: i'm testing the AntiKt4TruthDressedWZJets collection ...
   */
  // retrieve jet container
  const xAOD::JetContainer *jets = nullptr;
  ANA_CHECK(evtStore()->retrieve(jets, "AntiKt4TruthDressedWZJets"));
  // should contain more than two jets!
  ANA_MSG_DEBUG("Jet container size: " << jets->size());
  assert(jets->size() >= 2);

  // event info
  m_nRunNumber = eventInfo->runNumber();
  m_nEventNumber = eventInfo->eventNumber();

  // event weights
  const std::vector<float> weights = truthEvent->weights();
  // not the product
  // float mc_weights = std::accumulate(weights.begin(), weights.end(), 1, std::multiplies<float>());
  m_fMCWeight = weights[0];
  APPLYCUT(true, "Initial");

  // objects
  std::map<std::string, const xAOD::TruthParticle *> higgsMap{};
  std::vector<const xAOD::TruthParticle *> truthTauVec{};
  std::vector<const xAOD::Jet *> truthJetVec{};

  bool found_htautau = false;
  bool found_hbb = false;

  for (std::size_t i = 0; i < truthEvent->nTruthParticles(); i++)
  {
    const xAOD::TruthParticle *particle = truthEvent->truthParticle(i);

    // Some debug output
    if (i == 0)
    {
      ANA_MSG_DEBUG("Particle info: ");
      ANA_MSG_DEBUG(" - Barcode: " << particle->barcode());
      ANA_MSG_DEBUG(" - PDG ID : " << particle->pdgId());
      ANA_MSG_DEBUG(" - Pt     : " << particle->pt());
    }

    // fetch Higgs
    std::vector<unsigned int> trash_tautau_idx;
    if (!found_htautau && particle->pdgId() == 25 && particle->nChildren() == 2 && hasChild(particle, 15, trash_tautau_idx))
    {
      found_htautau = true;
      higgsMap.insert({"Htautau", particle});
    }

    std::vector<unsigned int> trash_bb_idx;
    if (!found_hbb && particle->pdgId() == 25 && particle->nChildren() == 2 && hasChild(particle, 5, trash_tautau_idx))
    {
      found_hbb = true;
      higgsMap.insert({"Hbb", particle});
    }
  }

  // fetch truth taus
  for (std::size_t i = 0; i < truthTaus->size(); i++)
  {
    if (isFromHiggs(truthTaus->at(i)))
    {
      truthTauVec.push_back(truthTaus->at(i));
    }
  }
  ANA_MSG_DEBUG("Truth taus vector size: " << truthTauVec.size());

  // fetch b-jets
  std::vector<int> btag_idx{};
  for (std::size_t i = 0; i < jets->size(); i++)
  {
    ANA_MSG_DEBUG("Jet truth flavour info: ");
    ANA_MSG_DEBUG(" - PartonTruthLabelID = " << jets->at(i)->auxdata<int>("PartonTruthLabelID"));
    ANA_MSG_DEBUG(" - HadronConeExclTruthLabelID = " << jets->at(i)->auxdata<int>("HadronConeExclTruthLabelID"));
    ANA_MSG_DEBUG(" - TrueFlavor = " << jets->at(i)->auxdata<int>("TrueFlavor"));
    if (isBJet(jets->at(i)))
    { // isBJet -> TruthFlavor == 5
      truthJetVec.push_back(jets->at(i));
      btag_idx.push_back(i);
    }
  }
  /* 
   * TODO: nbjet requirement is vetoing a lot of events with dR(b, b) < 0.4
   * this is expected(?) because two bjets are close to each other(?)
   */
  // int nBJets = btag_idx.size();

  // particles
  const xAOD::TruthParticle *tau0 = getFinal(higgsMap["Htautau"]->child(0));
  const xAOD::TruthParticle *tau1 = getFinal(higgsMap["Htautau"]->child(1));
  // const xAOD::TruthParticle* b0   = getFinal(higgsMap["Hbb"]->child(0));
  // const xAOD::TruthParticle* b1   = getFinal(higgsMap["Hbb"]->child(1));
  const xAOD::TruthParticle *b0 = higgsMap["Hbb"]->child(0);
  const xAOD::TruthParticle *b1 = higgsMap["Hbb"]->child(1);

  // if less than two truth b-tag jet, push the leading jet of the remaining jets to the vec
  if (truthJetVec.size() < 2)
  {
    for (std::size_t i = 0; i < jets->size(); i++)
    {
      if (!contains(btag_idx, i) && (jets->at(i)->p4().DeltaR(tau0->p4())) > 0.2 && (jets->at(i)->p4().DeltaR(tau1->p4())) > 0.2)
      {
        truthJetVec.push_back(jets->at(i));
      }
      if (truthJetVec.size() == 2)
        break;
    }
  }

  ANA_MSG_DEBUG(printVec(btag_idx, "btag_idx"));
  ANA_MSG_DEBUG("Jet vector size: " << truthJetVec.size());

  // kinematics
  TLorentzVector tau0_p4 = tau0->p4(), tau1_p4 = tau1->p4();
  TLorentzVector tauvis0_p4 = tauVisP4(tau0), tauvis1_p4 = tauVisP4(tau1);
  TLorentzVector b0_p4 = b0->p4(), b1_p4 = b1->p4();

  ANA_MSG_DEBUG("Htautau : " << higgsMap["Htautau"]->child(0)->pdgId() << ", " << higgsMap["Htautau"]->child(1)->pdgId());
  ANA_MSG_DEBUG("Hbb     : " << higgsMap["Hbb"]->child(0)->pdgId() << ", " << higgsMap["Hbb"]->child(1)->pdgId());

  APPLYCUT(isGoodEvent(), "Good Event");
  APPLYCUT(isOS(tau0, tau1) && isOS(b0, b1), "OS Charge");
  APPLYCUT(isGoodTau(tau0, 20., 2.5) && isGoodTau(tau1, 20., 2.5), "Tau Preselection"); 
  APPLYCUT(isGoodB(b0, 20., 2.4) && isGoodB(b1, 20., 2.4), "B-jet preselection");
  APPLYCUT(isNotOverlap(b0, b1, tau0, tau1, 0.2), "b-tau overlap removal");

  bool STT = isGoodTau(tau0, 100., 2.5) && isGoodB(b0, 45., 2.4);
  bool DTT = isGoodTau(tau0, 40., 2.5) && isGoodTau(tau1, 30., 2.5) && isGoodB(b0, 80., 2.4);

  APPLYCUT(STT || DTT, "Trigger selection (TO CHECK)");
  APPLYCUT((tau0_p4 + tau1_p4).M() > 60 * GeV, "Di-tau mass selection");

  // deltaRs
  m_fDeltaR_TauTau = tau0_p4.DeltaR(tau1_p4);
  m_fDeltaR_TauVisTauVis = tauvis0_p4.DeltaR(tauvis1_p4);
  m_fDeltaR_BB = b0_p4.DeltaR(b1_p4);

  // invariant masses
  m_fMTauVisTauVis = (tauvis0_p4 + tauvis1_p4).M() / GeV;
  m_fMBB = (b0_p4 + b1_p4).M() / GeV;

  m_nJets = truthJetVec.size();

  if (m_nJets >= 2)
  {
    const xAOD::Jet *bjet0 = truthJetVec[0];
    const xAOD::Jet *bjet1 = truthJetVec[1];

    TLorentzVector bjet0_p4, bjet1_p4;
    bjet0_p4 = bjet0->p4();
    bjet1_p4 = bjet1->p4();
    m_fDeltaR_BjetBjet = bjet0_p4.DeltaR(bjet1_p4);
  } 
  else
  {
    m_fDeltaR_BjetBjet = -1;
  }

  ANA_MSG_DEBUG("Found Higgs -> tautau, delta R(tau, tau) : " << m_fDeltaR_TauTau);
  ANA_MSG_DEBUG("Found Higgs -> bb,     delta R(b, b)     : " << m_fDeltaR_BB);

  m_cTree->Fill();

  return StatusCode::SUCCESS;
}

StatusCode TruthAnaHHbbtautau::finalize()
{
  ANA_MSG_INFO("Finalizing ...");
  m_cCutflow->print();
  return StatusCode::SUCCESS;
}

void TruthAnaHHbbtautau::initBranches()
{
  m_cTree->Branch("EventNumber", &m_nEventNumber);
  m_cTree->Branch("RunNumber", &m_nRunNumber);
  m_cTree->Branch("NJets", &m_nJets);
  m_cTree->Branch("Tau0_pt", &m_fTau0_pt);
  m_cTree->Branch("Tau1_pt", &m_fTau1_pt);
  m_cTree->Branch("Tau0_phi", &m_fTau0_phi);
  m_cTree->Branch("Tau1_phi", &m_fTau1_phi);
  m_cTree->Branch("Tau0_eta", &m_fTau0_eta);
  m_cTree->Branch("Tau1_eta", &m_fTau1_eta);
  m_cTree->Branch("B0_pt", &m_fB0_pt);
  m_cTree->Branch("B1_pt", &m_fB1_pt);
  m_cTree->Branch("B0_phi", &m_fB0_phi);
  m_cTree->Branch("B1_phi", &m_fB1_phi);
  m_cTree->Branch("B0_eta", &m_fB0_eta);
  m_cTree->Branch("B1_eta", &m_fB1_eta);
  m_cTree->Branch("Bjet0_pt", &m_fBjet0_pt);
  m_cTree->Branch("Bjet1_pt", &m_fBjet1_pt);
  m_cTree->Branch("Bjet0_phi", &m_fBjet0_phi);
  m_cTree->Branch("Bjet1_phi", &m_fBjet1_phi);
  m_cTree->Branch("Bjet0_eta", &m_fBjet0_eta);
  m_cTree->Branch("Bjet1_eta", &m_fBjet1_eta);
  m_cTree->Branch("DeltaR_BB", &m_fDeltaR_BB);
  m_cTree->Branch("DeltaR_BjetBjet", &m_fDeltaR_BjetBjet);
  m_cTree->Branch("DeltaR_TauTau", &m_fDeltaR_TauTau);
  m_cTree->Branch("DeltaR_TauVisTauVis", &m_fDeltaR_TauVisTauVis);
  m_cTree->Branch("MBB", &m_fMBB);
  m_cTree->Branch("MBjetBjet", &m_fMBjetBjet);
  m_cTree->Branch("MTauVisTauVis", &m_fMTauVisTauVis);
  m_cTree->Branch("MHH", &m_fMHH);
  m_cTree->Branch("PtBB", &m_fPtBB);
  m_cTree->Branch("PtTauTau", &m_fPtTauTau);
  m_cTree->Branch("MCWeight", &m_fMCWeight);
}

void TruthAnaHHbbtautau::resetBranches()
{
  m_nEventNumber = 0; // DONE
  m_nRunNumber = 0; // DONE
  m_nJets = 0; // DONE
  m_fTau0_pt = 0;
  m_fTau1_pt = 0;
  m_fTau0_phi = 0;
  m_fTau1_phi = 0;
  m_fTau0_eta = 0;
  m_fTau1_eta = 0;
  m_fB0_pt = 0;
  m_fB1_pt = 0;
  m_fB0_phi = 0;
  m_fB1_phi = 0;
  m_fB0_eta = 0;
  m_fB1_eta = 0;
  m_fBjet0_pt = 0;
  m_fBjet1_pt = 0;
  m_fBjet0_phi = 0;
  m_fBjet1_phi = 0;
  m_fBjet0_eta = 0;
  m_fBjet1_eta = 0;
  m_fDeltaR_BB = 0;
  m_fDeltaR_BjetBjet = 0; // DONE
  m_fDeltaR_TauTau = 0; // DONE
  m_fDeltaR_TauVisTauVis = 0; // DONE
  m_fMBB = 0; // DONE
  m_fMBjetBjet = 0;
  m_fMTauVisTauVis = 0; // DONE
  m_fMHH = 0;
  m_fPtBB = 0;
  m_fPtTauTau = 0;
  m_fMCWeight = 0; // DONE
}