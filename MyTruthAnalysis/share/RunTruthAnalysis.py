#!/usr/bin/env python

# Read the submission directory as a command line argument. You can
# extend the list of arguments with your private ones later on.
import optparse
parser = optparse.OptionParser()
parser.add_option( '-s', '--submission-dir', dest = 'submission_dir',
                   action = 'store', type = 'string', default = 'submitDir',
                   help = 'Submission directory for EventLoop' )
parser.add_option( '-i', '--input-dir', dest = 'input_dir',
                   action = 'store', type = 'string',
                   default = '/scratchfs/atlas/zhangbw/EVNT/mc16_13TeV.345835.aMcAtNloHerwig7EvtGen_UEEE5_CTEQ6L1_CT10ME_hh_ttbb_hh.merge.DAOD_TRUTH1.root/',
                   help = 'Path to input TRUTH1 files')
parser.add_option( '-p', '--file-pattern', dest = 'file_pattern',
                   action = 'store', type = 'string',
                   default = 'DAOD_TRUTH1.test.pool.*.root', 
                   help = 'Pattern of the input files for SampleHandler')
parser.add_option( '-n', '--n-events', dest = 'n_events',
                   action = 'store', type = 'int',
                   default = -1, help = 'Number of events to run')
( options, args ) = parser.parse_args()

# Set up (Py)ROOT.
import ROOT
ROOT.xAOD.Init().ignore()

# Set up the sample handler object.
import os
sh = ROOT.SH.SampleHandler()
sh.setMetaString( 'nc_tree', 'CollectionTree' )
inputFilePath = options.input_dir
ROOT.SH.ScanDir().filePattern( options.file_pattern ).scan( sh, inputFilePath )
sh.printContent()

# Create an EventLoop job.
job = ROOT.EL.Job()
job.outputAdd ( ROOT.EL.OutputStream ('TruthAna') )
job.sampleHandler( sh )
if options.n_events > 0:
    job.options().setDouble( ROOT.EL.Job.optMaxEvents, options.n_events )
job.options().setString( ROOT.EL.Job.optSubmitDirMode, 'unique-link')

# Create the algorithm's configuration.
from AnaAlgorithm.DualUseConfig import createAlgorithm
alg = createAlgorithm ( 'TruthAnaHHbbtautau', 'AnalysisAlg' )
alg.OutputLevel = ROOT.MSG.INFO

# Add our algorithm to the job
job.algsAdd( alg )

# Run the job using the direct driver.
driver = ROOT.EL.DirectDriver()
driver.submit( job, options.submission_dir )
