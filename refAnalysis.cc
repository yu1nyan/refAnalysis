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




tuple<int, int> ConvertCellPosition(int cellX, int cellY)
{
    return forward_as_tuple(cellX + 1, cellY + 1);
}

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

void SaveHodoMap(TH2* hist, TString outputFileDir, int nCellOneSide)
{
    TCanvas* canvas = new TCanvas("canvas", "", 1280, 1200);
    hist->Draw("text colz");
    hist->GetXaxis()->SetNdivisions(nCellOneSide);
    hist->GetYaxis()->SetNdivisions(nCellOneSide);
    gPad->SetRightMargin(0.17);
    hist->GetZaxis()->SetTitleOffset(1.4);
    hist->SetStats(kFALSE);
    canvas->SaveAs(outputFileDir);
    canvas->Clear();
}

void SaveGraph(TGraph* graph, TString outputFileDir)
{
    TCanvas* canvas = new TCanvas();
    graph->Draw("AP");
    canvas->SaveAs(outputFileDir);
    canvas->Clear();
}

void refAnalysis(string rootFileDirectory, string outputFileType = "png")
{
    // Make result directories
    // const string EachCellDir = rootFileDirectory + "/crosstalkEachCell/";
    // mkdir(EachCellDir.c_str(), 0777);


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

    // int allevt = 0; // scatter plot 用
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
        TH1D* hist = new TH1D("hist", "hist", 100, 0, 4000);
        for (int i = 0; i < NEntries; i++)
        {
            tree->GetEntry(i);
            // cout << adc[10] << endl;
            if(adc[10] >= 4085)
            {
                continue;
            }
            hist->Fill(adc[10]);
        }
        hist->Draw("E");

        TString outputFileDir = TString::Format("%s/ch10.%s", rootFileDirectory.c_str(), outputFileType.c_str());
        SaveHist(hist, outputFileDir);
    }


}

int main(int argc, char** argv)
{
    string rootFileDirectory;
    string outputFileType;

    if (argc <= 1 || argc >= 4)
    {
        cout << "refAnalysis <root file directory> <output file type>" << endl;
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
