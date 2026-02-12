
import numpy as np
from metric import Metric

class Detection:

    """Detection of seizure events on metric"""
    
    class DetectionParameters:

        """Sub-class to store the parameters of seizure detection"""
        
        def __init__(self,
                    threshold: float = 20,
                    minDuration: float = 2,
                    detectionStartIdx: int = 2400):
            self.threshold = threshold
            self.minDuration = minDuration
            self.detectionStartIdx = detectionStartIdx
            
    def __init__(self, metric: Metric, param: DetectionParameters = DetectionParameters()):
        
        self.metric = metric
        self.param  = param
                
        Nsubjects = len(self.metric.values_normalized)
        self.detectedEvents = [None for _ in range(Nsubjects)]
        self.mask = [None for _ in range(Nsubjects)]
        for n in range(Nsubjects):
            detectedEventsIdx = self._apply_thresholding(self.metric.values[n])
            self.detectedEvents[n] = detectedEventsIdx / self.metric.fs # in time domain
            self.mask[n] = np.zeros_like(self.metric.time[n])
            for event in detectedEventsIdx:
                start,stop = event
                self.mask[n][start:stop] = np.ones(stop-start)
                
                
        
        
    def _apply_thresholding(self, values):

        """Applies the two-steps thresholding on the metric values

        Parameters
        ----------
        values: numpy array
            metric values
        
        Returns
        -------
        detectedEventsIdx: numpy array (2D)
            locations of detected events

        """
        
        idxAboveThreshold = np.asarray(values[self.param.detectionStartIdx:] > self.param.threshold, dtype=bool) # Find all idx above hard threshold
        
        minNumSamplesAbove = np.floor(self.param.minDuration * self.metric.param.fs).astype('int') # Compute min number of consecutive samples that must be > threshold
        
        diff = np.diff(idxAboveThreshold.astype(int)) # +1 when crossing to get above threshold, -1 when crossing the other way
        starts = np.where(diff == 1)[0] + 1
        ends = np.where(diff == -1)[0] + 1
        if idxAboveThreshold[0]: # First sample is already above threshold
            starts = np.insert(starts, 0, 0)
        if idxAboveThreshold[-1]: # Last sample is still above threshold
            ends = np.append(ends, len(idxAboveThreshold))
        
        numSamplesAbove = ends - starts
        idxLongAboveThreshold = numSamplesAbove >= minNumSamplesAbove
        
        detectedEventsStarts = starts[idxLongAboveThreshold] + minNumSamplesAbove # Detection only really occurs after the minNumSamplesAbove (important for detection delay)
        detectedEventsEnds = ends[idxLongAboveThreshold]
        detectedEventsIdx = np.transpose(np.asarray([detectedEventsStarts, detectedEventsEnds], dtype='int')) + self.param.detectionStartIdx
                
        return detectedEventsIdx            