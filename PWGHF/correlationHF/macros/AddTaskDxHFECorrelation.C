//-*- Mode: C++ -*-
// $Id$

#ifndef __CINT__
//#include "AliESDtrackCuts.h"
//#include "AliAnalysisCuts.h"
//#include "AliFlowTrackSimple.h"      // added as hint for hidden library dependency to libPWGflowBase
//#include "AliFlowCandidateTrack.h"   // added as hint for hidden library dependency to libPWGflowTasks
//#include "AliCFContainer.h"          // added as hint for hidden library dependency to libCORRFW
//#include "AliAODRecoDecayHF2Prong.h" // added as hint for hidden library dependency to libPWGHFvertexingHF

#include "AliAnalysisTaskDxHFECorrelation.h"
#include "AliDxHFECorrelation.h"
#incldue "AliReducedParticle.h"
#include "AliHFCorrelator.h"
#include "AliHFAssociatedTrackCuts.h"
#include "AliAnalysisManager.h"
#include "AliAnalysisDataContainer.h"
#include "AliHFEcuts.h"
#include "AliLog.h"
#include "TObject.h"
#include "TClass.h"
#include "TDirectory.h"
#include "TROOT.h"
#include "AliRDHFCutsD0toKpi.h"
#include "AliHFEextraCuts.h"
#include "AliVEvent.h"
using namespace std;
#endif

const char* poolInfoName="PoolInfo";
AliAnalysisCuts* createDefaultPoolConfig(Bool_t usekine);
AliAnalysisCuts* createpPbPoolConfig(Bool_t usekine);
AliAnalysisCuts* createPbPbPoolConfig();

