
from apruns import APRuns
from spikescore import SpikeScore
from detection import Detection
from metric import Metric
from seizurescore import SeizureScore
from pareto import Pareto, get_AUC

import timescoring.annotations
import timescoring.scoring

import scipy.io as spio
import copy
import numpy as np
import pandas as pd
from datetime import datetime

# ====================================================
# Define various parameters
# ====================================================

# List of subjects
subjectList = ['P1','P2','P3','P4','P5','P6','S1','S2']
Nsubjects = len(subjectList)

numSamples = 3720
fs = 2 # Sample rate of AP detection results (0.5-s buffers) and metric computation
duration = numSamples / fs


# Training sets (leave-one-out)
trainingPoolSubjects = [[1,2,3,4,5,6,7], 
                        [0,2,3,4,5,6,7],
                        [0,1,3,4,5,6,7],
                        [0,1,2,4,5,6,7],
                        [0,1,2,3,5,6,7],
                        [0,1,2,3,4,6,7],
                        [0,1,2,3,4,5,7],
                        [0,1,2,3,4,5,6]]
trainingPoolSizes = [len(trainingPoolSubjects[i]) for i in range(len(trainingPoolSubjects))]

# Details about data
spikeFs = 20e3 # Sample rate of input signal of AP detection
spikeTimeTolerance = 1e-3 # delay ap in to out in behav < 0.1 ms
spikeLocTolerance = int(np.round(spikeTimeTolerance * spikeFs))
spikeRefCategory = 'ref'
spikeRecordingLength = int(np.round(duration * spikeFs))

baselineStartIdx = 201 # idx from which the baseline interval starts
baselineEndIdx = 700 # idx at which the baseline interval ends and the test interval starts
testStartTime = baselineEndIdx / fs
testNumSamples = numSamples - baselineEndIdx
dataAmpMult = 20000 # amplitude data was pre-multiplied by 20,000 to avoid loosing precision

# AP runs
APRunsDir = '../outputs/' # General folder where AP detection results are stored
APRunCategory = 'test' # Sub-folder in APRunsDir
corrThresh = 75 # Correlation threshold used for AP detection
APResultsDir = '{:s}{:s}/ap_out/corrThresh{:d}/'.format(APRunsDir, APRunCategory, corrThresh)
APRefResultsDir = '{:s}{:s}/ap_out/corrThresh{:d}/'.format(APRunsDir, spikeRefCategory, corrThresh)

# Metric computation parameters
fgSize = 20
bgSize = 180

# Detection parameters to vary
thresholdSweep = np.linspace(1,100,100)
dotSweep = np.linspace(0.5,10,20, dtype=int)

tieBreaker = 'detectionDelay'
tieBreakerDir = 'min'

# Scoring parameters
maxEventDuration = 300
minDurationBetweenEvents = 90
toleranceEnd = bgSize / fs # toleranceStart is adapted later to match the start of the I1 interval for each subject
minOverlap = 0

# Save results
saveDir = f'{APRunsDir}{APRunCategory}/'
saveToCSV = True
saveResFig = True


# ====================================================
# AP runs
# ====================================================   

# Define parameters of the AP runs to classify     
apParam = APRuns.APParam(fs=fs, runCategory=APRunCategory, corrThresh=corrThresh, baselineStartIdx=baselineStartIdx,
                         baselineEndIdx=baselineEndIdx, dataAmpMult=dataAmpMult)
# Get AP runs results
apRuns = APRuns(subjectList, apParam=apParam, results_dir=APResultsDir)

print('AP runs loaded')

# ====================================================
# Spike scoring
# ====================================================   

# Get reference AP runs results for spike metrics
apParam_ref = APRuns.APParam(fs=fs, runCategory=spikeRefCategory, corrThresh=corrThresh, baselineStartIdx=baselineStartIdx,
                         baselineEndIdx=baselineEndIdx, dataAmpMult=dataAmpMult)
apRuns_ref = APRuns(subjectList, apParam=apParam, results_dir=APRefResultsDir)

