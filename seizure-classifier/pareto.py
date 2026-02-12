
import numpy as np
from seizurescore import SeizureScore
import matplotlib.pyplot as plt
import utils.plot


def get_AUC(perf, xName, yName, flat=False, plotROC=False):

    """Computes the area under the ROC curve (AUC)
    
    Parameters
    ----------
    perf: 1D or 2D list of SeizureScore objects
    xName: name of objective on x axis
    yName: name of objective on y axis
    flat: indicates if perf is 1D or 2D
    plotROC

    Returns
    -------
    auc: float
        Area under the ROC curve (between 0 and 1)

    """
    
    if not flat:
        Ni = len(perf)
        Nj = len(perf[0])
        perf = [perf[i][j] for i in range(Ni) for j in range(Nj)]
            
    # Get arrays for raw x/y
    x0 = np.array([getattr(p, xName) for p in perf])
    y0 = np.array([getattr(p, yName) for p in perf])

    if plotROC:
        fig,ax = plt.subplots()
        ax.scatter(x0, y0, color='b', s=150, linewidth=0)
        ax.set_xlabel(xName, fontsize=utils.plot.LABELSIZE)
        ax.set_ylabel(yName, fontsize=utils.plot.LABELSIZE)
        ax.set_xlim(left=0)
        ax.set_ylim(bottom=0)
        ax.grid('on')
        plt.show()

    # Only take dominating x/y
    x = []; y = []
    for p in range(len(x0)):
        xp = x0[p]; yp = y0[p]
        if not np.isnan(xp) and not np.isnan(yp):
            duplicate = False
            q = 0
            while q < len(x) and not duplicate:
                xq = x[q]; yq = y[q]
                if (xq == xp and yq == yp):
                    duplicate = True
                q += 1
            dominated = False
            q = 0
            while q < len(x0) and not dominated:
                xq = x0[q]; yq = y0[q]
                if (xq > xp and yq > yp) \
                    or (xq > xp and yq == yp) \
                    or (xq == xp and yq > yp):
                        dominated = True
                q += 1
            if not duplicate and not dominated:
                x.append(xp)
                y.append(yp)
    x = np.array(x)
    y = np.array(y)

    # Handle specific cases
    if len(x) == 0 and len(y) == 0:
        # if there is an individual with sensitivity=0 and precision=NaN => auc=0
        if len(y0[x0==0.0])>0 and np.isnan(y0[x0==0.0]).all():
            auc = 0.0
        else:
            auc = np.nan
        return auc
    if np.isnan(x * y).all(): # all are at x=nan and/or y=nan
        auc = np.nan
        return auc
    if np.min(x) == 1:
        auc = np.max(y[x==1])
        return auc
    if np.min(y) == 1:
        auc = np.max(x[y==1])
        return auc
    if np.max(x) == 0 or np.max(y) == 0:
        auc = 0.0
        return auc
    if len(np.flatnonzero(np.logical_and(x == 1, y == 1))) > 0:
        auc = 1.0
        return auc
    
    # Add extreme values (hyp: convex curve)
    if np.max(x) != 1:
        x = np.append(x, 1)
        y = np.append(y, 0)
    if np.min(x) != 0:
        y = np.append(y, np.max(y[x == np.min(x)]))
        x = np.append(x,0)
        
    # Sort
    xs,ys = zip(*sorted(zip(x,y)))
    xs = np.array(xs); ys = np.array(ys)
    
    # Sum AUC
    auc = 0.0
    for i in range(1, len(xs)):
        auc += (xs[i] - xs[i-1]) * ys[i]
    
    return auc
    
