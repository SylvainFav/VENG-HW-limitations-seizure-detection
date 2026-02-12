
import numpy as np

def nanmean_rnan(arr):
    if np.isnan(arr).all():
        return np.nan
    else:
        return np.nanmean(arr)