spikeScore = [SpikeScore(apRuns_ref.apResults[n], apRuns.apResults[n], spikeLocTolerance, spikeRecordingLength, int(baselineStartIdx * spikeFs/fs)) for n in range(Nsubjects)]

# ====================================================
# Reference seizure events
# ====================================================

# Define reference annotations from EEG-defined seizure times
refAnnotations = [None for _ in range(Nsubjects)]
refSeizuresFilenames = [None for _ in range(Nsubjects)]
scoringParams = [None for _ in range(Nsubjects)] # Different preictal tolerance for each subject

for n,subject in enumerate(subjectList):

    refSeizuresFilenames[n] = '../dummy_ref_seizures/{:s}_seizure_info.mat'.format(subject)
    M = spio.loadmat(refSeizuresFilenames[n], squeeze_me=True)
    print(M['seizure_times'])
    refEvents = np.asarray(M['seizure_times'], dtype='float') - (baselineEndIdx/fs) # Remove time offset to start seizure detection at time 0
    print(refEvents)
    Nseizures = M['N_seizures']

    if Nseizures == 0: # Control group
        refAnnotations[n] = timescoring.annotations.Annotation([], fs, testNumSamples)
        toleranceStart = 0.0

    elif Nseizures == 1: # One seizure in recording
        refEvents_tuple = [None]
        refEvents_tuple[0] = (refEvents[2], refEvents[1]) # (Start I2, start I5)
        refAnnotations[n] = timescoring.annotations.Annotation(refEvents_tuple, fs, testNumSamples)
        # Define toleranceStart in scoringParam to accept detections from the start of PTZ injection (= start I1)
        toleranceStart = refEvents[2] - refEvents[0]

    else:
        raise ValueError('Non-supported number of seizures')
    
    # No min or max duration constrains on reference events
    scoringParams[n] = timescoring.scoring.EventScoring.Parameters(minDurationBetweenEvents=0.0,
                                                                   maxEventDuration=9e99,
                                                                   minOverlap=minOverlap,
                                                                   toleranceStart=toleranceStart,
                                                                   toleranceEnd=toleranceEnd)

# ====================================================   
# Seizure metric computation
# ====================================================   

# Compute metric
metricParam = Metric.MetricParameters(fgSize=fgSize, bgSize=bgSize, fs=fs)
metric = Metric(apRuns, param=metricParam)
print('Metrics computed')

# RMS score
rmsScore = metric.compute_rms_score(refAnnotations, scoringParams)

print('RMS score computed. Mean score: {:.2f} dB'.format(20*np.log10(np.nanmean(rmsScore))))


# =============================================================================
# Performance in threshold sweep
# =============================================================================

# Sweep over threshold and DoT to perform detection and save events in Annotation list
Ni = len(thresholdSweep)
Nj = len(dotSweep)
hypAnnotations = [[[None for _ in range(Nsubjects)] for _ in range(Nj)] for _ in range(Ni)]
for i in range(Ni):
    for j in range(Nj):
        detectionParam = Detection.DetectionParameters(threshold=thresholdSweep[i], minDuration=dotSweep[j], detectionStartIdx=0)
        detection_ij = Detection(metric, param=detectionParam)
        detectedEvents_ij = detection_ij.detectedEvents
        for n in range(Nsubjects):
            nevents = len(detectedEvents_ij[n])
            detectedEvents_ijn = [None for _ in range(nevents)]
            for k in range(nevents):
                detectedEvents_ijn[k] = (detectedEvents_ij[n][k][0], detectedEvents_ij[n][k][1])
            hyp_tmp = timescoring.annotations.Annotation(detectedEvents_ijn, fs, testNumSamples)
            # Apply min duration between events and max event duration constrains
            hyp_tmp = timescoring.scoring.EventScoring._mergeNeighbouringEvents(hyp_tmp, minDurationBetweenEvents)
            hypAnnotations[i][j][n] = timescoring.scoring.EventScoring._splitLongEvents(hyp_tmp, maxEventDuration)
            
print('Hypothesis annotations done')

# =============================================================================
# LOO training
# =============================================================================

