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
#include "TLorentzVector.h"
#include <TH1.h>

// My headers
#include "MyTruthAnalysis/MyTruthOneAnalysis.h"
#include "MyTruthAnalysis/HelperFunctions.h"

// std
#include <map>
#include <memory>
#include <algorithm>
#include <functional>
#include <numeric>
#include <cassert>


MyTruthOneAnalysis :: MyTruthOneAnalysis (const std::string& name,
                                  ISvcLocator *pSvcLocator)
  : EL::AnaAlgorithm (name, pSvcLocator)
{
  // Here you put any code for the base initialization of variables,
  // e.g. initialize all pointers to 0.  This is also where you
  // declare all properties for your algorithm.  Note that things like
  // resetting statistics variables or booking histograms should
  // rather go into the initialize() function.
  _bkp = std::make_unique<Bookkeeper>();
  
}



StatusCode MyTruthOneAnalysis :: initialize ()
{
  // Here you do everything that needs to be done at the very
  // beginning on each worker node, e.g. create histograms and output
  // trees.  This method gets called before any input files are
  // connected.
  ANA_MSG_INFO ("Initializing ...");
  
  // book histograms
  ANA_CHECK (book(TH1F("h_run_number", "Run Number", 100, 0, 100'000)));
  ANA_CHECK (book(TH1F("h_event_number", "Event Number", 100, 0, 100'000'000)));
  ANA_CHECK (book(TH1F("h_mc_weights", "MC weights", 100, -2, 2)));
  ANA_CHECK (book(TH1F("h_dRtautau", "#DeltaR(#tau-lepton, #tau-lepton)", 100, 0, 10)));
  ANA_CHECK (book(TH1F("h_dRtauvistauvis", "#DeltaR(#tau_{h}, #tau_{h})", 100, 0, 10)));
  ANA_CHECK (book(TH1F("h_dRbb", "#DeltaR(b-quark, b-quark)", 100, 0, 10)));
  ANA_CHECK (book(TH1F("h_dRbbCut", "#DeltaR(b-quark, b-quark)", 100, 0, 10)));
  ANA_CHECK (book(TH1F("h_dRbjetbjet", "#DeltaR(b-jet, b-jet)", 100, 0, 10)));
  ANA_CHECK (book(TH1F("h_mHtautau", "tau^{+}_{h}#tau^{-}_{h} invariant mass", 150, 0, 150)));
  ANA_CHECK (book(TH1F("h_mHbb", "b#bar{b} invariant mass", 150, 0, 150)));

  return StatusCode::SUCCESS;
}



StatusCode MyTruthOneAnalysis :: execute ()
{
  // Here you do everything that needs to be done on every single
  // events, e.g. read input variables, apply cuts, and fill
  // histograms and trees.  This is where most of your actual analysis
  // code will go.

  // retrieve the eventInfo object from the event store
  const xAOD::EventInfo* eventInfo = nullptr;
  ANA_CHECK (evtStore()->retrieve (eventInfo, "EventInfo"));

  const xAOD::TruthEventContainer* truthEventContainer = nullptr;
  ANA_CHECK (evtStore()->retrieve (truthEventContainer, "TruthEvents"));

  // print out run and event number from retrieved object
  ANA_MSG_DEBUG ("in execute, runNumber = " << eventInfo->runNumber() << ", eventNumber = " << eventInfo->eventNumber());

  ANA_MSG_DEBUG ("in execute, truth event container size = " << truthEventContainer->size());
  const xAOD::TruthEvent* truthEvent = nullptr;
  if (truthEventContainer->size() == 1) {
    truthEvent = truthEventContainer->at(0);
  } else {
    ANA_MSG_WARNING ("in execute, no truth event container!");
    return StatusCode::SUCCESS;
  }

  // retrieve truth taus container
  const xAOD::TruthParticleContainer* truthTaus = nullptr;
  ANA_CHECK (evtStore()->retrieve (truthTaus, "TruthTaus"));
  ANA_MSG_DEBUG ("Truth taus container size: " << truthTaus->size());

  /*
   * TODO: i'm testing the AntiKt4TruthDressedWZJets collection ...
   */
  // retrieve jet container
  const xAOD::JetContainer* jets = nullptr;
  ANA_CHECK (evtStore()->retrieve (jets, "AntiKt4TruthDressedWZJets"));
  // should contain more than two jets!
  ANA_MSG_DEBUG ("Jet container size: " << jets->size());
  assert(jets->size() >= 2);

  // event info
  float event_run_number = float(eventInfo->runNumber());
  float event_event_number = float(eventInfo->eventNumber());

  hist("h_run_number")->Fill(event_run_number);
  hist("h_event_number")->Fill(event_event_number);

  // event weights
  const std::vector<float> weights = truthEvent->weights();
  // not the product
  // float mc_weights = std::accumulate(weights.begin(), weights.end(), 1, std::multiplies<float>());
  float mc_weights = weights[0];
  hist("h_mc_weights")->Fill(mc_weights);
  _bkp->addCut(1, "Initial", mc_weights);

  // objects
  std::map<std::string, const xAOD::TruthParticle*> higgsMap{};
  std::vector<const xAOD::TruthParticle*> truthTauVec{};
  std::vector<const xAOD::Jet*> truthJetVec{};

  bool found_htautau = false;
  bool found_hbb = false;

  for (std::size_t i = 0; i < truthEvent->nTruthParticles(); i++) {
    const xAOD::TruthParticle* particle = truthEvent->truthParticle(i);

    // Some debug output
    if (i == 0) {
      ANA_MSG_DEBUG ("Particle info: ");
      ANA_MSG_DEBUG (" - Barcode: " << particle->barcode());
      ANA_MSG_DEBUG (" - PDG ID : " << particle->pdgId());
      ANA_MSG_DEBUG (" - Pt     : " << particle->pt());
    }

    // fetch Higgs
    std::vector< unsigned int > trash_tautau_idx;
    if (!found_htautau && particle->pdgId() == 25 && particle->nChildren() == 2 && hasChild(particle, 15, trash_tautau_idx)) {
      found_htautau = true;
      higgsMap.insert({"Htautau", particle});
    }

    std::vector< unsigned int > trash_bb_idx;
    if (!found_hbb && particle->pdgId() == 25 && particle->nChildren() == 2 && hasChild(particle, 5, trash_tautau_idx)) {
      found_hbb = true;
      higgsMap.insert({"Hbb", particle});
    }
  }
  
  // fetch truth taus
  for (std::size_t i = 0; i < truthTaus->size(); i++) {
    if (isFromHiggs(truthTaus->at(i))) {
      truthTauVec.push_back(truthTaus->at(i));
    }
  }
  ANA_MSG_DEBUG ("Truth taus vector size: " << truthTauVec.size());

  // fetch b-jets
  std::vector<int> btag_idx{};
  for (std::size_t i = 0; i < jets->size(); i++) {
    ANA_MSG_DEBUG ("Jet truth flavour info: ");
    ANA_MSG_DEBUG (" - PartonTruthLabelID = " << jets->at(i)->auxdata<int>("PartonTruthLabelID"));
    ANA_MSG_DEBUG (" - HadronConeExclTruthLabelID = " << jets->at(i)->auxdata<int>("HadronConeExclTruthLabelID"));
    ANA_MSG_DEBUG (" - TrueFlavor = " << jets->at(i)->auxdata<int>("TrueFlavor"));
    if (isBJet(jets->at(i))) {  // isBJet -> TruthFlavor == 5
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
  const xAOD::TruthParticle* tau0 = getFinal(higgsMap["Htautau"]->child(0));
  const xAOD::TruthParticle* tau1 = getFinal(higgsMap["Htautau"]->child(1));
  // const xAOD::TruthParticle* b0   = getFinal(higgsMap["Hbb"]->child(0));
  // const xAOD::TruthParticle* b1   = getFinal(higgsMap["Hbb"]->child(1));
  const xAOD::TruthParticle* b0 = higgsMap["Hbb"]->child(0);
  const xAOD::TruthParticle* b1 = higgsMap["Hbb"]->child(1);

  // if less than two truth b-tag jet, push the leading jet of the remaining jets to the vec
  if (truthJetVec.size() < 2) {
    for (std::size_t i = 0; i < jets->size(); i++) {
      if (!contains(btag_idx, i) && (jets->at(i)->p4().DeltaR(tau0->p4()))>0.2 && (jets->at(i)->p4().DeltaR(tau1->p4()))>0.2) {
        truthJetVec.push_back(jets->at(i));
      }
      if (truthJetVec.size() == 2) break;
    }
  }
  // if (truthJetVec.size() < 2) {
  //   ANA_MSG_WARNING("Jet size < 2!!!");
  // }
  _printVec(btag_idx, "btag_idx");
  ANA_MSG_DEBUG ("Jet vector size: " << truthJetVec.size());

  // kinematics
  TLorentzVector tau0_p4 = tau0->p4(), tau1_p4 = tau1->p4();
  TLorentzVector tauvis0_p4 = tauVisP4(tau0), tauvis1_p4 = tauVisP4(tau1);
  TLorentzVector b0_p4 = b0->p4(), b1_p4 = b1->p4();

  ANA_MSG_DEBUG ("Htautau : " << higgsMap["Htautau"]->child(0)->pdgId() << ", " << higgsMap["Htautau"]->child(1)->pdgId());
  ANA_MSG_DEBUG ("Hbb     : " << higgsMap["Hbb"]->child(0)->pdgId() << ", " << higgsMap["Hbb"]->child(1)->pdgId());

  if (!isGoodEvent()) {
    return StatusCode::SUCCESS;
  }
  _bkp->addCut(2, "GoodEvent", mc_weights);

  if (!(isOS(tau0, tau1) && isOS(b0, b1))) {
    return StatusCode::SUCCESS;
  }
  _bkp->addCut(3, "OS-Charge", mc_weights);

  if (!(isGoodTau(tau0, 20., 2.5) && isGoodTau(tau1, 20., 2.5))) {
    return StatusCode::SUCCESS;
  }
  _bkp->addCut(4, "TauPreSel", mc_weights);

  if (!(isGoodB(b0, 20., 2.4) && isGoodB(b1, 20., 2.4))) {
    return StatusCode::SUCCESS;
  }
  _bkp->addCut(5, "BJetPreSel", mc_weights);

  if (!isNotOverlap(b0, b1, tau0, tau1, 0.2)) {  // dR>0.2
    return StatusCode::SUCCESS;
  }
  _bkp->addCut(6, "OverlapRM", mc_weights);

  bool STT = isGoodTau(tau0, 100., 2.5) && isGoodB(b0, 45., 2.4);
  bool DTT = isGoodTau(tau0, 40., 2.5) && isGoodTau(tau1, 30., 2.5) && isGoodB(b0, 80., 2.4);

  if (!STT && !DTT) {  // mimic different trigger channels 
    return StatusCode::SUCCESS;
  }
  _bkp->addCut(7, "TriggerPt", mc_weights);

  if ((tau0_p4+tau1_p4).M() < 60 * GeV) {  // use tau lepton to mimic MMC ...
    return StatusCode::SUCCESS;
  }
  _bkp->addCut(8, "DiTauMass", mc_weights);

  // deltaRs
  float deltaRtautau = tau0_p4.DeltaR(tau1_p4);
  float deltaRtauvistauvis = tauvis0_p4.DeltaR(tauvis1_p4);
  float deltaRbb = b0_p4.DeltaR(b1_p4);

  // invariant masses
  float mHtautau = (tauvis0_p4 + tauvis1_p4).M() / GeV;
  float mHbb = (b0_p4 + b1_p4).M() / GeV;

  // fill histograms
  hist("h_dRtautau")->Fill(deltaRtautau, mc_weights);
  hist("h_dRtauvistauvis")->Fill(deltaRtauvistauvis, mc_weights);
  hist("h_dRbb")->Fill(deltaRbb, mc_weights);
  hist("h_mHtautau")->Fill(mHtautau, mc_weights);
  hist("h_mHbb")->Fill(mHbb, mc_weights);

  if (truthJetVec.size() >= 2) {
    const xAOD::Jet* bjet0 = truthJetVec[0];
    const xAOD::Jet* bjet1 = truthJetVec[1];

    TLorentzVector bjet0_p4, bjet1_p4;
    bjet0_p4 = bjet0->p4();
    bjet1_p4 = bjet1->p4();
    float deltaRbjetbjet = bjet0_p4.DeltaR(bjet1_p4);
    hist("h_dRbjetbjet")->Fill(deltaRbjetbjet, mc_weights);
  }

  if (deltaRbb < 0.8) {
    hist("h_dRbbCut")->Fill(deltaRbb, mc_weights);
  }

  ANA_MSG_DEBUG ("Found Higgs -> tautau, delta R(tau, tau) : " << deltaRtautau);
  ANA_MSG_DEBUG ("Found Higgs -> bb,     delta R(b, b)     : " << deltaRbb);

  return StatusCode::SUCCESS;
}



StatusCode MyTruthOneAnalysis :: finalize ()
{
  // This method is the mirror image of initialize(), meaning it gets
  // called after the last event has been processed on the worker node
  // and allows you to finish up any objects you created in
  // initialize() before they are written to disk.  This is actually
  // fairly rare, since this happens separately for each worker node.
  // Most of the time you want to do your post-processing on the
  // submission node after all your histogram outputs have been
  // merged.
  ANA_MSG_INFO ("Finalizing ...");
  ANA_MSG_INFO (">>> CUTFLOW <<<");
  _bkp->print();
  ANA_MSG_INFO (">>> CUTFLOW <<<");
  return StatusCode::SUCCESS;
}



void MyTruthOneAnalysis :: _printVec (const std::vector<int>& v, const std::string& message) const 
{
  std::string s = "{ ";
  for (int element: v) {
      s += std::to_string(element);
      s += " ";
  }
  s += "}";
  ANA_MSG_DEBUG ("print vector -> " << message << " " << s);
}
