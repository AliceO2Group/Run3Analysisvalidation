import typing

import ROOT

import o2qaplots.plot_base as pb


def calculate_efficiency(reconstructed: ROOT.TH3D, generated: ROOT.TH3D, eta_cut: float = None,
                         pt_range: typing.List[float] = None):
    """ Calculated the efficiency as function of the feature in axis.

    Args:
        reconstructed: histogram with the reconstructed information.
        generated: histogram with the generated information.
        eta_cut: applies the selection |n| < eta_cut to the efficiency
        pt_range: selects only particles with pt_range[0] < pt < pt_range[1]

    Returns:
        efficiency: a TGraph with the efficiencies
    """

    epsilon = 0.0001
    if eta_cut is not None:
        generated.GetYaxis().SetRangeUser(-eta_cut + epsilon, eta_cut - epsilon)
        reconstructed.GetYaxis().SetRangeUser(-eta_cut + epsilon, eta_cut - epsilon)

    if pt_range is not None:
        if len(pt_range) != 2:
            raise ValueError('You should pass exactly two values to the transverse momentum range (pt_range).')

        generated.GetXaxis().SetRangeUser(pt_range[0] + epsilon, pt_range[1] - epsilon)
        reconstructed.GetXaxis().SetRangeUser(pt_range[0] + epsilon, pt_range[1] - epsilon)

    generated_1d = generated.Project3D('x')
    reconstructed_1d = reconstructed.Project3D('x')

    efficiency = ROOT.TEfficiency(reconstructed_1d, generated_1d)

    return efficiency.CreateGraph()


class Efficiency(pb.PlottingTask):
    parser_description = 'Calculates the efficiency for the physical primary particles.'

    input_argument = pb.Configurable('particle', '-p', '--particle', help='particle to be processed', type=str,
                                     choices=['electron', 'pion', 'kaon', 'muon', 'proton'], default='pion')

    generated = pb.TaskInputObj(['qa-tracking-efficiency'], 'generatedKinematics')
    reconstructed = pb.TaskInputObj(['qa-tracking-efficiency'], 'reconstructedKinematics')

    efficiency = pb.ROOTObj(['qa-tracking-efficiency'], 'primaryTrackEfficiency')

    configurables = [pb.Configurable('eta', '--eta', '-e', default=1.4, type=float, help='Cut in |pseudorapidity| < e'),
                     pb.Configurable('pt_range', '--pt_range', '-pt', default=(0., 10.), nargs=2, action='append',
                                 type=float, help='Cut in pt < pt_max')]

    plotting_kwargs = {'draw_option': 'APE'}

    def process(self):
        eff = calculate_efficiency(self.reconstructed, self.generated, eta_cut=self.eta, pt_range=self.pt_range)

        return {self.efficiency: eff}


if __name__ == '__main__':
    pb.macro(Efficiency)
