
import numpy as np


def save_buffer(filename, data):
    with open(filename, 'wb') as file:
        data.tofile(file)
    return

BEHAVOUT_FOLDER = './outputs/ref/behav_out/'
APIN_FOLDER = './outputs/ref/ap_in/'

SUBJECTS = ['P1', 'P2', 'P3', 'P4', 'P5', 'P6', 'S1', 'S2']
NBUFFERS = 3720
FS = 20e3 # the AP detection algo expects data at 20 kS/S. If the behav model gives results at another rate, the data must be re-sampled
BEHAVOUT_BUFFERSIZE = 20000
APIN_BUFFERSIZE = 10000
PRE_PADDING_SIZE = int(np.round(BEHAVOUT_BUFFERSIZE - APIN_BUFFERSIZE)/2)
POST_PADDING_SIZE = int(np.round(BEHAVOUT_BUFFERSIZE - APIN_BUFFERSIZE)/2)
APIN_IDX = range(PRE_PADDING_SIZE, PRE_PADDING_SIZE+APIN_BUFFERSIZE)

AFEGAIN = 1.3e9 # approx gain taking into account data conversion
DATAGAIN = 20000 # artificially multiply data to avoid precision losses in files
DATAMULT = DATAGAIN / AFEGAIN

for n in range(len(SUBJECTS)):
    print(f'Running for subject {SUBJECTS[n]}')
    in_folder = BEHAVOUT_FOLDER + SUBJECTS[n] + '/'
    out_folder = APIN_FOLDER + SUBJECTS[n] + '/'
    for i in range(NBUFFERS):
        in_file = f'{in_folder}buffer{i+1:d}.txt'
        out_file = f'{out_folder}buffer{i+1:d}.bin'
        M = np.loadtxt(in_file)
        out_sig = M[APIN_IDX] * DATAMULT
        save_buffer(out_file, out_sig)