/// @file   AddTaskDxHFECorrelation.C
/// @author Matthias.Richter@ift.uib.no
/// @date   2012-05-09
/// @brief  Add the D0-HFE correlation task to the manager
///
int AddTaskDxHFECorrelation(TString configuration="", TString analysisName="PWGHFcorrelationDxHF")
{
  //First check to see if user wants to see help
  if (configuration.BeginsWith("help") || 
      configuration.BeginsWith("--help") || 
      configuration.BeginsWith("-h") || 
      configuration.BeginsWith("options") ) {
    cout <<"\n\n============================================" << endl;
    cout << "Keywords for AddTaskDxHFECorrelation.C:\n"
	 << "file=                         - Filename to store output in\n"
	 << "name=                         - Name of analysis, will correspond to directory inside the file \n"
	 << "cutname=                      - Filename where information on event pool for event-mixing is stored (if use external file)\n"
	 << "extraname=                    - extraname for directory and list if you run several tasks at once\n"
	 << "runD0MassReference            - If you also want to include D2H task for D0selection (for comparison purposes)\n"
	 << "mc                            - Run on MC\n"
	 << "usekine                       - To run on kinematical level \n"
	 << "triggermask=                  - Which triggers to use\n"
	 << "event-mixing/mixing           - Whether to also run event-mixing (NB! Use AddTaskDxHFECorrelationME.C for eventmixing)\n"
	 << "useTrackEff                   - If you want to use tracking efficiency (need to attach efficiency maps\n"
	 << "TrackEffName=                 - The file where the efficiency map is stored\n";
    cout << "useD0Eff                      - If you want to use tracking efficiency (need to attach efficiency maps\n"
	 << "PromptD0EffName=              - The file where the efficiency map for Prompt D0 is stored\n"
	 << "FeedDownD0EffName=            - The file where the efficiency map for Feeddown D0 is stored\n"
	 << "trigger=D/D0/electron         - Which particle to trigger on \n"
	 << "PbPb/Pb-Pb/system=1           - To use Pb-Pb collision system\n"
	 << "pPb/p-Pb/system=2/system=p-Pb - To use p-Pb collision system\n";
      cout << "\nD0 SETTINGS: \n"
	 << "fillD0scheme=both/D0/D0bar    - Which fillsheme to use for D0\n"
	 << "\nELECTRON SETTINGS: \n"
	 << "useinvmasscut                 - If you want to use invariant mass cut (default is 100MeV/c)\n" 
	 << "twoselectedinvmasscut         - If you want to use invariant mass selection with stricter cut on partner\n"
	 << "invmasscut=                   - If you want to specify a different invariant mass cut \n"
         << "ElSelection=                  - To only store a certain source of electron candidates (hadron/nonHFE/HFE/Onlyc/Onlyb\n"  
         << "storelastcutstep              - Store the last cutstep (so store all track with final cut info)\n"  
         << "filterbit=                    - which filterbit to use (default is 0)\n"  
         << "notusefilterbit               - not use filterbits at all\n";
    cout << "maxPtCombinedPID=             - Max Pt to use TOF PID, above only use TPC PID\n"
         << "EMCALPID                      - Use also Emcal for PID, together with TPC\n"  
	 << "tpcclusters=                  - How many TPC clusters to use on single track cuts for electrons (default=120)\n"
	 << "itsclusters=                  - How many itsclusters to be used in single track cuts for electrons (default=4) \n"
	 << "itsreq=                       - (kFirst,kAny,kNone) Which ITSpixel requirement you want to impose\n"
         << "impactparamcut=               - To set impact parameter cut in radial direction (used to select out all tracks)\n"  
         << "etacut                        - To set eta cut (used to select out all tracks) \n"  
	 << "elmcreco=                     - (aftertrackcuts/aftertofpid/afterfullpid) Where you want to stop in track selection to look for electrons for mc \n"
	 << "elreco=                       - Where you want to stop in track selection to look for electrons for (see code for different selections  \n\n";
    return;
  }

  TString ofilename;
  Int_t system=0;
  Bool_t bUseMC=kFALSE;
  Bool_t bEventMixing=kFALSE;
  Bool_t bRunD0MassReference=kFALSE;
  TString poolConfigFile="";
  // TODO: revise the logic for task options in order to forward every option
  // by default and filter out the options meant for the macro, that allows
  // to introduce new task options without the need to specifically forward them
  TString taskOptions;
  Int_t NrTPCclusters=120; // quick fix for problem sending hfe track cut object to addtask
  Int_t NrITSclusters=4;
  Int_t recoPass=2;
  Double_t RatioTPCncls=0.8;
  Double_t nSigTPCLow=-1.0;
  Double_t nSigTPCHigh=3.0;
  Int_t ITSreq=AliHFEextraCuts::kFirst;
  ULong64_t triggerMask=AliVEvent::kAnyINT;
  Int_t triggerParticle=AliDxHFECorrelation::kD;
  Bool_t bUseMCReco=kFALSE;
  Bool_t bUseKine=kFALSE;
  Bool_t bUseTrackEff=kFALSE;
  Bool_t bUseEMCAL=kFALSE;
  Bool_t bUseD0Eff=kFALSE;
  TString TrackEffMap="";
  TString D0EffMapPrompt="";
  TString D0EffMapFeedDown="";
  TString extraname="";
  TString cutFilenameD0="";
  TString cutFilenameEl="";
  Bool_t addPIDqa=kFALSE;
  Bool_t bTuneOnData=kFALSE;
  Bool_t cutOnClusters=kTRUE;
  Bool_t bUpgradeStudies=kFALSE;

  cout << endl << "===============================================" << endl;
  cout << "Setting up Correlation task: " << configuration << endl;

  // look for configuration arguments if nothing specified
  // in the function call
  if (configuration.IsNull() && gDirectory) {
    const char* confObjectName="run_single_task_configuration";
    TObject* confObject=gDirectory->FindObject(confObjectName);
    if (confObject) {
      configuration=confObject->GetTitle();
    }
  }
  {// deprecated, but keep for formatting
    {// deprecated, but keep for formatting
      TObjArray* tokens=configuration.Tokenize(" ");
      if (tokens) {
	TIter next(tokens);
	TObject* token;
	while ((token=next())) {
	  TString argument=token->GetName();
	  if (argument.BeginsWith("file=")) {
	    argument.ReplaceAll("file=", "");
	    ofilename=argument;
	    continue;
	  }
	  if (argument.BeginsWith("name=")) {
	    argument.ReplaceAll("name=", "");
	    analysisName=argument;
	    continue;
	  }
	  if (argument.BeginsWith("EMCALPID")) {
	    bUseEMCAL=kTRUE;
	    taskOptions+=" EMCALPID";
	    continue;
	  }
	  if (argument.BeginsWith("TrackEffName=")) {
	    argument.ReplaceAll("TrackEffName=", "");
	    bUseTrackEff=kTRUE;
	    TrackEffMap=argument;
	    taskOptions+=" useTrackEff";
	    continue;
	  }
	  if (argument.BeginsWith("PromptD0EffName=")) {
	    argument.ReplaceAll("PromptD0EffName=", "");
	    bUseD0Eff=kTRUE;
	    D0EffMapPrompt=argument;
	    taskOptions+=" useD0Eff";
	    continue;
	  }
	  if (argument.BeginsWith("FeedDownD0EffName=")) {
	    argument.ReplaceAll("FeedDownD0EffName=", "");
	    bUseD0Eff=kTRUE;
	    D0EffMapFeedDown=argument;
	    continue;
	  }
	  if(argument.BeginsWith("useTrackEff")) {
	    bUseTrackEff=kTRUE;
	    taskOptions+=" useTrackEff";
	    continue;
	  }
	  if(argument.BeginsWith("useD0Eff")) {
	    bUseD0Eff=kTRUE;
	    taskOptions+=" useD0Eff";
	    continue;
	  }
	  if (argument.BeginsWith("cutname=")) {
	    argument.ReplaceAll("cutname=", "");
	    poolConfigFile=argument;
	    continue;
	  }
	  if (argument.BeginsWith("cutFilenameD0=")) { 
	    argument.ReplaceAll("cutFilenameD0=", ""); 
	    cutFilenameD0=argument;                    
 	    continue;
 	  } 
	  //electron cutfile not yet enabled
	  /*
	    if (argument.BeginsWith("cutFilenameEl=")) { 
	    argument.ReplaceAll("cutFilenameEl=", ""); 
	    cutFilenameEl=argument;                    
	    continue;
	    } 
	  */  
	  if (argument.CompareTo("mc")==0){     
	    //	  if (argument.Compare("mc")) {
	    bUseMC=kTRUE;
	    taskOptions+=" "+argument;
	    continue;
	  }
	  if(argument.BeginsWith("tpcclusters=")){
	    argument.ReplaceAll("tpcclusters=", "");
	    NrTPCclusters=argument.Atoi();
	    ::Info("AddTaskDxHFEParticleSelection",Form("Setting nr TPC clusters to %d",NrTPCclusters));
	    continue;
	  }
	  if(argument.BeginsWith("RatioTPCncls=")){
	    argument.ReplaceAll("RatioTPCncls=", "");
	    RatioTPCncls=argument.Atof();
	    ::Info(Form("Minimum ratio of clusters TPC found/findable: %f GeV/c",RatioTPCncls));
	    continue;
	  }
	  if(argument.BeginsWith("nSigTPCLow=")){
	    argument.ReplaceAll("nSigTPCLow=", "");
	    nSigTPCLow=argument.Atof();
	    continue;
	  }
	  if(argument.BeginsWith("nSigTPCHigh=")){
	    argument.ReplaceAll("nSigTPCHigh=", "");
	    nSigTPCHigh=argument.Atof();
	    continue;
	  }
	  if (argument.BeginsWith("usekine") ||argument.BeginsWith("kine")) {
	    bUseKine=kTRUE;
	    taskOptions+=" usekine";
	    continue;
	  }
	  if (argument.BeginsWith("event-mixing") ||
	      argument.BeginsWith("mixing")/*deprecated, to be removed later*/) {
	    bEventMixing=kTRUE;
	    taskOptions+=" event-mixing";
	    continue;
	  }
	  if (argument.BeginsWith("PbPb") ||
	      argument.BeginsWith("system=1") ||
	      argument.BeginsWith("Pb-Pb")) {
	    system=1;
	    taskOptions+=" system=Pb-Pb";
	    continue;
	  }
	  if (argument.BeginsWith("system=p-Pb") ||
	      argument.BeginsWith("pPb") ||
	      argument.BeginsWith("p-Pb") ||
	      argument.BeginsWith("system=2")) {
	    system=2;
	    taskOptions+=" system=p-Pb";
	    continue;
	  }
	  if(argument.BeginsWith("elmcreco")){
	    bUseMCReco=kTRUE;
	    taskOptions+=" "+argument;
	    continue;
	  }
	  if (argument.BeginsWith("trigger=")) {
	    taskOptions+=" "+argument;
	    argument.ReplaceAll("trigger=","");
	    if (argument.CompareTo("D0")==0) triggerParticle=AliDxHFECorrelation::kD;
	    else if (argument.CompareTo("D")==0) triggerParticle=AliDxHFECorrelation::kD;
	    else if (argument.CompareTo("electron")==0) triggerParticle=AliDxHFECorrelation::kElectron;
	    continue;
	  }
	  if(argument.BeginsWith("addPIDqa")){
	    addPIDqa=kTRUE;
	    continue;
	  }

	  if (argument.CompareTo("runD0MassReference")==0){
	    bRunD0MassReference=kTRUE;
	    continue;
	  }
	  if(argument.BeginsWith("extraname=")){
	    argument.ReplaceAll("extraname=", "");
	    extraname=argument;
	    continue;
	  }
	  if(argument.BeginsWith("itsclusters=")){
	    argument.ReplaceAll("itsclusters=", "");
	    NrITSclusters=argument.Atoi();
	    continue;
	   }
	  if(argument.BeginsWith("itsreq=")){
	    argument.ReplaceAll("itsreq=", "");
	    if(argument.CompareTo("kFirst")==0) ITSreq=AliHFEextraCuts::kFirst;
	    else if(argument.CompareTo("kAny")==0) ITSreq=AliHFEextraCuts::kAny;
	    else if(argument.CompareTo("kNone")==0) ITSreq=AliHFEextraCuts::kNone;
	    continue;
	  }
	  if(argument.BeginsWith("triggermask=")){
	    argument.ReplaceAll("triggermask=", "");
	    if(argument.CompareTo("kINT7")==0) {triggerMask=AliVEvent::kINT7; cout<<"kINT7"<<endl;}
	    else if(argument.CompareTo("kEMC1")==0) {triggerMask=AliVEvent::kEMC1; cout<<"kEMC1"<<endl;}
	    else if(argument.CompareTo("kEMC7")==0) {triggerMask=AliVEvent::kEMC7; cout<<"kEMC7"<<endl;}
	    else if(argument.CompareTo("kEMC8")==0) {triggerMask=AliVEvent::kEMC8; cout<<"kEMC8"<<endl;}
	    else if(argument.CompareTo("allEMCAL")==0) {triggerMask=(AliVEvent::kEMC1|AliVEvent::kEMC7|AliVEvent::kEMC8); cout<<"all"<<endl;}
	    else if(argument.CompareTo("kEMCEGA")==0) {triggerMask=AliVEvent::kEMCEGA; cout<<"kEMCEGA"<<endl;}
	    else if(argument.CompareTo("kAnyInt")==0) {triggerMask=AliVEvent::kAnyINT; cout<<"kAnyINT"<<endl;}
	    else if(argument.CompareTo("kMB")==0) {triggerMask=AliVEvent::kMB; cout<<"kMB"<<endl;}
	    continue;
	  }
	  if(argument.BeginsWith("recopass=")){
	    argument.ReplaceAll("recopass=", "");
	    recoPass=argument.Atoi();
	    continue;
	   }
	  if (argument.BeginsWith("tuneondata")) {
	    bTuneOnData=kTRUE;
	    cout <<"Use tuneondata for PIDresponsetask" << endl;
	    continue;
	  }
	  if(argument.BeginsWith("TPCcrossedrows")){
	    cutOnClusters=kFALSE;
	    cout << "Cut on TPC crossed rows "  << endl; 
	    continue;
	  }
	  if(argument.BeginsWith("upgradestudies")){
	    bUpgradeStudies=kTRUE;
	    system=1;
	    cout << "Settings for upgradestudies" << endl;
	  }
	  cout << "Adding argument " << argument << endl;
	  taskOptions+=" "+argument;
	  
	}	
      }
      delete tokens;
    }
  }

  AliAnalysisManager *pManager = AliAnalysisManager::GetAnalysisManager();
  if (!pManager) {
    ::Error("AddTaskDxHFECorrelation", "No analysis manager to connect to.");
    return;
  }

  if(bUseMCReco && bUseKine) {
    ::Fatal("AddTaskDxHFECorrelation","CAN'T SET BOTH usekine AND elmcreco AT THE SAME TIME");
    return;
  }
  
  
  // check for existence of PID task and add if not available
  const char* pidTaskName="PIDResponseTask";
  const char* pidTaskMacro="$ALICE_ROOT/ANALYSIS/macros/AddTaskPIDResponse.C";
  AliAnalysisTask* pidTask=pManager->GetTask(pidTaskName);
  if (!pidTask) {
    gROOT->LoadMacro(pidTaskMacro);
    TString pidFunction;
    // isMC, autoMCeds, tuneOnData, recoPass, cachePID, detResponse, useTPCEtaCorrection, recoDataPass
    pidFunction.Form("AddTaskPIDResponse(%d, %d, %d, %d, %d, %s, %d, %d, -1)", bUseMC, kTRUE, bTuneOnData, recoPass, kFALSE,"\"\"" ,kTRUE, kTRUE);
    gROOT->ProcessLine(pidFunction);
    if (pManager->GetTask(pidTaskName)==NULL) {
      ::Error("AddTaskDxHFECorrelation", Form("failed to add PID task '%s' from macro '%s'",
					      pidTaskName, pidTaskMacro));
      return 0;
    }
    if(addPIDqa){
      gROOT->LoadMacro("$ALICE_ROOT/ANALYSIS/macros/AddTaskPIDqa.C");
      AliAnalysisTaskPIDqa *pidQA = AddTaskPIDqa();
    }
  } else {
    // TODO: would like to check if the PID task was set up
    // with consistent parameters, however there are no getters at the moment
    ::Info("AddTaskDxHFECorrelation", Form("PID task '%s' already existing", pidTaskName));
  }

  // optionally add D0Mass task for reference analysis
  if (bRunD0MassReference && !bEventMixing) {

    TString path("AddTaskD0Mass.C");
    if (gSystem->AccessPathName(path)!=0) {
      // first try local macro, than AliRoot default path
      path="$ALICE_PHYSICS/PWGHF/vertexingHF/macros/AddTaskD0Mass.C";
    }
    if (gSystem->AccessPathName(path)==0) {
      cout << "Setting up D0Mass reference task " << path << endl;
    } else {
      cout << "Can not find D0Mass reference task " << path << endl;
    }
    gROOT->LoadMacro(path);

    const char* taskName=AliAnalysisTaskSED0Mass::Class()->GetName();
    if (pManager->GetTask(taskName)) {
      ::Warning("AddTaskDxHFECorrelation", Form("task '%s' already existing, skipping ...",
						taskName));
    }
    else{
      //flag, readMC,filldistr,cutonDistr, system, flagD0D0bar,minC,maxC,finDirname,finname, finObjname,flagAOD049,FillMassPt, FillImptPar
      AliAnalysisTaskSED0Mass *d0massTask = AddTaskD0Mass(0,bUseMC,kTRUE,kTRUE, 0, 0, 0, 0, "", "","D0toKpiCuts", kFALSE, false, false);
    }
  }

  if(triggerParticle==AliDxHFECorrelation::kElectron)
    analysisName="HFExD";
  if (ofilename.IsNull()) ofilename=AliAnalysisManager::GetCommonFileName();
  ofilename+=":"+analysisName;

  if(cutFilenameD0=="")
    {
      ///______________________________________________________________________
      /// Cuts For D0
      AliRDHFCutsD0toKpi* RDHFD0toKpi=new AliRDHFCutsD0toKpi();
      // TODO: we might want to move this to separate functions if more data
      // sets are going to be handled
      
      //////////////////// p-p ///////////////////
      if (system==0) {
	RDHFD0toKpi->SetStandardCutsPP2010();
      }

      ////////////////// Pb-Pb //////////////////
      else if (system==1) {
	// TODO: think about p-Pb
	RDHFD0toKpi->SetStandardCutsPbPb2011();
	
	if(bUpgradeStudies){
	  
	  RDHFD0toKpi->SetOptPileup(AliRDHFCuts::kNoPileupSelection);
	  RDHFD0toKpi->SetUsePhysicsSelection(kFALSE);
	  RDHFD0toKpi->SetUseAnyTrigger();
	  RDHFD0toKpi->SetUseCentrality(AliRDHFCuts::kCentOff);
	}
	else{
	  // For centrality 0-10%, add centrality flattening
	  //NB! NEED FOR THE MOMENT THE FILE!
	  TFile *fFlat=TFile::Open("CentrDistrBins005.root","READ");
	  TCanvas *c=fFlat->Get("cintegral");
	  TH1F *hfl=(TH1F*)c->FindObject("hint");
	  RDHFD0toKpi->SetHistoForCentralityFlattening(hfl,0.,10.,0.,0);
	  //  RDHFD0toKpi->SetUseCentrality(AliRDHFCuts::kCentV0M);
	  RDHFD0toKpi->SetMinCentrality(0.);// 40.*1.01
	  RDHFD0toKpi->SetMaxCentrality(10.);// 80.*1.01
	}
      }
      
      /////////////// p-Pb ///////////////////
      else if (system==2) {  
	//New cuts (from cutfile used to make D0 eff map)
	//____________________________________________________
	//Set Centrality
	Float_t minc=0,maxc=100;
	
	// Cuts for D0
	AliRDHFCutsD0toKpi* RDHFD0toKpi=new AliRDHFCutsD0toKpi();
	RDHFD0toKpi->SetName("D0toKpiCuts");
	RDHFD0toKpi->SetTitle("Cuts for D0 analysis");
	
	// PILE UP REJECTION
	RDHFD0toKpi->SetOptPileup(1);  	   //per DATI (spegni per MC)		
	RDHFD0toKpi->ConfigurePileupCuts(5,0.8);  //per DATI (spegni per MC)
	
	//Event cuts
	RDHFD0toKpi->SetMinVtxContr(1);
	RDHFD0toKpi->SetMaxVtxZ(10.);
	
	//Trigger selection
	RDHFD0toKpi->SetTriggerClass("");
	RDHFD0toKpi->SetTriggerMask(triggerMask);
	
	//Quality tracks for daughters
	AliESDtrackCuts* esdTrackCuts=new AliESDtrackCuts();
	esdTrackCuts->SetRequireSigmaToVertex(kFALSE);
	esdTrackCuts->SetRequireTPCRefit(kTRUE);
	esdTrackCuts->SetRequireITSRefit(kTRUE);
	//esdTrackCuts->SetMinNClustersITS(4); // default is 5
	//esdTrackCuts->SetMinNClustersTPC(120);
	esdTrackCuts->SetClusterRequirementITS(AliESDtrackCuts::kSPD, AliESDtrackCuts::kAny); // default is kBoth, otherwise kAny
	esdTrackCuts->SetMinDCAToVertexXY(0.);
	esdTrackCuts->SetEtaRange(-0.8,0.8);
	esdTrackCuts->SetPtRange(0.3,1.e10);
	
	RDHFD0toKpi->AddTrackCuts(esdTrackCuts);
	
	//D0 selection topological cuts
	const Int_t nptbins =14;
	const Double_t ptmax = 9999.;
	const Int_t nvars=11;
	Float_t ptbins[nptbins+1];
	ptbins[0]=0.;
	ptbins[1]=0.5;	
	ptbins[2]=1.;
	ptbins[3]=2.;
	ptbins[4]=3.;
	ptbins[5]=4.;
	ptbins[6]=5.;
	ptbins[7]=6.;
	ptbins[8]=7.;
	ptbins[9]=8.;
	ptbins[10]=12.;
	ptbins[11]=16.;
	ptbins[12]=20.;
	ptbins[13]=24.;
	ptbins[14]=ptmax;
	
	RDHFD0toKpi->SetGlobalIndex(nvars,nptbins);
	RDHFD0toKpi->SetPtBins(nptbins+1,ptbins);
	
	Float_t cutsMatrixD0toKpiStand[nptbins][nvars]={{0.400,350.*1E-4,0.8,0.5,0.5,1000.*1E-4,1000.*1E-4,-0.000325,0.80,0.,3.2},/* pt<0.5*/
							{0.400,350.*1E-4,0.8,0.5,0.5,1000.*1E-4,1000.*1E-4,-0.000325,0.80,0.,3.2},/* 0.5<pt<1*/
							{0.400,300.*1E-4,0.8,0.4,0.4,1000.*1E-4,1000.*1E-4,-35000.*1E-8,0.90,0.,0.},/* 1<pt<2 */
							{0.400,300.*1E-4,0.8,0.7,0.7,1000.*1E-4,1000.*1E-4,-30000.*1E-8,0.90,0.,0.},/* 2<pt<3 */
							{0.400,300.*1E-4,0.8,0.7,0.7,1000.*1E-4,1000.*1E-4,-30000.*1E-8,0.90,0.,0.},/* 3<pt<4 */
							{0.400,300.*1E-4,0.8,0.7,0.7,1000.*1E-4,1000.*1E-4,-15000.*1E-8,0.90,0.,0.},/* 4<pt<5 */
							{0.400,300.*1E-4,0.8,0.7,0.7,1000.*1E-4,1000.*1E-4,-10000.*1E-8,0.90,0.,0.},/* 5<pt<6 */
							{0.400,300.*1E-4,0.8,0.7,0.7,1000.*1E-4,1000.*1E-4,-8000.*1E-8,0.85,0.,0.},/* 6<pt<7 */
							{0.400,300.*1E-4,0.8,0.7,0.7,1000.*1E-4,1000.*1E-4,-8000.*1E-8,0.85,0.,0.},/* 7<pt<8 */
							{0.400,300.*1E-4,0.9,0.7,0.7,1000.*1E-4,1000.*1E-4,-5000.*1E-8,0.85,0.,0.},/* 8<pt<12 */
							{0.400,300.*1E-4,1.0,0.7,0.7,1000.*1E-4,1000.*1E-4,10000.*1E-8,0.85,0.,0.},/* 12<pt<16 */
							{0.400,300.*1E-4,1.0,0.7,0.7,1000.*1E-4,1000.*1E-4,10000.*1E-8,0.85,0.,0.},/* 16<pt<20 */
							{0.400,300.*1E-4,1.0,0.7,0.7,1000.*1E-4,1000.*1E-4,10000.*1E-8,0.85,0.,0.},/* 20<pt<24 */
							{0.400,300.*1E-4,1.0,0.7,0.7,1000.*1E-4,1000.*1E-4,10000.*1E-8,0.85,0.,0.}};/* pt>24 */
	
	//CREATE TRANSPOSE MATRIX...REVERSE INDICES as required by AliRDHFCuts
	Float_t **cutsMatrixTransposeStand=new Float_t*[nvars];
	for(Int_t iv=0;iv<nvars;iv++)cutsMatrixTransposeStand[iv]=new Float_t[nptbins];
	
	for (Int_t ibin=0;ibin<nptbins;ibin++){
	  for (Int_t ivar = 0; ivar<nvars; ivar++){
	    cutsMatrixTransposeStand[ivar][ibin]=cutsMatrixD0toKpiStand[ibin][ivar];      
	  }
	}
	
	RDHFD0toKpi->SetCuts(nvars,nptbins,cutsMatrixTransposeStand);
	RDHFD0toKpi->SetUseSpecialCuts(kTRUE);
	RDHFD0toKpi->SetRemoveDaughtersFromPrim(kTRUE);
	
	for(Int_t iv=0;iv<nvars;iv++) delete [] cutsMatrixTransposeStand[iv];
	delete [] cutsMatrixTransposeStand;
	cutsMatrixTransposeStand=NULL;
	
	//D0 pid settings
	Bool_t pidflag=kTRUE;
	RDHFD0toKpi->SetUsePID(pidflag);
	if(pidflag) cout<<"PID is used"<<endl;
	else cout<<"PID is not used"<<endl;
	
	AliAODPidHF* pidObj=new AliAODPidHF();
	Int_t mode=1;
	const Int_t nlims=2;
	Double_t plims[nlims]={0.6,0.8}; //TPC limits in momentum [GeV/c]
	Bool_t compat=kTRUE; //effective only for this mode
	Bool_t asym=kTRUE;
	Double_t sigmas[5]={2.,1.,0.,3.,0.}; //to be checked and to be modified with new implementation of setters by Rossella
	pidObj->SetAsym(asym);// if you want to use the asymmetric bands in TPC
	pidObj->SetMatch(mode);
	pidObj->SetPLimit(plims,nlims);
	pidObj->SetSigma(sigmas);
	pidObj->SetCompat(compat);
	pidObj->SetPCompatTOF(2.);
	pidObj->SetSigmaForTPCCompat(3.);
	pidObj->SetSigmaForTOFCompat(3.);
	pidObj->SetTPC(kTRUE);
	pidObj->SetTOF(kTRUE);
	pidObj->SetOldPid(kFALSE);
	RDHFD0toKpi->SetPidHF(pidObj);
	RDHFD0toKpi->SetUsePID(kTRUE);
	RDHFD0toKpi->SetUseDefaultPID(kFALSE); //to use the AliAODPidHF
	RDHFD0toKpi->SetLowPt(kFALSE);
	RDHFD0toKpi->SetMaximumPforPID(999.);
	
	//activate pileup rejection (for pp)
	//  RDHFD0toKpi->SetOptPileup(AliRDHFCuts::kRejectPileupEvent);
	
	TString cent="";
	//[FIXME] needed for pPb?
	//centrality selection (Pb-Pb)
	RDHFD0toKpi->SetMinCentrality(minc);
	RDHFD0toKpi->SetMaxCentrality(maxc);
	cent=Form("%.0f%.0f",minc,maxc);
	RDHFD0toKpi->SetUseCentrality(AliRDHFCuts::kCentV0A); //kCentOff,kCentV0M,kCentTRK,kCentTKL,kCentCL1,kCentInvalid
      }
      else {
	//warning, no system set
      }
      
    }else{ //If there is a cutfile for D0
    cout<<"Getting D0 cut object from: "<<cutFilenameD0<<endl;
    TFile *filecuts;
    //TString finname="Cutlist.root";
    filecuts=TFile::Open(cutFilenameD0.Data());
    TString fRDHFcutsObj="D0toKpiCuts";
    AliRDHFCutsD0toKpi* RDHFD0toKpi=new AliRDHFCutsD0toKpi();
    RDHFD0toKpi = (AliRDHFCutsD0toKpi*)filecuts->Get(fRDHFcutsObj.Data());
    RDHFD0toKpi->PrintAll();
  }  
  if(cutFilenameEl=="")
    {
      ///______________________________________________________________________
      /// Cuts for HFE
      TString hfeCutsName;
      if (system==0){
	hfeCutsName="HFE Standard Cuts";
      }
      
      if (system==1){
	hfeCutsName="HFE Cuts PbPb";
      }
      
      if (system==2){
	hfeCutsName="HFE Cuts pPb";
      }
      
      AliHFEcuts *hfecuts = new AliHFEcuts("hfeCutsTPCTOF", hfeCutsName);
      hfecuts->CreateStandardCuts();
      
      hfecuts->SetTPCmodes(AliHFEextraCuts::kFound,AliHFEextraCuts::kFoundOverFindable);
      hfecuts->SetMinNClustersTPC(NrTPCclusters);	//Default = 80
      hfecuts->SetMinNClustersTPCPID(80);	//Default = 80
      hfecuts->SetMinRatioTPCclusters(RatioTPCncls); 	//Default = 0.6
      
      ///ITS
      hfecuts->SetCutITSpixel(ITSreq);        	//Cut on SPD
      //hfecuts->SetCutITSdrift(AliHFEextraCuts::kAny); 	//Cut on SDD
      //hfecuts->SetCheckITSLayerStatus(kFALSE);
      hfecuts->SetMinNClustersITS(NrITSclusters); //Default = 4
      
      ///TOF
      hfecuts->SetTOFPIDStep(kTRUE);
      
      ///Additional Cuts
      hfecuts->SetPtRange(0.30, 10);
      hfecuts->SetMaxImpactParam(1.,2.);
      hfecuts->SetVertexRange(10.);
      
      //
      // PID for HFE
      // PID for Only TOF
      AliHFEpid *fPIDOnlyTOF = new AliHFEpid("hfePidTOF");
      if(!fPIDOnlyTOF->GetNumberOfPIDdetectors()) { 
	fPIDOnlyTOF->AddDetector("TOF",0);
      }
      fPIDOnlyTOF->ConfigureTOF(3); // number of sigma TOF
      fPIDOnlyTOF->InitializePID();
      
      // PID object for TPC and TOF combined
      AliHFEpid *fPID = new AliHFEpid("hfePid");
      if(!fPID->GetNumberOfPIDdetectors()) { 
	fPID->AddDetector("TOF",0);
	fPID->AddDetector("TPC",1);
      }
      //Add settings for asymmetric cut on nSigma TPC
      const int paramSize=4;
      Double_t params[paramSize];
      memset(params, 0, sizeof(Double_t)*paramSize);
      if(bUseEMCAL)  params[0]=-3.;
      else params[0]= nSigTPCLow;
      fPID->ConfigureTPCdefaultCut(NULL, params,  nSigTPCHigh);
      fPID->InitializePID();
      
      // PID for Only TPC
      AliHFEpid *fPIDOnlyTPC = new AliHFEpid("hfePidTPC");
      if(!fPIDOnlyTPC->GetNumberOfPIDdetectors()) { 
	fPIDOnlyTPC->AddDetector("TPC",0);
      }
      fPIDOnlyTPC->ConfigureTPCdefaultCut(NULL, params,  nSigTPCHigh);
      fPIDOnlyTPC->InitializePID();
      
      //Create TList of HFE pid and track cuts
      TList *listHFE = new TList;
      listHFE->SetName("cut objects HFE");
      listHFE->Add(hfecuts);
      listHFE->Add(fPID);
      listHFE->Add(fPIDOnlyTOF);
      listHFE->Add(fPIDOnlyTPC); 
    }  // Should this extend further? (default cuts if cutlist-file is not present)
  else {//if there is a cutfile for electrons
    ///______________________________________________________________________
    /// Cuts for HFE
    TFile *filecuts;
    TString finname="Cutlist.root";
    filecuts=TFile::Open(finname.Data());
    AliHFEcuts *hfecuts = new AliHFEcuts();//("hfeCutsTPCTOF","HFE Standard Cuts");
    
    TString fHFEcutsObj="hfeCutsTPCTOF";
    hfecuts=(AliHFEcuts*)filecuts->Get(fHFEcutsObj.Data());
    
    // ________________________________________________________________________
    // PID for HFE
    // PID for Only TOF
    
    AliHFEpid *fPIDOnlyTOF = new AliHFEpid("hfePidTOF");
    TString fHFEpidTOFobj="hfePidTOF";
    fPIDOnlyTOF=(AliHFEpid*)filecuts->Get(fHFEpidTOFobj.Data());
    
    // PID object for TPC and TOF combined
    // Check if PID is set from outside (passed as argument)
    
    AliHFEpid* fPID = new AliHFEpid("hfePid");
    TString fHFEpidobj="hfePid";
    fPID=(AliHFEpid*)filecuts->Get(fHFEpidobj.Data());
    
    AliHFEpid *fPIDOnlyTPC = new AliHFEpid("hfePidTPC");
    TString fHFEpidTPCobj="hfePidTPC";
    fPIDOnlyTPC=(AliHFEpid*)filecuts->Get(fHFEpidTPCobj.Data());
    
    
    //=========================================================
    //Create TList of cut (and pid) objects for D0 or electron
    TList *listHFE = new TList;
    /*      if(Particle==AliAnalysisTaskDxHFEParticleSelection::kD0){
	    Cutlist->SetName("cut objects D0");
	    Cutlist->Add(RDHFD0toKpi);
	    }
	    else if(Particle==AliAnalysisTaskDxHFEParticleSelection::kElectron){
    */
    listHFE->SetName("cut objects HFE");
    listHFE->Add(hfecuts);
    listHFE->Add(fPID);
    listHFE->Add(fPIDOnlyTOF);
    listHFE->Add(fPIDOnlyTPC);
  }

  ///______________________________________________________________________
  /// Info for Pool
  // TODO: Don't think we need the MC part of AliHFCorrelator, needs to be checked
  AliAnalysisCuts* poolConfiguration=NULL;
  if (poolConfigFile.IsNull()) {
    // load the default configuration from below if no file is specified
    if (system==0) poolConfiguration=createDefaultPoolConfig(bUseKine);
    else if (system==1) poolConfiguration=createPbPbPoolConfig();
    else if (system==2) poolConfiguration=createpPbPoolConfig(bUseKine);
  } else {
    // load configuration from file, and abort if something goes wrong
    TFile* filePoolConfiguration=TFile::Open(poolConfigFile.Data());
    if(!filePoolConfiguration){
      ::Error("AddTaskDxHFECorrelation", Form("Pool configuration object file %s not found, exiting", poolConfigFile.Data()));
      return 0;
    }
    TObject* pObj=filePoolConfiguration->Get(poolInfoName);
    if (!pObj) {
      ::Error("AddTaskDxHFECorrelation", Form("No Pool configuration object with name '%s' found in file %s, exiting", poolInfoName, poolConfigFile.Data()));
      return 0;
    }
    poolConfiguration = dynamic_cast<AliHFAssociatedTrackCuts*>(pObj);
    if (!poolConfiguration) {
      ::Error("AddTaskDxHFECorrelation", Form("Pool configuration object '%s' has inconsistent class type %s, exiting", poolInfoName, pObj->ClassName()));
      return 0;
    }
  }

  if(!poolConfiguration){
    ::Fatal("AddTaskDxHFECorrelation", Form("Pool configuration not found"));
    return 0;
  } 
  poolConfiguration->Print();


  //Taken out, causes problem when adding more than one task
  /*const char* taskName=AliAnalysisTaskDxHFECorrelation::Class()->GetName();
    if (pManager->GetTask(taskName)) {
    ::Warning("AddTaskDxHFECorrelation", Form("task '%s' already existing, skipping ...",
    taskName));
    return 0;
    }*/
 
  // ******************************** OPENING THE EFFICIENCY MAPS  ************************************
  if(bUseTrackEff){
    //********************
    //Track/Electron efficiency
    //********************

    if(!poolConfiguration){
      cout << "Setting up AliHFAssociatedTrackCuts" << endl;
      poolConfiguration = dynamic_cast<AliHFAssociatedTrackCuts*>();
    }
    //

    cout << "Getting Efficiency map object from file \n" << TrackEffMap.Data() << "\n "<<  endl;
    TFile* effFile=new TFile(TrackEffMap.Data());
    if(!effFile->IsOpen()){
      cout<<"Input file not found for efficiency! Exiting..."<<endl;
      return 0;
    }
    
    TCanvas *c = (TCanvas*)effFile->Get("c");
    if(!c) {cout << "No canvas !" << endl;}
    TH3D *h3D = (TH3D*)c->FindObject("heff_rebin");

    if(!h3D) {cout << "No Single track efficiency histo " << endl; ;}
	
    poolConfiguration->AliHFAssociatedTrackCuts::SetEfficiencyWeightMap(h3D);
  }

  if(bUseD0Eff){
    //********************
    //D0 efficiency
    //********************

    TFile* fileeffD0Prompt=TFile::Open(D0EffMapPrompt.Data());
    if(!fileeffD0Prompt->IsOpen()){
      ::Fatal("AddTaskDxHFECorrelation", Form("Input file not found for efficiency with charm correction! Exiting..."));
      return 0;
    }
    TCanvas *c1 = (TCanvas*)fileeffD0Prompt->Get("c1");
    if(!c1) {::Fatal("AddTaskDxHFECorrelation", Form("No canvas inside D0 eff map file for prompt")); return 0;}
    TH2D *hEff = (TH2D*)c1->FindObject("h_Eff");
    if(!hEff) {::Fatal("AddTaskDxHFECorrelation", Form("No efficiency histo for Prompt D0")); return 0;}
    poolConfiguration->AliHFAssociatedTrackCuts::SetTriggerEffWeightMap(hEff); 

    if(bUseMC){
      //Only Add feeddown correction for MC      
      TFile* fileeffD0FeedDown=TFile::Open(D0EffMapFeedDown.Data());
      if(!fileeffD0FeedDown->IsOpen()){
	::Fatal("AddTaskDxHFECorrelation", Form("Input file not found for efficiency with feeddown correction! Exiting..."));
	return 0;
      }
      /* Old method. Has been updated to work with new eff-map style
	TCanvas *c2 = (TCanvas*)fileeffD0FeedDown->Get("c2");
	if(!c2) {::Fatal("AddTaskDxHFECorrelation", Form("No canvas inside D0 eff map file for feeddown")); return 0;}*/
      //TH2D *hEff2 = (TH2D*)c2->FindObject("h_Eff");
      TH2D *hEff2 = (TH2D*)fileeffD0FeedDown->Get("h_Eff");
      if(!hEff2) {::Fatal("AddTaskDxHFECorrelation", Form("No efficiency histo for Feeddown D0")); return 0;}
      poolConfiguration->AliHFAssociatedTrackCuts::SetTriggerEffWeightMapB(hEff2); 
    }

  }

 
  AliAnalysisTaskDxHFECorrelation *pTask=new AliAnalysisTaskDxHFECorrelation(taskOptions);
  if (!pTask) {
    ::Error("AddTaskDxHFECorrelation", "failed to create task.");
    return 0;
  }

  TList *listD0 = new TList;
  listD0->Add(RDHFD0toKpi);
  if(bUseD0Eff) listD0->Add(poolConfiguration);

  pTask->SetCutsD0(listD0);
  pTask->SetCutsHFE(listHFE);
  pTask->SetCuts(poolConfiguration);

  pManager->AddTask(pTask);


  // The AnalysisManager handles the output file name in the following way:
  // The output file names are set by the function SetOutputFiles
  // If the file name given to the container begins with one of the initialized
  // file names, the data is stored in the corresponding file in a folder with
  // the full name specified to the container
  // E.g. output file has been set to "myanalysis", the container is created with
  // file name "myanalysis_A", data ends up in file "myanalysis" in folder
  // "myanalysis_A"
  // IMPORTANT: choosing a file name with a different stem at this point will
  // probably lead to an empty file.

  TString listName;
  TString cutnameD0="cutsD0Corr";
  TString cutnameEl="cutsElCorr";
  TString cutnamePool="PoolInfo";
  TString QASelName="QASelection";
  if(triggerParticle==AliDxHFECorrelation::kElectron){
    cutnameD0+="Eltrigg";
    cutnameEl+="Eltrigg";
    cutnamePool+="Eltrigg";
    listName="HFExDlist";
  }
  else listName="DxHFElist";

  listName+=extraname;
  cutnameD0+=extraname;
  cutnameEl+=extraname;
  cutnamePool+=extraname;


  if(bEventMixing){ 
    ofilename+="ME";
    listName+="ME";
    cutnameD0+="ME";
    cutnameEl+="ME";
    cutnamePool+="ME";
    QASelName+="ME";
  }

  if(bEventMixing) ::Info("AddTaskDxHFECorrelation", Form("\ninitializing analysis '%s'%s, output file '%s', Event Mixing Analysis\n", analysisName.Data(), bUseMC?" (using MC)":"", ofilename.Data()));
  if(!bEventMixing)  ::Info("AddTaskDxHFECorrelation", Form("\ninitializing analysis '%s'%s, output file '%s', Single Event Analysis\n", analysisName.Data(), bUseMC?" (using MC)":"", ofilename.Data()));


  AliAnalysisDataContainer *pContainer=pManager->CreateContainer(listName, TList::Class(), AliAnalysisManager::kOutputContainer, ofilename.Data());    
  AliAnalysisDataContainer *pContainer2=pManager->CreateContainer(cutnameD0,AliRDHFCutsD0toKpi::Class(),AliAnalysisManager::kOutputContainer, ofilename.Data()); //cuts D0
  AliAnalysisDataContainer *pContainer3=pManager->CreateContainer(cutnameEl,TList::Class(),AliAnalysisManager::kOutputContainer, ofilename.Data()); //cuts El
  AliAnalysisDataContainer *pContainer4=pManager->CreateContainer(cutnamePool,AliHFAssociatedTrackCuts::Class(),AliAnalysisManager::kOutputContainer, ofilename.Data()); // contains event pool info
  AliAnalysisDataContainer *pContainer5=pManager->CreateContainer(QASelName, TList::Class(), AliAnalysisManager::kOutputContainer, ofilename.Data());    

  pManager->ConnectInput(pTask,0,pManager->GetCommonInputContainer());
  pManager->ConnectOutput(pTask,1,pContainer);
  pManager->ConnectOutput(pTask,2,pContainer2);
  pManager->ConnectOutput(pTask,3,pContainer3);
  pManager->ConnectOutput(pTask,4,pContainer4);
  pManager->ConnectOutput(pTask,5,pContainer5);

  return 1;
}

