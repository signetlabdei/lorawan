import sem
import numpy as np
import matplotlib.pyplot as plt

# Create our SEM campaign
ns_3_dir = '../../../'
script = 'aloha-throughput'
results_dir = 'aloha-results'
campaign = sem.CampaignManager.new(ns_3_dir, script, results_dir,
                                   check_repo=False, overwrite=True)

# Define the parameter space we are interested in exploring
params = {
    'nDevices': list(np.logspace(0, 3))
}
runs = 10

# Run simulations with the above parameter space
campaign.run_missing_simulations(params, runs)

example_result = campaign.db.get_complete_results()[0]


def get_psucc(result):
    """
    Extract the probability of success from the simulation output
    """
    outcomes = [float(a) for a in result['output']['stdout'].split()]
    return outcomes[1]/outcomes[0]


# def get_duration(result):
#     """
#     Extract the duration of packets from the simulation output
#     """

duration = 0.256256
simtime = 100
G = np.array(params['nDevices'])*duration/simtime

succprobs = np.mean(campaign.get_results_as_numpy_array(params, get_psucc,
                                                        runs),
                    axis=-1).squeeze()

S = np.multiply(succprobs, G)
S_theory = np.multiply(G, np.exp(-2*G))

plt.plot(G, S)
plt.plot(G, S_theory, '--')
plt.legend(["LoRaWAN module", "Theory"])
plt.show()
