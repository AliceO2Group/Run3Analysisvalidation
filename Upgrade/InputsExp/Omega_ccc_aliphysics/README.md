# Omega_{ccc}
Significance estimate

Introduction
There is a macro which takes in Inputs:
1. generated signal
2. reconstructed signal
3. reconstructed background

The first two are used to estimate the "efficiency x acceptance" which is one of the ingredient needed to estimate the "raw-signal-per-event".

The reconstructed background is needed to estimate the "background-per-event".

Then, the significance/sqrt{events} is computed from the raw-signal-per-event and the background-per-event.

The goal of the macro is to estimate the significance/sqrt{events} for different trial
The macro works in the following way:
- select the variable you want to study for the trial (e.g. d0(rphi) of Omega_c "d0rphi_omegac_cuts");
- define inside the macro a set of cut values you want to apply on this variable;
- choose if you want to add an additional set of predefined cuts which are applied in all the trials (e.g. chi2/ndf < 4, cos(pointing angle)>0.99) --> "bool useSpecialCuts = true" to use the pre-defined set of cuts as default;
- then the macro runs over the signal trees (generated and reconstructed) to get the efficiency x acceptance for each trial;
- runs over the background tree to get the background-per-event for each trial;
- compute the raw-signal-per-event using the efficiency x acceptance, the theoretical inputs for dN/dy and the estimated B.R.
- finally computes the significance/sqrt{events} with the expected raw-signal-per-event and the. expected background-per-event
This procedure is done automatically for 3 pT bins: 0-2, 2-4, 4-10 GeV/c.

Description of the output
For each variable a dedicated directory is created (e.g. d0rphi_omegac_cuts) and there inside all the related trials are stored (e.g. trial_4,5).
Each directory trial_* contains the following files:
- tablecuts_trial_*_*.pdf --> a pdf where all the applied selections are reported. Example: tablecuts_trial_d0rphi_omegac_withSpecialCuts.pdf --> there are some fixed cuts for Omega_c,cc,ccc applied to all the trials and at the bottom the list of the cut values for the trial on the d0(rphi) of Omega_c
- mass_signal_low/mid/high.pdf --> histograms of invariant mass distribution for reconstructed signal. Filled to check that the reconstruction is done properly.
- mass_background_*_*_low/mid/high.pdf --> histograms of invariant mass distribution obtained from the background tree
- minv_signal_fit_*_low/mid/high.pdf --> histograms of invariant mass distribution for reconstructed signal fitted with a gaussian. The width of this gaussian is used to define the +/-3 sigma region around the expected mass value where we take the background counts
- background_counts_*_*_high.pdf --> histograms with invariant mass distribution from background candidates. Here are reported the counts of background in the +/-3 sigma region around the mass peak, which are used to evaluate the background-per-event
- generated_pt/rapidity_distribution.pdf --> generated distributions of mother particle (e.g. Omega_ccc). In particular:
	- Gen (top left): distribution with all generated mother particle without any selections
	- GenAcc (top right): distribution with all mothers inside the rapidity gap and the daughters inside the pseudorapidity gap
	- GenLimAcc (bottom left): distribution with all mothers inside the rapidity gap
- reconstruction_*_*_withSpecialCuts.pdf --> distribution of reconstructed mother particle for the different trial. The label "withSpecialCuts" means that the set of fixed cut is applied.
- efficiency_*_*_withSpecialCuts.pdf --> contains the efficiency and acceptance calculation. In particular:
	- RecoCuts/GenAcc --> pure efficiency term
	- GenAcc/GenLimAcc --> pure acceptance term
	- RecoCuts/GenLimAcc --> efficiency x acceptance, which is used in the raw signal estimate
- signif_estimate_shm_coal_omegaccc_*_*.pdf --> contains the ingredients and the estimate of the significance/sqrt{events}. In particular:
	- Raw signal (top left) --> raw signal per event obtained as S = dN/dy x (effXacc) x BR; the dashed line refers to dN/dy from coalescence, while the solid line from SHM; the different colors refer to the different trial (i.e. we estimate the raw signal per event for each trial)
	- Background (top right) --> background per event 
	- Significance/sqrt{event} (bottom left) 
	- Inputs (bottom right) --> contains the details of the background and the theoretical inputs used in the calculation
	
All what described here is referred to the studies for the Omega_ccc, but is also valid for the Omega_cc.
	
	