// old signature kept for backward compatibility
int AddTaskDxHFECorrelation(Bool_t bUseMC, TString analysisName)
{
  TString arguments(bUseMC?"mc":"");
  if (!analysisName.IsNull()) {
    arguments+=Form(" name=%s", analysisName.Data())
  }
  AddTaskDxHFECorrelation(arguments)
}

// Note: AliHFAssociatedTrackCuts keeps an instance of the external
// pointer, the arrays thus need to be global
// TODO: try a proper implementation of AliHFAssociatedTrackCuts later
//[TODO] Make these values dependent on "system" parameter

//2013 pPb values
const Int_t    nofMBins=2;
const Double_t MBins[nofMBins+1]={0,50,500};
const Double_t * MultiplicityBins = MBins;
const Int_t    nofZBins=3;
const Double_t ZBins[nofZBins+1]={-10,-2.5,2.5,10};

/* 2010pp values
   const Int_t    nofMBins=5;
   const Double_t MBins[nofMBins+1]={0,20,40,60,80,500};
   const Double_t * MultiplicityBins = MBins;
   const Int_t    nofZBins=5;
   const Double_t ZBins[nofZBins+1]={-10,-5,-2.5,2.5,5,10};
*/
const Double_t *ZVrtxBins = ZBins;

AliAnalysisCuts* createDefaultPoolConfig(Bool_t usekine)
{
  AliHFAssociatedTrackCuts* HFCorrelationCuts=new AliHFAssociatedTrackCuts();
  HFCorrelationCuts->SetName("PoolInfo");
  HFCorrelationCuts->SetTitle("Info on Pool for EventMixing");

  // NEED to check this
  if(usekine){
    HFCorrelationCuts->SetMinEventsToMix(8);
    HFCorrelationCuts->SetMaxNEventsInPool(10);
    HFCorrelationCuts->SetMinNTracksInPool(50);
  }
  else{
    HFCorrelationCuts->SetMinEventsToMix(8);
    HFCorrelationCuts->SetMaxNEventsInPool(200);
    HFCorrelationCuts->SetMinNTracksInPool(100);

  }
  HFCorrelationCuts->SetNofPoolBins(nofZBins,nofMBins); // Note: the arrays have dimension x+1
  HFCorrelationCuts->SetPoolBins(ZVrtxBins,MultiplicityBins);

  TString description = "Info on Pool for EventMixing";   
  HFCorrelationCuts->AddDescription(description);

  return HFCorrelationCuts;
}

