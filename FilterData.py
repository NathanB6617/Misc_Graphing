#####################################################################################################
## Name: FilterData.py                                                                             ## 
## Author: Nathan Burns                                                                            ##
## Date(s): Summer 2026                                                                            ##
## Purpose: This code sorts out data so that anything below a threshold (typically the pedestal)   ##
## is not read as data. It then returns the total number of data points there are after sorting    ## 
## and it saves a graph of the data in a directory so the user can see them just in case.          ##
#####################################################################################################

import re
from pathlib import Path
import ROOT


# Keeps files in run order
def run_number(file_path):
    match = re.search(r'Run(\d+)', file_path.name)
    if match:
        return int(match.group(1))
    return -1

# graph in ROOT and save
def GraphFilterData(HG_vals, run_name, thresholdL, thresholdU):
    hist = ROOT.TH1F(f"Histogram_{run_name}", run_name, 8000, 0, 8000)

    for hg in HG_vals:
        hist.Fill(hg)

    c = ROOT.TCanvas(f"c_{run_name}", run_name, 800, 600)
    hist.SetXTitle("Channels")
    hist.GetXaxis().SetTitleOffset(1.2)
    hist.GetXaxis().CenterTitle()

    hist.SetYTitle("Number of Hits")
    hist.GetYaxis().SetTitleOffset(1.2)
    hist.GetYaxis().CenterTitle()
    hist.Draw()

    ROOT.gPad.Update()
    ymax = ROOT.gPad.GetUymax()
    ymin = ROOT.gPad.GetUymin()
    l = ROOT.TLine(thresholdL, ymin, thresholdL, ymax)
    l2 = ROOT.TLine(thresholdU, ymin, thresholdU, ymax)
    l.SetLineStyle(2)
    l.SetLineColor(ROOT.kRed)
    l2.SetLineStyle(2)
    l2.SetLineColor(ROOT.kRed)
    l.Draw("SAME")
    l2.Draw("SAME")
    
    c.Update()

    c.SaveAs(f"/home/lfhcal/Nathan/Sr90_Light_Yield_Testing/RAW_DATA/TEST/{run_name}.png")
    c.Close()


def count_events(file_path, thresholdL, thresholdU):
    HG_vals = []

    with open(file_path, "r") as f:

        # Skip header
        for _ in range(9):
            next(f)

        for line in f:

            cols = line.split()

            # Skip malformed rows
            if len(cols) < 4:
                continue

            # Only look at channel 0
            if int(cols[1]) != 0:
                continue

            hg = int(cols[3])

            if (hg >= thresholdL and hg <= thresholdU):
                HG_vals.append(hg)

    # Quick check that the threshold worked
    GraphFilterData(HG_vals, file_path.stem, thresholdL, thresholdU)

    return len(HG_vals)



def main(path, thresholdL, thresholdU):
    path = Path(path)

    counts = []

    for file_path in sorted(path.glob("*.txt"), key=run_number):

        print(f"Reading {file_path.name}")

        n_events = count_events(file_path, thresholdL, thresholdU)
        counts.append(n_events)

        print(f"  Remaining events: {n_events}")

    print(counts)




if __name__ == "__main__":
    # change these values if you want to call manually from the terminal
    main("/home/lfhcal/Nathan/Sr90_Light_Yield_Testing/Test7", thresholdL=150, thresholdU=6000)
