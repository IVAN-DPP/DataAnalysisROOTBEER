                                                                  
#include "RootBeerUtil.h"	//gets everything for RootBeer scheme
#include "particleDEF.h"
#include "TFile.h"              
#include "TObject.h"
#include "TTree.h"
#include "TDirectory.h"
#include "TLorentzVector.h"
#include "TLorentzRotation.h"
#include "RooStats/ToyMCSampler.h"
#include "TMath.h"
#include <vector>



// ********** required global variables *********************************
//                                                                      *
// They are global to get easily filled by parseArgs()
int 	nEventsTot=0;			// ie do all events in all files
char  	InFileName[128];		// in file name parsed from args 
char  	OutFileName[128];		// out file name parsed from args

class TRootBeer *rootbeer;        	// The main class
//                                                                      *
// ********** end of required global variables **************************

// ********* required functions defined in this code ******************** 
//                                                                      *
void printUsage();			// print usage when run as exe       
int  parseArgs(int,char **);		// parse args when run as exe
//                                                                      *
// ********* end of required functions defined in this code ************* 

// ********* my functions defined in this code ************************** 
//                                                                      *
void pos2neg1(int, char *,char *); // main func. (same name as this macro)
//                                                                      *
// ********* end of my functions defined in this code ******************* 

typedef struct{
 public:
  Int_t evnt_charge;                
  Double_t evnt_mass;
  Double_t evnt_momentum;
  Double_t evnt_beta;
  Double_t evnt_cosx;
  Double_t evnt_cosy;
  Double_t evnt_cosz;
  Double_t Time;
  Double_t DTime;
} VariableList;


// ---------- FUNCTION DEFINITIONS BEGIN HERE ---------------------

// ---------- Required main() for making an executable -------------
#ifdef ROOTEXE
int main(int argc,char **argv){		
  if((parseArgs(argc,argv))!=0) return(-1);	//parse input args
  pos2neg1(nEventsTot,InFileName,OutFileName);	//call user function
  return(0);
}
#endif
// ---------- end of required main() for making an executable ------

// ---------- user main function  -----------------------------------------------
// This gets called by main() when running as an executable,
// or interactively from the root> prompt.
// should have the same name as this file (minus the .C suffix)
// customize the "non-required" sections
 