class Pareto():

    """Ranks the performance according to Pareto optima"""
    
    class _PerfSubSet():

        """Sub-class representing a subset of all tested performance"""
        
        def __init__(self, idx, paretoObj):
            
            # idx refers to indices in the original list 

            self.idx = np.array(idx)
            self.set_i = np.array([self.idx[n] // paretoObj.Nj for n in range(len(self.idx))])
            self.set_j = np.array([self.idx[n] % paretoObj.Nj  for n in range(len(self.idx))])
                        
            obj1_tmp = np.array([getattr(perfk, paretoObj.objective1Name) for perfk in paretoObj.perf])
            obj2_tmp = np.array([getattr(perfk, paretoObj.objective2Name) for perfk in paretoObj.perf])
            tb_tmp = np.array([getattr(perfk, paretoObj.tieBreakerName) for perfk in paretoObj.perf])
            
            self.objective1 = obj1_tmp[self.idx]
            self.objective2 = obj2_tmp[self.idx]
            self.tieBreaker = tb_tmp[self.idx]
            
            self.perf = [paretoObj.perf[k] for k in self.idx]
            self.paretoObj = paretoObj
        
            return
        
        def plot(self, ax=[], color=utils.plot.COLORS['BLUE'], size=50, marker='o'):

            """Plots one subset"""
            
            if ax == []:
                fig,ax = plt.subplots()
            ax.scatter(self.objective1, self.objective2, color=color, s=size, linewidth=0, marker=marker)
            ax.set_xlabel(self.paretoObj.objective1Name, fontsize=utils.plot.LABELSIZE)
            ax.set_ylabel(self.paretoObj.objective2Name, fontsize=utils.plot.LABELSIZE)
            ax.set_xlim(left=0)
            ax.set_ylim(bottom=0)
            ax.grid('on')
            plt.show()
            
            return
        
    def __init__(self, perf: SeizureScore, 
                 objective1Name='', 
                 objective2Name='', 
                 tieBreakerName='', 
                 tieBreakerDir='', 
                 plotSets=False):
        
        self.Ni = len(perf)
        self.Nj = len(perf[0])
        
        self.objective1Name = objective1Name
        self.objective2Name = objective2Name
        self.tieBreakerName = tieBreakerName
        self.tieBreakerDir  = tieBreakerDir
        
        self.perf = [perf[i][j] for i in range(self.Ni) for j in range(self.Nj)]

        # Initial complete set
        self.unSortedSet = Pareto._PerfSubSet(range(len(self.perf)), self)

        # Pareto dominance
        sortedSetIdx = self._sort_non_dominated(self.unSortedSet)
        self.sortedSet = Pareto._PerfSubSet(sortedSetIdx, self)
        
        # Select one individual with best performance
        selectedIdx = self._select_individual(self.sortedSet)
        self.selectedSet = Pareto._PerfSubSet([selectedIdx], self)
        self.selected_i = self.selectedSet.set_i[0]
        self.selected_j = self.selectedSet.set_j[0]

        # Strict Pareto dominance to select individuals that will be used for test AUC
        strictSortedSetIdx = self._sort_strict_non_dominated(self.sortedSet)
        self.strictSortedSet = Pareto._PerfSubSet(strictSortedSetIdx, self)
        self.Pareto_i = self.strictSortedSet.set_i
        self.Pareto_j = self.strictSortedSet.set_j
        
        if plotSets:
            fig,ax = plt.subplots(figsize=utils.plot.FIGSIZE)
            self.unSortedSet.plot(ax=ax, color=utils.plot.COLORS['BLACK'], size=350)
            self.sortedSet.plot(ax=ax, color=utils.plot.COLORS['BLUE'], size=250)
            self.strictSortedSet.plot(ax=ax, color=utils.plot.COLORS['GREEN'], size=150)
            self.selectedSet.plot(ax=ax, color=utils.plot.COLORS['ORANGE'], size=500, marker='*')

    
    def _sort_non_dominated(self, unSortedSet):

        """Removes dominated individuals from the unsorted set
        If one individual has the same obj1 as another one, but worse obj2, both individuals are still considered as non-dominated

        Parameters
        ----------
        unSortedSet: _PerfSubSet
            initial set
        
        Returns
        -------
        paretoFront: list of ints
            idx of non-dominated elements in the unsorted set

        """
        
        paretoFront = []
        
        obj1 = unSortedSet.objective1
        obj2 = unSortedSet.objective2
                
        for p,obj1p in enumerate(obj1): # idx of elem studied = p
            obj2p = obj2[p]
            if not np.isnan(obj1p) and not np.isnan(obj2p):
                dominated = False
                q = 0
                while q < len(obj1) and not dominated: # idx of elem compared = q
                    obj1q = obj1[q]
                    obj2q = obj2[q]
                    if (obj1q > obj1p and obj2q > obj2p):
                        dominated = True
                    q += 1
                if not dominated:
                    paretoFront.append(unSortedSet.idx[p])
        return paretoFront
    
    def _sort_strict_non_dominated(self, unSortedSet):

        """Removes dominated individuals from the unsorted set

        Parameters
        ----------
        unSortedSet: _PerfSubSet
            initial set
        
        Returns
        -------
        paretoFront: list of ints
            idx of non-dominated elements in the unsorted set

        """
        
        paretoFront = []
        
        obj1 = unSortedSet.objective1
        obj2 = unSortedSet.objective2
                
        # ndominates = [0 for _ in range(len(obj1))]
        for p,obj1p in enumerate(obj1):
            obj2p = obj2[p]
            if not np.isnan(obj1p) and not np.isnan(obj2p):
                dominated = False
                q = 0
                while q < len(obj1) and not dominated:
                    obj1q = obj1[q]
                    obj2q = obj2[q]
                    if (obj1q > obj1p and obj2q > obj2p) \
                        or (obj1q > obj1p and obj2q == obj2p) \
                        or (obj1q == obj1p and obj2q > obj2p):
                        dominated = True
                    q += 1
                if not dominated:
                    paretoFront.append(unSortedSet.idx[p])

        return paretoFront    
    
    def _select_individual(self, optiSet):

        """Selects one individual among all possible in Pareto-optimal set
        Methodology (in this order):
            - Select the one(s) with best obj1 
            - Among those, select the one(s) with best obj2
            - Among those, select the one(s) with the best tie-breaker metric


        Parameters
        ----------
        optiSet: _PerfSubSet
            Set of Pareto-optimal individuals

        Returns
        -------
        idx of selected individual

        """
        
        # Methodology: find max obj1, then max obj2, then take idx avg
        
        max1 = np.max(optiSet.objective1)
        idx_max1 = np.flatnonzero(optiSet.objective1 == max1)
        if len(idx_max1) == 1:
            return optiSet.idx[idx_max1[0]]
        
        max2 = np.max(optiSet.objective2[idx_max1])
        idx_max12 = idx_max1[optiSet.objective2[idx_max1] == max2]
        if len(idx_max12) == 1:
            return optiSet.idx[idx_max12[0]]
        
        if self.tieBreakerDir == 'min':
            opti_tb = np.min(optiSet.tieBreaker[idx_max12])
        elif self.tieBreakerDir == 'max':
            opti_tb = np.max(optiSet.tieBreaker[idx_max12])
        else:
            median_idx = idx_max12[np.floor(len(idx_max12)/2).astype(int)]
            return optiSet.idx[median_idx]

        idx_opti_tb = idx_max12[optiSet.tieBreaker[idx_max12] == opti_tb]
        if len(idx_opti_tb) == 1:
            return optiSet.idx[idx_opti_tb[0]]
        else:
            median_idx = idx_opti_tb[np.floor(len(idx_opti_tb)/2).astype(int)]
            return optiSet.idx[median_idx]
    
    def _get_perfect_idx(self, subSet):
        """Returns idx in subSet that correspond to a perfect classification (both objectives = 1)"""
        perfectIdx = subSet.idx[np.logical_and(subSet.objective1 == 1, subSet.objective2 == 1)]
        return perfectIdx
            

        