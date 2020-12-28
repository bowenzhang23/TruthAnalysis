#ifndef MyTruthAnalysis_TruthAnaBase_H
#define MyTruthAnalysis_TruthAnaBase_H

// Base class
#include <AnaAlgorithm/AnaAlgorithm.h>

// My class
#include "MyTruthAnalysis/Cutflow.h"

// std
#include <memory>

class TruthAnaBase : public EL::AnaAlgorithm
{
public:
  // this is a standard algorithm constructor
  TruthAnaBase(const std::string &name, ISvcLocator *pSvcLocator);

  // these are the functions inherited from Algorithm
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;

protected:
  std::unique_ptr<Cutflow> m_cCutflow; //!

private:
};

#endif

#define APPLYCUT(criteria, name) \
  if (!(criteria))                 \
  {                              \
    return StatusCode::SUCCESS;  \
  }                              \
  m_cCutflow->addCut(name, m_fMCWeight);

#define APPLYCOUNT(criteria, name) \
  if ((criteria))                 \
  {                              \
    m_cCutflow->addCut(name, m_fMCWeight);  \
  }
  