
import numpy as np
import matplotlib.pyplot as plt
import matplotlib as mpl
mpl.rcParams['axes.linewidth'] = 1
mpl.rcParams['font.sans-serif'] = "Tahoma"
mpl.rcParams['font.family'] = "sans-serif"
mpl.rcParams['axes.axisbelow'] = True # grid an axes below plots
plt.rcParams['svg.fonttype'] = 'none'
plt.close('all')

COLORS = {}
COLORS["BLUE"] = '#0070C0'
COLORS["ORANGE"] = '#E46C0A'
COLORS["GREEN"] = '#77933C'
COLORS["PURPLE"] = '#7030A0'
COLORS["CYAN"] = '#00B0F0'
COLORS["YELLOW"] = '#F3C90A'
COLORS["RED"] = '#C00000'
COLORS["WHITE"] = '#FFFFFF'
COLORS["BLACK"] = '#000000'
COLORS["GRAY"] = '#808080'
COLORS["GREY"] = '#808080'
COLORS["LIGHTGRAY"] = "#CCCCCC"
COLORS["LIGHTPURPLE"] = "#D0B1E8"
COLORS["LIGHTBLUE"] = "#99D4FF"
COLORS_LIST = list(COLORS.keys())


FIGSIZE = (12,12)
LANDSCAPE_FIGSIZE = (18,12)
PORTRAIT_FIGSIZE = (12,18)
LARGE_FIGSIZE = (24,24)
THICK_LINEWIDTH = 4
THIN_LINEWIDTH = 1
LINEWIDTH = 3
TICKSIZE = 14
TICKWIDTH = 2
TICKLENGTH = 6
LABELSIZE = 20
LEGENDSIZE = 18
TITLESIZE = 20

N_MAJOR_GRID = 5
N_MINOR_GRID = 25
ALPHA_MINOR_GRID = 0.4

def plot_ref_annotation(ax, refAnnotation, ybounds, patchColor=COLORS["RED"], patchAlpha=0.25, xmult=1/60, timeOffset=0):
    
    ymin,ymax = ybounds
    
    for k in range(len(refAnnotation.events)):
        start,stop = refAnnotation.events[k]
        ax.add_patch(mpl.patches.Rectangle(((start-timeOffset)*xmult, ymin), (stop-start)*xmult, ymax-ymin, facecolor=patchColor, alpha=patchAlpha))
        
    return

def plot_hyp_annotation(ax, performance, threshold, ybounds):
    
    ymin,ymax = ybounds
    
    eventScore = performance.eventScores[0]
    tpMask = eventScore.tpMask
    hypEvents = eventScore.hyp.events
    fs = eventScore.fs
    
    # Indicate TP mask
    tpRising = np.where(np.diff(tpMask.astype('int')) == 1)[0]
    tpFalling = np.where(np.diff(tpMask.astype('int')) == -1)[0]
    for i in range(len(tpRising)):
        ax.axvline(tpRising[i]/fs/60, ls='--', color=COLORS['GREEN'])
    for i in range(len(tpFalling)):
        ax.axvline(tpFalling[i]/fs/60, ls='--', color=COLORS['GREEN'])   
    
    # Indicate FP
    for event in hypEvents:
        if np.all(~tpMask[round(event[0]*fs):round(event[1]*fs)]):
            ax.axvline(event[0]/60, ls='--', color=COLORS['RED'])
            ax.axvline(event[1]/60, ls='--', color=COLORS['RED'])

    return

def plot_perfectDetection_thresholds(threshold, DoT, titles, thresholdmax, DoTmax, figName=None):
    
    def get_countour_curves(x,y):
        xu = []
        ymin = []
        ymax = []
        for xi in x:
            if xi not in xu:
                xu.append(xi)
                y_xi = y[x == xi]
                ymin.append(np.min(y_xi))
                ymax.append(np.max(y_xi))
        area = 0.0
        for i in range(1,len(xu)):
            dx = xu[i] - xu[i-1]
            ymean = (y[i] + y[i-1])/2
            area += dx * ymean
            
        xu = np.append(xu[0], xu)
        ymin = np.append(ymax[0], ymin)
        ymax = np.append(ymax[0], ymax)
        xu = np.append(xu, xu[-1])
        ymin = np.append(ymin, ymin[-1])
        ymax = np.append(ymax, ymin[-1])
        
        return np.array(xu), np.array(ymin), np.array(ymax), area
    
    
    N = len(threshold)
    Nrows = 2
    Ncols = np.ceil(N/Nrows).astype(int)
    fig,ax = plt.subplots(nrows=Nrows, ncols=Ncols, sharex='col', sharey='row', figsize=(12,6))
    for i in range(Nrows):
        for j in range(Ncols):
            k = i * Ncols + j
            # ax[i,j].scatter(threshold[k], DoT[k], color=COLORS['BLUE'])
            ax[i,j].set_xticks([0,25,50,75,100])
            ax[i,j].set_yticks([0,25,50,75,100])
            ax[i,j].tick_params(axis='both', labelsize=15)
            xu,ymin,ymax,area = get_countour_curves(threshold[k], DoT[k])
            ax[i,j].plot(xu,ymin, color=COLORS['BLUE'])
            ax[i,j].plot(xu,ymax, color=COLORS['BLUE'])
            ax[i,j].fill_between(xu, ymin, ymax, facecolor=COLORS['LIGHTBLUE'])
            ax[i,j].text(thresholdmax/2,DoTmax/2, f'area: {area:.1f}',
                         fontsize=LABELSIZE-2, horizontalalignment='center', verticalalignment='center')
            ax[i,j].set_title(titles[k], fontsize=TITLESIZE, weight='bold')
            ax[i,j].grid(True)
            ax[i,j].set_xlim(0, thresholdmax)
            ax[i,j].set_ylim(0, DoTmax)
            ax[-1,j].set_xlabel('Threshold', fontsize=19)
        ax[i,0].set_ylabel('DoT', fontsize=19)
    plt.show()

    if figName is None:
        figName = 'perfectClassification'

    plt.savefig(figName + '.png')
    plt.savefig(figName + '.svg', format='svg')
    

    
    