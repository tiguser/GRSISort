#ifndef TTigressData_H
#define TTigressData_H

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <set>

#include "Globals.h"
#include "TFragment.h"
#include "TChannel.h"

#include "TGRSIDetectorData.h"

class TTigressData : public TGRSIDetectorData {

  private:
    std::set<UShort_t>     fCoreSet;        //!
    std::vector<UShort_t>  fClover_Nbr;     //!
    std::vector<UShort_t>  fCore_Nbr;       //!
    std::vector<TFragment> fCore_Frag;      //!

    std::vector<UShort_t>  fSeg_Clover_Nbr; //!
    std::vector<UShort_t>  fSeg_Core_Nbr;   //!
    std::vector<UShort_t>  fSegment_Nbr;    //!
    std::vector<TFragment> fSegment_Frag;   //! 

    std::vector<UShort_t>  fBgo_Clover_Nbr;  //!
    std::vector<UShort_t>  fBgo_Core_Nbr;   //!
    std::vector<UShort_t>  fBgo_Nbr;        //!
    std::vector<TFragment> fBgo_Frag;       //!


    static bool fIsSet; //!

  public:
    TTigressData();          //!
    virtual ~TTigressData();      //!

    std::set<UShort_t> GetCoreSet() { return fCoreSet;} //!
    static void Set(bool flag=true) { fIsSet=flag; }    //!
    static bool IsSet()             { return fIsSet; }  //!

    virtual void Clear(Option_t *opt = "");    //!
    virtual void Print(Option_t *opt = "") const;    //!

    inline void SetCloverNumber(const UShort_t  &CloverNumber) {fClover_Nbr.push_back(CloverNumber); }  //!
    inline void SetCoreNumber(const UShort_t    &CoreNumber)   {fCore_Nbr.push_back(CoreNumber);     }  //!
    inline void SetCoreFragment(const TFragment &CoreFrag)     {fCore_Frag.push_back(CoreFrag);      }  //!
  
    inline void SetCore(const UShort_t &CloverNbr, const UShort_t &CoreNbr, const TFragment &CoreFrag)  {
      SetCloverNumber(CloverNbr);
      SetCoreNumber(CoreNbr);
      SetCoreFragment(CoreFrag);    
    }  //!

    inline void SetCore(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic)  {
      if(!frag || !channel || !mnemonic) return;

      if(mnemonic->outputsensor.compare(0,1,"b")==0) {  return; }  //make this smarter.

      UShort_t CoreNbr=5;
      if(mnemonic->arraysubposition.compare(0,1,"B")==0)
        CoreNbr=0;
      else if(mnemonic->arraysubposition.compare(0,1,"G")==0)
        CoreNbr=1;
      else if(mnemonic->arraysubposition.compare(0,1,"R")==0)
        CoreNbr=2;
      else if(mnemonic->arraysubposition.compare(0,1,"W")==0)
        CoreNbr=3;

      GetCoreSet().insert(mnemonic->arrayposition*60+CoreNbr);
      SetCore(mnemonic->arrayposition,CoreNbr,*frag);
    }; //! 


    inline void SetSegCloverNumber(const UShort_t  &CloverNumber)  { fSeg_Clover_Nbr.push_back(CloverNumber);}  //!
    inline void SetSegCoreNumber(const   UShort_t  &CoreNumber)    { fSeg_Core_Nbr.push_back(CoreNumber);    }  //!
    inline void SetSegmentNumber(const   UShort_t  &SegmentNumber) { fSegment_Nbr.push_back(SegmentNumber);  }  //!
    inline void SetSegmentFragment(const TFragment &SegmentFrag)   { fSegment_Frag.push_back(SegmentFrag);   }  //!

    inline void SetSegment(const UShort_t &SegCloverNbr, const UShort_t &SegCoreNbr, const UShort_t &SegmentNumber,const TFragment &SegmentFrag)  {
      SetSegCloverNumber(SegCloverNbr);
      SetSegCoreNumber(SegCoreNbr);
      SetSegmentNumber(SegmentNumber);
      SetSegmentFragment(SegmentFrag);
    }  //!

