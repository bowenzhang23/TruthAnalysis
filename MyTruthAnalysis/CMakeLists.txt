# The name of the package:
atlas_subdir (MyTruthAnalysis)

# Add the shared library:
atlas_add_library (MyTruthAnalysisLib
  MyTruthAnalysis/*.h Root/*.cxx
  PUBLIC_HEADERS MyTruthAnalysis
  LINK_LIBRARIES AnaAlgorithmLib xAODEventInfo 
  xAODTruth xAODJet)

if (XAOD_STANDALONE)
 # Add the dictionary (for AnalysisBase only):
 atlas_add_dictionary (MyTruthAnalysisDict
  MyTruthAnalysis/MyTruthAnalysisDict.h
  MyTruthAnalysis/selection.xml
  LINK_LIBRARIES MyTruthAnalysisLib)
endif ()

if (NOT XAOD_STANDALONE)
  # Add a component library for AthAnalysis only:
  atlas_add_component (MyTruthAnalysis
    src/components/*.cxx
    LINK_LIBRARIES MyTruthAnalysisLib)
endif ()

# Install files from the package:
atlas_install_joboptions( share/*_jobOptions.py )
atlas_install_scripts( share/RunTruthAnalysis.py )
