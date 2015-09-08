#ifndef TTIGRESS_H
#define TTIGRESS_H


#include <vector>
#include <iostream>
#include <set>
#include <stdio.h>

#include "TTigressHit.h"
#ifndef __CINT__
#include "TTigressData.h"
#else
class TTigressData;
#endif

#include "TMath.h"
#include "TVector3.h" 

#include "TGRSIDetector.h" 

using namespace std;

class TTigress : public TGRSIDetector {

  public:
    TTigress();
      TTigress(const TTigress&);
    virtual ~TTigress();

  public: 
    void BuildHits(TGRSIDetectorData *data =0,Option_t *opt = ""); //!
    void BuildAddBack(Option_t *opt="");  //!

    TTigressHit *GetTigressHit(const int i) { return &tigress_hits.at(i);     } //!
    TGRSIDetectorHit *GetHit(const int i)   { return GetTigressHit(i);       } //!
    Int_t GetMultiplicity()                 { return tigress_hits.size();    } //!

    TTigressHit *GetAddBackHit(int i)       { return &addback_hits.at(i);     } //!
    Int_t GetAddBackMultiplicity()          { return addback_hits.size();    } //!

    void AddHit(TTigressHit *hit,Option_t *opt); 
    void AddTigressHit(const TTigressHit&); 
    void AddAddBackHit(const TTigressHit&); 
    
    void PushBackHit(TGRSIDetectorHit* hit) { }

    static TVector3 GetPosition(int DetNbr ,int CryNbr, int SegNbr, double distance = 110.);    //!
    void FillData(TFragment*,TChannel*,MNEMONIC*); //!
    void FillBGOData(TFragment*,TChannel*,MNEMONIC*); //!

  private: 
    TTigressData *tigdata;        //!

    std::vector <TTigressHit> tigress_hits;
    std::vector <TTigressHit> addback_hits;

    static bool fSetSegmentHits;      //!
    static bool fSetBGOHits;          //!
    
    static bool fSetCoreWave;         //!
    static bool fSetSegmentWave;      //!
    static bool fSetBGOWave;          //!

    static double GeBlue_Position[17][9][3];  //!  detector segment XYZ
    static double GeGreen_Position[17][9][3];  //!
    static double GeRed_Position[17][9][3];  //!
    static double GeWhite_Position[17][9][3];  //!

  public:
    static bool SetSegmentHits()    { return fSetSegmentHits;  }  //!
    static bool SetBGOHits()        { return fSetBGOHits;      }  //!

    static bool SetCoreWave()    { return fSetCoreWave;      }  //!
    static bool SetSegmentWave() { return fSetSegmentWave;  }  //!
    static bool SetBGOWave()   { return fSetBGOWave;    }     //!

  public:         
    virtual void Clear(Option_t *opt = "");     //!
    virtual void Print(Option_t *opt = "") const; //!
    virtual void Copy(TTigress&) const;           //!

   ClassDef(TTigress,1)  // Tigress Physics structure


};








#endif