    inline void SetSegment(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic)  {
      if(!frag || !channel || !mnemonic) return;
      UShort_t CoreNbr=5;
      if(mnemonic->arraysubposition.compare(0,1,"B")==0)
        CoreNbr=0;
      else if(mnemonic->arraysubposition.compare(0,1,"G")==0)
        CoreNbr=1;
      else if(mnemonic->arraysubposition.compare(0,1,"R")==0)
        CoreNbr=2;
      else if(mnemonic->arraysubposition.compare(0,1,"W")==0)
        CoreNbr=3;
      SetSegment(mnemonic->arrayposition,CoreNbr,mnemonic->segment,*frag);
    }; 


    inline void SetBgoCloverNumber(const UShort_t  &CloverNumber)  { fBgo_Clover_Nbr.push_back(CloverNumber);}  //!
    inline void SetBgoCoreNumber(const   UShort_t  &CoreNumber)    { fBgo_Core_Nbr.push_back(CoreNumber);    }  //!
    inline void SetBgoNumber(const   UShort_t  &BgoNumber) { fBgo_Nbr.push_back(BgoNumber);  }  //!
    inline void SetBgoFragment(const TFragment &BgoFrag)   { fBgo_Frag.push_back(BgoFrag);   }  //!


    inline void SetBgo(const UShort_t &BgoCloverNbr, const UShort_t &BgoCoreNbr, const UShort_t &BgoNumber,const TFragment &BgoFrag)  {
      SetBgoCloverNumber(BgoCloverNbr);
      SetBgoCoreNumber(BgoCoreNbr);
      SetBgoNumber(BgoNumber);
      SetBgoFragment(BgoFrag);
    }  //!


    inline void SetBgo(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic)  {
      if(!frag || !channel || !mnemonic) return;
      UShort_t CoreNbr=5;
      if(mnemonic->arraysubposition.compare(0,1,"B")==0)
        CoreNbr=0;
      else if(mnemonic->arraysubposition.compare(0,1,"G")==0)
        CoreNbr=1;
      else if(mnemonic->arraysubposition.compare(0,1,"R")==0)
        CoreNbr=2;
      else if(mnemonic->arraysubposition.compare(0,1,"W")==0)
        CoreNbr=3;
      SetBgo(mnemonic->arrayposition,CoreNbr,mnemonic->segment,*frag);
    }; 




    inline UShort_t  GetCloverNumber(const unsigned int &i)  { return fClover_Nbr.at(i);} //!
    inline UShort_t  GetCoreNumber(const unsigned int &i)    { return fCore_Nbr.at(i);  } //!
    inline TFragment GetCoreFragment(const unsigned int &i)  { return fCore_Frag.at(i); } //!

    inline UShort_t  GetSegCloverNumber(const unsigned int &i) {return fSeg_Clover_Nbr.at(i);} //!
    inline UShort_t  GetSegCoreNumber(const unsigned int &i)   {return fSeg_Core_Nbr.at(i);}   //!
    inline UShort_t  GetSegmentNumber(const unsigned int &i)   {return fSegment_Nbr.at(i);}    //!
    inline TFragment GetSegmentFragment(const unsigned int &i) {return fSegment_Frag.at(i);}   //!


    inline UShort_t  GetBgoCloverNumber(const unsigned int &i) {return fBgo_Clover_Nbr.at(i);} //!
    inline UShort_t  GetBgoCoreNumber(const unsigned int &i)   {return fBgo_Core_Nbr.at(i);}   //!
    inline UShort_t  GetBgoNumber(const unsigned int &i)       {return fBgo_Nbr.at(i);}        //!
    inline TFragment GetBgoFragment(const unsigned int &i)     {return fBgo_Frag.at(i);}       //!

    inline unsigned int GetCoreMultiplicity()    { return fCore_Nbr.size();    }  //!
    inline unsigned int GetSegmentMultiplicity() { return fSegment_Nbr.size(); }  //!
    inline unsigned int GetBgoMultiplicity()     { return fBgo_Nbr.size();     }  //!
};


#endif





