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
    if (argc != 2)
    {
        cout << "Usage: " << argv[0] << " file.lhe" << endl;
        return 1;
    }
    //Open a stream connected to an event file:
    ifstream ifs(argv[1]);
    
    //Create the Reader object:
    LHEF::Reader reader(ifs);

    //Print out the header information:
    cout << reader.headerBlock;

    //Print out the addinional comments in the init block:
    cout << reader.initComments;

    //Print out the beam energies:
    cout << "Beam A: " << reader.heprup.EBMUP.first << " GeV, Beam B: "
         << reader.heprup.EBMUP.second << " GeV." << endl;
              
    TFile output("LHETree.root", "RECREATE");
    TTree* tree = new TTree("tree", "LHE events");
    
    double lep_eta, lep_pt;
    double n_eta, n_pt;
    double vbs1_eta, vbs1_pt, vbs2_eta, vbs2_pt;
    double vjet1_eta, vjet1_pt, vjet2_eta, vjet2_pt;
    double deltaeta_vbs, mjj_vbs, mjj_vjet, vjets_tot_pt;
    double mww, mww_t; 
    
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
    tree->Branch("deltaeta_vbs", &deltaeta_vbs);
    tree->Branch("mjj_vbs", &mjj_vbs);
    tree->Branch("mjj_vjet", &mjj_vjet);
    tree->Branch("vjets_tot_pt", &vjets_tot_pt);
    tree->Branch("mww", &mww);
    tree->Branch("mww_t", &mww_t);
    
    //Now loop over all the events     
    long iEv = 0;
    
    while ( reader.readEvent() ) 
    {
        iEv++;
        cout << "event " << iEv << endl;
        

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
        
        //PG loop over particles in the event
        for (int iPart = 0 ; iPart < reader.hepeup.IDUP.size (); iPart++)
        {
            cout    << "\t part type [" << iPart << "] " << reader.hepeup.IDUP.at (iPart)
                    << "\t status " << reader.hepeup.ISTUP.at (iPart)
                    << endl;
                    
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
                    if(p1.Eta() > p2.Eta())
                    {
                        jet_vbs1 = p1;
                        jet_vbs2 = p2;
                    }
                    else
                    {
                        jet_vbs1 = p2;
                        jet_vbs2 = p1;
                    }
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
        vjets_tot_pt = (jet_w1 + jet_w2).Pt();
        
        // Mass of the two W
        mww = (jet_w1 + jet_w2 + lep_mom + nu_mom).M();
        // Mass of the two W with only transverse neutrino
        TLorentzVector trans_nu_mom = nu_mom;
        trans_nu_mom.SetPz(0);
        mww_t = (jet_w1 + jet_w2 + lep_mom + trans_nu_mom).M();

        cout << "Mjj vbs jet: " << mjj_vbs << " | ";
        cout << "DeltaEta vbs jet: " << deltaeta_vbs << " --> ";
        
        tree->Fill();
    }//end of loop over events
    
    ifs.close();
    tree->Write();
    output.Close();

    return 0;
}
