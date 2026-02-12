
import numpy as np
import matplotlib.pyplot as plt
from apruns import APRuns
import utils.plot

class Metric:

    """Computation of seizure metric"""
    
    class MetricParameters:
         
        def __init__(self, 
            fgSize: float = 20,
            bgSize: float = 180,
            fs: float = 2):
            
            self.fgSize = fgSize
            self.bgSize = bgSize
            self.fs     = fs


    def __init__(self,
                 apRuns: APRuns,
                 param: MetricParameters = MetricParameters(),
                 plotAmpFreq=False):
        
        self.apRuns = apRuns
        self.param  = param
        self.fs     = param.fs
        
        Nsubjects = len(self.apRuns.apResults)
        self.amplitude_slope    = [None for _ in range(Nsubjects)]
        self.frequency_slope    = [None for _ in range(Nsubjects)]
        self.amplitude          = [None for _ in range(Nsubjects)]
        self.frequency          = [None for _ in range(Nsubjects)]
        self.values             = [None for _ in range(Nsubjects)]
        self.values_normalized  = [None for _ in range(Nsubjects)]
        self.time               = [None for _ in range(Nsubjects)]
        for i in range(Nsubjects):
            self.amplitude_slope[i], self.frequency_slope[i], self.amplitude[i], self.frequency[i] = self._compute_slopes(self.apRuns.apResults[i])
            self.values[i] = self._compute_metric(self.amplitude_slope[i], self.frequency_slope[i])
            self.time[i] = self.apRuns.apResults[i].time[self.apRuns.apParam.baselineEndIdx:] # Starts at zero
        
                
    def _compute_slopes(self, apRes: APRuns.APSubjectResult):

        """ Computes the relative variations of amplitude and frequency using two sliding windows

        Parameters
        ----------
        apRes: Results of AP detection on one subject

        Returns
        -------
        amp_slope: numpy array
            Variations in AP amplitude
        freq_slope: numpy array
            Variations in AP firing rate
        amp_smooth: numpy array
            Mean AP amplitude in the foreground window
        freq_smooth: numpy array
            Mean AP firing rate in the foreground window

        """
        
        amp     = apRes.amplitude
        freq    = apRes.frequency
        
        N = len(amp)
        
        baselineStartIdx = self.apRuns.apParam.baselineStartIdx
                
        fgSize = self.param.fgSize
        bgSize = self.param.bgSize
        

        bgAmpSum    = 0.0
        bgFreqSum   = 0.0        
        fgAmpSum    = 0.0
        fgFreqSum   = 0.0
        
        amp_slope   = np.zeros(N)
        freq_slope  = np.zeros(N)
        amp_smooth  = np.zeros(N)
        freq_smooth = np.zeros(N)
                
        for i in range(baselineStartIdx, N):
            
            if i >= baselineStartIdx + bgSize + fgSize:
                bgAmp = np.nanmean(amp[i-fgSize-bgSize+1:i-fgSize+1])
                bgFreq = np.nanmean(freq[i-fgSize-bgSize+1:i-fgSize+1])
                fgAmp = np.nanmean(amp[i-fgSize+1:i+1])
                fgFreq = np.nanmean(freq[i-fgSize+1:i+1])
                amp_slope[i] = fgAmp / bgAmp
                freq_slope[i] = fgFreq / bgFreq
            
                amp_smooth[i] = fgAmp
                freq_smooth[i] = fgFreq
    
        return amp_slope, freq_slope, amp_smooth, freq_smooth
        
    
    def _compute_metric(self, amplitude_slope, frequency_slope):

        """Computes the seizure metric based on normalized variations of AP amplitude and firing rate

        Parameters
        ----------
        amplitude_slope: numpy array
            Variations in AP amplitude
        frequency_slope: numpy array
            Variations in AP firing rate            

        Returns
        -------
        metric_values: numpy array
            Values of the seizure metric, only defined from the end of the baseline interval

        """

        
        baselineStartIdx        = self.apRuns.apParam.baselineStartIdx
        nonZeroSlopeStartIdx    = baselineStartIdx + self.param.fgSize + self.param.bgSize
        baselineEndIdx          = self.apRuns.apParam.baselineEndIdx
            
        amplitude_slope_norm = (amplitude_slope[baselineEndIdx:] - 1) / np.std(amplitude_slope[nonZeroSlopeStartIdx:baselineEndIdx])
        frequency_slope_norm = (frequency_slope[baselineEndIdx:] - 1) / np.std(frequency_slope[nonZeroSlopeStartIdx:baselineEndIdx])
        metric_values = (amplitude_slope_norm + 1) * (frequency_slope_norm + 1)

        return metric_values
    

    def compute_rms_score(self, refAnnotations, scoringParams):
        
        """
        Computes the RMS score on metric values

        Parameters
        ----------
        Returns
        -------
        """

        Nsubjects = len(refAnnotations)
        rmsScore = np.zeros(Nsubjects)
        for n in range(Nsubjects):

            ref_mask = refAnnotations[n].mask
            metric = self.values[n]
            time = self.time[n]
            fs = refAnnotations[n].fs

            baseline_mask = np.full(len(ref_mask), True, dtype=bool)
            for event in refAnnotations[n].events:
                eventStartExtended = max((0, round(fs*(event[0]-scoringParams[n].toleranceStart))))
                eventEndExtended = min((len(baseline_mask), round(fs*(event[1]+scoringParams[n].toleranceEnd))))
                baseline_mask[eventStartExtended:eventEndExtended] = False # put baseline mask to False during seizure events + tolerances
            baseline_idx = np.flatnonzero(baseline_mask)
            seizure_idx = np.flatnonzero(ref_mask) # only seizure events, not on tolerances

            if len(seizure_idx) > 0: # at least one seizure in the recording
                rmsBaseline = np.sqrt(np.nanmean(metric[baseline_idx]**2))
                rmsSeizure = np.sqrt(np.nanmean(metric[seizure_idx]**2))
                rmsScore[n] = rmsSeizure / rmsBaseline
            else:
                rmsScore[n] = np.nan # rmsScore is undefined if no seizure in the recording

        return rmsScore



    
    def plot(self, refAnnotations, subjectList):

        """Plot the metric values over time with ref seizures annotations"""
                
        Nsubjects = len(self.time)
        ymin,ymax = (-20,150)
        
        Ncols = 2
        Nrows = int(np.ceil(Nsubjects/2))

        fig,ax = plt.subplots(nrows=Nrows, ncols=Ncols, sharex='col', sharey='row', figsize=utils.plot.PORTRAIT_FIGSIZE)
        ax[0,0].set_title('Seizure metric', weight='bold', fontsize=utils.plot.TITLESIZE)
        
        for i in range(Ncols):
            for j in range(Nrows):
                n = j + Nrows * i
                ax[j,i].tick_params(axis='both', labelsize=utils.plot.TICKSIZE)
                ax[j,i].set_ylim(ymin,ymax)
                ax[j,i].set_ylabel(subjectList[n], fontsize=utils.plot.LABELSIZE)
                ax[j,i].plot(self.time[n]/60, self.values[n], color=utils.plot.COLORS["BLUE"], linewidth=utils.plot.THIN_LINEWIDTH)
                ax[j,i].grid(True)
                utils.plot.plot_ref_annotation(ax[j,i], refAnnotations[n], (ymin,ymax), timeOffset=0)
                
            ax[-1,i].set_xlabel('Time [min]', fontsize=utils.plot.LABELSIZE)
        
        return
    
    
    def plot_amp_freq(self, refAnnotations, subjectList, figName):

        """Plot the amplitude/frequency values over time with ref seizures annotations"""

        baselineEndIdx = self.apRuns.apParam.baselineEndIdx
        Nsubjects = len(self.time)
        Ncols = 4
        Nrows = int(np.ceil(Nsubjects*2/Ncols))
        (yminAmp,ymaxAmp) = (0,100)
        (yminFreq,ymaxFreq) = (0,300)
        fig,ax = plt.subplots(nrows=Nrows, ncols=Ncols, sharex='col', sharey='row', figsize=utils.plot.PORTRAIT_FIGSIZE)
        for n in range(Nsubjects):
            i = 2*n // Nrows
            j = (2*n) % Nrows
            # Amp plot
            ax[j,i].tick_params(axis='both', labelsize=utils.plot.TICKSIZE)
            ax[j,i].set_ylim(yminAmp,ymaxAmp)
            ax[j,i].set_ylabel('Amp '+subjectList[n] + ' [µV]', fontsize=utils.plot.LABELSIZE)
            ax[j,i].plot(self.time[n]/60, self.amplitude[n][baselineEndIdx:] * 1e6, color=utils.plot.COLORS["BLUE"], linewidth=utils.plot.THIN_LINEWIDTH)
            ax[j,i].grid(True)
            utils.plot.plot_ref_annotation(ax[j,i], refAnnotations[n], (yminFreq,ymaxFreq), timeOffset=0)
            # Freq plot
            ax[j+1,i].tick_params(axis='both', labelsize=utils.plot.TICKSIZE)
            ax[j+1,i].set_ylim(yminFreq,ymaxFreq)
            ax[j+1,i].set_ylabel('Freq '+subjectList[n] + ' [Hz]', fontsize=utils.plot.LABELSIZE)
            ax[j+1,i].plot(self.time[n]/60, self.frequency[n][baselineEndIdx:], color=utils.plot.COLORS["BLUE"], linewidth=utils.plot.THIN_LINEWIDTH)
            ax[j+1,i].grid(True)
            utils.plot.plot_ref_annotation(ax[j+1,i], refAnnotations[n], (yminFreq,ymaxFreq), timeOffset=0)
        plt.savefig(figName)
    
    def plot_with_results(self, testPerformance, subjectList, threshold, saveFig=False, figName=''):
        
        """Plot the metric values over time with ref and hyp seizures annotations"""

        Nsubjects = len(subjectList)
        ymin,ymax = (-20,150)
        
        Ncols = 2
        Nrows = int(np.ceil(Nsubjects/2))
        
        fig,ax = plt.subplots(nrows=Nrows, ncols=Ncols, sharex='col', sharey='row', figsize=utils.plot.PORTRAIT_FIGSIZE)
        ax[0,0].set_title('Seizure metric', weight='bold', fontsize=utils.plot.TITLESIZE)

        for i in range(Ncols):
            for j in range(Nrows):
                n = j + Nrows * i
                ax[j,i].tick_params(axis='both', labelsize=utils.plot.TICKSIZE)
                ax[j,i].set_ylim(ymin,ymax)
                if n < Nsubjects:
                    ax[j,i].set_ylabel(subjectList[n], fontsize=utils.plot.LABELSIZE)
                    ax[j,i].plot(self.time[n]/60, self.values[n], color=utils.plot.COLORS["BLUE"], linewidth=utils.plot.THIN_LINEWIDTH)
                    ax[j,i].grid(True)
                    utils.plot.plot_ref_annotation(ax[j,i], testPerformance[n].ref[0], (ymin,ymax))
                    utils.plot.plot_hyp_annotation(ax[j,i], testPerformance[n], threshold[n], (ymin,ymax))
                
            ax[-1,i].set_xlabel('Time [min]', fontsize=utils.plot.LABELSIZE)
            
        if saveFig:
            
            if figName is None:
                figName = 'results/unknow_result.png'
            
            plt.savefig(figName)
            
        return
            
            
        
        
        
        
