// g2o - General Graph Optimization
// Copyright (C) 2011 G. Grisetti, R. Kuemmerle, W. Burgard
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "sensor_odometry3d.h"

#include "g2o/stuff/logger.h"
#include "g2o/types/slam3d/isometry3d_mappings.h"

namespace g2o {
using namespace std;

SensorOdometry3D::SensorOdometry3D(const std::string& name_)
    : BinarySensor<Robot3D, EdgeSE3, WorldObjectSE3>(name_) {
  _information.setIdentity();
  _information *= 100;
  _information(3, 3) = 10000;
  _information(4, 4) = 10000;
  _information(5, 5) = 10000;
  setInformation(_information);
}

void SensorOdometry3D::addNoise(EdgeType* e) {
  EdgeType::ErrorVector noise = _sampler.generateSample();
  EdgeType::Measurement n = internal::fromVectorMQT(noise);
  e->setMeasurement(e->measurement() * n);
  e->setInformation(information());
}

void SensorOdometry3D::sense() {
  if (!robot()) return;

  RobotType* r = dynamic_cast<RobotType*>(robot());
  if (!r) return;

  PoseObject *pprev = 0, *pcurr = 0;
  std::list<PoseObject*>::reverse_iterator it = r->trajectory().rbegin();
  if (it != r->trajectory().rend()) {
    pcurr = *it;
    ++it;
  }
  if (it != r->trajectory().rend()) {
    pprev = *it;
    ++it;
  }
  if (!(pcurr && pprev)) {
    G2O_ERROR("fatal, trajectory empty");
    return;
  }
  _robotPoseObject = pprev;
  EdgeType* e = mkEdge(pcurr);
  if (e) {
    if (graph()) {
      graph()->addEdge(e);
      e->setMeasurementFromState();
      addNoise(e);
    }
  }
  _robotPoseObject = pcurr;
}

}  // namespace g2o
