// Tell emacs that this is a C++ source
// This file is really -*- C++ -*-.
#ifndef G4DETECTORS_PHG4PROTOTYPE2HCALCELLRECO_H
#define G4DETECTORS_PHG4PROTOTYPE2HCALCELLRECO_H

#include "PHG4ParameterInterface.h"

#include <fun4all/SubsysReco.h>
#include <phool/PHTimeServer.h>
#include <string>
#include <map>
#include <vector>

class PHCompositeNode;
class PHG4CylinderCell;

class PHG4Prototype2HcalCellReco : public SubsysReco, public PHG4ParameterInterface
{
 public:

  PHG4Prototype2HcalCellReco(const std::string &name = "Prototype2HcalCELLRECO");

  virtual ~PHG4Prototype2HcalCellReco(){}
  
  //! module initialization
  int InitRun(PHCompositeNode *topNode);
  
    //! event processing
  int process_event(PHCompositeNode *topNode);
  
  //! end of process
  int End(PHCompositeNode *topNode);
  
  void SetDefaultParameters();

  void Detector(const std::string &d) {detector = d;}
  void checkenergy(const int i=1) {chkenergyconservation = i;}

  void   set_timing_window(const double tmi, const double tma);
  
 protected:
  int CheckEnergy(PHCompositeNode *topNode);
  std::string detector;
  std::string hitnodename;
  std::string cellnodename;
  PHTimeServer::timer _timer;
  int chkenergyconservation;

  double tmin;
  double tmax;
};

#endif // G4DETECTORS_PHG4PROTOTYPE2HCALCELLRECO_H
