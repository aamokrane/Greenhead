/*
 * TopologyGenerator.h
 *
 *  Created on: Mar 25, 2014
 *      Author: ahmedamokrane
 */

#ifndef TOPOLOGYGENERATOR_H_
#define TOPOLOGYGENERATOR_H_

/*
 *
 */

#include "SubstrateInfrastructure.h"

class TopologyGenerator {
public:
	TopologyGenerator();
	virtual ~TopologyGenerator();

	static SubstrateInfrastructure* generateTopologySecondVersion(central_controller_parameters_t* centralControllerParams);

	static SubstrateInfrastructure* generateTopologyNSFNET(central_controller_parameters_t* centralControllerParams);

	static SubstrateInfrastructure* generateTopologyTestSAVI(central_controller_parameters_t* centralControllerParams);

	static SubstrateInfrastructure* generateTopology(central_controller_parameters_t* centralControllerParams);

};

#endif /* TOPOLOGYGENERATOR_H_ */
