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
def GraphData(HG_vals, run_name):
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
    c.Update()

    c.SaveAs(f"/home/lfhcal/Nathan/Sr90_Light_Yield_Testing/RAW_DATA/TEST/{run_name}.png")
    c.Close()


def makeHist(file_path):
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

            # Only look at channel 0 - CHANGE THIS TO WHATEVER CHANNEL YOU WANT TO READ
            if int(cols[1]) != 0:
                continue

            hg = int(cols[3])
            HG_vals.append(hg)

    GraphData(HG_vals, file_path.stem)



def main(path):
    path = Path(path)

    for file_path in sorted(path.glob("*.txt"), key=run_number):

        print(f"Reading {file_path.name}")

        makeHist(file_path)



if __name__ == "__main__":
    # change these values if you want to call manually from the terminal
    main("/home/lfhcal/Jessie/Cosmic_Data/SPS/Run2")
