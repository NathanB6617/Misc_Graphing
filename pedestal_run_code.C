#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <tuple>
#include "TGraph.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TLegend.h"
#include "TLatex.h"
#include "TH1D.h"
#include "TPaveText.h"
#include "TStyle.h"
#include <TH1F.h>
#include <TMath.h>

//this code is based off of my previous code, readTxtFilesFromFolder.

//this code is for the CAEN digitizer readings we got when we used an LED to test the high and low gains
//before running this code, please sort the readings into a preferred destination and label each folder 
//however you like. In this code, the labels on the folder are 41V, 43V, and 45V (as they were the voltage
//we applied). Should you use this code, please be careful as you will have to likely slightly modify 
//the names of the folders and the path to the main folder. 

//HOWEVER, main big thing is that this code uses #include <filesystem> which is not directly supported in 
//ROOT, therefore you will have to run root .L readTxtFilesFromFolder.C and not just execute it directly 
//in ROOT. You may also need a compilier like clang++ if you want to run outside of ROOT.

//to run outside of ROOT: 
//1) clang++ -std=c++17 -O2 -o readTxtFilesFromFolder readTxtFilesFromFolder.C `root-config --cflags --libs`
//2) ./readTxtFilesFromFolder
//to run inside of ROOT: 
//1) root (to open ROOT) 
//2) .L pedestal_run_.C (forces ROOT to run with a compilier) 
//3) main() (calls main)


using namespace std;
namespace fs = std::filesystem;

//graphing function prototype
void caenPhotonGraphs(const vector<double>& x_vals, const vector<double>& y_vals, const fs::directory_entry& entry, int num_iter, int folderChoice);

int main()
{
    //IMPORTANT
    //this is the path to the FOLDER where the files you want to read are. Simply click on the folder in your
    //file explorer and click "copy as path" and then paste it here. It may add in parentheses when you copy
    //as path, if so simply delete them. The only parentheses should be the one after the R and at the end.

    //declare a variable for the number of iterations, starting at 1 for the first
    int num_iter = 1;
    string path = R"(/home/operator2/Downloads/Pedestal_runs_8k_all_voltage_readings)";

    //check if path exists
    if (!fs::exists(path)) {
        cerr << "Directory does not exist: " << path << endl;
        return 1;
    }

    //prompt the user what folder they would like
    int folderChoice;
    cout << "Enter the voltage folder you would like to read from (41, 43, 45): " << endl;
    cin >> folderChoice;

    //build the selected folder path
    string selectedFolder = path + "//" + to_string(folderChoice) + "V";

    //check if inputted folder exists
    if (!fs::exists(selectedFolder)) {
        cerr << "Selected folder does not exist: " << selectedFolder << endl;
        return 1;
    }

    //loops over every file in the "path" which in this case is the folder, therefore
    //it loops over every file in the folder.
    for (const auto& entry : fs::recursive_directory_iterator(selectedFolder)) {
	
        //this if statement just checks if it is a "regular file" and is a built in function.
        if (entry.is_regular_file()) {

            //check that the file is a .txt file
            if (entry.path().extension() != ".txt") {
                //still state what file you are reading from, but now state that it isn't a .txt
                cout << "Reading from file: " << entry.path() << endl;
                cerr << "This file is not a .txt file. Moving on to the next one" << endl;
                cout << endl << endl;
                //continue to the next iteration
                continue;
            }

            //state which folder and file you are reading from
            cout << "Reading from folder: " << entry.path().parent_path().filename() << endl;
            cout << "Reading from file: " << entry.path() << endl;

            //create and open the file in the folder
            ifstream inputFile;
            inputFile.open(entry.path());

            //make sure the file opened correctly
            if (inputFile.fail()) {
                cerr << "Couldn't open the file!" << endl;
                continue;
            }

            //extra check to debug if the file is empty for some reason
            if (inputFile.peek() == std::ifstream::traits_type::eof()) {
                cerr << "File is empty or unreadable." << endl;
                cout << endl << endl;
                continue;
            }

            //skip the first 9 lines (they should just be the ones telling us stuff about the run)
            string line;
            for (int i = 0; i < 9; ++i) {
            getline(inputFile, line);
            }

            //create vectors to store the low and high gains
            vector<double> x_vals;
            vector<double> y_vals;
            double num = 0;

            //read in the data lines from the txt files (only the low and high gains in columns 3 and 4)
            double col[7];
            while (inputFile >> col[0] >> col[1] >> col[2] >> col[3] >> col[4] >> col[5] >> col[6]) {
                x_vals.push_back(col[2]); //Low Gain (from column 3 of txt file)
                y_vals.push_back(col[3]); //High Gain (from column 4 of txt file)
            }

            //close input file
            inputFile.close();

            //check if data was read
            int n = x_vals.size();
            if (n == 0) {
                cerr << "No data found after skipping first 9 lines." << endl;
                continue;
            }
	    
	    //call the function to plot the graphs	  
            caenPhotonGraphs(x_vals, y_vals, entry, num_iter, folderChoice);
	   
	    
	    //increment the iteration
	    num_iter++;
	    
            //add a divide between file to make it look better in terminal
            cout << endl << endl;
        }
    }
    
    return 0;
}