# Definition of training and test sets in leave-one-out method
hypTrainingSet = [[[[None for _ in range(trainingPoolSizes[n])] for _ in range(Nj)] for _ in range(Ni)] for n in range(Nsubjects)]
refTrainingSet = [[None for _ in range(trainingPoolSizes[n])] for n in range(Nsubjects)]
trainingSetScoringParams = [[None for _ in range(trainingPoolSizes[n])] for n in range(Nsubjects)]
hypTestSet = [[[None for _ in range(Nj)] for _ in range(Ni)] for _ in range(Nsubjects)]
refTestSet = [None for _ in range(Nsubjects)]
testSetScoringParams = [None for _ in range(Nsubjects)]

for n in range(Nsubjects):
    for i in range(Ni):
        for j in range(Nj):
            for k,nTraining in enumerate(trainingPoolSubjects[n]):
                hypTrainingSet[n][i][j][k] = copy.deepcopy(hypAnnotations[i][j][nTraining])
            hypTestSet[n][i][j] = copy.deepcopy(hypAnnotations[i][j][n])
    for k,nTraining in enumerate(trainingPoolSubjects[n]):
        refTrainingSet[n][k] = copy.deepcopy(refAnnotations[nTraining])
        trainingSetScoringParams[n][k] = copy.deepcopy(scoringParams[nTraining])
    refTestSet[n] = copy.deepcopy(refAnnotations[n])
    testSetScoringParams[n] = copy.deepcopy(scoringParams[n])

# Evaluation of performance of training set
trainingPerf = [[[SeizureScore(refTrainingSet[n], hypTrainingSet[n][i][j], trainingSetScoringParams[n], basic=True) for j in range(Nj)] for i in range(Ni)] for n in range(Nsubjects)]
print('LOO training seizure score computed')

# Sorting of training results
paretoTraining = [Pareto(trainingPerf[n], objective1Name='eventSensitivity', objective2Name='eventPrecision', tieBreakerName=tieBreaker, tieBreakerDir=tieBreakerDir, plotSets=False) for n in range(Nsubjects)]
selectedThresh = [thresholdSweep[paretoTraining[n].selected_i] for n in range(Nsubjects)]
selectedDoT = [dotSweep[paretoTraining[n].selected_j] for n in range(Nsubjects)]

print('LOO training Pareto front extracted')

# =============================================================================
# Post-training test AUC
# =============================================================================

# Extract AUC on test set
testParetoPerf = [[SeizureScore([refTestSet[n]], [hypTestSet[n][paretoTraining[n].Pareto_i[k]][paretoTraining[n].Pareto_j[k]]], [scoringParams[n]], basic=True) for k in range(len(paretoTraining[n].Pareto_i))] for n in range(Nsubjects)]
testAUC = [get_AUC(testParetoPerf[n], 'eventSensitivity', 'eventPrecision', flat=True, plotROC=False) for n in range(Nsubjects)]
print('Post-classification individual AUC computed. Mean AUC: {:.2f} %'.format(np.nanmean(testAUC)*100))

# =============================================================================
# Test
# =============================================================================

# Evaluate on test set with one single selected pair of thresholds
testPerf = [SeizureScore([refTestSet[n]], [hypTestSet[n][paretoTraining[n].selected_i][paretoTraining[n].selected_j]], [testSetScoringParams[n]]) for n in range(Nsubjects)]
print('Test set seizure score computed')

