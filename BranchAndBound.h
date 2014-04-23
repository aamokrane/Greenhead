/*
 * BranchAndBound.h
 *
 *  Created on: May 23, 2013
 *      Author: ahmedamokrane
 */

#ifndef BRANCHANDBOUND_H_
#define BRANCHANDBOUND_H_

#include<vector>
#include "Embed.h"
#include "VDCRequestPartitioned.h"
#include "EmbeddingSolution.h"
#include "SubstrateInfrastructure.h"

/*
 *
 */
class BranchAndBound {
public:
	static vector<EmbeddingSolution*>* generateAllPossibleSolutions(VDCRequestPartitioned* vdcRequestsPartitioned,  SubstrateInfrastructure* instrastructure);

	BranchAndBound();
	virtual ~BranchAndBound();
};

#endif /* BRANCHANDBOUND_H_ */
