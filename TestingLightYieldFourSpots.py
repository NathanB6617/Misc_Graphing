#####################################################################################################
## Name: GraphingLightYieldEdge.py                                                                 ##
## Author: Nathan Burns                                                                            ##
## Date(s): Summer 2026                                                                            ##
## Purpose: Read four consecutive light yield text files, convert the ADC values to PE, determine ##
## the MPV of each distribution, and graph the four positions along one edge of a scintillating    ##
## tile.                                                                                           ##
#####################################################################################################

import ROOT
from pathlib import Path
import matplotlib.pyplot as plt
import numpy as np
import re


# Sort files numerically by run number
def run_number(file_path):
    match = re.search(r'Run(\d+)', file_path.name)

    if match:
        return int(match.group(1))

    return -1


# Read the high gain data from one txt file
def readData(file_path):

    LG_vals = []
    HG_vals = []

    with open(file_path, "r") as f:

        # Skip header
        for _ in range(9):
            next(f)

        for line in f:

            cols = line.split()

            if len(cols) < 4:
                continue

            channel_num = int(cols[1])

            # Only use channel 0
            if channel_num != 0:
                continue

            LG_vals.append(int(cols[2]))
            HG_vals.append(int(cols[3]))

    return LG_vals, HG_vals


# Convert ADC to Photoelectrons
def ADCtoPE(HG_vals, gain):

    HG_PE = []

    for val in HG_vals:
        HG_PE.append(val / gain)

    return HG_PE


# Fit the distribution to obtain the MPV
def MPVData(HG_vals, name):

    hist = ROOT.TH1F(f"hist_{id(HG_vals)}", "hist", 250, 0, 250)

    for x in HG_vals:
        hist.Fill(x)

    result = ROOT.GetMPV(hist, str(name))

    mpv = result[0]
    mpvErr = result[1]
    Chi2 = result[2]
    NDf = result[3]

    print(f"MPV = {mpv:.3f} ± {mpvErr:.3f}")

    if NDf != 0:
        ratio = Chi2 / NDf

        if ratio >= 3:
            print(f"WARNING: Chi2/NDF = {ratio:.2f}")

    return mpv, mpvErr


# Graph one edge (4 positions)
def edgeGraphData(mpv_vals, mpvErr_vals):

    grid = np.array([mpv_vals])
    err_grid = np.array([mpvErr_vals])

    plt.figure(figsize=(8, 2.5))

    plt.imshow(
        grid,
        cmap="viridis",
        origin="lower",
        aspect="equal"
    )

    for col in range(len(mpv_vals)):

        plt.text(
            col,
            0,
            f"{grid[0, col]:.2f}\n±{err_grid[0, col]:.2f}",
            ha="center",
            va="center",
            color="white",
            fontsize=10
        )

    plt.xticks(
        range(len(mpv_vals)),
        [f"Pos {i+1}" for i in range(len(mpv_vals))]
    )

    plt.yticks([])

    plt.colorbar(label="MPV (PE)")

    plt.title("Tile Edge Light Yield")

    plt.tight_layout()

    plt.show()


def main(path, gain, start_index=0):

    path = Path(path)

    all_mpv_vals = []
    all_mpvErr_vals = []

    ROOT.gROOT.ProcessLine(".L LandauGauss.C+")

    # Sort all txt files
    files = sorted(path.glob("*.txt"), key=run_number)

    # Read only four consecutive files
    edge_files = files[start_index:start_index + 3]

    if len(edge_files) != 3:
        print("Error: Could not find four consecutive txt files.")
        return

    for file_path in edge_files:

        print(f"\nReading {file_path.name}")

        LG_vals, HG_vals = readData(file_path)

        HG_PE = ADCtoPE(HG_vals, gain)

        mpv_val, mpvErr = MPVData(HG_PE, file_path.name)

        all_mpv_vals.append(mpv_val)
        all_mpvErr_vals.append(mpvErr)

    edgeGraphData(all_mpv_vals, all_mpvErr_vals)


if __name__ == "__main__":

    main("/home/lfhcal/Nathan/Sr90_Light_Yield_Testing/SingleRow/Test1", gain=41.0173, start_index=0)
