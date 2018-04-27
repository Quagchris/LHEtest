// c++ -o prova01 prova01.cpp `root-config --cflags --glibs`

#include "LHEF.h"
#include "TH1.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TString.h"
#include <sstream>
#include <algorithm>
#include "TLorentzVector.h"

using namespace std;

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        cout << "Usage: " << argv[0] << " output.root file1.lhe file2.lhe .." << endl;
        return 1;
    }

    char* rootfile = argv[1];     
    TFile output( rootfile, "RECREATE");
    TTree* tree = new TTree("tree", "LHE events");
    
    double lep_eta, lep_pt;
    double n_eta, n_pt;
    double vbs1_eta, vbs1_pt, vbs2_eta, vbs2_pt;
    double vjet1_eta, vjet1_pt, vjet2_eta, vjet2_pt;
    double deltaeta_vbs, mjj_vbs, mjj_vjet, abs_deta;
    double mww;
    double zepp_l, zepp_q1, zepp_q2;
    
    TH1F *q1 = new TH1F("q1", "PGID of jet1", 80, -10, 10);
    TH1F *q2 = new TH1F("q2", "PGID of jet2", 80, -10, 10);    
    TH1F* l1 = new TH1F("l1", "PDGID of lepton", 160, -20, 20);
    
    tree->Branch("deltaeta_vbs", &deltaeta_vbs);
    tree->Branch("abs_deta", &abs_deta);
    tree->Branch("mjj_vbs", &mjj_vbs);
    tree->Branch("zepp_l", &zepp_l);
    tree->Branch("zepp_q1", &zepp_q1);
    tree->Branch("zepp_q2", &zepp_q2);
    
    tree->Branch("lep_eta", &lep_eta);
    tree->Branch("lep_pt", &lep_pt);
    tree->Branch("n_eta", &n_eta);
    tree->Branch("n_pt", &n_pt);
    tree->Branch("vbs1_eta", &vbs1_eta);
    tree->Branch("vbs1_pt", &vbs1_pt);
    tree->Branch("vbs2_eta", &vbs2_eta);
    tree->Branch("vbs2_pt", &vbs2_pt);
    tree->Branch("vjet1_eta", &vjet1_eta);
    tree->Branch("vjet1_pt", &vjet1_pt);
    tree->Branch("vjet2_eta", &vjet2_eta);
    tree->Branch("vjet2_pt", &vjet2_pt);
    tree->Branch("mjj_vjet", &mjj_vjet);
    tree->Branch("mww", &mww);
    
    long iEv = 0;
    for (int i = 2; i < argc; i++)
    {
        //Open a stream connected to an event file:
        ifstream ifs(argv[i]);
        
        //Create the Reader object:
        LHEF::Reader reader(ifs);
        
        //Now loop over all the events     
        while ( reader.readEvent() ) 
        {
            iEv++;
            cout << "Event " << iEv << endl;
            

            vector<int> charged_leptons; 
            vector<int> neutrinos;
            vector<int> leptons;
            vector<int> quarks; 
            vector<int> gluons;
            vector<int> jets;
            
            //TlorentVector of vbs and vjet quarks
            TLorentzVector jet_vbs1, jet_vbs2, jet_w1, jet_w2;
            //TlorentVector of lepton and neutrino
            TLorentzVector lep_mom, nu_mom; 
            
            //Save quadrimomentum of all particles mapped with position in the event
            map<int, TLorentzVector> momenta;
            
            //Save eta of leptons and quarks in final state for Zeppenfeld var
            
            //PG loop over particles in the event
            for (int iPart = 0 ; iPart < reader.hepeup.IDUP.size (); iPart++)
            {
//                 cout    << "\t part type [" << iPart << "] " << reader.hepeup.IDUP.at (iPart)
//                         << "\t status " << reader.hepeup.ISTUP.at (iPart)
//                         << endl;
                        
                //Saving info of final state particles only
                if (reader.hepeup.ISTUP.at (iPart) ==1) 
                {
                    int ID = reader.hepeup.IDUP.at(iPart);
                    TLorentzVector momentum  
                    (
                        //PUP --> quadrimomentum
                        reader.hepeup.PUP.at (iPart).at (0), //PG px
                        reader.hepeup.PUP.at (iPart).at (1), //PG py
                        reader.hepeup.PUP.at (iPart).at (2), //PG pz
                        reader.hepeup.PUP.at (iPart).at (3) //PG E
                    );
                    
                    // Save momentum
                    momenta[iPart] =  momentum;   
                    if( abs(ID) == 11 || abs(ID) == 13 )
                    {  // eletrons, positrons, muon, antimuon
                        charged_leptons.push_back(iPart);
                        leptons.push_back(iPart);
                        lep_mom = momentum;
                        lep_eta = momentum.Eta();
                        lep_pt = momentum.Pt();
                        l1->Fill(ID);
                    }
                    
                    if ( abs(ID) == 12 || abs(ID) == 14 )
                    {  // neutrinos
                        neutrinos.push_back(iPart);
                        leptons.push_back(iPart);
                        nu_mom = momentum;
                        n_eta = momentum.Eta();
                        n_pt = momentum.Pt();
                    }
                    
                    if ( abs(ID) < 7 )
                    {  // quark in final state 
                        quarks.push_back(iPart);
                        jets.push_back(iPart);
                    }
                    
                    if ( abs(ID) == 21 )
                    {  //gluons in final state
                        gluons.push_back(iPart);
                        jets.push_back(iPart);
                    }   
                            
                }// info of final state particles
            }//end of loop over particles
            
            
            // The vbs jet are selected looking for the couple with higher mass.
            // First of all select two of the jets from the list. 
            int v1, v2; // jets vector indexes for selected vbs particles
            double mjj, mjj_max = 0; 
            TLorentzVector p1, p2;
            
            for (int i=0; i<jets.size(); i++)
            {
                p1 = momenta[jets[i]];
                for (int j = i+1; j<jets.size(); j++)
                {
                    p2 = momenta[jets[j]];
                    mjj = (p1 + p2).M();
                    if(mjj > mjj_max)
                    {
                       jet_vbs1 = p1;
                       jet_vbs2 = p2;
                       mjj_max = mjj;
                       v1 = jets[i];
                       v2 = jets[j];
                    }
                }
            }
            // now we read the jets vector to select the v-jets  
            jets.erase(remove(jets.begin(), jets.end(), v1), jets.end());
            jets.erase(remove(jets.begin(), jets.end(), v2), jets.end());
            jet_w1 = momenta[jets[0]];
            jet_w2 = momenta[jets[1]];
            
            //selection requirements for each event
            if ((jet_w1 + jet_w2).M() > 75 && (jet_w1 + jet_w2).M() < 85 && (jet_vbs1 + jet_vbs2).M() > 130)
            {
                q1->Fill(reader.hepeup.IDUP.at(jets[0]));
                q2->Fill(reader.hepeup.IDUP.at(jets[1]));

                vbs1_pt = jet_vbs1.Pt();
                vbs1_eta = jet_vbs1.Eta();
                vbs2_pt = jet_vbs2.Pt();
                vbs2_eta = jet_vbs2.Eta();
                vjet1_pt = jet_w1.Pt();
                vjet1_eta = jet_w1.Eta();
                vjet2_pt = jet_w2.Pt();
                vjet2_eta = jet_w2.Eta();
                deltaeta_vbs = vbs1_eta - vbs2_eta;
                mjj_vbs = (jet_vbs1 + jet_vbs2).M();
                mjj_vjet = (jet_w1 + jet_w2).M();

                //Zeppenfeld var
                double deta = abs(deltaeta_vbs);
                double meta = .5*(vbs1_eta + vbs2_eta);
                zepp_l = (lep_eta - meta)/deta;
                abs_deta = deta;
                zepp_q1 = (vjet1_eta - meta)/deta;
                zepp_q2 = (vjet2_eta - meta)/deta;

                // Mass of the two W
                mww = (jet_w1 + jet_w2 + lep_mom + nu_mom).M();

    //             cout << "Mjj vbs jet: " << mjj_vbs << " | " 
    //                 << "DeltaEta vbs jet: " << deltaeta_vbs << endl
    //                 << "------------------------------------------------" << endl;

                tree->Fill();
             }
        }//end of loop over events
        
        ifs.close();
    }//end of loop over files
    
    l1->Write();
    q1->Write();
    q2->Write();
    tree->Write();
    output.Close();
    
    return 0;
}
