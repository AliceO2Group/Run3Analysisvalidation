// Comparison of AliPhysics and O2 histograms

#include "utils_plot.h"

// vectors of histogram specifications
using VecSpecHis = std::vector<std::tuple<TString, TString, TString, int, bool, bool, TString>>;

// Add histogram specification in the vector.
void AddHistogram(VecSpecHis& vec, TString label, TString nameRun2, TString nameRun3, int rebin, bool logH, bool logR, TString proj = "x")
{
  vec.push_back(std::make_tuple(label, nameRun2, nameRun3, rebin, logH, logR, proj));
}

Int_t Compare(TString filerun3 = "AnalysisResults_O2.root", TString filerun2 = "AnalysisResults_ALI.root", TString options = "", bool doRatio = false)
{
  gStyle->SetOptStat(0);
  gStyle->SetPalette(0);
  gStyle->SetCanvasColor(0);
  gStyle->SetFrameFillColor(0);

  TFile* fRun3 = new TFile(filerun3.Data());
  if (fRun3->IsZombie()) {
    printf("Failed to open file %s\n", filerun3.Data());
    return 1;
  }
  TFile* fRun2 = new TFile(filerun2.Data());
  if (fRun2->IsZombie()) {
    printf("Failed to open file %s\n", filerun2.Data());
    return 1;
  }

  TString pathListRun2 = "HFVertices/clistHFVertices";
  TList* lRun2 = nullptr;
  fRun2->GetObject(pathListRun2.Data(), lRun2);
  if (!lRun2) {
    printf("Failed to load list %s from %s\n", pathListRun2.Data(), filerun2.Data());
    return 1;
  }

  TString labelParticle = "";

  // Histogram specification: axis label, AliPhysics name, O2Physics path/name, rebin, log scale histogram, log scale ratio, projection axis

  VecSpecHis vecHisEvents;
  AddHistogram(vecHisEvents, "primary vtx x (cm)", "hPrimVertX", "hf-tag-sel-collisions/hPrimVtxX", 1, 1, 0);
  AddHistogram(vecHisEvents, "primary vtx y (cm)", "hPrimVertY", "hf-tag-sel-collisions/hPrimVtxY", 1, 1, 0);
  AddHistogram(vecHisEvents, "primary vtx z (cm)", "hPrimVertZ", "hf-tag-sel-collisions/hPrimVtxZ", 1, 1, 0);
  AddHistogram(vecHisEvents, "primary vtx N contributors", "fHistPrimVertContr", "hf-tag-sel-collisions/hNContributors", 1, 1, 0);

  VecSpecHis vecHisTracks;
  AddHistogram(vecHisTracks, "#it{p}_{T} before selections (GeV/#it{c})", "hPtAllTracks", "hf-tag-sel-tracks/hPtNoCuts", 2, 1, 0);
  AddHistogram(vecHisTracks, "#it{p}_{T} (2-prong sel.)", "hPtSelTracks2prong", "hf-tag-sel-tracks/hPtCuts2Prong", 2, 1, 0);
  AddHistogram(vecHisTracks, "#it{p}_{T} (3-prong sel.)", "hPtSelTracks3prong", "hf-tag-sel-tracks/hPtCuts3Prong", 2, 1, 0);
  AddHistogram(vecHisTracks, "#it{p}_{T} (bachelor sel.)", "hPtSelTracksbachelor", "hf-tag-sel-tracks/hPtCutsV0bachelor", 2, 1, 0);
  AddHistogram(vecHisTracks, "DCA XY to prim. vtx. (2-prong sel.) (cm)", "hImpParSelTracks2prong", "hf-tag-sel-tracks/hDCAToPrimXYVsPtCuts2Prong", 2, 1, 0, "y");
  AddHistogram(vecHisTracks, "DCA XY to prim. vtx. (3-prong sel.) (cm)", "hImpParSelTracks3prong", "hf-tag-sel-tracks/hDCAToPrimXYVsPtCuts3Prong", 2, 1, 0, "y");
  AddHistogram(vecHisTracks, "DCA XY to prim. vtx. (bachelor sel.) (cm)", "hImpParSelTracksbachelor", "hf-tag-sel-tracks/hDCAToPrimXYVsPtCutsV0bachelor", 2, 1, 0, "y");
  AddHistogram(vecHisTracks, "#it{#eta} (2-prong sel.)", "hEtaSelTracks2prong", "hf-tag-sel-tracks/hEtaCuts2Prong", 2, 0, 0);
  AddHistogram(vecHisTracks, "#it{#eta} (3-prong sel.)", "hEtaSelTracks3prong", "hf-tag-sel-tracks/hEtaCuts3Prong", 2, 0, 0);
  AddHistogram(vecHisTracks, "#it{#eta} (bachelor sel.)", "hEtaSelTracksbachelor", "hf-tag-sel-tracks/hEtaCutsV0bachelor", 2, 0, 0);

  VecSpecHis vecHisSkim;
  AddHistogram(vecHisSkim, "secondary vtx x - 2prong (cm)", "h2ProngVertX", "hf-track-index-skim-creator/hVtx2ProngX", 5, 1, 0);
  AddHistogram(vecHisSkim, "secondary vtx y - 2prong (cm)", "h2ProngVertY", "hf-track-index-skim-creator/hVtx2ProngY", 5, 1, 0);
  AddHistogram(vecHisSkim, "secondary vtx z - 2prong (cm)", "h2ProngVertZ", "hf-track-index-skim-creator/hVtx2ProngZ", 5, 1, 0);
  AddHistogram(vecHisSkim, "secondary vtx x - 3prong (cm)", "h3ProngVertX", "hf-track-index-skim-creator/hVtx3ProngX", 5, 1, 0);
  AddHistogram(vecHisSkim, "secondary vtx y - 3prong (cm)", "h3ProngVertY", "hf-track-index-skim-creator/hVtx3ProngY", 5, 1, 0);
  AddHistogram(vecHisSkim, "secondary vtx z - 3prong (cm)", "h3ProngVertZ", "hf-track-index-skim-creator/hVtx3ProngZ", 5, 1, 0);

  VecSpecHis vecHisCand2;
  AddHistogram(vecHisCand2, "XX element of PV cov. matrix (cm^{2})", "hCovMatPrimVXX2Prong", "hf-cand-creator-2prong/hCovPVXX", 1, 1, 0);
  AddHistogram(vecHisCand2, "XX element of SV cov. matrix (cm^{2})", "hCovMatSecVXX2Prong", "hf-cand-creator-2prong/hCovSVXX", 1, 1, 0);

  VecSpecHis vecHisCand3;
  AddHistogram(vecHisCand3, "XX element of PV cov. matrix (cm^{2})", "hCovMatPrimVXX3Prong", "hf-cand-creator-3prong/hCovPVXX", 1, 1, 0);
  AddHistogram(vecHisCand3, "XX element of SV cov. matrix (cm^{2})", "hCovMatSecVXX3Prong", "hf-cand-creator-3prong/hCovSVXX", 1, 1, 0);
  AddHistogram(vecHisCand3, "YY element of PV cov. matrix (cm^{2})", "hCovMatPrimVYY3Prong", "hf-cand-creator-3prong/hCovPVYY", 1, 1, 0);
  AddHistogram(vecHisCand3, "YY element of SV cov. matrix (cm^{2})", "hCovMatSecVYY3Prong", "hf-cand-creator-3prong/hCovSVYY", 1, 1, 0);
  AddHistogram(vecHisCand3, "ZZ element of PV cov. matrix (cm^{2})", "hCovMatPrimVZZ3Prong", "hf-cand-creator-3prong/hCovPVZZ", 1, 1, 0);
  AddHistogram(vecHisCand3, "ZZ element of SV cov. matrix (cm^{2})", "hCovMatSecVZZ3Prong", "hf-cand-creator-3prong/hCovSVZZ", 1, 1, 0);

  VecSpecHis vecHisD0;
  AddHistogram(vecHisD0, "#it{p}_{T} prong 0 (GeV/#it{c})", "hPtD0Dau0", "hf-task-d0/hPtProng0", 2, 1, 0);
  AddHistogram(vecHisD0, "#it{p}_{T} prong 1 (GeV/#it{c})", "hPtD0Dau1", "hf-task-d0/hPtProng1", 2, 1, 0);
  AddHistogram(vecHisD0, "#it{p}_{T} D^{0} (GeV/#it{c})", "hPtD0", "hf-task-d0/hPtCand", 2, 1, 0);
  AddHistogram(vecHisD0, "2-prong mass (#pi K) (GeV/#it{c}^{2})", "hInvMassD0", "hf-task-d0/hMass", 2, 0, 0);
  AddHistogram(vecHisD0, "d0d0 (cm^{2})", "hd0Timesd0", "hf-task-d0/hd0d0", 2, 1, 0);
  AddHistogram(vecHisD0, "d0 prong 0 (cm)", "hImpParD0Dau0", "hf-task-d0/hd0Prong0", 2, 1, 0);
  AddHistogram(vecHisD0, "d0 prong 1 (cm)", "hImpParD0Dau1", "hf-task-d0/hd0Prong1", 2, 1, 0);
  AddHistogram(vecHisD0, "impact parameter error (cm)", "hImpParErrD0Dau0", "hf-task-d0/hd0ErrProng0", 1, 1, 0);
  AddHistogram(vecHisD0, "impact parameter error (cm)", "hImpParErrD0Dau1", "hf-task-d0/hd0ErrProng1", 1, 1, 0);
  AddHistogram(vecHisD0, "decay length (cm)", "hDecLenD0", "hf-task-d0/hDecLength", 2, 1, 0);
  AddHistogram(vecHisD0, "decay length XY (cm)", "hDecLenXYD0", "hf-task-d0/hDecLengthxy", 2, 1, 0);
  AddHistogram(vecHisD0, "decay length error (cm)", "hDecLenErrD0", "hf-task-d0/hDecLenErr", 1, 1, 0);
  AddHistogram(vecHisD0, "decay length XY error (cm)", "hDecLenXYErrD0", "hf-task-d0/hDecLenXYErr", 1, 1, 0);
  AddHistogram(vecHisD0, "cos pointing angle", "hCosPointD0", "hf-task-d0/hCPA", 2, 1, 0);

  labelParticle = "D^{0} #rightarrow #pi K";
  VecSpecHis vecHisD0MC;
  AddHistogram(vecHisD0MC, labelParticle + ", matched prompt: #it{p}_{T}^{rec} (GeV/#it{c})", "hPtRecoPromptD0Kpi", "hf-task-d0/hPtRecSigPrompt", 2, 1, 0);
  AddHistogram(vecHisD0MC, labelParticle + ", gen. prompt: #it{p}_{T}^{gen} (GeV/#it{c})", "hPtGenLimAccPromptD0Kpi", "hf-task-d0/hPtGenPrompt", 2, 1, 0);
  AddHistogram(vecHisD0MC, labelParticle + ", matched non-prompt: #it{p}_{T}^{rec} (GeV/#it{c})", "hPtRecoFeeddwD0Kpi", "hf-task-d0/hPtRecSigNonPrompt", 2, 1, 0);
  AddHistogram(vecHisD0MC, labelParticle + ", gen. non-prompt: #it{p}_{T}^{gen} (GeV/#it{c})", "hPtGenLimAccFeeddwD0Kpi", "hf-task-d0/hPtGenNonPrompt", 2, 1, 0);

  VecSpecHis vecHisDs;
  AddHistogram(vecHisDs, "#it{p}_{T} D_{s}^{#pm} (GeV/#it{c})", "hPtDs", "hf-task-ds/hPt", 2, 1, 0);
  AddHistogram(vecHisDs, "3-prong mass (K K #pi) (GeV/#it{c}^{2})", "hInvMassDs", "hf-task-ds/hMass", 5, 0, 0);
  AddHistogram(vecHisDs, "decay length (cm)", "hDecLenDs", "hf-task-ds/hDecayLength", 4, 1, 0);
  AddHistogram(vecHisDs, "cos pointing angle", "hCosPointDs", "hf-task-ds/hCPA", 2, 1, 0);

  VecSpecHis vecHisDPlus;
  AddHistogram(vecHisDPlus, "#it{p}_{T} prong 0 (GeV/#it{c})", "hPtDplusDau0", "hf-task-dplus/hPtProng0", 2, 1, 0);
  AddHistogram(vecHisDPlus, "#it{p}_{T} prong 1 (GeV/#it{c})", "hPtDplusDau1", "hf-task-dplus/hPtProng1", 2, 1, 0);
  AddHistogram(vecHisDPlus, "#it{p}_{T} prong 2 (GeV/#it{c})", "hPtDplusDau2", "hf-task-dplus/hPtProng2", 2, 1, 0);
  AddHistogram(vecHisDPlus, "#it{p}_{T} D^{+} (GeV/#it{c})", "hPtDplus", "hf-task-dplus/hPt", 2, 1, 0);
  AddHistogram(vecHisDPlus, "3-prong mass (#pi K #pi) (GeV/#it{c}^{2})", "hInvMassDplus", "hf-task-dplus/hMass", 5, 0, 0);
  AddHistogram(vecHisDPlus, "impact par. XY (cm)", "hImpParXYDplus", "hf-task-dplus/hImpactParameterXY", 4, 1, 0);
  AddHistogram(vecHisDPlus, "decay length (cm)", "hDecLenDplus", "hf-task-dplus/hDecayLength", 4, 1, 0);
  AddHistogram(vecHisDPlus, "decay length XY (cm)", "hDecLenXYDplus", "hf-task-dplus/hDecayLengthXY", 4, 1, 0);
  AddHistogram(vecHisDPlus, "norm. decay length XY", "hNormDecLenXYDplus", "hf-task-dplus/hNormalisedDecayLengthXY", 2, 1, 0);
  AddHistogram(vecHisDPlus, "cos pointing angle", "hCosPointDplus", "hf-task-dplus/hCPA", 2, 1, 0);
  AddHistogram(vecHisDPlus, "cos pointing angle XY", "hCosPointXYDplus", "hf-task-dplus/hCPAxy", 2, 1, 0);
  AddHistogram(vecHisDPlus, "norm. IP", "hNormIPDplus", "hf-task-dplus/hMaxNormalisedDeltaIP", 4, 1, 0);
  AddHistogram(vecHisDPlus, "decay length error (cm)", "hDecLenErrDplus", "hf-task-dplus/hDecayLengthError", 2, 1, 0);
  AddHistogram(vecHisDPlus, "decay length XY error (cm)", "hDecLenXYErrDplus", "hf-task-dplus/hDecayLengthXYError", 2, 1, 0);
  AddHistogram(vecHisDPlus, "prong 0 impact parameter (cm)", "hImpParDplusDau0", "hf-task-dplus/hd0Prong0", 2, 1, 0);
  AddHistogram(vecHisDPlus, "prong 1 impact parameter (cm)", "hImpParDplusDau1", "hf-task-dplus/hd0Prong1", 2, 1, 0);
  AddHistogram(vecHisDPlus, "prong 2 impact parameter (cm)", "hImpParDplusDau2", "hf-task-dplus/hd0Prong2", 2, 1, 0);
  AddHistogram(vecHisDPlus, "prong impact parameter error (cm)", "hImpParErrDplusDau", "hf-task-dplus/hImpactParameterError", 2, 1, 0);
  AddHistogram(vecHisDPlus, "sq. sum of prong imp. par. (cm^{2})", "hSumSqImpParDplusDau", "hf-task-dplus/hImpactParameterProngSqSum", 2, 1, 0);

  VecSpecHis vecHisLc;
  AddHistogram(vecHisLc, "#it{p}_{T} prong 0 (GeV/#it{c})", "hPtLcDau0", "hf-task-lc/Data/hPtProng0", 2, 1, 0);
  AddHistogram(vecHisLc, "#it{p}_{T} prong 1 (GeV/#it{c})", "hPtLcDau1", "hf-task-lc/Data/hPtProng1", 2, 1, 0);
  AddHistogram(vecHisLc, "#it{p}_{T} prong 2 (GeV/#it{c})", "hPtLcDau2", "hf-task-lc/Data/hPtProng2", 2, 1, 0);
  AddHistogram(vecHisLc, "#it{p}_{T} #Lambda_{c}^{#plus} (GeV/#it{c})", "hPtLc", "hf-task-lc/Data/hPt", 2, 1, 0);
  AddHistogram(vecHisLc, "3-prong mass (p K #pi) (GeV/#it{c}^{2})", "hInvMassLc", "hf-task-lc/Data/hMass", 2, 0, 0, "x");
  AddHistogram(vecHisLc, "prong 0 DCAxy to prim. vertex (cm)", "hImpParLcDau0", "hf-task-lc/Data/hd0Prong0", 2, 1, 0);
  AddHistogram(vecHisLc, "prong 1 DCAxy to prim. vertex (cm)", "hImpParLcDau1", "hf-task-lc/Data/hd0Prong1", 2, 1, 0);
  AddHistogram(vecHisLc, "prong 2 DCAxy to prim. vertex (cm)", "hImpParLcDau2", "hf-task-lc/Data/hd0Prong2", 2, 1, 0);
  AddHistogram(vecHisLc, "decay length (cm)", "hDecLenLc", "hf-task-lc/Data/hDecLength", 2, 1, 0);
  AddHistogram(vecHisLc, "decay length xy (cm)", "hDecLenLcXY", "hf-task-lc/Data/hDecLengthxy", 2, 1, 0);
  AddHistogram(vecHisLc, "proper lifetime (#Lambda_{c}) * #it{c} (cm)", "hCt", "hf-task-lc/Data/hCt", 2, 1, 0);
  AddHistogram(vecHisLc, "cos pointing angle", "hCosPointLc", "hf-task-lc/Data/hCPA", 2, 1, 0);
  AddHistogram(vecHisLc, "cos pointing angle xy", "hCosPointXYLc", "hf-task-lc/Data/hCPAxy", 2, 1, 0);
  AddHistogram(vecHisLc, "#it{#eta}", "hEtaLc", "hf-task-lc/Data/hEta", 2, 1, 0);
  AddHistogram(vecHisLc, "#it{#Phi}", "hPhiLc", "hf-task-lc/Data/hPhi", 2, 1, 0);

  labelParticle = "#Lambda_{c}^{#plus} #rightarrow p K #pi";
  VecSpecHis vecHisLcMCPt;
  AddHistogram(vecHisLcMCPt, labelParticle + ", matched prompt: #it{p}_{T}^{rec} (GeV/#it{c})", "hPtRecoPromptLcpKpi", "hf-task-lc/MC/reconstructed/prompt/hPtRecSigPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCPt, labelParticle + ", gen. prompt: #it{p}_{T}^{gen} (GeV/#it{c})", "hPtGenLimAccPromptLcpKpi", "hf-task-lc/MC/generated/prompt/hPtGenPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCPt, labelParticle + ", matched non-prompt: #it{p}_{T}^{rec} (GeV/#it{c})", "hPtRecoFeeddwLcpKpi", "hf-task-lc/MC/reconstructed/nonprompt/hPtRecSigNonPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCPt, labelParticle + ", gen. non-prompt: #it{p}_{T}^{gen} (GeV/#it{c})", "hPtGenLimAccFeeddwLcpKpi", "hf-task-lc/MC/generated/nonprompt/hPtGenNonPrompt", 2, 1, 0);

  VecSpecHis vecHisLcMCEta;
  AddHistogram(vecHisLcMCEta, labelParticle + ", matched prompt: #it{#eta}", "hEtaLcPrompt", "hf-task-lc/MC/reconstructed/prompt/hEtaRecSigPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCEta, labelParticle + ", gen. prompt:#it{#eta}", "hEtaGenLimAccPromptLcpKpi", "hf-task-lc/MC/generated/prompt/hEtaGenPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCEta, labelParticle + ", matched non-prompt: #it{#eta}", "hEtaLcNonPrompt", "hf-task-lc/MC/reconstructed/nonprompt/hEtaRecSigNonPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCEta, labelParticle + ", gen. non-prompt: #it{#eta}", "hEtaGenLimAccFeeddwLcpKpi", "hf-task-lc/MC/generated/nonprompt/hEtaGenNonPrompt", 2, 1, 0);

  VecSpecHis vecHisLcMCPhi;
  AddHistogram(vecHisLcMCPhi, labelParticle + ", matched prompt: #it{#Phi}", "hPhiLcPrompt", "hf-task-lc/MC/reconstructed/prompt/hPhiRecSigPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCPhi, labelParticle + ", gen. prompt:#it{#Phi}", "hPhiGenLimAccPromptLcpKpi", "hf-task-lc/MC/generated/prompt/hPhiGenPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCPhi, labelParticle + ", matched non-prompt: #it{#Phi}", "hPhiLcNonPrompt", "hf-task-lc/MC/reconstructed/nonprompt/hPhiRecSigNonPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCPhi, labelParticle + ", gen. non-prompt: #it{#Phi}", "hPhiGenLimAccFeeddwLcpKpi", "hf-task-lc/MC/generated/nonprompt/hPhiGenNonPrompt", 2, 1, 0);

  VecSpecHis vecHisLcMCPrompt;
  AddHistogram(vecHisLcMCPrompt, "#it{p}_{T} prong 0 (GeV/#it{c})", "hPtLcDau0Prompt", "hf-task-lc/MC/reconstructed/prompt/hPtRecProng0SigPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCPrompt, "#it{p}_{T} prong 1 (GeV/#it{c})", "hPtLcDau1Prompt", "hf-task-lc/MC/reconstructed/prompt/hPtRecProng1SigPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCPrompt, "#it{p}_{T} prong 2 (GeV/#it{c})", "hPtLcDau2Prompt", "hf-task-lc/MC/reconstructed/prompt/hPtRecProng2SigPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCPrompt, "3-prong mass (p K #pi) (GeV/#it{c}^{2})", "hInvMassLcPrompt", "hf-task-lc/MC/reconstructed/prompt/hMassRecSigPrompt", 2, 0, 0, "x");
  AddHistogram(vecHisLcMCPrompt, "prong 0 DCAxy to prim. vertex (cm)", "hImpParLcDau0Prompt", "hf-task-lc/MC/reconstructed/prompt/hd0RecProng0SigPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCPrompt, "prong 1 DCAxy to prim. vertex (cm)", "hImpParLcDau1Prompt", "hf-task-lc/MC/reconstructed/prompt/hd0RecProng1SigPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCPrompt, "prong 2 DCAxy to prim. vertex (cm)", "hImpParLcDau2Prompt", "hf-task-lc/MC/reconstructed/prompt/hd0RecProng2SigPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCPrompt, "decay length (cm)", "hDecLenLcPrompt", "hf-task-lc/MC/reconstructed/prompt/hDecLengthRecSigPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCPrompt, "decay length xy (cm)", "hDecLenLcXYPrompt", "hf-task-lc/MC/reconstructed/prompt/hDecLengthxyRecSigPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCPrompt, "proper lifetime (#Lambda_{c}) * #it{c} (cm)", "hCtPrompt", "hf-task-lc/MC/reconstructed/prompt/hCtRecSigPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCPrompt, "cos pointing angle", "hCosPointLcPrompt", "hf-task-lc/MC/reconstructed/prompt/hCPARecSigPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCPrompt, "cos pointing angle xy", "hCosPointXYLcPrompt", "hf-task-lc/MC/reconstructed/prompt/hCPAxyRecSigPrompt", 2, 1, 0);

  VecSpecHis vecHisLcMCNonPrompt;
  AddHistogram(vecHisLcMCNonPrompt, "#it{p}_{T} prong 0 (GeV/#it{c})", "hPtLcDau0NonPrompt", "hf-task-lc/MC/reconstructed/nonprompt/hPtRecProng0SigNonPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCNonPrompt, "#it{p}_{T} prong 1 (GeV/#it{c})", "hPtLcDau1NonPrompt", "hf-task-lc/MC/reconstructed/nonprompt/hPtRecProng1SigNonPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCNonPrompt, "#it{p}_{T} prong 2 (GeV/#it{c})", "hPtLcDau2NonPrompt", "hf-task-lc/MC/reconstructed/nonprompt/hPtRecProng2SigNonPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCNonPrompt, "3-prong mass (p K #pi) (GeV/#it{c}^{2})", "hInvMassLcNonPrompt", "hf-task-lc/MC/reconstructed/nonprompt/hMassRecSigNonPrompt", 2, 0, 0, "x");
  AddHistogram(vecHisLcMCNonPrompt, "prong 0 DCAxy to prim. vertex (cm)", "hImpParLcDau0NonPrompt", "hf-task-lc/MC/reconstructed/nonprompt/hd0RecProng0SigNonPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCNonPrompt, "prong 1 DCAxy to prim. vertex (cm)", "hImpParLcDau1NonPrompt", "hf-task-lc/MC/reconstructed/nonprompt/hd0RecProng1SigNonPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCNonPrompt, "prong 2 DCAxy to prim. vertex (cm)", "hImpParLcDau2NonPrompt", "hf-task-lc/MC/reconstructed/nonprompt/hd0RecProng2SigNonPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCNonPrompt, "decay length (cm)", "hDecLenLcNonPrompt", "hf-task-lc/MC/reconstructed/nonprompt/hDecLengthRecSigNonPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCNonPrompt, "decay length xy (cm)", "hDecLenLcXYNonPrompt", "hf-task-lc/MC/reconstructed/nonprompt/hDecLengthxyRecSigNonPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCNonPrompt, "proper lifetime (#Lambda_{c}) * #it{c} (cm)", "hCtNonPrompt", "hf-task-lc/MC/reconstructed/nonprompt/hCtRecSigNonPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCNonPrompt, "cos pointing angle", "hCosPointLcNonPrompt", "hf-task-lc/MC/reconstructed/nonprompt/hCPARecSigNonPrompt", 2, 1, 0);
  AddHistogram(vecHisLcMCNonPrompt, "cos pointing angle xy", "hCosPointXYLcNonPrompt", "hf-task-lc/MC/reconstructed/nonprompt/hCPAxyRecSigNonPrompt", 2, 1, 0);

  VecSpecHis vecHisJpsi;
  AddHistogram(vecHisJpsi, "#it{p}_{T} prong 0 (GeV/#it{c})", "hPtJpsiDau0", "hf-task-jpsi/hPtProng0", 2, 1, 0);
  AddHistogram(vecHisJpsi, "#it{p}_{T} prong 1 (GeV/#it{c})", "hPtJpsiDau1", "hf-task-jpsi/hPtProng1", 2, 1, 0);
  AddHistogram(vecHisJpsi, "#it{p}_{T} J/#psi (GeV/#it{c})", "hPtJpsi", "hf-task-jpsi/hPtCand", 2, 1, 0);
  AddHistogram(vecHisJpsi, "2-prong mass (e^{#plus} e^{#minus}) (GeV/#it{c}^{2})", "hInvMassJpsi", "hf-task-jpsi/hMass", 2, 0, 0);
  AddHistogram(vecHisJpsi, "d0d0 (cm^{2})", "hd0Timesd0Jpsi", "hf-task-jpsi/hd0d0", 2, 1, 0);
  AddHistogram(vecHisJpsi, "d0 prong 0 (cm)", "hImpParJpsiDau0", "hf-task-jpsi/hd0Prong0", 2, 1, 0);
  AddHistogram(vecHisJpsi, "d0 prong 1 (cm)", "hImpParJpsiDau1", "hf-task-jpsi/hd0Prong1", 2, 1, 0);
  AddHistogram(vecHisJpsi, "decay length (cm)", "hDecLenJpsi", "hf-task-jpsi/hDecLength", 2, 1, 0);
  AddHistogram(vecHisJpsi, "decay length XY (cm)", "hDecLenXYJpsi", "hf-task-jpsi/hDecLengthxy", 2, 1, 0);
  AddHistogram(vecHisJpsi, "cos pointing angle", "hCosPointJpsi", "hf-task-jpsi/hCPA", 2, 1, 0);
  AddHistogram(vecHisJpsi, "decay length error (cm)", "hDecLenErrJpsi", "hf-task-jpsi/hDecLenErr", 1, 1, 0);
  AddHistogram(vecHisJpsi, "decay length XY error (cm)", "hDecLenXYErrJpsi", "hf-task-jpsi/hDecLenXYErr", 1, 1, 0);

  // vector of specifications of vectors: name, VecSpecHis, pads X, pads Y
  std::vector<std::tuple<TString, VecSpecHis, int, int>> vecSpecVecSpec;

  // Add vector specifications in the vector.
  if (options.Contains(" events "))
    vecSpecVecSpec.push_back(std::make_tuple("events", vecHisEvents, 4, 2));
  if (options.Contains(" tracks "))
    vecSpecVecSpec.push_back(std::make_tuple("tracks", vecHisTracks, 5, 3));
  if (options.Contains(" skim "))
    vecSpecVecSpec.push_back(std::make_tuple("skim", vecHisSkim, 5, 3));
  if (options.Contains(" cand2 "))
    vecSpecVecSpec.push_back(std::make_tuple("cand2", vecHisCand2, 5, 3));
  if (options.Contains(" cand3 "))
    vecSpecVecSpec.push_back(std::make_tuple("cand3", vecHisCand3, 5, 3));
  if (options.Contains(" d0 "))
    vecSpecVecSpec.push_back(std::make_tuple("d0", vecHisD0, 5, 3));
  if (options.Contains(" d0-mc "))
    vecSpecVecSpec.push_back(std::make_tuple("d0-mc", vecHisD0MC, 2, 2));
  if (options.Contains(" ds "))
    vecSpecVecSpec.push_back(std::make_tuple("ds", vecHisDs, 2, 2));
  if (options.Contains(" dplus "))
    vecSpecVecSpec.push_back(std::make_tuple("dplus", vecHisDPlus, 5, 4));
  if (options.Contains(" lc "))
    vecSpecVecSpec.push_back(std::make_tuple("lc", vecHisLc, 5, 3));
  if (options.Contains(" lc-mc-pt "))
    vecSpecVecSpec.push_back(std::make_tuple("lc-mc-pt", vecHisLcMCPt, 2, 2));
  if (options.Contains(" lc-mc-eta "))
    vecSpecVecSpec.push_back(std::make_tuple("lc-mc-eta", vecHisLcMCEta, 2, 2));
  if (options.Contains(" lc-mc-phi "))
    vecSpecVecSpec.push_back(std::make_tuple("lc-mc-phi", vecHisLcMCPhi, 2, 2));
  if (options.Contains(" lc-mc-prompt "))
    vecSpecVecSpec.push_back(std::make_tuple("lc-mc-prompt", vecHisLcMCPrompt, 5, 3));
  if (options.Contains(" lc-mc-nonprompt "))
    vecSpecVecSpec.push_back(std::make_tuple("lc-mc-nonprompt", vecHisLcMCNonPrompt, 5, 3));
  if (options.Contains(" jpsi "))
    vecSpecVecSpec.push_back(std::make_tuple("jpsi", vecHisJpsi, 5, 3));

  // Histogram plot vertical margins
  Float_t marginHigh = 0.05;
  Float_t marginLow = 0.05;
  bool logScaleH = false;
  // Ratio plot vertical margins
  Float_t marginRHigh = 0.05;
  Float_t marginRLow = 0.05;
  bool logScaleR = false;
  Float_t yMin, yMax;
  Int_t nRun2, nRun3, rebin;

  TH1F* hRun2 = nullptr;
  TH1D* hRun3 = nullptr;
  TH1F* hRatio = nullptr;
  TString labelAxis = "";
  TString nameHisRun2 = "";
  TString nameHisRun3 = "";
  TString projAx = "";
  TCanvas* canHis = nullptr;
  TCanvas* canRat = nullptr;

  // loop over lists
  for (const auto& specVecSpec : vecSpecVecSpec) {
    auto nameSpec = std::get<0>(specVecSpec); // list name
    auto vecSpec = std::get<1>(specVecSpec);  // list of histogram specs.
    int nPadsX = std::get<2>(specVecSpec);    // number of horizontal pads
    int nPadsY = std::get<3>(specVecSpec);    // number of vertical pads
    Printf("\nProcessing histogram list: %s (%d)", nameSpec.Data(), (int)vecSpec.size());
    if (nPadsX * nPadsY < vecSpec.size()) {
      Printf("Not enough pads (%d)", nPadsX * nPadsY);
      return 1;
    }

    canHis = new TCanvas(Form("canHis_%s", nameSpec.Data()), Form("Histos_%s", nameSpec.Data()), 3000, 1600);
    SetCanvas(canHis, nPadsX, nPadsY);
    if (doRatio) {
      canRat = new TCanvas(Form("canRat_%s", nameSpec.Data()), Form("Ratios_%s", nameSpec.Data()), 3000, 1600);
      SetCanvas(canRat, nPadsX, nPadsY);
    }

    // loop over histograms
    for (int index = 0; index < vecSpec.size(); index++) {
      auto spec = vecSpec[index];
      labelAxis = std::get<0>(spec);
      nameHisRun2 = std::get<1>(spec);
      nameHisRun3 = std::get<2>(spec);
      rebin = std::get<3>(spec);
      logScaleH = std::get<4>(spec);
      logScaleR = std::get<5>(spec);
      projAx = std::get<6>(spec);

      // Get AliPhysics histogram.
      hRun2 = (TH1F*)lRun2->FindObject(nameHisRun2.Data());
      if (!hRun2) {
        printf("Failed to load %s from %s\n", nameHisRun2.Data(), filerun2.Data());
        return 1;
      }

      // Get O2 histogram.
      auto oRun3 = fRun3->Get(nameHisRun3.Data());
      if (!oRun3) {
        printf("Failed to load %s from %s\n", nameHisRun3.Data(), filerun3.Data());
        return 1;
      }

      if (oRun3->InheritsFrom("TH3")) {
        if (projAx == "x") {
          hRun3 = ((TH3D*)oRun3)->ProjectionX();
        } else if (projAx == "y") {
          hRun3 = ((TH3D*)oRun3)->ProjectionY();
        }
      } else if (oRun3->InheritsFrom("TH2")) {
        if (projAx == "x") {
          hRun3 = ((TH2D*)oRun3)->ProjectionX();
        } else if (projAx == "y") {
          hRun3 = ((TH2D*)oRun3)->ProjectionY();
        }
      } else {
        hRun3 = (TH1D*)oRun3;
      }

      Printf("%d (%s, %s): bins: %d, %d, ranges: %g-%g, %g-%g",
             index, nameHisRun2.Data(), nameHisRun3.Data(),
             hRun2->GetNbinsX(), hRun3->GetNbinsX(),
             hRun2->GetXaxis()->GetBinLowEdge(1), hRun2->GetXaxis()->GetBinUpEdge(hRun2->GetNbinsX()),
             hRun3->GetXaxis()->GetBinLowEdge(1), hRun3->GetXaxis()->GetBinUpEdge(hRun3->GetNbinsX()));

      nRun2 = hRun2->GetEntries();
      nRun3 = hRun3->GetEntries();

      // Histograms
      auto padH = canHis->cd(index + 1);
      hRun2->Rebin(rebin);
      hRun3->Rebin(rebin);
      hRun2->SetLineColor(1);
      hRun2->SetLineWidth(2);
      hRun3->SetLineColor(2);
      hRun3->SetLineWidth(1);
      hRun2->SetTitle(Form("Entries: Run2: %d, Run3: %d;%s;Entries", nRun2, nRun3, labelAxis.Data()));
      hRun2->GetYaxis()->SetMaxDigits(3);
      yMin = TMath::Min(hRun3->GetMinimum(0), hRun2->GetMinimum(0));
      yMax = TMath::Max(hRun3->GetMaximum(), hRun2->GetMaximum());
      SetHistogram(hRun2, yMin, yMax, marginLow, marginHigh, logScaleH);
      SetPad(padH, logScaleH);
      hRun2->Draw();
      hRun3->Draw("same");
      TLegend* legend = new TLegend(0.8, 0.72, 1., 0.92);
      legend->AddEntry(hRun2, "Run2", "L");
      legend->AddEntry(hRun3, "Run3", "L");
      legend->Draw();

      // Ratio
      if (doRatio) {
        auto padR = canRat->cd(index + 1);
        hRatio = (TH1F*)hRun3->Clone(Form("hRatio%d", index));
        hRatio->Divide(hRun2);
        hRatio->SetTitle(Form("Entries ratio: %g;%s;Run3/Run2", (double)nRun3 / (double)nRun2, labelAxis.Data()));
        yMin = hRatio->GetMinimum(0);
        yMax = hRatio->GetMaximum();
        SetHistogram(hRatio, yMin, yMax, marginRLow, marginRHigh, logScaleR);
        SetPad(padR, logScaleR);
        hRatio->Draw();
      }
    }
    canHis->SaveAs(Form("comparison_histos_%s.png", nameSpec.Data()));
    if (doRatio) {
      canRat->SaveAs(Form("comparison_ratios_%s.png", nameSpec.Data()));
    }
    delete canHis;
    delete canRat;
  }
  return 0;
}
