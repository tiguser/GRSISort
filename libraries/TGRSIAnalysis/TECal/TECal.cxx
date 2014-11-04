#include "TECal.h"

ClassImp(TECal)

TECal::TECal(){
   printf("Opening file: GriffEfficiency.root\n");
   TFile *effFile = new TFile("GriffEfficiency.root","UPDATE");
}


TECal::TECal(const char * filename){
   printf("Opening file: %s\n",filename);
   TFile *effFile = new TFile(filename,"UPDATE");
}

TECal::~TECal(){
   effFile->Close();
   delete effFile;

}


void TECal::CalibrateEnergy(){


}


void TECal::CalibrateEfficiency(){


}

Bool_t TECal::FitEnergyCal(){


   return true;
}

