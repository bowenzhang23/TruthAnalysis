#ifndef MyTruthAnalysis_MyTruthOneAnalysis_H
#define MyTruthAnalysis_MyTruthOneAnalysis_H

// Base class
#include <AnaAlgorithm/AnaAlgorithm.h>

// My class
#include "MyTruthAnalysis/Bookkeeper.h"

// std
#include <memory>


class MyTruthOneAnalysis : public EL::AnaAlgorithm
{
 public:
  // this is a standard algorithm constructor
  MyTruthOneAnalysis (const std::string& name, ISvcLocator* pSvcLocator);

  // these are the functions inherited from Algorithm
  virtual StatusCode initialize () override;
  virtual StatusCode execute () override;
  virtual StatusCode finalize () override;

 private:
  void _printVec(const std::vector<int>& v, const std::string& message) const;
  std::unique_ptr<Bookkeeper> _bkp;
};

#endif
