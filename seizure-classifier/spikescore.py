
import numpy as np
from apruns import APRuns

class SpikeScore:

    def __init__(self,
                 aprun_ref: APRuns.APSubjectResult,
                 aprun_hyp: APRuns.APSubjectResult,
                 locTolerance: int,
                 recordingLength: int,
                 startLoc: int):
        
        self.aprun_ref = aprun_ref
        self.aprun_hyp = aprun_hyp
        self.locTolerance = locTolerance
        self.recordingLength = recordingLength
        self.startLoc = startLoc

        ref_mask = self._createSpikeMask(self.aprun_ref, locTolerance=self.locTolerance)
        hyp_mask = self._createSpikeMask(self.aprun_hyp, locTolerance=0)

        tp_mask = ref_mask & hyp_mask
        fp_mask = ~ref_mask & hyp_mask

        tp = np.sum(tp_mask)
        fp = np.sum(fp_mask)

        self.sensitivity, self.precision, self.f1 = self._computeScore(tp, fp, len(self.aprun_ref.spike_locs))

        return

    
    def _createSpikeMask(self, aprun, locTolerance):
        
        mask = np.zeros(self.recordingLength, dtype=int)
        for loc in aprun.spike_locs:
            if loc >= self.startLoc:
                if locTolerance > 0:
                    mask[loc-locTolerance:loc+locTolerance] = 1
                else:
                    mask[loc] = 1

        return mask
    
    def _computeScore(self, tp, fp, Nref):
    
        if Nref > 0:
            sensitivity = tp / Nref
        else:
            sensitivity = np.nan
        
        if tp + fp > 0:
            precision = tp / (tp + fp)
        else:
            precision = np.nan

        if np.isnan(sensitivity) or np.isnan(precision):
            f1 = np.nan
        elif (sensitivity + precision) == 0:
            f1 = 0.0
        else:
            f1 = 2*sensitivity*precision / (sensitivity+precision)
        
        return sensitivity, precision, f1

    