AliAnalysisCuts* createpPbPoolConfig(Bool_t usekine)
{
  AliHFAssociatedTrackCuts* HFCorrelationCuts=new AliHFAssociatedTrackCuts();
  HFCorrelationCuts->SetName("PoolInfo");
  HFCorrelationCuts->SetTitle("Info on Pool for EventMixing");

  // NEED to check this
  if(usekine){
    HFCorrelationCuts->SetMinEventsToMix(8);
    HFCorrelationCuts->SetMaxNEventsInPool(10);
    HFCorrelationCuts->SetMinNTracksInPool(50);
  }
  else{
    HFCorrelationCuts->SetMinEventsToMix(1);//[STUDY] Can be adjusted up towards at least 8
    HFCorrelationCuts->SetMaxNEventsInPool(200);
    HFCorrelationCuts->SetMinNTracksInPool(10000);
    HFCorrelationCuts->SetTargetFracTracks(0.0025);
  }
  HFCorrelationCuts->SetNofPoolBins(nofZBins,nofMBins); // Note: the arrays have dimension x+1
  HFCorrelationCuts->SetPoolBins(ZVrtxBins,MultiplicityBins);
  
  TString description = "Info on Pool for EventMixing";   
  HFCorrelationCuts->AddDescription(description);
  
  return HFCorrelationCuts;
}

AliAnalysisCuts* createPbPbPoolConfig()
{
  AliHFAssociatedTrackCuts* HFCorrelationCuts=new AliHFAssociatedTrackCuts();
  HFCorrelationCuts->SetName("PoolInfo");
  HFCorrelationCuts->SetTitle("Info on Pool for EventMixing");

  // NEED to check this
  HFCorrelationCuts->SetMaxNEventsInPool(250);
  HFCorrelationCuts->SetMinNTracksInPool(80);
  HFCorrelationCuts->SetMinEventsToMix(5);
  HFCorrelationCuts->SetNofPoolBins(nofZBins,nofMBins); // Note: the arrays have dimension x+1
  HFCorrelationCuts->SetPoolBins(ZVrtxBins,MultiplicityBins);

  TString description = "Info on Pool for EventMixing";   
  HFCorrelationCuts->AddDescription(description);

  return HFCorrelationCuts;
}
