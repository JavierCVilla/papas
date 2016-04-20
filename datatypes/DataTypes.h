//
//  Created by Alice Robson on 29/11/15.
//  TODO RENAME THIS to DATATYPES
//
#ifndef DataTypes_H
#define DataTypes_H
/**
 file datatypes.h
 */
#include <string>
#include <list>
#include <utility>
#include "particle.h"
#include "particledata.h"
#include "TVector3.h"
#include "path.h"
#include "../pfalgo/distance.h"
#include <iostream>


/// Function to create a new TLorentzVector
TLorentzVector makeParticleLorentzVector(int pdgid, double theta, double  phi,
                                         double energy);



class Cluster  {
public:
  Cluster(double energy, const TVector3& position, double size_m, long id);
  Cluster() = default;
  Cluster(Cluster&& c)   = default;
  Cluster(const Cluster&) = default ;;//=default {std::cout<< "copy" ;} ;
  Cluster& operator+=(const Cluster& rhs);
  double angularSize() const {return m_angularSize;};
  double size() const     {return m_size;};
  double pt() const       {return m_pt;};
  double energy() const   {return m_energy;};
  double eta() const      {return m_position.Eta();};
  long ID() const         {return m_uniqueID;}
  TVector3 position() const {return m_position;};
  void setEnergy(double energy);
  void setSize(double value) ;
  std::vector<long> subClusters() const { return m_subClusters;};
  static double s_maxEnergy; //AJR is this in the right place
  
  friend std::ostream& operator<<(std::ostream& os, const Cluster& cluster); //TODO move to helper class
  
protected:
  long m_uniqueID;
  double m_size;
  double m_angularSize;
  double m_pt;
  TVector3 m_position;
  double m_energy;
  std::vector<long> m_subClusters;
  
};


class Track{
public:
  Track(TVector3 p3, double charge, const Path& path, long id);
  Track() : m_uniqueID(0), m_path(Path::NullPath) {};
  //Track(Track& T);
  //Track(const Track& T);
  //Track(Track&& c);
  //Track(const Track&& c);
  //Track& operator=(const Track& T);
  //Track& operator=(Track& T);
  //Track& operator=(Track&&);
  //~Track();
  
  double pt() const       {return m_p3.Perp();}
  double energy() const   {return m_p3.Mag();}
  double eta() const      {return m_p3.Eta();}
  double charge() const   {return m_charge;}
  long   ID() const       {return m_uniqueID;}
  TVector3 p3() const     {return m_p3;}
  const Path& path() const     {return m_path;} //const
  void setEnergy(double energy);
  void setSize(double value) ;
  static double s_maxenergy; //AJR is this in the right place

protected:
  long m_uniqueID;
  double m_pt;
  TVector3 m_p3;
  double m_charge;
  const Path& m_path; //TODO wanted this to be const not owned by track but useful to know where it is
  
};


class SimParticle: public Particle {
public:
  bool isElectroMagnetic() const;
  SimParticle() = default;
  SimParticle(long uniqueid, int pdgid, TLorentzVector tlv, double  field=0., TVector3 vertex= TVector3(0., 0.,0.));
  Path& path()   {if (m_isHelix) return m_helix; else return m_path;};
  const class Path& constPath() const   {if (m_isHelix) return m_helix; else return m_path;};
  Helix& helix() {return m_helix;}
  TVector3 pathPosition(std::string name);
  long uniqueID() const { return m_uniqueid;}
private:
  long m_uniqueID;
  TVector3 m_vertex;
  class Path m_path;
  class Helix m_helix;
  bool m_isHelix;
};

#endif