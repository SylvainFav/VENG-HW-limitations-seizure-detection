# VENG-HW-limitations-seizure-detection

*For research purposes only*

This set of codes is linked to the publication [Favresse2026], studying the impact of hardware VENG-specific front end design choices and non-idealities on the performance of a seizure detection algorithm from [Raffoul2022] and [Stumpp2021]. It includes:
* *afe-behav:* a behavioral model of the analog and digital front end, implemented in C and separated into modules for each circuit block.
* *rt-ap-algo:* a C implementation of the algorithm for the detection of action potentials (APs), with real-time operation potential.
* *seizure-classifier:* a Python implementation of the classifier used to detect seizure based on AP detection results, based on the timescoring library.

The real experimental VENG data belongs to the Institute of Neuroscience, UCLouvain (represented by Prof. Riëm El Tahry) and the BEAMS department, ULB (represented by Prof. Antoine Nonclercq) and is therefore not shared publicly.

## References
* [Favresse2026]:
* [Raffoul2022]: R. Raffoul et al., "Action Potential Detection Algorithm Adaptable to Individual Nerve and Recording Setup," 2022 IEEE Biomedical Circuits and Systems Conference (BioCAS), Taipei, Taiwan, 2022, pp. 655-659, doi: 10.1109/BioCAS54905.2022.9948681.
* [Stumpp2021]: L. Stumpp et al., "Vagus Nerve Electroneurogram-Based Detection of Acute Pentylenetetrazol Induced Seizures in Rats," International Journal of Neural Systems, Vol. 31, No. 07, p.2150024, May 2021, doi: 10.1142/S0129065721500246.

## Running the workflow

The whole workflow must be ran step by step.
1. *gen_dummy_in.py* (launched with *python3 gen_dummy_in.py*): generates dummy inputs for all 8 rats.
2. *afe-behav/main.c* (launched with *make run*): runs the behavioral model of the front end for all 8 rats. The model can be configured in *afe-behav/include/setup.h*.
3. *behavout2apin.c* (launched with *python3 behavout2apin.py*): transforms the output of the behavioral model into a format used for the AP detection algorithm.
4. *rt-ap-algo/main.c* (launched with *make run*): runs the AP detection algorithm for all 8 rats. The algorithm parameters can be configured in *rt-ap-algo/include/setup.h*.
5. *seizure-classifier/main.py* (launched with *python3 main.py*): runs the classification of seizure events for all 8 rats.
All intermediate and final results are stored in *outputs/CATEGORY/*.

