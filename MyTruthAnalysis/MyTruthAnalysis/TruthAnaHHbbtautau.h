#ifndef MyTruthAnalysis_TruthAnaHHbbtautau_H
#define MyTruthAnalysis_TruthAnaHHbbtautau_H

// Base class
#include <AnaAlgorithm/AnaAlgorithm.h>

// My class
#include "MyTruthAnalysis/Cutflow.h"
#include "MyTruthAnalysis/TruthAnaBase.h"

// std
#include <memory>

class TTree;

enum class CHAN { UNKNOWN=0, RESOLVED, BOOSTED }; //!

class TruthAnaHHbbtautau : public TruthAnaBase
{
public:
  // this is a standard algorithm constructor
  TruthAnaHHbbtautau(const std::string &name, ISvcLocator *pSvcLocator);

  // these are the functions inherited from Algorithm
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;

private:
  void initBranches();
  void resetBranches();

private:
  TTree *m_cTree = nullptr;          //!
  CHAN m_eChannel = CHAN::UNKNOWN;   //!

private:
  unsigned long long m_nEventNumber; //!
  unsigned long long m_nRunNumber;   //!
  unsigned long long m_nJets;        //!
  unsigned long long m_nFatJets;     //!

  double m_fTau0_pt;                 //!
  double m_fTau1_pt;                 //!
  double m_fTau0_phi;                //!
  double m_fTau1_phi;                //!
  double m_fTau0_eta;                //!
  double m_fTau1_eta;                //!

  double m_fTauVis0_pt;              //!
  double m_fTauVis1_pt;              //!
  double m_fTauVis0_phi;             //!
  double m_fTauVis1_phi;             //!
  double m_fTauVis0_eta;             //!
  double m_fTauVis1_eta;             //!

  double m_fB0_pt;                   //!
  double m_fB1_pt;                   //!
  double m_fB0_phi;                  //!
  double m_fB1_phi;                  //!
  double m_fB0_eta;                  //!
  double m_fB1_eta;                  //!

  double m_fBjet0_pt;                //!
  double m_fBjet1_pt;                //!
  double m_fBjet0_phi;               //!
  double m_fBjet1_phi;               //!
  double m_fBjet0_eta;               //!
  double m_fBjet1_eta;               //!

  // Large R jet
  double m_fDiBjet_pt;               //!
  double m_fDiBjet_m;                //!
  double m_fDiBjet_phi;              //!
  double m_fDiBjet_eta;              //!

  double m_fDeltaR_BB;               //!
  double m_fDeltaR_BjetBjet;         //!
  double m_fDeltaR_TauTau;           //!
  double m_fDeltaR_TauVisTauVis;     //!
  double m_fDeltaR_BB_TauTau;        //!

  double m_fDeltaR_BjetBjet_TauVisTauVis; //!
  double m_fDeltaR_DiBjet_TauVisTauVis;   //!

  double m_fMBB;                     //!
  double m_fMTauTau;                 //!
  double m_fMBjetBjet;               //!
  double m_fMTauVisTauVis;           //!
  double m_fMHH;                     //!

  double m_fPtBB;                    //!
  double m_fPtTauTau;                //!
  double m_fMCWeight;                //!

  unsigned long long m_nChannel;     //!
};

#endif
