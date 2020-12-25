# Set up

(1) make a workspace
mkdir new_folder
cd $_

(2) create a script to set up
in `setup.sh`:

```
setupATLAS -q
lsetup git
mkdir -p build run
git clone git@github.com:peppapiggyme/TruthAnalysis.git source
pushd build
asetup AnalysisBase,21.2.104,here
\cp CMakeLists.txt ../source/
cmake ../source
make
source x86*/setup.sh
popd
```

(3) every login, do this
```
setupATLAS -q && pushd build && asetup && source x86*/setup.sh && popd
```

(4) to execute
```
RunTruthAnalysis.py <options>
```

# To do
sooner or later move to new signal samples
```
mc16_13TeV.600023.PhH7EG_PDF4LHC15_HHbbtautauHadHad_cHHH01d0.merge.EVNT.e7954_e7400
```
