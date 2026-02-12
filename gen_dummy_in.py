
import numpy as np


# Simply Python code to generate dummy inputs (noise) to run the workflow

SUBJECTS = ['P1', 'P2', 'P3', 'P4', 'P5', 'P6', 'S1', 'S2']
NBUFFERS = 3720
FS = 80e3
BUFFERSIZE = 80000
STD_AMP = 20e-6
DATA_FOLDER = 'dummy_inputs/'

def save_buffer(filename, data):
    with open(filename, 'wb') as file:
        data.tofile(file)
    return

for n in range(len(SUBJECTS)):
    print(f'Starting for subject {SUBJECTS[n]}')
    for i in range(NBUFFERS):
        r1 = np.random.randn(BUFFERSIZE) * STD_AMP
        r2 = np.random.randn(BUFFERSIZE) * STD_AMP
        save_buffer(f'{DATA_FOLDER}{SUBJECTS[n]}/buffer1_{i+1:d}.bin', r1)
        save_buffer(f'{DATA_FOLDER}{SUBJECTS[n]}/buffer2_{i+1:d}.bin', r2)