void pos2neg1(int nEvents, char *file, char *outFileName){   // main user function
  int	event=0;             	// Event counter for current file
  int	eventTot=0;	       	// Total event count
  class	TFile *outFile;         // the file to put the results
  char 	inFile[128];	       	// holds the current file name
  int 	fileNo=0;	       	// current file no if sorting though a list
  //                                                                    *
  // ********* end of variables which are required **********************


  // ******************* my variables ***********************************
  //                                                                    *
  int 	sect_no=0;			// hold the number of a bank sector
  char 	name[100];			// hold temp names for hists 
  //                                                                    *
  // ******************* end of my variables ****************************
  
  // ************************* required init stuff **********************
  //                                                                    *
#ifdef ROOTEXE					//if running as an executable
  outFile=new TFile(outFileName,"recreate");	//open output file 
#endif
  eventTot=0;			 		// init the total event counter
  //
  //                                                                    *
  // ****************** end of required init stuff **********************

  // ******* User initialisation ****************************************
  const  char *mybanks[]={"SCPB","TGPB","EVNT","TAGR","null"};	// make a list of the banks you want to use
  // ** DONT FORGET TO DEFINE THE BANKS YOU WANT RIGHT HERE
  //
  //Set up my histograms (variables already defined as globals above)
  //------------- Definición del Arbol ------------//
  VariableList *pos = new VariableList();
  VariableList *neg = new VariableList();
  VariableList *Neutron = new VariableList();
  VariableList *Sigma = new VariableList();
  VariableList *DTProton = new VariableList();
  VariableList *DTKaon = new VariableList();
  VariableList *DTPion = new VariableList();
  TTree *treepos = new TTree("EVNT_POS","EVNT_POS");
  
  treepos->Branch("EVNT_Charge",&(pos->evnt_charge));
  treepos->Branch("EVNT_Mass",&(pos->evnt_mass));
  treepos->Branch("EVNT_Pmoemntum",&(pos->evnt_momentum));
  treepos->Branch("EVNT_Beta",&(pos->evnt_beta));
  treepos->Branch("EVNT_Cx",&(pos->evnt_cosx));
  treepos->Branch("EVNT_Cy",&(pos->evnt_cosy));
  treepos->Branch("EVNT_Cz",&(pos->evnt_cosz));

  TTree *treeneg = new TTree("EVNT_NEG","EVNT_NEG");
  
  treeneg->Branch("EVNT_Charge",&(neg->evnt_charge));
  treeneg->Branch("EVNT_Mass",&(neg->evnt_mass));
  treeneg->Branch("EVNT_Pmoemntum",&(neg->evnt_momentum));
  treeneg->Branch("EVNT_Beta",&(neg->evnt_beta));
  treeneg->Branch("EVNT_Cx",&(neg->evnt_cosx));
  treeneg->Branch("EVNT_Cy",&(neg->evnt_cosy));
  treeneg->Branch("EVNT_Cz",&(neg->evnt_cosz));
  
  TTree *treepart = new TTree("Particles","Paricles");

  treepart->Branch("Neutron_Missing_Mass",&(Neutron->evnt_mass));
  treepart->Branch("Pion_Plus_Invariant_Mass",&(Sigma->evnt_beta));           //I will change this for the Sigma particle
  treepart->Branch("Delta_T_Proton",&(DTProton->DTime));
  treepart->Branch("Delta_T_Kaon",&(DTKaon->DTime));
  treepart->Branch("Delta_T_Pion_Minus",&(DTPion->DTime));
  
  Int_t q;               //Charge
  Double_t m;            //Mass
  Double_t cx,cy,cz;     //Direction cosines
  Double_t p;            //Momentum
  Double_t b;            //Beta v/c
  const Double_t c = 29.9792458; // Speed of light value (cm/ns) 
  const Double_t corr = 20.0; //
  //  Number of particles positives and negatives //

  Int_t npos=0;          //#Positives
  Int_t nneg=0;          //#Negatives
  Int_t total=0;         //#Total of particles
  std::vector<int> rowPos;  //Position of the two positive particles 
  Int_t rowNeg=-1;          

  // Pion -
  
  Double_t PmPion;
  Double_t BPionMinus;
  Double_t CxPion;
  Double_t CyPion;
  Double_t CzPion;
  Double_t EtPion;
  Double_t TimePionMinus;              //Best times with SCPB
  Double_t PathPionMinus;
  Double_t ZPionMinus;
  Double_t T1Pi, T2Pi;
  TLorentzVector *P4PionMinus = new TLorentzVector();

  // Proton and Kaon+
  
  Int_t rowProton=-1;
  Int_t rowKaon=-1;

  Double_t PmProton, PmKaonPlus;
  Double_t BProton, BKaon;
  Double_t CxProton, CxKaonPlus;
  Double_t CyProton, CyKaonPlus;
  Double_t CzProton, CzKaonPlus;
  Double_t EtProton, EtKaonPlus;
  Double_t TimeProton, TimeKaon;              //Best times with SCPB
  Double_t PathProton, PathKaon;
  Double_t ZProton, ZKaon;
  Double_t T1P, T1K;
  Double_t T2P, T2K;
  Double_t BestDTKaon;                    //Find the smallest value for abs(DT)
  TLorentzVector *P4Proton = new TLorentzVector();
  TLorentzVector *P4KaonPlus = new TLorentzVector();

  //Photon
  
  Double_t PhotonE=0;
  TLorentzVector *P4Photon = new TLorentzVector();

  //Target

  TLorentzVector *P4Target = new TLorentzVector(0,0,0,DEUTERON_MASS);

  // Neutron Missing Mass
  
  TLorentzVector *P4Neutron= new TLorentzVector();
  Double_t NeutronMass;

  // Invariant mass Sigma -  

  TLorentzVector * P4SigmaMinus = new TLorentzVector();
  
      
  //                                                                    *
  // ******* End of User initialisation  ********************************


  // ********************** main file loop **********************************************************
  //                                                                                                *
  
  while((fileNo=getNextFile(inFile,file))!=-1){  // loop while files are still avialable 

    // Start of user stuff to do before sorting each file  ---------------
    fprintf(stderr,"Sorting file - %s\n",inFile);
    // End of user stuff to do before sorting each file  ---------------


    if((rootbeer=createBeerObject(inFile))==NULL) return; 	// create rootbeer object
    rootbeer->SetBankStatus(mybanks,ON);			// switch on the banks required
    rootbeer->StartServer();                  			// start the server running
    rootbeer->ListServedBanks();			       	// list the banks which will be served

    event=0;		//zero the event counter for the file

    // ********************** main event loop ******************************************************
    //                                                                                             *
    //outFile->cd();
    
    while(event>=0){                                           	// do all events
      event = rootbeer->GetEvent();                            	// get next event

      // Start of user customized section ----------------------------
      
      
      if(TAGR_NH<=0 || EVNT_NH<3) continue;          //Al menos un foton y al menos dos particulas
      
      
      npos=0;
      nneg=0;
      rowPos.clear();
      for(int row=0;row<EVNT_NH;row++){

	if(EVNT[row].Status<=0 || EVNT[row].DCstat<=0 || EVNT[row].SCstat<=0) continue;
	
	q=EVNT[row].Charge;
	m=EVNT[row].Mass;
	p=EVNT[row].Pmom;
	b=EVNT[row].Beta;
	cx=EVNT[row].Cx;
	cy=EVNT[row].Cy;
	cz=EVNT[row].Cz;

	
	if(EVNT[row].Mass>=5 || EVNT[row].Mass<=-5) continue;
        
	if(q==0) continue;            //Particulas con cargas negativas y positivas
	
	else if(q>0){
	  pos->evnt_charge = q;
	  pos->evnt_mass = m;
	  pos->evnt_momentum = p;
	  pos->evnt_beta = b;
	  pos->evnt_cosx = cx;
	  pos->evnt_cosy = cy;
	  pos->evnt_cosz = cz;
	  npos++;
	  treepos->Fill();
	  rowPos.push_back(row);
	}
	else if(q<0){
	  neg->evnt_charge = q;
	  neg->evnt_mass = m;
	  neg->evnt_momentum = p;
	  neg->evnt_beta = b;
	  neg->evnt_cosx = cx;
	  neg->evnt_cosy = cy;
	  neg->evnt_cosz = cz;
	  nneg++;
	  treeneg->Fill();
	  rowNeg=row;
	}
	

	total++;

		
      }  // -------- FINAL EVNT ----------- //

      
      if(npos!=(2) || nneg!=(1)) continue;

      
      //Pion -

      PmPion=EVNT[rowNeg].Pmom;
      CxPion=EVNT[rowNeg].Cx;
      CyPion=EVNT[rowNeg].Cy;
      CzPion=EVNT[rowNeg].Cz;

      EtPion=std::sqrt(PmPion*PmPion+PI_CHARGED_MASS*PI_CHARGED_MASS);

      P4PionMinus->SetPxPyPzE(PmPion*CxPion,PmPion*CyPion,PmPion*CzPion,EtPion);    
      BPionMinus=P4PionMinus->Beta();
           
      // Proton and Kaon

      if(EVNT[rowPos.at(0)].Mass > EVNT[rowPos.at(1)].Mass){
	rowProton=rowPos.at(0);
	rowKaon=rowPos.at(1);
      }

      else if(EVNT[rowPos.at(0)].Mass < EVNT[rowPos.at(1)].Mass){
	rowProton=rowPos.at(1);
	rowKaon=rowPos.at(0);
      }

      else {
	rowProton=rowPos.at(1);
	rowKaon=rowPos.at(0);
      }

      //if(EVNT[rowProton].Mass<=0.8 || EVNT[rowProton].Mass>=1.1) continue;
      //if(EVNT[rowKaon].Mass<=0.3 || EVNT[rowKaon].Mass>=0.7) continue;
      PmProton=EVNT[rowProton].Pmom;
      CxProton=EVNT[rowProton].Cx;
      CyProton=EVNT[rowProton].Cy;
      CzProton=EVNT[rowProton].Cz;

      EtProton=std::sqrt(PmProton*PmProton+PROTON_MASS*PROTON_MASS);

      P4Proton->SetPxPyPzE(PmProton*CxProton,PmProton*CyProton,PmProton*CzProton,EtProton);
      BProton=P4Proton->Beta();
      // Here must stay Kaon, but we changed the Kaon Mass to Pion+ Mass!  
      
      PmKaonPlus=EVNT[rowKaon].Pmom;
      CxKaonPlus=EVNT[rowKaon].Cx;
      CyKaonPlus=EVNT[rowKaon].Cy;
      CzKaonPlus=EVNT[rowKaon].Cz;

      EtKaonPlus=std::sqrt(PmKaonPlus*PmKaonPlus+PI_CHARGED_MASS*PI_CHARGED_MASS);
      
      P4KaonPlus->SetPxPyPzE(PmKaonPlus*CxKaonPlus,PmKaonPlus*CyKaonPlus,PmKaonPlus*CzKaonPlus,EtKaonPlus);
      BKaon = P4KaonPlus->Beta();
      //Best photon with SCPB, EVNT and TAGR

      //Pion with SCPB
      TimePionMinus=SCPB[EVNT[rowNeg].SCstat-1].Time;
      PathPionMinus=SCPB[EVNT[rowNeg].SCstat-1].Path;
      ZPionMinus=EVNT[rowNeg].Z;
      T1Pi = TimePionMinus - double(PathPionMinus)/double(BPionMinus*c);
      //Proton with SCPB
      TimeProton=SCPB[EVNT[rowProton].SCstat-1].Time;
      PathProton=SCPB[EVNT[rowProton].SCstat-1].Path;
      ZProton=EVNT[rowProton].Z;
      T1P = TimeProton - double(PathProton)/double(BProton*c);
      
      //Kaon with SCPB
      TimeKaon=SCPB[EVNT[rowKaon].SCstat-1].Time;
      PathKaon=SCPB[EVNT[rowKaon].SCstat-1].Path;
      ZKaon=EVNT[rowKaon].Z;
      T1K = TimeKaon - double(PathKaon)/double(BKaon*c);

      //DT=T1-T2 TAGR Bank
      BestDTKaon=1.0;                                            //This value begin with 1.0 ns because is the maximum
      for(int row=0;row<TAGR_NH;row++){ 		         //value that abs(DTKaon->DTime) can take
	if(TAGR[row].STAT!=15 && TAGR[row].STAT!=7) continue;
	//Pion with TAGR
	T2Pi=TAGR[row].TPHO-double(corr+ZPionMinus)/double(c);
	DTPion->DTime = T2Pi-T1Pi;                           //Fill treepar
	//Proton with TAGR
	T2P=TAGR[row].TPHO-double(corr+ZProton)/double(c);
	DTProton->DTime = T2P-T1P;                           //Fill treepar
	//Kaon with TAGR
	T2K=TAGR[row].TPHO-double(corr+ZKaon)/double(c);
	DTKaon->DTime = T2K-T1K;                             //Fill treepar
	
	if(BestDTKaon > TMath::Abs(DTKaon->DTime)){                                  //Find the smallest value of DTKaon->DTime
	  BestDTKaon = TMath::Abs(DTKaon->DTime);                                    //The Best value would be DTKaon->DTime Approx 0
	  PhotonE = TAGR[row].ERG;
	}
      }

      /*
      //Photon TGPB BANK
      for(int row=0; row<TGPB_NH; row++){
	
	if((TGPB[row].pointer) < 0){
	  PhotonE = TGPB[row].Energy;
	}
      } // -------- FINAL TGPB ----------- //
      */
      P4Photon->SetPxPyPzE(0.0, 0.0, PhotonE, PhotonE);


      //Missing mass neutron
      
      
      *P4Neutron=(*P4Photon+*P4Target)-(*P4Proton+*P4KaonPlus+*P4PionMinus);
      
      NeutronMass=P4Neutron->M();
      Neutron->evnt_mass = NeutronMass;
      
      // Invariant mass Sigma -

      if(NeutronMass>1.05) continue;
      *P4SigmaMinus=*P4Neutron+*P4PionMinus;
      Sigma->evnt_beta = P4SigmaMinus->M(); 
      treepart->Fill();
     
      
      // End of user customized section ----------------------------

      eventTot++;
      if(eventTot%1000 == 0) fprintf(stdout,"done %d\n",eventTot);
      if((nEvents>0)&&(eventTot >=nEvents)) break;		//break if nEvents done
    }
    //std::cout << nneg << "\t" << npos << "\t" << npos+nneg<< "\t" <<total <<"\n";
    //                                                                                             *
    // ********************** end of main event loop ***********************************************

    // Start of user stuff to do after sorting each file  ---------------
    fprintf(stdout,"sorted %d events from file: %s\n",abs(event),inFile);
    // End of user stuff to do after sorting each file  ---------------

    delete rootbeer;                              		//Call the class destructor
    if((nEvents>0)&&(eventTot >=nEvents)) break;		//break if nEvents done
  }
  
  //                                                                                                *
  // ********************** end of main file loop ***************************************************



  // ******** Do any final stuff here ***********************************
  //                                                                    *
  
  // Start of user stuff to do at the end  ------------------------
  fprintf(stdout,"Sorted Total of  %d events\n",eventTot);
  // End of user stuff to do at the end  --------------------------

#ifdef ROOTEXE
  //if it's an executable, write to the output file
  
  outFile->Write();
  outFile->Close();
#endif
  //                                                                    *
  // ********************************************************************
}
// ---------- end of user main function  -----------------------------------------------



