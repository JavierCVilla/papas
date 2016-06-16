//
//  Created by Alice Robson on 09/11/15.
//
//

#include "Cluster.h"
#include "Helix.h"
#include "Id.h"
#include "PFParticle.h"
#include "Path.h"
#include "displaypfobjects.h"
#include "pTrack.h"
#include <iostream>
//#include "Id.h"

namespace papas {

GBlob::GBlob(const Cluster& cluster) {
  TVector3 pos = cluster.position();
  double radius = cluster.size();
  double thetaphiradius = cluster.angularSize();
  double max_energy = Cluster::s_maxEnergy;
  double iradius = radius * cluster.energy() / max_energy;
  double ithetaphiradius = thetaphiradius * cluster.energy() / max_energy;

  // set the layer
  // m_layer = Id::layer(cluster.id());

  // set the color according to particle type
  int color = 1;

  /*if (cluster.pdgid() == 22 or cluster.pdgid() == 11)
     color = 2;
  else if (cluster.pdgid() > 0)
     color = 4;*/

  // AJRTODO implement the other projections
  //
  // TEllipse m_contourYz = TEllipse(pos.Z(), pos.Y(), radius);
  // TEllipse m_contourXz = TEllipse(pos.Z(), pos.X(), radius);
  // TEllipse m_contourThetaphi = TEllipse(math.pi/2. - pos.Theta(), pos.Phi(),
  //                                thetaphiradius);
  // self.inner_xy = TEllipse(pos.X(), pos.Y(), iradius)
  // self.inner_yz = TEllipse(pos.Z(), pos.Y(), iradius)
  // self.inner_xz = TEllipse(pos.Z(), pos.X(), iradius)
  // self.inner_thetaphi = TEllipse(math.pi/2. - pos.Theta(), pos.Phi(),
  //                              ithetaphiradius)
  // inners = [self.inner_xy, self.inner_yz, self.inner_xz,
  //          self.inner_thetaphi]

  m_contours["xy"] = std::unique_ptr<TEllipse>{new TEllipse(pos.X(), pos.Y(), radius)};
  m_inners["xy"] = std::unique_ptr<TEllipse>{new TEllipse(pos.X(), pos.Y(), iradius)};
  m_contours["yz"] = std::unique_ptr<TEllipse>{new TEllipse(pos.Z(), pos.Y(), radius)};
  m_inners["yz"] = std::unique_ptr<TEllipse>{new TEllipse(pos.Z(), pos.Y(), iradius)};
  m_contours["xz"] = std::unique_ptr<TEllipse>{new TEllipse(pos.Z(), pos.X(), radius)};
  m_inners["xz"] = std::unique_ptr<TEllipse>{new TEllipse(pos.Z(), pos.X(), iradius)};
  m_contours["ECAL_thetaphi"] =
      std::unique_ptr<TEllipse>{new TEllipse(M_PI_2 - pos.Theta(), pos.Phi(), thetaphiradius)};
  m_inners["ECAL_thetaphi"] = std::unique_ptr<TEllipse>{new TEllipse(M_PI_2 - pos.Theta(), pos.Phi(), ithetaphiradius)};
  m_contours["HCAL_thetaphi"] =
      std::unique_ptr<TEllipse>{new TEllipse(M_PI_2 - pos.Theta(), pos.Phi(), thetaphiradius)};
  m_inners["HCAL_thetaphi"] = std::unique_ptr<TEllipse>{new TEllipse(M_PI_2 - pos.Theta(), pos.Phi(), ithetaphiradius)};

  // Loop thorugh inners and outers applying settings
  for (auto const& contour : m_contours) {
    contour.second->SetLineColor(4);
    contour.second->SetFillStyle(0);
  }
  for (auto const& inner : m_inners) {
    inner.second->SetFillColor(color);
    inner.second->SetFillStyle(3002);
  }
}

void GBlob::Draw(const std::string& projection, const std::string& opt) const {

  // some things are not sensible so skip these
  if ((m_layer == papas::Layer::kEcal && projection == "HCAL_thetaphi") ||
      (m_layer == papas::Layer::kHcal && projection == "ECAL_thetaphi"))
    return;

  std::string useopt = opt + "psame";

  // find the approprite projection and plot it
  if (m_contours.find(projection) != m_contours.end()) {
    m_contours.at(projection)->Draw(useopt.c_str());  // "at" instead of  []; otherwise fails because of const ness
  }

  // TODOAJR else
  // raise ValueError('implement drawing for projection ' + projection )
  if (m_inners.find(projection) != m_inners.end()) {
    m_inners.at(projection)->Draw(useopt.c_str());
  }
}

// TODO remname graph to line??
GTrajectory::GTrajectory(const std::vector<double>& X, const std::vector<double>& Y, const std::vector<double>& Z,
                         const std::vector<double>& tX, const std::vector<double>& tY, int markerstyle, double markersize,
                         int linestyle, int linecolor, int linewidth) {
  int npoints = X.size();
  // pass the vectors to the various projections
  m_graphs["xy"] = std::unique_ptr<TGraph>{new TGraph(npoints, &X[0], &Y[0])};
  m_graphs["yz"] = std::unique_ptr<TGraph>{new TGraph(npoints, &Z[0], &Y[0])};
  m_graphs["xz"] = std::unique_ptr<TGraph>{new TGraph(npoints, &Z[0], &X[0])};

  m_graphs["ECAL_thetaphi"] = std::unique_ptr<TGraph>{new TGraph(npoints, &tX[0], &tY[0])};
  m_graphs["HCAL_thetaphi"] = std::unique_ptr<TGraph>{new TGraph(npoints, &tX[0], &tY[0])};

  // AJRTODO add in other projections

  // set graph styles
  for (auto const& graph : m_graphs) {
    graph.second->SetMarkerStyle(markerstyle);
    graph.second->SetMarkerSize(markersize);
    graph.second->SetLineStyle(linestyle);
    graph.second->SetLineColor(linecolor);
    graph.second->SetLineWidth(linewidth);
  }
}

/*
GTrajectory::GTrajectory(const  PFParticle& particle, int linestyle,
                         int linecolor) //AJRTODo generalise argumtnet to be a list of things with points
{
  const Path::Points& points = particle.path()->points();
   int npoints = points.size();
   std::vector<double> X;
   std::vector<double> Y;
   std::vector<double> Z;
   std::vector<double> tX; // for thetaphi graphs
   std::vector<double> tY; // for thetaphi graphs

   //Extract vectors of x, y and z values
  int i=0;
      for (auto p : points) {

         X.push_back(p.second.X());
         Y.push_back(p.second.Y());
         Z.push_back(p.second.Z());
        if (i==1) {
         tX.push_back(M_PI_2 - p.second.Theta());
         tY.push_back(p.second.Phi());
        }
        else
        {
          TVector3 vec = particle.p4().Vect();
          tX.push_back(M_PI_2 - vec.Theta());
          tY.push_back(vec.Phi());

        }
        i += 1;
      //first point is wrong and should be tppoint = description.p4().Vect()

        //std::cout << "X " << X[i] << "Y " << Y[i]<< "Z " << Z[i];
   }

   InitGraph(X, Y, Z, tX, tY, linestyle, linecolor)*/

void GTrajectories::addStraight(Path::Ptr path, TVector3 tvec, int linestyle, int linecolor, int linewidth) {
  addNamedPoints(path->points(), tvec, linestyle, linecolor, linewidth);
}

void GTrajectories::addPoints(const std::vector<TVector3>& points, TVector3 tvec, int linestyle, int linecolor,
                              int linewidth) {
  int npoints = points.size();
  std::vector<double> X;
  std::vector<double> Y;
  std::vector<double> Z;
  std::vector<double> tX;  // for thetaphi graphs
  std::vector<double> tY;  // for thetaphi graphs

  // Extract vectors of x, y and z values
  int i = 0;
  for (auto p : points) {

    X.push_back(p.X());
    Y.push_back(p.Y());
    Z.push_back(p.Z());
    if (i == 1) {
      tX.push_back(M_PI_2 - p.Theta());
      tY.push_back(p.Phi());
    } else {
      tX.push_back(M_PI_2 - tvec.Theta());
      tY.push_back(tvec.Phi());
    }
    i += 1;
    // first point is wrong and should be tppoint = description.p4().Vect()
  }
  m_gTrajectories.push_back(GTrajectory(X, Y, Z, tX, tY, 0, 0, linestyle, linecolor, linewidth));
}

void GTrajectories::addNamedPoints(const Path::Points& points, TVector3 tvec, int linestyle, int linecolor,
                                   int linewidth) {
  int npoints = points.size();
  std::vector<double> X;
  std::vector<double> Y;
  std::vector<double> Z;
  std::vector<double> tX;  // for thetaphi graphs
  std::vector<double> tY;  // for thetaphi graphs

  // Extract vectors of x, y and z values
  int i = 0;
  for (auto p : points) {

    X.push_back(p.second.X());
    Y.push_back(p.second.Y());
    Z.push_back(p.second.Z());
    if (i == 1) {
      tX.push_back(M_PI_2 - p.second.Theta());
      tY.push_back(p.second.Phi());
    } else {

      tX.push_back(M_PI_2 - tvec.Theta());
      tY.push_back(tvec.Phi());
    }
    i += 1;
    // first point is wrong and should be tppoint = description.p4().Vect()
  }
  m_gTrajectories.push_back(GTrajectory(X, Y, Z, tX, tY, 2, 0.7, linestyle, linecolor, linewidth));
}

void GTrajectories::addHelix(Path::Ptr path, TVector3 tvec, int linestyle, int linecolor) {

  int npoints = 100;
  std::vector<double> X;
  std::vector<double> Y;
  std::vector<double> Z;
  std::vector<double> tX;  // for thetaphi graphs
  std::vector<double> tY;  // for thetaphi graphs
  std::shared_ptr<Helix> sp_helix = std::dynamic_pointer_cast<Helix>(path);

  double maxTime = sp_helix->maxTime();
  std::vector<TVector3> points;

  for (int i = 0; i < 100; i++) {
    double time = (maxTime / 100) * (i);
    points.push_back(sp_helix->pointAtTime(time));
  }

  addPoints(points, tvec, linestyle, linecolor, 1);
  /*  if abs(self.desc.pdgid()) in [11,13]:
  def set_graph_style(graph):
  graph.SetLineWidth(3)
  graph.SetLineColor(5)
  set_graph_style(self.graphline_xy)
  set_graph_style(self.graphline_xz)
  set_graph_style(self.graphline_yz)
  set_graph_style(self.graphline_thetaphi)*/
  // m_gTrajectories.push_back(GTrajectory(X, Y, Z, tX, tY, linestyle, linecolor));
}

void GTrajectory::setColor(int color) {
  for (auto const& graph : m_graphs) {
    graph.second->SetMarkerColor(color);
  }
}

void GTrajectory::Draw(const std::string& projection /*,
                       const std::string&  opt*/) const {
  std::string usedopt = "lpsame";  // AJRTODO

  m_graphs.at(projection)->Draw(usedopt.c_str());  // "at" instead of  [] otherwise fails because of const ness

  // raise ValueError('implement drawing for projection ' + projection )
};

/*
///Constructor for showing tracks
GTrajectories::GTrajectories(const std::vector<TVector3>& points)
//AJRTODO const std::list<Particle>& particles)
{
   //TrajClass = GTrajectory ; //AJRTODO GStraightTrajectoryif is_neutral else GHelixTrajectory
   m_gTrajectories.push_back(GTrajectory(points));
}*/

GTrajectories::GTrajectories(const PFParticle& particle)
// AJRTODO const std::list<Particle>& particles)
{
  // Path& path= sp.path();

  if (particle.charge() != 0) {
    if (abs(particle.pdgId())==211) {
      addHelix(particle.path(), particle.p4().Vect(), 4, 3);
      addStraight(particle.path(), particle.p4().Vect(), 0, 0, 0);
    }
    else {
      addHelix(particle.path(), particle.p4().Vect(), 1, 7);
      addStraight(particle.path(), particle.p4().Vect(), 0, 0, 0);
    }

  } else {
    if (particle.pdgId()==22)
      addStraight(particle.path(), particle.p4().Vect(), 2, 2, 1);
    else
      addStraight(particle.path(), particle.p4().Vect(), 1, 3, 1);
  }
}

GTrajectories::GTrajectories(const Track& track)
// AJRTODO const std::list<Particle>& particles)
{
  // Path& path= sp.path();

  // std::vector<TVector3>& points= track.get
  // TrajClass = GTrajectory ; //AJRTODO GStraightTrajectoryif is_neutral else GHelixTrajectory
  if (track.charge() != 0) {
    addHelix(track.path(), track.p3(), 1, 1);
    addStraight(track.path(), track.p3(), 0, 0, 0);
  } else
    addStraight(track.path(), track.p3(), 1, 1, 1);
}

/// Constructor for showing clusters
GTrajectories::GTrajectories(const Cluster& cluster) { m_gBlobs.push_back(GBlob(cluster)); }

void GTrajectories::Draw(const std::string& projection) const {
  // draw tracks
  for (auto const& traj : m_gTrajectories) {
    traj.Draw(projection);
  }
  // draw clusters
  for (auto const& traj : m_gBlobs) {
    traj.Draw(projection);
  }
}

}  // end namespace papas