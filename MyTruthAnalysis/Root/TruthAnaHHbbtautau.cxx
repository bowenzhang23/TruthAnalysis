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

using std::vector;

TruthAnaHHbbtautau::TruthAnaHHbbtautau(const std::string &name,
                                       ISvcLocator *pSvcLocator)
    : TruthAnaBase(name, pSvcLocator)
{
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
    truthEvent = truthEventContainer->at(0);
  else
  {
    ANA_MSG_WARNING("in execute, no truth event container!");
    return StatusCode::SUCCESS;
  }

  // retrieve truth taus container
  const xAOD::TruthParticleContainer *truthTaus = nullptr;
  ANA_CHECK(evtStore()->retrieve(truthTaus, "TruthTaus"));
  ANA_MSG_DEBUG("Truth taus container size: " << truthTaus->size());

  // retrieve jet container
  const xAOD::JetContainer *jets = nullptr;
  ANA_CHECK(evtStore()->retrieve(jets, "AntiKt4TruthDressedWZJets"));
  // should contain at least two jets!
  ANA_MSG_DEBUG("Jet container size: " << jets->size());
  
  const xAOD::JetContainer *fatjets = nullptr;
  ANA_CHECK(evtStore()->retrieve(fatjets, "AntiKt10TruthTrimmedPtFrac5SmallR20Jets"));
  // should contain at least one large radius jets!
  ANA_MSG_DEBUG("Jet container size: " << fatjets->size());
  
  // event info
  m_nRunNumber = eventInfo->runNumber();
  m_nEventNumber = eventInfo->eventNumber();

  // event weights
  const vector<float> weights = truthEvent->weights();
  // not the product
  // float mc_weights = std::accumulate(weights.begin(), weights.end(), 1, std::multiplies<float>());
  m_fMCWeight = weights[0];
  APPLYCUT(true, "Initial");
  APPLYCUT(jets->size() > 2 || fatjets->size() > 1, "Number of truth jets")
  
  // objects
  std::map<std::string, const xAOD::TruthParticle *> higgsMap{};
  vector<const xAOD::TruthParticle *> truthTauVec{};
  vector<const xAOD::Jet *> truthJetVec{};
  vector<const xAOD::Jet *> truthFatJetVec{};

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
    vector<unsigned> trash_tautau_idx;
    if (!found_htautau && particle->pdgId() == 25 && particle->nChildren() == 2 && hasChild(particle, 15, trash_tautau_idx))
    {
      found_htautau = true;
      higgsMap.insert({"Htautau", particle});
    }

    vector<unsigned> trash_bb_idx;
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

  // particles
  const xAOD::TruthParticle *tau0 = getFinal(higgsMap["Htautau"]->child(0));
  const xAOD::TruthParticle *tau1 = getFinal(higgsMap["Htautau"]->child(1));
  const xAOD::TruthParticle* b0   = getFinal(higgsMap["Hbb"]->child(0));
  const xAOD::TruthParticle* b1   = getFinal(higgsMap["Hbb"]->child(1));
  // const xAOD::TruthParticle *b0 = higgsMap["Hbb"]->child(0);
  // const xAOD::TruthParticle *b1 = higgsMap["Hbb"]->child(1);

  // fetch small R b-jets
  vector<int> btag_idx{};
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

  // fetch large R di-b-jet
  vector<int> dibtag_idx{};
  for (std::size_t i = 0; i < fatjets->size(); i++)
  {
    if (isDiBJet(fatjets->at(i), b0, b1))
    {
      truthFatJetVec.push_back(fatjets->at(i));
      dibtag_idx.push_back(i);
    }
  }

  // Categories
  if (truthFatJetVec.size() >= 1 && truthJetVec.size() >= 2)
    m_eChannel = CHAN::BOTH;
  else if (truthFatJetVec.size() >= 1)
    m_eChannel = CHAN::BOOSTED;
  else if (truthJetVec.size() >= 2)
    m_eChannel = CHAN::RESOLVED;
  else
    m_eChannel = CHAN::UNKNOWN;
  
  // to be saved in the ntuple
  m_nChannel = static_cast<unsigned long long>(m_eChannel);

  // if less than two truth b-tag jet, push the leading jet of the remaining jets to the vec
  if (truthJetVec.size() < 2)
  {
    for (std::size_t i = 0; i < jets->size(); i++)
    {
      if (!contains(btag_idx, i) && (jets->at(i)->p4().DeltaR(tau0->p4())) > 0.4 && (jets->at(i)->p4().DeltaR(tau1->p4())) > 0.4)
      {
        truthJetVec.push_back(jets->at(i));
      }
      if (truthJetVec.size() == 2)
        break;
    }
  }

  // if less than one double b-tag fatjet, 
  if (truthFatJetVec.size() < 1)
  {
    for (std::size_t i = 0; i < fatjets->size(); i++)
    {
      if (!contains(dibtag_idx, i) && (fatjets->at(i)->p4().DeltaR(tau0->p4())) > 1.0 && (fatjets->at(i)->p4().DeltaR(tau1->p4())) > 1.0)
      {
        truthFatJetVec.push_back(fatjets->at(i));
      }
      if (truthFatJetVec.size() == 1)
        break;
    }
  }

  // kinematics
  TLorentzVector tau0_p4 = tau0->p4(), tau1_p4 = tau1->p4();
  TLorentzVector tauvis0_p4 = tauVisP4(tau0), tauvis1_p4 = tauVisP4(tau1);
  TLorentzVector b0_p4 = b0->p4(), b1_p4 = b1->p4();

  ANA_MSG_DEBUG(printVec(btag_idx, "btag_idx"));
  ANA_MSG_DEBUG("Jet vector size: " << truthJetVec.size());
  
  ANA_MSG_DEBUG("Htautau : " << higgsMap["Htautau"]->child(0)->pdgId() << ", " << higgsMap["Htautau"]->child(1)->pdgId());
  ANA_MSG_DEBUG("Hbb     : " << higgsMap["Hbb"]->child(0)->pdgId() << ", " << higgsMap["Hbb"]->child(1)->pdgId());

  APPLYCUT(isGoodEvent(), "Empty cut for testing");
  APPLYCUT(isOS(tau0, tau1) && isOS(b0, b1), "OS Charge");
  APPLYCUT(isGoodTau(tau0, 20., 2.5) && isGoodTau(tau1, 20., 2.5), "Tau Preselection"); 
  APPLYCUT(isGoodB(b0, 20., 2.4) && isGoodB(b1, 20., 2.4), "B-jet preselection");
  APPLYCUT(isNotOverlap(b0, b1, tau0, tau1, 0.2), "b-tau overlap removal");

  // mimic single tau trigger selection
  bool STT = isGoodTau(tau0, 100., 2.5) && isGoodB(b0, 45., 2.4);

  // mimic di-tau trigger selection
  bool DTT = isGoodTau(tau0, 40., 2.5) && isGoodTau(tau1, 30., 2.5) && isGoodB(b0, 80., 2.4);

  // event must pass single tau trigger or di-tau trigger
  APPLYCUT(STT || DTT, "Trigger selection (TO CHECK)");
  APPLYCUT((tau0_p4 + tau1_p4).M() > 60 * GeV, "Di-tau mass selection");

  // 4-momenta
  m_fTau0_pt = tau0_p4.Pt() / GeV;
  m_fTau0_phi = tau0_p4.Phi();
  m_fTau0_eta = tau0_p4.Eta();
  m_fTau1_pt = tau1_p4.Pt() / GeV;
  m_fTau1_phi = tau1_p4.Phi();
  m_fTau1_eta = tau1_p4.Eta();

  m_fTauVis0_pt = tauvis0_p4.Pt() / GeV;
  m_fTauVis0_phi = tauvis0_p4.Phi();
  m_fTauVis0_eta = tauvis0_p4.Eta();
  m_fTauVis1_pt = tauvis1_p4.Pt() / GeV;
  m_fTauVis1_phi = tauvis1_p4.Phi();
  m_fTauVis1_eta = tauvis1_p4.Eta();

  m_fB0_pt = b0_p4.Pt() / GeV;
  m_fB0_phi = b0_p4.Phi();
  m_fB0_eta = b0_p4.Eta();
  m_fB1_pt = b1_p4.Pt() / GeV;
  m_fB1_phi = b1_p4.Phi();
  m_fB1_eta = b1_p4.Eta();

  // deltaRs
  m_fDeltaR_TauTau = tau0_p4.DeltaR(tau1_p4);
  m_fDeltaR_TauVisTauVis = tauvis0_p4.DeltaR(tauvis1_p4);
  m_fDeltaR_BB = b0_p4.DeltaR(b1_p4);

  // Higgs Pt
  m_fPtBB = (b0_p4 + b1_p4).Pt() / GeV;
  m_fPtTauTau = (tau0_p4 + tau1_p4).Pt() / GeV;

  // invariant masses
  m_fMTauTau = (tau0_p4 + tau1_p4).M() / GeV;
  m_fMTauVisTauVis = (tauvis0_p4 + tauvis1_p4).M() / GeV;
  m_fMBB = (b0_p4 + b1_p4).M() / GeV;
  m_fMHH = (tau0_p4 + tau1_p4 + b0_p4 + b1_p4).M() / GeV;

  // n jets after adding additional non b-tagged jet
  // TODO many veto those events
  m_nJets = truthJetVec.size();
  m_nFatJets = truthFatJetVec.size();

  APPLYCOUNT(m_eChannel == CHAN::UNKNOWN, "Channel = Unknown")
  APPLYCOUNT(m_eChannel == CHAN::RESOLVED, "Channel = Resolved");
  APPLYCOUNT(m_eChannel == CHAN::BOOSTED, "Channel = Boosted");
  APPLYCOUNT(m_eChannel == CHAN::BOTH, "Channel = Both");

  if (m_nJets >= 2) // only make sense for Resolved and Both channel
  {
    std::sort(truthJetVec.begin(), truthJetVec.end(), 
      [](const xAOD::Jet *a, const xAOD::Jet *b) { return a->pt() > b->pt(); });
    const xAOD::Jet *bjet0 = truthJetVec[0];
    const xAOD::Jet *bjet1 = truthJetVec[1];

    TLorentzVector bjet0_p4, bjet1_p4;
    bjet0_p4 = bjet0->p4();
    bjet1_p4 = bjet1->p4();
    m_fBjet0_pt = bjet0_p4.Pt() / GeV;
    m_fBjet0_phi = bjet0_p4.Phi();
    m_fBjet0_eta = bjet0_p4.Eta();
    m_fBjet1_pt = bjet1_p4.Pt() / GeV;
    m_fBjet1_phi = bjet1_p4.Phi();
    m_fBjet1_eta = bjet1_p4.Eta();
    m_fDeltaR_BjetBjet = bjet0_p4.DeltaR(bjet1_p4);
    m_fMBjetBjet = (bjet0_p4 + bjet1_p4).M() / GeV;
  }

  if (m_nFatJets >= 1) // only make sense for Boosted and Both channel
  {
    std::sort(truthFatJetVec.begin(), truthFatJetVec.end(), 
      [](const xAOD::Jet *a, const xAOD::Jet *b) { return a->pt() > b->pt(); });
    const xAOD::Jet *dibjet = truthJetVec[0];

    TLorentzVector dibjet_p4;
    dibjet_p4 = dibjet->p4();
    m_fDiBjet_pt = dibjet_p4.Pt() / GeV;
    m_fDiBjet_m = dibjet_p4.M() / GeV;
    m_fDiBjet_phi = dibjet_p4.Phi();
    m_fDiBjet_eta = dibjet_p4.Eta();
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

// ----------------------------------------------------------------------------
// Function for trees
// ----------------------------------------------------------------------------

void TruthAnaHHbbtautau::initBranches()
{
  m_cTree->Branch("EventNumber", &m_nEventNumber);
  m_cTree->Branch("RunNumber", &m_nRunNumber);
  m_cTree->Branch("NJets", &m_nJets);
  m_cTree->Branch("NFatJets", &m_nFatJets);
  m_cTree->Branch("Tau0_pt", &m_fTau0_pt);
  m_cTree->Branch("Tau1_pt", &m_fTau1_pt);
  m_cTree->Branch("Tau0_phi", &m_fTau0_phi);
  m_cTree->Branch("Tau1_phi", &m_fTau1_phi);
  m_cTree->Branch("Tau0_eta", &m_fTau0_eta);
  m_cTree->Branch("Tau1_eta", &m_fTau1_eta);
  m_cTree->Branch("TauVis0_pt", &m_fTauVis0_pt);
  m_cTree->Branch("TauVis1_pt", &m_fTauVis1_pt);
  m_cTree->Branch("TauVis0_phi", &m_fTauVis0_phi);
  m_cTree->Branch("TauVis1_phi", &m_fTauVis1_phi);
  m_cTree->Branch("TauVis0_eta", &m_fTauVis0_eta);
  m_cTree->Branch("TauVis1_eta", &m_fTauVis1_eta);
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
  m_cTree->Branch("DiBjet_pt", &m_fDiBjet_pt);
  m_cTree->Branch("DiBjet_m", &m_fDiBjet_m);
  m_cTree->Branch("DiBjet_phi", &m_fDiBjet_phi);
  m_cTree->Branch("DiBjet_eta", &m_fDiBjet_eta);
  m_cTree->Branch("DeltaR_BB", &m_fDeltaR_BB);
  m_cTree->Branch("DeltaR_BjetBjet", &m_fDeltaR_BjetBjet);
  m_cTree->Branch("DeltaR_TauTau", &m_fDeltaR_TauTau);
  m_cTree->Branch("DeltaR_TauVisTauVis", &m_fDeltaR_TauVisTauVis);
  m_cTree->Branch("MBB", &m_fMBB);
  m_cTree->Branch("MTauTau", &m_fMTauTau);
  m_cTree->Branch("MBjetBjet", &m_fMBjetBjet);
  m_cTree->Branch("MTauVisTauVis", &m_fMTauVisTauVis);
  m_cTree->Branch("MHH", &m_fMHH);
  m_cTree->Branch("PtBB", &m_fPtBB);
  m_cTree->Branch("PtTauTau", &m_fPtTauTau);
  m_cTree->Branch("MCWeight", &m_fMCWeight);
  m_cTree->Branch("Channel", &m_nChannel);
}

void TruthAnaHHbbtautau::resetBranches()
{
  m_nEventNumber = 0; // DONE
  m_nRunNumber = 0; // DONE
  m_nJets = 0; // DONE
  m_nFatJets = 0; // DONE
  m_fTau0_pt = 0; // DONE
  m_fTau1_pt = 0; // DONE
  m_fTau0_phi = 0; // DONE
  m_fTau1_phi = 0; // DONE
  m_fTau0_eta = 0; // DONE
  m_fTau1_eta = 0; // DONE
  m_fTauVis0_pt = 0; // DONE
  m_fTauVis1_pt = 0; // DONE
  m_fTauVis0_phi = 0; // DONE
  m_fTauVis1_phi = 0; // DONE
  m_fTauVis0_eta = 0; // DONE
  m_fTauVis1_eta = 0; // DONE
  m_fB0_pt = 0; // DONE
  m_fB1_pt = 0; // DONE
  m_fB0_phi = 0; // DONE
  m_fB1_phi = 0; // DONE
  m_fB0_eta = 0; // DONE
  m_fB1_eta = 0; // DONE
  m_fBjet0_pt = 0; // DONE
  m_fBjet1_pt = 0; // DONE
  m_fBjet0_phi = 0; // DONE
  m_fBjet1_phi = 0; // DONE
  m_fBjet0_eta = 0; // DONE
  m_fBjet1_eta = 0; // DONE
  m_fDiBjet_pt = 0; // DONE
  m_fDiBjet_m = 0; // DONE
  m_fDiBjet_phi = 0; // DONE
  m_fDiBjet_eta = 0; // DONE
  m_fDeltaR_BB = 0; // DONE
  m_fDeltaR_BjetBjet = 0; // DONE
  m_fDeltaR_TauTau = 0; // DONE
  m_fDeltaR_TauVisTauVis = 0; // DONE
  m_fMBB = 0; // DONE
  m_fMTauTau = 0; // DONE
  m_fMBjetBjet = 0; // DONE
  m_fMTauVisTauVis = 0; // DONE
  m_fMHH = 0; // DONE
  m_fPtBB = 0; // DONE
  m_fPtTauTau = 0; // DONE
  m_fMCWeight = 0; // DONE
  m_nChannel = 0; // DONE
}