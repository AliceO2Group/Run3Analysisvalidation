#pylint: disable=missing-docstring
import math

def geometrybarrel():
    radius_list = [0.0050, 0.0120, 0.0250, 0.0375, 0.0700, 0.1200, 0.2000, 0.3000,
                   0.4500, 0.6000, 0.8000, 1.0000]
    halflength_list = [0.15, 0.15, 0.15, 0.62, 0.62, 0.62, 0.62, 0.62, 1.32, 1.32,
                       1.32, 1.32]
    theta_list = []
    eta_list = []
    for i, _ in enumerate(radius_list):
        angle = math.atan(radius_list[i] / halflength_list[i])

        eta = - math.log(math.tan(angle / 2.))
        theta_list.append(angle)
        eta_list.append(eta)

        print("layer=%d, radius=%.2f, halflength=%.2f, angle=%.2f, etamax =%.2f"
              % (i, radius_list[i], halflength_list[i], theta_list[i],
                 eta_list[i]))


geometrybarrel()