void caenPhotonGraphs(const vector<double>& x_vals, const vector<double>& y_vals, const fs::directory_entry& entry, int num_iter, int folderChoice) {

    //this function should take the two vectors that will be made from the txt files and plot them 
    //in ROOT. 

    //make a variable for the size of the graph
    int n;
    n = x_vals.size();

    //get the file name again so we can use it as a title
    string file = entry.path().filename().string();
    string filename = file.substr(0, file.find_last_of("."));

    //also get the voltage based on the files name
    //this way of doing it only works with how it is set up in my lab
    //For my lab:  Change the voltage applied based on what you did (3.1 for first, 3.3 for second and third)
    string vpp_path = entry.path();
    string vpp_folder = vpp_path.substr(vpp_path.length() - 15);
    char vpp_last_num = vpp_folder[0];
    //change this value if voltage applied range was changed
    string vpp_legend = "Voltage Applied to LED: 3.3";
    vpp_legend += vpp_last_num;
    vpp_legend += "V";
    
    //make the title of the graph
    string vol_title = "3.3" + string(1, vpp_last_num) + "V";
    string graphTitle = "Low Gain vs High Gain: " + vol_title;

    //create canvas
    TCanvas* c1 = new TCanvas("c1", graphTitle.c_str(), 1200, 900);
    c1->Divide(1,2);
    c1->cd(1);

    //now make a histogram of the high gains (y_vals) and low gains (x_vals)
    TH1D* h1 = new TH1D("h1", "Histogram of High Gain", 800, 0, 100);

    //set and center the x axis
    h1->SetXTitle("Number of Channels Hit");
    h1->GetXaxis()->SetTitleOffset(1.2);
    h1->GetXaxis()->CenterTitle();
    
    //fill the histogram based on the data
    for (int i=0; i<y_vals.size(); i++) {
      h1->Fill(y_vals[i]);
    }

    //add in a gaussian distribution
    TF1  *fitResult = new TF1("f1","gaus", 0, 100);
    h1->Fit(fitResult, "R");
    fitResult->SetLineColor(kOrange);

    //get the mean, SD, and normalization
    double mean, SD, norm;
    mean = fitResult->GetParameter(1);
    SD = fitResult->GetParameter(2);
    norm = fitResult->GetParameter(0);
    
    //go ahead and set line color to make sure the legend gets it right
    h1->SetLineColor(kRed);
    
    //add a legend, including what the points are, the line, and the slope
    TLegend* legend2 = new TLegend(0.65, 0.75, 0.85, 0.88);
    legend2->SetTextSize(0.03);
    legend2->AddEntry(h1, "High Gain", "l");

    //create new text on the graph to put the mean, SD, and norm
    TPaveText *pt2 = new TPaveText(0.65, 0.55, 0.85, 0.70, "brNDC");
    pt2->AddText(Form("Mean: %.2f", mean));
    pt2->AddText(Form("Standard dev: %.2f", SD));
    pt2->AddText(Form("Normalization: %.2f", norm)); 
    pt2->SetTextSize(0.04);
    pt2->SetFillColor(0); 
    pt2->SetTextAlign(12);

    //draw the histogram on the canvas
    h1->Draw();
    gStyle->SetOptStat(0);
    legend2->Draw("SAME");
    pt2->Draw("SAME");
    fitResult->Draw("SAME");

    //switch to the second divide on the canvas
    c1->cd(2);

    //make the second histogram
    TH1D* h2 = new TH1D("Data", "Histogram of Low Gain", 800, 0, 100);

    //fill the histogram based on the data
    for (int i=0; i<y_vals.size(); i++) {
      h2->Fill(x_vals[i]);
    }
    
    //add in a gaussian distribution
    TF1  *fitResult2 = new TF1("f2","gaus", 0, 100);
    h2->Fit(fitResult2, "R");
    fitResult2->SetLineColor(kOrange);
    
    //get the mean, SD, and normalization
    double mean2, SD2, norm2;
    mean2 = fitResult2->GetParameter(1);
    SD2 = fitResult2->GetParameter(2);
    norm2 = fitResult2->GetParameter(0);

    //create new text on the graph to put the mean, SD, and norm
    TPaveText *pt = new TPaveText(0.65, 0.55, 0.85, 0.70, "brNDC");
    pt->AddText(Form("Mean: %.2f", mean2));
    pt->AddText(Form("Standard dev: %.2f", SD2));
    pt->AddText(Form("Normalization: %.2f", norm2)); 
    pt->SetTextSize(0.04);
    pt->SetFillColor(0); 
    pt->SetTextAlign(12);
    
    //set and center the x axis
    h2->SetXTitle("Number of Channels Hit");
    h2->GetXaxis()->SetTitleOffset(1.2);
    h2->GetXaxis()->CenterTitle();
    h2->SetLineColor(kBlue);
    
    //add a legend, including what the points are, the line, and the slope
    TLegend* legend = new TLegend(0.65, 0.75, 0.85, 0.88);
    legend->SetTextSize(0.03);
    legend->AddEntry(h2, "Low Gain", "l");
    
    //draw the histogram on the canvas
    h2->Draw();
    gStyle->SetOptStat(0);
    legend->Draw("SAME");
    pt->Draw("SAME");
    fitResult2->Draw("SAME");
    
    //save the file in the corresponding folder
    //IMPORTANT if reusing code you will have to change the file path to your selected folder
    c1->Print(Form("~/Downloads/Pedestal_runs_8k_all_voltage_readings/All_graphs/Graph_%iV_%s.png", folderChoice, vol_title.c_str()));

    //update the graph
    c1->Update();
}

