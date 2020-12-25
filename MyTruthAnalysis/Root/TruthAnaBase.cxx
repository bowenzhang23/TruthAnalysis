// AsgTools
#include <AsgTools/MessageCheck.h>

// ROOT
#include <TLorentzVector.h>
#include <TH1.h>

// My headers
#include "MyTruthAnalysis/TruthAnaBase.h"
#include "MyTruthAnalysis/HelperFunctions.h"

// std
#include <memory>

using namespace TruthAna;

TruthAnaBase::TruthAnaBase(const std::string &name,
                            ISvcLocator *pSvcLocator)
    : EL::AnaAlgorithm(name, pSvcLocator)
{
  m_cCutflow = std::make_unique<Cutflow>();
}

StatusCode TruthAnaBase::initialize()
{
  return StatusCode::SUCCESS;
}

StatusCode TruthAnaBase::execute()
{
  return StatusCode::SUCCESS;
}

StatusCode TruthAnaBase::finalize()
{
  return StatusCode::SUCCESS;
}