# Print results
print('\n\n===== Selected parameters ====')
print('\tMetric sampling rate: {:.1f} S/s'.format(fs))
print('\tRun category: {:s}'.format(APRunCategory))
print('\tAP correlation threshold: {:.1f}%'.format(corrThresh))
print('\tMetric FG size: {:d} samples'.format(fgSize))
print('\tMetric BG size: {:d} samples'.format(bgSize))
print('')
print('===== Classification results ====')
for n in range(Nsubjects):
    print('Subject {:s}'.format(subjectList[n]))
    print('\tSelected threshold: {:.1f}'.format(selectedThresh[n]))
    print('\tSelected DoT: {:.1f}'.format(selectedDoT[n]))
    print('\tSample Sensitivity: {:.3f}%'.format(testPerf[n].sampleSensitivity * 100))
    print('\tSample Precision: {:.3f}%'.format(testPerf[n].samplePrecision * 100))
    print('\tSample F1: {:.3f}%'.format(testPerf[n].sampleF1 * 100))
    print('\tSample FPRate: {:.3f} FP/day'.format(testPerf[n].sampleFPRate))
    print('\tEvent Sensitivity: {:.3f}%'.format(testPerf[n].eventSensitivity * 100))
    print('\tEvent Precision: {:.3f}%'.format(testPerf[n].eventPrecision * 100))
    print('\tEvent F1: {:.3f}%'.format(testPerf[n].eventF1 * 100))
    print('\tEvent FPRate: {:.3f} FP/day'.format(testPerf[n].eventFPRate))
    print('\tMedian Detection Delay: {:.3f}s'.format(testPerf[n].detectionDelay))
    print('\tSpike Sensitivity: {:.3f}%'.format(spikeScore[n].sensitivity*100))
    print('\tSpike Precision: {:.3f}%'.format(spikeScore[n].precision * 100))
    print('\tSpike F1: {:.3f}%'.format(spikeScore[n].f1 * 100))
    print('\tRMS Score: {:.3f} dB'.format(20*np.log10(rmsScore[n])))
    print('\tPost-training AUC: {:.3f} %'.format(testAUC[n] * 100))
    print('')
    
print('===== Agregated results ====')
print('\tMean Spike Sensitivity: {:.3f}%'.format(np.nanmean([sscore.sensitivity for sscore in spikeScore])*100))
print('\tMean Spike Precision: {:.3f}%'.format(np.nanmean([sscore.precision for sscore in spikeScore])*100))
print('\tMean RMS Score: {:.3f} dB'.format(20*np.log10(np.nanmean(rmsScore))))
print('\tMean Event Sensitivity: {:.3f}%'.format(np.nanmean([perf.eventSensitivity for perf in testPerf]) * 100))
print('\tMean Event Precision: {:.3f}%'.format(np.nanmean([perf.eventPrecision for perf in testPerf]) * 100))
print('\tMean Event F1: {:.3f}%'.format(np.nanmean([perf.eventF1 for perf in testPerf]) * 100))
print('\tMean Event FAR: {:.3f} FP/day'.format(np.nanmean([perf.eventFPRate for perf in testPerf])))
print('\tMean Detection Delay: {:.3f} s'.format(np.nanmean([perf.detectionDelay for perf in testPerf])))
print('\tMean AUC: {:.3f} %'.format(np.nanmean(testAUC)*100))   

# Plot and save results
save_name = '{:s}/classification_{:s}_{:d}_{:s}'.format(saveDir, APRunCategory, corrThresh, datetime.now().strftime('%Y-%m-%d_%H-%M'))
metric.plot_amp_freq(refAnnotations, subjectList, save_name + '_amp_freq.png')
metric.plot_with_results(testPerf, subjectList, selectedThresh, saveFig=saveResFig, figName=save_name + '.png')


