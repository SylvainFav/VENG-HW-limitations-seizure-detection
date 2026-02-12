
import numpy as np
import timescoring.scoring
import utils.misc

class SeizureScore:

    """Computes the event and sample scoring of hypothesis annotations compared to reference annotations"""
    
    def __init__(self, ref, hyp, scoringParams, basic=False):

        # "basic" parameter refers to the level of details in the performance computation
        
        Nrec = len(ref)
        
        if basic:
            results = {"duration": np.zeros(Nrec),
                       "tp_event": np.zeros(Nrec),
                       "fp_event": np.zeros(Nrec),
                       "refTrue_event": np.zeros(Nrec),
                       "event_sensitivity": np.zeros(Nrec),
                       "event_precision": np.zeros(Nrec),
                       "event_f1": np.zeros(Nrec),
                       "event_fprate": np.zeros(Nrec),
                       "detection_delay": np.zeros(Nrec)
                       }
            self.eventScores = [None for _ in range(Nrec)]
            
        else:
            results = {"duration": np.zeros(Nrec),
                       "tp_sample": np.zeros(Nrec),
                       "fp_sample": np.zeros(Nrec),
                       "refTrue_sample": np.zeros(Nrec),
                       "sample_sensitivity": np.zeros(Nrec),
                       "sample_precision": np.zeros(Nrec),
                       "sample_f1": np.zeros(Nrec),
                       "sample_fprate": np.zeros(Nrec),
                       "tp_event": np.zeros(Nrec),
                       "fp_event": np.zeros(Nrec),
                       "refTrue_event": np.zeros(Nrec),
                       "event_sensitivity": np.zeros(Nrec),
                       "event_precision": np.zeros(Nrec),
                       "event_f1": np.zeros(Nrec),
                       "event_fprate": np.zeros(Nrec),
                       "detection_delay": np.zeros(Nrec)
                       }
            self.sampleScores = [None for _ in range(Nrec)]
            self.eventScores = [None for _ in range(Nrec)]
        
        self.ref = ref
        self.hyp = hyp
        

        for n in range(Nrec):
            
            ref_n = ref[n]
            hyp_n = hyp[n]

            eventScore = timescoring.scoring.EventScoring(ref_n, hyp_n, param=scoringParams[n])
            results["duration"][n] = (len(ref_n.mask)-1) / ref_n.fs
            results["tp_event"][n] = eventScore.tp
            results["fp_event"][n] = eventScore.fp
            results["refTrue_event"][n] = eventScore.refTrue
            results["detection_delay"][n] = np.mean(SeizureScore._computeDectectionDelay(eventScore))
            results["event_sensitivity"][n], results["event_precision"][n], results["event_f1"][n], results["event_fprate"] = SeizureScore._computeScore(results["tp_event"][n], results["fp_event"][n], results["refTrue_event"][n], results["duration"][n])
            self.eventScores[n] = eventScore

            if not basic:
                sampleScore = timescoring.scoring.SampleScoring(ref_n, hyp_n)        
                results["tp_sample"][n] = sampleScore.tp
                results["fp_sample"][n] = sampleScore.fp
                results["refTrue_sample"][n] = sampleScore.refTrue
                results["sample_sensitivity"][n], results["sample_precision"][n], results["sample_f1"][n], results["sample_fprate"] = SeizureScore._computeScore(results["tp_sample"][n], results["fp_sample"][n], results["refTrue_sample"][n], results["duration"][n])
                self.sampleScores[n] = sampleScore

        self.detectionDelay = utils.misc.nanmean_rnan(results["detection_delay"])
        self.eventSensitivity = utils.misc.nanmean_rnan(results["event_sensitivity"])
        self.eventPrecision = utils.misc.nanmean_rnan(results["event_precision"])
        self.eventF1 = utils.misc.nanmean_rnan(results["event_f1"])
        self.eventFPRate = utils.misc.nanmean_rnan(results["event_fprate"])

        if not basic:
            self.sampleSensitivity = utils.misc.nanmean_rnan(results["sample_sensitivity"])
            self.samplePrecision = utils.misc.nanmean_rnan(results["sample_precision"])
            self.sampleF1 = utils.misc.nanmean_rnan(results["sample_f1"])
            self.sampleFPRate = utils.misc.nanmean_rnan(results["sample_fprate"])
        
        self.results = results

    def __eq__(self, other):
        if type(other) is type(self):
            equal_attributes = ['sampleSensitivity', 'samplePrecision', 'sampleF1', 'sampleFPRate',\
                                'eventSensitivity', 'eventPrecision', 'eventF1', 'eventFPRate', \
                                'detectionDelay']
            for attr in equal_attributes:
                if getattr(self, attr) != getattr(other, attr):
                    return False
            return True
        else:
            return False
    
    def __ne__(self, other):
        return not self.__eq__(other)
    
    def __str__(self):
        return 'SeizureScore object\n' + str(self.__dict__) + '\n'
    
    def _computeDectectionDelay(eventScore):

        """Computes the detection delay based on the tpMask in eventScore"""
        
        tpStartIdxs = np.where(np.diff(eventScore.tpMask.astype('int')) == 1)[0]
        tpStopIdxs = np.where(np.diff(eventScore.tpMask.astype('int')) == -1)[0]
        
        if len(tpStartIdxs) == 0:
            return np.nan

        else:
            detectionDelays = [None for _ in range(len(tpStartIdxs))]
            for i in range(len(tpStartIdxs)): # for each ref event
                hypStartRise = np.flatnonzero(np.diff(eventScore.hyp.mask[tpStartIdxs[i]:tpStopIdxs[i]].astype('int')) == 1)
                if len(hypStartRise) == 0:
                    # hyp start was before start of tpMask
                    detectionDelays[i] = 0.0
                else:
                    refStart = np.flatnonzero(np.diff(eventScore.ref.mask[tpStartIdxs[i]:tpStopIdxs[i]].astype('int')) == 1)[0]
                    detectionDelays[i] = (hypStartRise[0] - refStart) / eventScore.fs
            
            return detectionDelays
    
    def _computeScore(tp, fp, refTrue, duration):

        """Computes scores"""

        # Sensitivity
        if refTrue > 0:
            sensitivity = tp / refTrue
        else: # no ref event
            sensitivity = np.nan
        
        # Precision
        if tp + fp > 0:
            precision = tp / (tp + fp)
        else:
            precision = np.nan  # no hyp event
            # precision = 1.0
        
        # F1 Score
        if np.isnan(sensitivity) or np.isnan(precision):
            f1 = np.nan
        elif (sensitivity + precision) == 0:  # No overlap ref & hyp
            f1 = 0
        else:
            f1 = 2 * sensitivity * precision / (sensitivity + precision)
        
        # FP Rate
        fpRate = fp / (duration / 3600 / 24)  # FP per day
        
        return sensitivity, precision, f1, fpRate    
