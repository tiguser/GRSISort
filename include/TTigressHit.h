#ifndef TIGRESSHIT_H
#define TIGRESSHIT_H

#include <cstdio>
#include <cmath>
#ifndef __CINT__
#include <tuple>
#endif

#include "TFragment.h"
#include "TChannel.h"
#include "TCrystalHit.h"
//#include "TTigress.h"

#include "TMath.h"
#include "TVector3.h"
#include "TClonesArray.h"

#include "TGRSIDetectorHit.h"


class TTigressHit : public TGRSIDetectorHit {
  public:
    TTigressHit();
    TTigressHit(const TTigressHit&);
    virtual ~TTigressHit();

  private:
    UShort_t first_segment;        
    Int_t    first_segment_charge;   //!
    Bool_t   is_crys_set;            //!
    UShort_t crystal;                //!

    //TGRSIDetectorHit core;  this!
    std::vector<TGRSIDetectorHit> segments;
    std::vector<TGRSIDetectorHit> bgo;
  
    //need to do sudo tracking to build addback.
    TVector3 lasthit;                //!
    #ifndef __CINT__
    std::tuple<int,int,int> lastpos; //!
    #endif

  public:
    void SetHit() {}
    /////////////////////////    /////////////////////////////////////
    void SetCore(TGRSIDetectorHit &core)      { Copy(core); }               //!
    void AddSegment(TGRSIDetectorHit &seg)    { segment.push_back(temp);  } //!
    void AddBGO(TGRSIDetectorHit &bgo)        { bgo.push_back(temp);  }     //!

    /////////////////////////    /////////////////////////////////////
    inline int GetInitialHit()           {  return first_segment;  }      //!
  
    TVector3 GetPosition(Double_t dist=110.); 

    inline double GetDoppler(double beta,TVector3 *vec=0); 

    inline int GetSegmentMultiplicity() { return segment.size(); } //!
    inline int GetBGOMultiplicity()     { return bgo.size();     } //!
    inline TGRSIDetectorHit &GetSegment(const int &i) const { return segment.at(i);}
    inline TGRSIDetectorHit &GetBGO(const int &i) const     { return bgo.At(i);  }  
    inline TGRSIDetectorHit &GetCore()                      { return *this;  } 

    void CheckFirstHit(int charge,int segment);                    //!

//    static bool Compare(TTigressHit lhs, TTigressHit rhs);        //!     { return (lhs.GetDetectorNumber() < rhs.GetDetectorNumber()); }
//    static bool CompareEnergy(TTigressHit lhs, TTigressHit rhs);  //!     { return (lhs.GetDetectorNumber() < rhs.GetDetectorNumber()); }
    
    void SumHit(TTigressHit&);                           //!
    #ifndef __CINT__
    inline std::tuple<int,int,int> GetLastPosition() {return lastpos;} //!
    #endif                         

  public:
    virtual void Clear(Option_t *opt = "");                          //!
    virtual void Copy(TObject&) const;                             //!
    virtual void Print(Option_t *opt = "") const;                    //!

  ClassDef(TTigressHit,1)
};




#endif