// ---------- required parseArgs() function  -------------------------------------------
// parses the input args when running as an executable
// set up the no of events, and input and output file names in global variables
// or prints usage
int parseArgs(int argc,char **argv){
  switch(argc){
  case(1):				// 1 or 2 args not enough
  case(2):
    printUsage();
    return(-1);
    break;
  case(3):				// 3 args pos2neg1 <infile> <outfile>
    if((strstr(argv[1],"-N"))!=NULL){
      printUsage();
      return(-1);
    }
    else{
      strcpy(InFileName,argv[1]);
      strcpy(OutFileName,argv[2]);
    }
    break;
  case(4):				// 4 args must be "pos2neg1 -N# <infile> <outfile>" 
    if(strncmp(argv[1],"-N",2)!=0){  	//print usage then exit */
      printUsage();
      return(-1);
    }
    else{
      sscanf(argv[1]+2,"%d",&nEventsTot);	//get the required event no
      strcpy(InFileName,argv[2]);
      strcpy(OutFileName,argv[3]);
    }      
    break;
  default:
    printUsage();
    return(-1);
    break;
  }
  return(0);
}
// ---------- end of required parseArgs() function  -------------------------------------

// ---------- required printUsage() function  -------------------------------------------
// When running as executable, gets called if no args, or silly args
void printUsage(){
  fprintf(stderr,"\nusage:\n\n");
  fprintf(stderr,"pos2neg1 -h   Print this message\n\n");
  fprintf(stderr,"pos2neg1 [-Nevents] <infile> <outfile.root> \n");
  fprintf(stderr,"pos2neg1 [-Nevents] <-Lfilelist> <outfile.root> \n");
}
// ---------- end of required printUsage() function  -------------------------------------------
