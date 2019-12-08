/*
コマンドライン引数：ROOTファイルを含むディレクトリ名（末尾に/を含んでいても含んでいなくても良い）
ROOTファイル名に関する制約：小文字の ".root" を含んでいなければならない
*/

// C++ headers
#include <iostream>
#include <vector>
#include <cmath>
#include <regex>
#include <tuple>
#include <string>

// C headers
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>


// ROOT headers
#include <TFile.h>
#include <TTree.h>
#include <TH1.h>
#include <TH2.h>
#include <TF1.h>
#include <TStyle.h>
#include <TROOT.h>
#include <TColor.h>
#include <TCanvas.h>
#include <TChain.h>
#include <TPaveStats.h>
#include <TError.h>
#include <TGraph.h>
#include <TLine.h>
#include <TEllipse.h>



using namespace std;

#include "const.h"



void SaveHist(TH1* hist, TString outputFileDir, TString drawOption = "", bool setLogy = false, int histWidth = 0, int histHeight = 0)
{
    TCanvas* canvas;
    if (histWidth == 0 || histHeight == 0)
    {
        canvas = new TCanvas();
    }
    else
    {
        canvas = new TCanvas("canvas", "", histWidth, histHeight);
    }

    if (setLogy)
    {
        canvas->SetLogy();
    }
    hist->Draw(drawOption);
    canvas->SaveAs(outputFileDir);
    canvas->Clear();
}

void changeStatsBoxSize(TH1* hist, double x1, double x2, double y1, double y2)
{
    gPad->Update();
    TPaveStats* st = (TPaveStats*) hist->FindObject("stats");
    st->SetX1NDC(x1);
    st->SetX2NDC(x2);
    st->SetY1NDC(y1);
    st->SetY2NDC(y2);
    st->Draw();
}

void changeOptionStat(TH1* hist, int option)
{
    gPad->Update();
    TPaveStats* st = (TPaveStats*) hist->FindObject("stats");
    st->SetOptStat(option);
    st->Draw();
}

void changeOptionFit(TH1* hist, int option)
{
    gPad->Update();
    TPaveStats* st = (TPaveStats*) hist->FindObject("stats");
    st->SetOptFit(option);
    st->Draw();
}

void refAnalysis(string rootFileDirectory, string outputFileType = "png")
{
    // Make result directories
    const string ResultDir = rootFileDirectory + "/result/";
    mkdir(ResultDir.c_str(), 0777);


    // ROOTファイル名の取得
    DIR* dp = opendir(rootFileDirectory.c_str());
    vector<string> rootFileNames;
    if (dp != NULL)
    {
        struct dirent* dent;
        do
        {
            dent = readdir(dp);
            if (dent != NULL)
            {
                string fileName = dent->d_name;
                if (fileName.find(".root") != string::npos)
                {
                    rootFileNames.push_back(fileName);
                }
            }
        } while (dent != NULL);
        closedir(dp);
    }

    const double HistMin = -9.5;
    const double HistMax = 90.5;
    const int NBin = 100;
    TH1D* hAdcValue = new TH1D("hAdcValue", "ADC Value (ch10);ADC Value;Number of events", 500, 0, 500);
    TH1D* hist = new TH1D("hist", "Light intensity (ch10);Light intensity (p.e.);Number of events", NBin, HistMin, HistMax);



    vector <int> adcValues;
    int evt = 0;
    int maxAdcValue = 0;
    int maxEvents = 0;

    for (string rootFileName : rootFileNames)
    {
        // ファイルオープン・ツリー取得
        TString rootFileDir = TString::Format("%s/%s", rootFileDirectory.c_str(), rootFileName.c_str());
        TFile* file = TFile::Open(rootFileDir);
        TTree* tree = (TTree*) file->Get("mppc");

        // ツリーの中身を取り出せるように設定
        UShort_t adc[32];

        int NEntries = tree->GetEntries();
        cout << "Number of events: " << NEntries << endl;
        tree->SetBranchAddress("chg", adc);

        for (int i = 0; i < NEntries; i++)
        {
            tree->GetEntry(i);
            // if(maxAdcValue < adc[UsingCh])
            // {
            //     maxEvents = evt;
            //     maxAdcValue = adc[UsingCh];
            // }
            // ++evt;
            // adcValues.push_back(adc[UsingCh]);
            if(adc[UsingCh] > OverFlowCut)
                continue;

            double pe = (adc[UsingCh] - AdcPedestal) / Gain;
            // cout << "pe = " << pe << endl;
            hAdcValue->Fill(adc[UsingCh]);
            hist->Fill(pe);
        }
    }



    double maxBin = hist->GetMaximumBin() + HistMin;
    cout << maxBin << endl;
    RunMode runMode;
    if(maxBin < ThresholdRef)
    {
        runMode = RunMode::WithRef;
        // TF1* poissonInt = new TF1("poissonInt", "[0]*TMath::PoissonI(x, [1])", -0.5, 5.5);
        hist->Fit("gaus", "", "", maxBin-3, maxBin+5);
        changeStatsBoxSize(hist, 0.7, 0.98, 0.6, 0.95);
        changeOptionStat(hist, 10);
        changeOptionFit(hist, 110);
    }

    else
    {
        runMode = RunMode::WithoutRef;
        // hist->Rebin(NRebin);
        hist->Fit("gaus", "", "", maxBin-FitRangeGausLeft, maxBin+FitRangeGausRight);
        changeStatsBoxSize(hist, 0.12, 0.5, 0.5, 0.92);
        changeOptionStat(hist, 10);
        changeOptionFit(hist, 110);
    }





    TString outputFileDir = TString::Format("%s/ch10_calibrated.%s", ResultDir.c_str(), outputFileType.c_str());

    TFile histFile(TString::Format("%shist.root", ResultDir.c_str()), "RECREATE");
    SaveHist(hist, outputFileDir);
    hist->Write();
    histFile.Close();

    outputFileDir = TString::Format("%sch10_raw.%s", ResultDir.c_str(), outputFileType.c_str());
    SaveHist(hAdcValue, outputFileDir);

}

int main(int argc, char** argv)
{
    string rootFileDirectory;
    string outputFileType;

    if (argc <= 1 || argc >= 4)
    {
        cout << "refAnalysis <root file directory> <output file type>" << endl;
        return -1;
    }
    if (argc >= 2)
    {
        rootFileDirectory = argv[1];
    }
    if (argc >= 3)
    {
        outputFileType = argv[2];
    }
    else
    {
        outputFileType = "png";
    }

    refAnalysis(rootFileDirectory, outputFileType);

    return 0;
}
