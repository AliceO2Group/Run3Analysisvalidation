// Validation utilities

// vectors of histogram specifications
using VecSpecHis = std::vector<std::tuple<TString, TString, TString, int, bool, bool, TString>>;

// Add histogram specification in the vector.
void AddHistogram(VecSpecHis& vec, TString label, TString nameAli, TString nameO2, int rebin, bool logH, bool logR, TString proj = "x")
{
  vec.push_back(std::make_tuple(label, nameAli, nameO2, rebin, logH, logR, proj));
}
