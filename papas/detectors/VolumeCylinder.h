//
//  Created by Alice Robson on 29/11/15.
//
//
#ifndef VOLUMECYLINDER_H
#define VOLUMECYLINDER_H

#include "TVector3.h"
#include "papas/datatypes/Definitions.h"
#include "papas/detectors/SurfaceCylinder.h"

namespace papas {

/**
  Defines inner and outer cyclinders of a detector element
 */
class VolumeCylinder {
public:
  VolumeCylinder(papas::Layer layer, double outerrad, double outerz, double innerrad = 0., double innerz = 0.);
  ~VolumeCylinder();
  bool contains(const TVector3& point) const;
  const SurfaceCylinder& inner() const { return m_inner; }        ///< inner cyclinder of volume
  const SurfaceCylinder& outer() const { return m_outer; }        ///< outer cyclinder of volume
  papas::Position innerLayer() const { return m_inner.layer(); }  /// enum describing layer of inner cyclinder

private:
  SurfaceCylinder m_outer;
  SurfaceCylinder m_inner;
};
}  // end namespace papas
#endif