if saveToCSV:
    
    # Save results to csv
    saveRes = {}
    
    saveRes['subject'] = [None for _ in range(Nsubjects+1)]
    saveRes['sampling_rate'] = fs
    saveRes['run_category'] = APRunCategory
    saveRes['correlation_threshold'] = corrThresh
    saveRes['fg_size'] = fgSize
    saveRes['bg_size'] = bgSize
    
    saveRes['threshold']            = np.zeros(Nsubjects+1)
    saveRes['dot']                  = np.zeros(Nsubjects+1)
    saveRes['spike_sensitivity']    = np.zeros(Nsubjects+1)
    saveRes['spike_precision']      = np.zeros(Nsubjects+1)
    saveRes['spike_f1']             = np.zeros(Nsubjects+1)
    saveRes['rms_score']            = np.zeros(Nsubjects+1)
    saveRes['sample_sensitivity']   = np.zeros(Nsubjects+1)
    saveRes['sample_precision']     = np.zeros(Nsubjects+1)
    saveRes['sample_f1']            = np.zeros(Nsubjects+1)
    saveRes['sample_fprate']        = np.zeros(Nsubjects+1)
    saveRes['event_sensitivity']    = np.zeros(Nsubjects+1)
    saveRes['event_precision']      = np.zeros(Nsubjects+1)
    saveRes['event_f1']             = np.zeros(Nsubjects+1)
    saveRes['event_fprate']         = np.zeros(Nsubjects+1)
    saveRes['detection_delay']      = np.zeros(Nsubjects+1)
    saveRes['test_auc']             = np.zeros(Nsubjects+1)
    
    for n in range(Nsubjects):
        saveRes['subject'][n]               = subjectList[n]
        saveRes['threshold'][n]             = selectedThresh[n]
        saveRes['dot'][n]                   = selectedDoT[n]
        saveRes['spike_sensitivity'][n]     = spikeScore[n].sensitivity
        saveRes['spike_precision'][n]       = spikeScore[n].precision
        saveRes['spike_f1'][n]              = spikeScore[n].f1
        saveRes['rms_score'][n]             = 20*np.log10(rmsScore[n])
        saveRes['sample_sensitivity'][n]    = testPerf[n].sampleSensitivity
        saveRes['sample_precision'][n]      = testPerf[n].samplePrecision
        saveRes['sample_f1'][n]             = testPerf[n].sampleF1
        saveRes['sample_fprate'][n]         = testPerf[n].sampleFPRate
        saveRes['event_sensitivity'][n]     = testPerf[n].eventSensitivity
        saveRes['event_precision'][n]       = testPerf[n].eventPrecision
        saveRes['event_f1'][n]              = testPerf[n].eventF1
        saveRes['event_fprate'][n]          = testPerf[n].eventFPRate
        saveRes['detection_delay'][n]       = testPerf[n].detectionDelay
        saveRes['test_auc'][n]              = testAUC[n]
        
    saveRes['subject'][Nsubjects]               = 'Mean'
    saveRes['threshold'][Nsubjects]             = np.nanmean(selectedThresh)
    saveRes['dot'][Nsubjects]                   = np.nanmean(selectedDoT)
    saveRes['spike_sensitivity'][Nsubjects]     = np.nanmean(saveRes['spike_sensitivity'][:Nsubjects-1])
    saveRes['spike_precision'][Nsubjects]       = np.nanmean(saveRes['spike_precision'][:Nsubjects-1])
    saveRes['spike_f1'][Nsubjects]              = np.nanmean(saveRes['spike_f1'][:Nsubjects-1])
    saveRes['rms_score'][Nsubjects]             = 20*np.log10(np.nanmean(rmsScore))
    saveRes['sample_sensitivity'][Nsubjects]    = np.nanmean(saveRes['sample_sensitivity'][:Nsubjects-1])
    saveRes['sample_precision'][Nsubjects]      = np.nanmean(saveRes['sample_precision'][:Nsubjects-1])
    saveRes['sample_f1'][Nsubjects]             = np.nanmean(saveRes['sample_f1'][:Nsubjects-1])
    saveRes['sample_fprate'][Nsubjects]         = np.nanmean(saveRes['sample_fprate'][:Nsubjects-1])
    saveRes['event_sensitivity'][Nsubjects]     = np.nanmean(saveRes['event_sensitivity'][:Nsubjects-1])
    saveRes['event_precision'][Nsubjects]       = np.nanmean(saveRes['event_precision'][:Nsubjects-1])
    saveRes['event_f1'][Nsubjects]              = np.nanmean(saveRes['event_f1'][:Nsubjects-1])
    saveRes['event_fprate'][Nsubjects]          = np.nanmean(saveRes['event_fprate'][:Nsubjects-1])
    saveRes['detection_delay'][Nsubjects]       = np.nanmean(saveRes['detection_delay'][:Nsubjects-1])
    saveRes['test_auc'][Nsubjects]              = np.nanmean(testAUC*100)

    saveRes_df = pd.DataFrame(saveRes)

    saveRes_df.to_csv(save_name + '.csv')