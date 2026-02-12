
import numpy as np

class APRuns:

    """Stores results from AP detection among all subjects"""

    class APParam:

        """Sub-class storing the parameters used for AP detection"""

        def __init__(self,
                    fs: float = 2,
                    runCategory: str = 'no_afe',
                    corrThresh: float = 75,
                    baselineStartIdx: int = 201,
                    baselineEndIdx: int = 700,
                    dataAmpMult: float = 1/20000):
                                
            self.fs                 = fs
            self.runCategory        = runCategory
            self.corrThresh         = corrThresh
            self.baselineStartIdx   = baselineStartIdx
            self.baselineEndIdx     = baselineEndIdx
            self.dataAmpMult        = dataAmpMult

    
    class APSubjectResult:

        """Sub-class storing the results from AP detection on one subject"""

        def __init__(self,
                     summaryFilename: str,
                     spikeFilename: str,
                     dataAmpMult: float,
                     timeOffset: float,
                     fs: float):
            
            self.summaryFilename = summaryFilename
            self.spikeFilename = spikeFilename
            self.amplitude, self.frequency, _ = self._read_summary_file() # Get AP amplitude and frequency per buffer
            self.amplitude *= 1/dataAmpMult
            self.time = np.arange(0, len(self.amplitude)/fs, 1/fs)  - timeOffset

            self.spike_locs, self.spike_amps = self._read_spikes_file() # Get time stamps and amplitudes of each detected spike
            self.spike_amps *= 1/dataAmpMult

        def _read_summary_file(self):

            Nwaves = 3

            with open(self.summaryFilename, 'r') as file:
                rl = file.readlines()
                N = len(rl)
                data = np.zeros((Nwaves, N))
                for i in range(N):
                    line = rl[i].replace('\n','').split(',')
                    for j in range(Nwaves):
                        elem = line[j]
                        if 'nan' in elem:
                            data[j,i] = np.nan
                        else:
                            data[j,i] = float(elem)

            return data
        
        def _read_spikes_file(self):
            m = np.loadtxt(self.spikeFilename, delimiter=',')
            spike_locs = np.asarray(m[:,0], dtype=int)
            spike_amps = np.asarray(m[:,1], dtype=float)
            return spike_locs, spike_amps
    
    def __init__(self,
                 subjectList,
                 apParam: APParam = APParam(),
                 results_dir = './ap_runs/'):
        
        self.Nsubjects  = len(subjectList)
        self.apParam   = apParam
        
        results = []
        for subject in subjectList:
    
            summaryFilename = results_dir + subject + '/out.txt'
            spikeFilename = results_dir + subject + '/ap_list.txt'
            results.append(self.APSubjectResult(summaryFilename, 
                                                spikeFilename, 
                                                self.apParam.dataAmpMult, 
                                                self.apParam.baselineEndIdx/self.apParam.fs, 
                                                self.apParam.fs))
            
        self.apResults = results
               
       