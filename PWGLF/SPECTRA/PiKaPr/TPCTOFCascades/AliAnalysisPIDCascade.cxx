#include "AliAnalysisPIDCascade.h"
#include "AliAnalysisPIDV0.h"
#include "AliAnalysisPIDTrack.h"
AliAnalysisPIDCascade::AliAnalysisPIDCascade():
  TObject(),
  fInvMXi(0),
  fInvMO(0),
  fCascRadius(0),
  fCascDCA(0),
  fCascCosinePA(0),
  fPtCasc(0),
  fEtaCasc(0),
  fCascDCAPV(-999)
{
  fBachAnalysisPIDTrack = new AliAnalysisPIDTrack();
  fV0AnalysisPIDV0 = new AliAnalysisPIDV0();
};

AliAnalysisPIDCascade::AliAnalysisPIDCascade(const AliAnalysisPIDCascade &source):
  TObject(source),
  fV0AnalysisPIDV0(source.fV0AnalysisPIDV0),
  fBachAnalysisPIDTrack(source.fBachAnalysisPIDTrack),
  fInvMXi(source.fInvMXi),
  fInvMO(source.fInvMO),
  fCascRadius(source.fCascRadius),
  fCascDCA(source.fCascDCA),
  fCascCosinePA(source.fCascCosinePA),
  fPtCasc(source.fPtCasc),
  fEtaCasc(source.fEtaCasc),
  fCascDCAPV(source.fCascDCAPV)
{
};
AliAnalysisPIDCascade &AliAnalysisPIDCascade::operator=(const AliAnalysisPIDCascade &source) {
  if(&source == this) return *this;
  TObject::operator=(source);
  fV0AnalysisPIDV0 = source.fV0AnalysisPIDV0;
  fBachAnalysisPIDTrack = source.fBachAnalysisPIDTrack;
  fInvMXi = source.fInvMXi;
  fInvMO = source.fInvMO;
  fCascRadius = source.fCascRadius;
  fCascDCA = source.fCascDCA;
  fCascCosinePA = source.fCascCosinePA;
  fPtCasc = source.fPtCasc;
  fEtaCasc = source.fEtaCasc;
  fCascDCAPV = source.fCascDCAPV;
  return *this;
};
void AliAnalysisPIDCascade::Update(AliAnalysisPIDV0* V0, AliAnalysisPIDTrack* BachTrack, Double_t *InvMasses, Double_t CascRadius,  Double_t CascDCA, Double_t CascCosinePA, Double_t PtCasc, Double_t EtaCasc, Double_t CascDCAPV, Int_t Charge) {
  fV0AnalysisPIDV0 = V0;
  fBachAnalysisPIDTrack = BachTrack;
  fInvMXi=InvMasses[0];
  fInvMO=InvMasses[1];
  fCascRadius = CascRadius;
  fCascDCA = CascDCA;
  fCascCosinePA = CascCosinePA;
  fPtCasc = PtCasc;
  fEtaCasc = EtaCasc;
  fCascDCAPV = CascDCAPV;
  fCharge = Charge;
};
AliAnalysisPIDCascade::~AliAnalysisPIDCascade()
{
  delete fV0AnalysisPIDV0;
  delete fBachAnalysisPIDTrack;
};

Double_t AliAnalysisPIDCascade::CalculateCascDCAPV() {
  return fCascRadius * (TMath::Sqrt(1. - fCascCosinePA * fCascCosinePA));
};
