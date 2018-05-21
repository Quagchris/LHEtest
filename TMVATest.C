#include <cstdlib>
#include <iostream>
#include <string>
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Tools.h"
#include "TMVA/TMVAGui.h"
#include <TMVA/Config.h>
#include <fstream>
#include <sstream>

using namespace std;

int TMVATest()
{
    
    TMVA::Tools::Instance();
    
    
    //SigTree
    TString sname = "./treeS_cut.root";
    TFile* Sig_TFile = TFile::Open( sname );
    
    //BkgTree
    TString bname = "./treeB_cut.root";
    TFile* Bkg_TFile = TFile::Open( bname );
    
    TTree* Sig_Tree = (TTree*)Sig_TFile->Get("treeS_cut");
    TTree* Bkg_Tree = (TTree*)Bkg_TFile->Get("treeB_cut");
    
    //output file and .txt config file
    stringstream ss;
    ss << "TMVA.root";
    TString outfileName(ss.str());
    TFile* outputFile = TFile::Open( outfileName , "RECREATE" );  
    
    ss << "_config.txt";
    
    ofstream out(ss.str());
    
    TMVA::Factory *TMVAtest = new TMVA::Factory( "TMVAClassificationTest", outputFile,
                                               "!V:!Silent:Color:DrawProgressBar:Transformations=I:AnalysisType=Classification" );
    TMVA::DataLoader *dataloader=new TMVA::DataLoader("datasetTest");

    //Add trainingVariables
    dataloader->AddVariable("mjj_vbs", 'F');
    dataloader->AddVariable("abs_deta", 'F');
    dataloader->AddVariable("zepp_l", 'F');
    dataloader->AddVariable("zepp_q1", 'F');
    dataloader->AddVariable("zepp_q2", 'F');
    
    //Add Sig and Bkg trees
    dataloader->AddSignalTree(Sig_Tree, 1.);
    dataloader->AddBackgroundTree(Bkg_Tree, 1.);

     
    //Add SpectatorVariables
//     dataloader->AddSpectator ("prod_eta", 'F');
   
    TCut mycuts = "";
    TCut mycutb = "";
    
    dataloader->PrepareTrainingAndTestTree( mycuts, mycutb,
                                        "nTrain_Signal=50000:nTrain_Background=50000:SplitMode=Random:NormMode=NumEvents:!V" );
    
    // adding a BDT
    // ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
    int    NTrees         = 10000 ;
    bool   optimizeMethod = false ; 
    string BoostType      = "AdaBoost" ; 
    float  AdaBoostBeta   = 0.5 ; 
    string PruneMethod    = "NoPruning" ; 
    int    PruneStrength  = 1 ; 
    int    MaxDepth       = 6 ; 
    string SeparationType = "GiniIndex" ;
    
    out << "--------------------BDT--------------------" << endl;
    out << "NTrees:         " << NTrees << endl; 
    out << "optimizeMethod: " << "false" << endl;
    out << "BoostType:      " << BoostType << endl;
    out << "AdaBoostBeta:   " << AdaBoostBeta << endl;
    out << "PruneMethod:    " << PruneMethod << endl;
    out << "PruneStrength:  " << PruneStrength << endl;
    out << "MaxDepth:       " << MaxDepth << endl;
    out << "SeparationType: " << SeparationType << endl;
    out << "\n";
    
    //Booking options for the BDT
    TString Option = Form ("!H:!V:CreateMVAPdfs:NTrees=%d:BoostType=%s:AdaBoostBeta=%f:PruneMethod=%s:PruneStrength=%d:MaxDepth=%d:SeparationType=%s:Shrinkage=0.1:UseYesNoLeaf=F:MinNodeSize=2:nCuts=400", 
        NTrees, BoostType.c_str (), AdaBoostBeta, PruneMethod.c_str (), PruneStrength, MaxDepth, SeparationType.c_str ()) ;
            
    TMVAtest->BookMethod(dataloader, TMVA::Types::kBDT, "BDT", Option.Data()) ;
    
    // adding a Rectangular Cut
    // ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----

    out << "--------------------CUT--------------------" << endl;
    out << "FitMethod:  " << "GA" << endl;
    out << "VarProp:    " << "FSmart" << endl;
     TMVAtest->BookMethod( dataloader, TMVA::Types::kCuts, "Cuts",
                            "!H:!V:CreateMVAPdfs:FitMethod=GA:EffSel:VarProp=FSmart" );

    // start the training
    // ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
    
    TMVAtest->TrainAllMethods();
    TMVAtest->TestAllMethods();
    TMVAtest->EvaluateAllMethods();
    
    outputFile->Close();
    
    delete TMVAtest;
    delete dataloader;

    return 0 ;
}

int main()
{
    return TMVATest();
}
