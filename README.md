# VENG-HW-limitations-seizure-detection

*For research purposes only*

This set of codes is linked to the publication [Favresse2026], studying the impact of hardware VENG-specific front end design choices and non-idealities on the performance of a seizure detection algorithm from [Raffoul2022] and [Stumpp2021]. It includes:
* *afe-behav:* a behavioral model of the analog and digital front end, implemented in C and separated into modules for each circuit block.
* *rt-ap-algo:* a C implementation of the algorithm for the detection of action potentials (APs), with real-time operation potential.
* *seizure-classifier:* a Python implementation of the classifier used to detect seizure based on AP detection results, based on the timescoring library.

## References
* [Favresse2026]:
* [Raffoul2022]: R. Raffoul et al., "Action Potential Detection Algorithm Adaptable to Individual Nerve and Recording Setup," 2022 IEEE Biomedical Circuits and Systems Conference (BioCAS), Taipei, Taiwan, 2022, pp. 655-659, doi: 10.1109/BioCAS54905.2022.9948681.
* [Stumpp2021]: L. Stumpp et al., "Vagus Nerve Electroneurogram-Based Detection of Acute Pentylenetetrazol Induced Seizures in Rats," International Journal of Neural Systems, Vol. 31, No. 07, p.2150024, May 2021, doi: 10.1142/S0129065721500246.


