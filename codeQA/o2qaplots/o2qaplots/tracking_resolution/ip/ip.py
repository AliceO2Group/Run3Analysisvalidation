import o2qaplots.plot as pl
import ROOT
from o2qaplots.plot_base import (
    Configurable,
    PlottingTask,
    ROOTObj,
    TaskInput,
    macro,
)


def plot_1d_legend(x, *args, **kwargs):
    return pl.plot_1d([x[0][0]], *args, legend=x[0][1], **kwargs)


class ImpactParameter(PlottingTask):
    parser_description = (
        "Plots the Impact Parameter resolution in the rphi and z direction. "
        "Currently it can only handle ONE file at at time. This will change soon."
    )
    parser_command = "ip"

    ip_rphi_pt = TaskInput(
        "qa-tracking-resolution/impactParameter/impactParameterRPhiVsPt"
    )
    ip_rphi_eta = TaskInput(
        "qa-tracking-resolution/impactParameter/impactParameterRPhiVsEta"
    )
    ip_rphi_phi = TaskInput(
        "qa-tracking-resolution/impactParameter/impactParameterRPhiVsPhi"
    )
    ip_z_pt = TaskInput("qa-tracking-resolution/impactParameter/impactParameterZVsPt")
    ip_z_eta = TaskInput("qa-tracking-resolution/impactParameter/impactParameterZVsEta")
    ip_z_phi = TaskInput("qa-tracking-resolution/impactParameter/impactParameterZVsPhi")

    ip_histograms = [
        ip_rphi_pt,
        ip_rphi_eta,
        ip_rphi_phi,
        ip_rphi_eta,
        ip_z_eta,
        ip_z_phi,
    ]

    show_fits = Configurable(
        "--show-fits", "-sf", action="store_true", default=False, help=""
    )

    plotting_function = plot_1d_legend

    def process(self):
        fit_slices, legends = calculate_ip_resolution(self.ip_rphi_pt)
        return {
            ROOTObj(f"qa-tracking-resolution/fit_slice_{i}"): (fit_slice, legend)
            for fit_slice, legend, i in zip(fit_slices, legends, range(len(fit_slices)))
        }


def calculate_ip_resolution(ip_vs_var):
    """Calculates the impact parameter (ip) resolution vs a particular variable.

    Args:
        ip_vs_var: a ROOT TH2 histogram with the ip in the y axis and the
            dependent variable in the x axis.
    """

    ROOT.TH1.AddDirectory(False)
    projections = [
        ip_vs_var.ProjectionY(ip_vs_var.GetName() + f"_{i}", i, i)
        for i in range(1, ip_vs_var.GetNbinsX() + 1)
    ]

    function = ROOT.TF1("gaus", "gaus", -1000, 1000)
    legends = []

    for hist_slice, i in zip(projections, range(1, len(projections) + 1)):
        hist_slice.GetYaxis().SetTitle(
            f"Counts ({ip_vs_var.GetXaxis().GetBinLowEdge(i)}, "
            f"{ip_vs_var.GetXaxis().GetBinUpEdge(i)})"
        )

        hist_slice.GetXaxis().SetRangeUser(-400, 400)
        fit_results = hist_slice.Fit(function, "QRS")

        chi2 = 0

        if int(fit_results) == 0:

            if fit_results.Ndf() > 0:
                chi2 = fit_results.Chi2() / fit_results.Ndf()

            mean, mean_error = fit_results.Parameter(1), fit_results.ParError(1)
            sigma, sigma_error = fit_results.Parameter(2), fit_results.ParError(2)
        else:
            chi2 = -999
            sigma, sigma_error = hist_slice.GetStdDev(), hist_slice.GetStdDevError()
            mean, mean_error = hist_slice.GetMean(), hist_slice.GetMeanError()

        legend = ROOT.TLegend()
        legend.SetHeader(f"Gaussian fit results (#chi^{{2}}/NDF = {chi2:.2f})")
        legend.AddEntry(ROOT.nullptr, f"Mean = {mean:.2f} #pm {mean_error:.2f}")
        legend.AddEntry(ROOT.nullptr, f"Sigma = {sigma:.2f} #pm {sigma_error:.2f}")

        legends.append(legend)

    return projections, legends


if __name__ == "__main__":
    macro(ImpactParameter)
