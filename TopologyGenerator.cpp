/*
 * TopologyGenerator.cpp
 *
 *  Created on: Mar 25, 2014
 *      Author: ahmedamokrane
 */

#include "TopologyGenerator.h"


TopologyGenerator::TopologyGenerator() {
	// TODO Auto-generated constructor stub

}

TopologyGenerator::~TopologyGenerator() {
	// TODO Auto-generated destructor stub
}





SubstrateInfrastructure* TopologyGenerator::generateTopology(central_controller_parameters_t* centralControllerParams){

	SubstrateInfrastructure* result = NULL;

	switch(centralControllerParams->provisionedNetworkTopology){
	case RANDOM_TOPOLOGY:
		result = generateTopologySecondVersion(centralControllerParams);
		break;

	case NSFNET_TOPOLOGY:
		result = generateTopologyNSFNET(centralControllerParams);
		break;

	case TEST_TOPOLOGY_SAVI:
		result = generateTopologyTestSAVI(centralControllerParams);
		break;
	};

	return result;
}



SubstrateInfrastructure* TopologyGenerator::generateTopologyNSFNET(central_controller_parameters_t* centralControllerParams) {

	/*
	 * Remarks:
	 * 		tabElectrictyX: USD/KWh
	 * 		tabSolarTracesX: Watt/square meter
	 * 		tabFPCoeffX: Tons Of Carbon/Watt
	 */



	int numberOfDataCenters = 4;
	int numberOfNodes = 14;


	//New York

	//North Carolina

	//Time is adjusted to be 0 in north Carolina, + 5 in California

	SubstrateInfrastructure* substrateInf = new SubstrateInfrastructure(numberOfNodes, numberOfDataCenters);
	vector<DataCenter*>* dataCenters = new vector<DataCenter*>();
	double tabPUE1[24] = {1.19, 1.195, 1.199, 1.19, 1.2,1.209, 1.22, 1.23, 1.25,1.27,1.26, 1.28, 1.29,1.31,1.32, 1.35,1.33,1.30,1.29, 1.27,1.27,1.26, 1.22, 1.21};

	double tabElectricty1[24] = {0.126, 0.132, 0.144, 0.14, 0.137, 0.135, 0.132, 0.128, 0.129, 0.133, 0.136, 0.124, 0.129, 0.136, 0.145, 0.141, 0.148, 0.153, 0.137, 0.135, 0.132, 0.129, 0.126, 0.125}; //Per KWh
	//double tabFPCoeff[8] = {0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001}; //Ton/KWh (Adjust to 0.00001)
	double tabFPCoeff1[24] = {0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006}; //Ton/KWh
	double tabSolarTraces1[24] = {0.0, 0.0, 0.0, 0.0, 0.0, 42.0, 52.0, 59.0, 150, 315, 440, 503, 530, 471, 369, 210, 55, 0, 0, 0, 0, 0, 0}; //Watt/squareMeter
	//Table of Solar traces gives the possible power per square meter, data from http://rredc.nrel.gov, January 4th 2012

	//double tabRenewables1[24] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1000.0, 20000, 25000, 25500, 33000.0, 40000, 43000, 45000,41000, 43000, 36000, 21000, 5000, 0.0, 0.0, 0.0};

	//double alphaGreen = 0.0127; //Carbon tax
	double alphaGreen = centralControllerParams->costPerTonOfCarbon;



	DataCenter* dc = new DataCenter();
	location_t* locat = new location_t;
	locat->maxDistance = -1;
	locat->xLocation = 0;
	locat->yLocation = 0;
	dc->location = locat;
	dc->nbIntervalsPerDay = 24;
	//dc->squareMeterSolarPanels = 133;
	dc->squareMeterSolarPanels = 650;

	vector<datacenter_parameter_t*>* pue = new vector<datacenter_parameter_t*>();
	vector<datacenter_parameter_t*>* electricty = new vector<datacenter_parameter_t*>();	//The electricity Price ($ per wh)
	vector<datacenter_parameter_t*>* carbonFPCoeff = new vector<datacenter_parameter_t*>(); //The coefficient that gives the amount of Carbon per unit of power (tonne per KWh)
	vector<datacenter_parameter_t*>* carbonFPUnitCost = new vector<datacenter_parameter_t*>();	//The cost per tonne of Carbon ($ per tonne)
	vector<datacenter_parameter_t*>* amountOfRenewables = new vector<datacenter_parameter_t*>(); //The amount of renewables
	for(int j = 0; j<24; j++){
		datacenter_parameter_t* param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd =  j + 0.999;
		param->value = tabPUE1[j];
		pue->push_back(param);

		//ELECTRICTY PRICE
		param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd = (j+1) -1;
		param->value = tabElectricty1[j];
		electricty->push_back(param);

		//AMOUNT OF RENEWABLES
		param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd = (j+1) -1;
		param->value = tabSolarTraces1[j]*dc->squareMeterSolarPanels;
		amountOfRenewables->push_back(param);

		//CARBON COEFFICIENT
		param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd = (j+1) -1;
		param->value = tabFPCoeff1[j]; //Paris 0.001 ton/KWh, Finland 0.002, California 0.003, New York 0.004, Illinois 0.005, North Carolina 0.004
		carbonFPCoeff->push_back(param);

		//CARBON COST
		param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd = (j+1) -1;
		param->value = alphaGreen; //Same Carbon tax or price everywhere
		carbonFPUnitCost->push_back(param);
	}

	dc->amountOfRenewablesOverTime = amountOfRenewables;
	dc->carbonFootPrintCoefficientOverTime = carbonFPCoeff;
	dc->carbonFootPrintUnitCostOverTime = carbonFPUnitCost;
	dc->electrictyPriceOverTime = electricty;
	dc->pueOverTime = pue;

	dataCenters->push_back(dc);


	//California

	double tabPUE2[24] = {1.22, 1.23, 1.25,1.27,1.26, 1.28, 1.29,1.31,1.32, 1.35,1.33,1.30,1.29, 1.27,1.27,1.26, 1.22, 1.21, 1.209, 1.2, 1.19, 1.19, 1.195, 1.199};
	double tabElectricty2[24] = {0.15, 0.137, 0.165, 0.10, 0.192, 0.19, 0.187, 0.1813, 0.18, 0.165, 0.142, 0.14, 0.141, 0.139, 0.143, 0.145, 0.165, 0.17, 0.18, 0.175, 0.159, 0.150, 0.142, 0.1361}; //Per KWh
	//double tabFPCoeff2[24] = {0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003}; //Ton/KWh

	//double tabElectricty2[24] = {0.05, 0.037, 0.065, 0.10, 0.092, 0.09, 0.087, 0.0813, 0.08, 0.065, 0.042, 0.04, 0.041, 0.039, 0.043, 0.045, 0.065, 0.07, 0.08, 0.075, 0.059, 0.050, 0.042, 0.0361}; //Per KWh
	//double tabFPCoeff2[24] = {0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009}; //Ton/KWh


	double tabSolarTraces2[24] = {0, 0, 0, 34, 42, 89, 82.21, 45.0, 0.0, 0.0, 0.0, 0.0, 0.0, 31.0, 150, 315, 440, 503, 530, 471, 369, 210, 55}; //Watt/squareMeter

	//We add some wind to California, during the first hours

	//double tabRenewables2[24] = {0.0, 0.0, 1000.0, 20000, 25000, 25500, 33000.0, 40000, 43000, 45000,41000, 43000, 36000, 21000, 5000, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

	//alphaGreen = 0.0127; //Carbon tax



	dc = new DataCenter();
	locat = new location_t;
	locat->maxDistance = -1;
	locat->xLocation = 0;
	locat->yLocation = 100;
	dc->location = locat;
	dc->nbIntervalsPerDay = 24;
	//dc->squareMeterSolarPanels = 105;
	dc->squareMeterSolarPanels = 650;

	pue = new vector<datacenter_parameter_t*>();
	electricty = new vector<datacenter_parameter_t*>();	//The electricity Price ($ per Kwh)
	carbonFPCoeff = new vector<datacenter_parameter_t*>(); //The coefficient that gives the amount of Carbon per unit of power (tonne per KWh)
	carbonFPUnitCost = new vector<datacenter_parameter_t*>();	//The cost per tonne of Carbon ($ per tonne)
	amountOfRenewables = new vector<datacenter_parameter_t*>(); //The amount of renewables
	for(int j = 0; j<24; j++){
		datacenter_parameter_t* param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd =  j + 0.999;
		param->value = tabPUE2[j];
		pue->push_back(param);

		//ELECTRICTY PRICE
		param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd =  j + 0.999;
		param->value = tabElectricty2[j];
		electricty->push_back(param);

		//AMOUNT OF RENEWABLES
		param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd =  j + 0.999;
		param->value = tabSolarTraces2[j]*dc->squareMeterSolarPanels;
		amountOfRenewables->push_back(param);

		//CARBON COEFFICIENT
		param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd =  j + 0.999;
		param->value = tabFPCoeff1[j]; //Paris 0.001 ton/KWh, Finland 0.002, California 0.003, New York 0.004, Illinois 0.005
		carbonFPCoeff->push_back(param);

		//CARBON COST
		param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd =  j + 0.999;
		param->value = alphaGreen; //Same Carbon tax or price everywhere
		carbonFPUnitCost->push_back(param);
	}

	dc->amountOfRenewablesOverTime = amountOfRenewables;
	dc->carbonFootPrintCoefficientOverTime = carbonFPCoeff;
	dc->carbonFootPrintUnitCostOverTime = carbonFPUnitCost;
	dc->electrictyPriceOverTime = electricty;
	dc->pueOverTime = pue;


	dataCenters->push_back(dc);



	//Illinois

	double tabPUE3[24] = {1.19, 1.195, 1.199, 1.22, 1.23, 1.25,1.27,1.26, 1.28, 1.29,1.31,1.32, 1.35,1.33,1.30,1.29, 1.27,1.27,1.26, 1.22, 1.21, 1.209, 1.2, 1.19};
	double tabElectricty3[24] = {0.123, 0.123, 0.123, 0.124, 0.127, 0.136, 0.145, 0.141, 0.148, 0.142, 0.137, 0.135, 0.132, 0.129, 0.126, 0.125, 0.126, 0.132, 0.144, 0.14, 0.137, 0.135, 0.132, 0.128}; //Per KWh

	//From http://www.powersmartpricing.org/chart/?price_dt=1/24/2013&display=table
	//double tabFPCoeff3[24] = {0.0005, 0.0005, 0.0005, 0.0005, 0.0005, 0.0005, 0.0005, 0.0005, 0.0005, 0.0005, 0.0005, 0.0005, 0.0005, 0.0005, 0.0005, 0.0005, 0.0005, 0.0005, 0.0005, 0.0005, 0.0005, 0.0005, 0.0005, 0.0005}; //Ton/KWh
	//double tabRenewables3[24] = {0.0, 0.0, 0.0, 0.0, 0.0, 1000.0, 20000, 25000, 25500, 33000.0, 40000, 43000, 45000,41000, 43000, 36000, 21000, 5000, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	double tabSolarTraces3[24] = {17, 140, 237, 304, 415, 408, 213, 111, 49, 19, 0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0}; //Renewables of Mississipi from January 2004 (http://rredc.nrel.gov)

	//alphaGreen = 0.0127; //Carbon tax


	dc = new DataCenter();
	locat = new location_t;
	locat->maxDistance = -1;
	locat->xLocation = 100;
	locat->yLocation = 0;
	dc->location = locat;
	dc->nbIntervalsPerDay = 24;
	//dc->squareMeterSolarPanels = 116;
	dc->squareMeterSolarPanels = 650;

	pue = new vector<datacenter_parameter_t*>();
	electricty = new vector<datacenter_parameter_t*>();	//The electricity Price ($ per Kwh)
	carbonFPCoeff = new vector<datacenter_parameter_t*>(); //The coefficient that gives the amount of Carbon per unit of power (tonne per KWh)
	carbonFPUnitCost = new vector<datacenter_parameter_t*>();	//The cost per tonne of Carbon ($ per tonne)
	amountOfRenewables = new vector<datacenter_parameter_t*>(); //The amount of renewables
	for(int j = 0; j<24; j++){
		datacenter_parameter_t* param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd =  j + 0.999;
		param->value = tabPUE3[j];
		pue->push_back(param);

		//ELECTRICTY PRICE
		param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd =  j + 0.999;
		param->value = tabElectricty3[j];
		electricty->push_back(param);

		//AMOUNT OF RENEWABLES
		param = new datacenter_parameter_t;
		param->tBegin = j*60;
		param->tEnd =  j + 0.999;
		param->value = tabSolarTraces3[j]*dc->squareMeterSolarPanels;
		amountOfRenewables->push_back(param);

		//CARBON COEFFICIENT
		param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd =  j + 0.999;
		param->value = tabFPCoeff1[j]; //Paris 0.001 ton/KWh, Finland 0.002, California 0.003, New York 0.004, Illinois 0.005
		carbonFPCoeff->push_back(param);

		//CARBON COST
		param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd =  j + 0.999;
		param->value = alphaGreen; //Same Carbon tax or price everywhere
		carbonFPUnitCost->push_back(param);
	}

	dc->amountOfRenewablesOverTime = amountOfRenewables;
	dc->carbonFootPrintCoefficientOverTime = carbonFPCoeff;
	dc->carbonFootPrintUnitCostOverTime = carbonFPUnitCost;
	dc->electrictyPriceOverTime = electricty;
	dc->pueOverTime = pue;

	dataCenters->push_back(dc);



	//Texas

	double tabPUE4[24] = {1.30,1.29, 1.27,1.27,1.26, 1.22, 1.21, 1.209, 1.2, 1.19, 1.19, 1.195, 1.199, 1.22, 1.23, 1.25,1.27,1.26, 1.28, 1.29,1.31,1.32, 1.35,1.33};
	double tabElectricty4[24] = {0.144, 0.146, 0.145, 0.142, 0.14, 0.138, 0.137, 0.136, 0.138, 0.14, 0.139, 0.143, 0.156, 0.158, 0.157, 0.163, 0.167, 0.163, 0.151, 0.157, 0.151, 0.142, 0.147, 0.145}; //Per KWh (From Qi Paper)
	//double tabFPCoeff4[24] = {0.0007, 0.0007, 0.0007, 0.0007, 0.0007, 0.0007, 0.0007, 0.0007, 0.0007, 0.0007, 0.0007, 0.0007, 0.0007, 0.0007, 0.0007, 0.0007, 0.0007, 0.0007, 0.0007, 0.0007, 0.0007, 0.0007, 0.0007, 0.0007}; //Ton/KWh

	//double tabRenewables4[24] = {41000, 35000, 21300, 29900, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 100.0, 1100.0, 21000, 25500, 25500, 34000.0, 39000, 40000, 40700,40900};
	//double tabSolarTraces4[24] = {140, 237, 304, 415, 234, 108, 81, 49, 19, 0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, 17}; //Renewables of Mississipi from

	double tabSolarTraces4[24] = {35, 65, 82, 75, 42, 23, 0.0, 0.0, 0.0, 0, 17, 140, 237, 304, 415, 234, 108, 81, 49, 19, 0, 0, 0, 13}; //Watt/Square meter, We adjust to Texas Time and add some wind during night


	//alphaGreen = 0.0127; //Carbon tax



	dc = new DataCenter();
	locat = new location_t;
	locat->maxDistance = -1;
	locat->xLocation = 100;
	locat->yLocation = 100;
	dc->location = locat;
	dc->nbIntervalsPerDay = 24;
	//dc->squareMeterSolarPanels = 95;
	dc->squareMeterSolarPanels = 650;

	pue = new vector<datacenter_parameter_t*>();
	electricty = new vector<datacenter_parameter_t*>();	//The electricity Price ($ per Kwh)
	carbonFPCoeff = new vector<datacenter_parameter_t*>(); //The coefficient that gives the amount of Carbon per unit of power (tonne per KWh)
	carbonFPUnitCost = new vector<datacenter_parameter_t*>();	//The cost per tonne of Carbon ($ per tonne)
	amountOfRenewables = new vector<datacenter_parameter_t*>(); //The amount of renewables
	for(int j = 0; j<24; j++){
		datacenter_parameter_t* param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd =  j + 0.999;
		param->value = tabPUE4[j];
		pue->push_back(param);

		//ELECTRICTY PRICE
		param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd =  j + 0.999;
		param->value = tabElectricty4[j];
		electricty->push_back(param);

		//AMOUNT OF RENEWABLES
		param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd =  j + 0.999;
		param->value = tabSolarTraces4[j]*dc->squareMeterSolarPanels;
		amountOfRenewables->push_back(param);

		//CARBON COEFFICIENT
		param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd =  j + 0.999;
		param->value = tabFPCoeff1[j]; //Paris 0.001 ton/KWh, Finland 0.002, California 0.003, New York 0.004, Illinois 0.005
		carbonFPCoeff->push_back(param);

		//CARBON COST
		param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd =  j + 0.999;
		param->value = alphaGreen; //Same Carbon tax or price everywhere
		carbonFPUnitCost->push_back(param);
	}

	dc->amountOfRenewablesOverTime = amountOfRenewables;
	dc->carbonFootPrintCoefficientOverTime = carbonFPCoeff;
	dc->carbonFootPrintUnitCostOverTime = carbonFPUnitCost;
	dc->electrictyPriceOverTime = electricty;
	dc->pueOverTime = pue;

	dataCenters->push_back(dc);


	//Add the datacenters to the list of datacenters

	substrateInf->dataCenters = dataCenters;





	//Generate the links between the nodes;
	vector<vector<double> *>* bw = new vector<vector<double> *>(numberOfDataCenters + numberOfNodes);
	vector<vector<double> *>* availablebw = new vector<vector<double> *>(numberOfDataCenters + numberOfNodes);
	vector<vector<double> *>* dl = new vector<vector<double> *>(numberOfDataCenters + numberOfNodes);
	vector<link_t*>* linksList = new vector<link_t*>();


	for(int i=0; i<numberOfDataCenters+numberOfNodes; i++){
		bw->at(i) = new vector<double>(numberOfDataCenters+numberOfNodes, 0);
		dl->at(i) = new vector<double>(numberOfDataCenters+numberOfNodes, INIFINITY);
		availablebw->at(i) = new vector<double>(numberOfDataCenters+numberOfNodes, 0);
	}
	//double DCToProvisionedBW = 10000.0;

	//Connect Every datacenter to only one provisioned network node

	//Datacenters
	//NC 0,	CA 1, IL 2, TX 3
	//Node in the provisioned Network
	//NY 0, NJ 1, MI 2, PA 3, DC 4, GA 5, IL 06, NE 7, TX 8, CO 9, UT 10, WA 11, CA1 12, CA2 13,
	//NY<-->DC

	int nodeForDC[4] = {0, 12, 6, 8};

	for(int i=0; i<numberOfDataCenters; i++){
		//int i = ;//; Datacenter
		int j = nodeForDC[i] + numberOfDataCenters;
		bw->at(i)->at(j) = centralControllerParams->accessBandwidthForDC;
		availablebw->at(i)->at(j) = centralControllerParams->accessBandwidthForDC;
		dl->at(i)->at(j) = 0.001;

		bw->at(j)->at(i) = centralControllerParams->accessBandwidthForDC;
		availablebw->at(j)->at(i) = centralControllerParams->accessBandwidthForDC;
		dl->at(j)->at(i) = 0.001;

		link_t* l1 = new link_t;
		l1->bw = centralControllerParams->accessBandwidthForDC;
		l1->delay = 0.001;
		l1->nodeFrom = i;
		l1->nodeTo = j;
		link_t* l2 = new link_t;
		l2->bw = centralControllerParams->accessBandwidthForDC;
		l2->delay = 0.001;
		l2->nodeFrom = j;
		l2->nodeTo = i;
		linksList->push_back(l2);

	}

	//Connect the nodes in the provisioned network according to NSF Topology
	int connectivity[14][14];
	for(int i=0; i<14; i++){
		for(int j=0; j<14; j++){
			connectivity[i][j] = 0;
		}
	}

	connectivity[0][2] = 1;
	connectivity[0][3] = 1;
	connectivity[0][4] = 1;

	connectivity[1][2] = 1;
	connectivity[1][3] = 1;
	connectivity[1][4] = 1;

	connectivity[2][10] = 1;

	connectivity[3][5] = 1;
	connectivity[3][6] = 1;

	connectivity[4][8] = 1;

	connectivity[5][8] = 1;

	connectivity[6][7] = 1;
	connectivity[6][11] = 1;

	connectivity[7][9] = 1;

	connectivity[8][9] = 1;
	connectivity[8][13] = 1;

	connectivity[9][10] = 1;

	connectivity[10][12] = 1;

	connectivity[11][12] = 1;
	connectivity[11][13] = 1;

	connectivity[12][13] = 1;

	for(int i=0; i<14; i++){
		for(int j=i+1; j<14; j++){
			int indI = i + numberOfDataCenters;
			int indJ = j + numberOfDataCenters;

			bw->at(indI)->at(indJ) = centralControllerParams->provisionedNetworkLinkCapacity*connectivity[i][j];
			availablebw->at(indI)->at(indJ) = centralControllerParams->provisionedNetworkLinkCapacity*connectivity[i][j];
			dl->at(indI)->at(indJ) = 0.001;

			bw->at(indJ)->at(indI) = centralControllerParams->provisionedNetworkLinkCapacity*connectivity[i][j];
			availablebw->at(indJ)->at(indI) = centralControllerParams->provisionedNetworkLinkCapacity*connectivity[i][j];
			dl->at(indJ)->at(indI) = 0.001;

			if(connectivity[i][j] > 0){
				link_t* l1 = new link_t;
				l1->bw = centralControllerParams->provisionedNetworkLinkCapacity;
				l1->delay = 0.001;
				l1->nodeFrom = indI;
				l1->nodeTo = indJ;
				//l1->numberOfMappings = 0;
				l1->mapping = NULL;
				link_t* l2 = new link_t;
				l2->bw = centralControllerParams->provisionedNetworkLinkCapacity;
				l2->delay = 0.001;
				l2->nodeFrom = indJ;
				l2->nodeTo = indI;
				l2->mapping = NULL;
				linksList->push_back(l2);
				//linksList->push_back(l2);
			}


		}
	}

	substrateInf->links = linksList;
	substrateInf->delayBetweenNodes = dl;
	substrateInf->bandwidthBetweenNodes = bw;
	substrateInf->availableBandwidthBetweenNodes = availablebw;

	return substrateInf;

}



SubstrateInfrastructure* TopologyGenerator::generateTopologyTestSAVI(central_controller_parameters_t* centralControllerParams) {

	/*
	 * Remarks:
	 * 		tabElectrictyX: USD/KWh
	 * 		tabSolarTracesX: Watt/square meter
	 * 		tabFPCoeffX: Tons Of Carbon/Watt
	 */

	int numberOfDataCenters = 2;
	int numberOfNodes = 0; //if 0 it means that the data centers are connected in a mesh fashion
	centralControllerParams->numberOfDatacenters = 2;
	centralControllerParams->numberOfNodesProvisionedNetwork = 0;

	SubstrateInfrastructure* substrateInf = new SubstrateInfrastructure(numberOfNodes, numberOfDataCenters);
	vector<DataCenter*>* dataCenters = new vector<DataCenter*>();


	//North Carolina

	//Time is adjusted to be 0 in north Carolina, + 5 in California

	double tabPUE1[24] = {1.19, 1.195, 1.199, 1.19, 1.2,1.209, 1.22, 1.23, 1.25,1.27,1.26, 1.28, 1.29,1.31,1.32, 1.35,1.33,1.30,1.29, 1.27,1.27,1.26, 1.22, 1.21};

	double tabElectricty1[24] = {0.126, 0.132, 0.144, 0.14, 0.137, 0.135, 0.132, 0.128, 0.129, 0.133, 0.136, 0.124, 0.129, 0.136, 0.145, 0.141, 0.148, 0.153, 0.137, 0.135, 0.132, 0.129, 0.126, 0.125}; //Per KWh
	//double tabFPCoeff[8] = {0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001}; //Ton/KWh (Adjust to 0.00001)
	double tabFPCoeff1[24] = {0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006, 0.0006}; //Ton/KWh
	double tabSolarTraces1[24] = {0.0, 0.0, 0.0, 0.0, 0.0, 42.0, 52.0, 59.0, 150, 315, 440, 503, 530, 471, 369, 210, 55, 0, 0, 0, 0, 0, 0}; //Watt/squareMeter

	double alphaGreen = centralControllerParams->costPerTonOfCarbon;

	DataCenter* dc = new DataCenter();
	location_t* locat = new location_t;
	locat->maxDistance = -1;
	locat->xLocation = 0;
	locat->yLocation = 0;
	dc->location = locat;
	dc->nbIntervalsPerDay = 24;
	//dc->squareMeterSolarPanels = 133;
	dc->squareMeterSolarPanels = 650;

	vector<datacenter_parameter_t*>* pue = new vector<datacenter_parameter_t*>();
	vector<datacenter_parameter_t*>* electricty = new vector<datacenter_parameter_t*>();	//The electricity Price ($ per wh)
	vector<datacenter_parameter_t*>* carbonFPCoeff = new vector<datacenter_parameter_t*>(); //The coefficient that gives the amount of Carbon per unit of power (tonne per KWh)
	vector<datacenter_parameter_t*>* carbonFPUnitCost = new vector<datacenter_parameter_t*>();	//The cost per tonne of Carbon ($ per tonne)
	vector<datacenter_parameter_t*>* amountOfRenewables = new vector<datacenter_parameter_t*>(); //The amount of renewables
	for(int j = 0; j<24; j++){
		datacenter_parameter_t* param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd =  j + 0.999;
		param->value = tabPUE1[j];
		pue->push_back(param);

		//ELECTRICTY PRICE
		param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd = (j+1) -1;
		param->value = tabElectricty1[j];
		electricty->push_back(param);

		//AMOUNT OF RENEWABLES
		param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd = (j+1) -1;
		param->value = tabSolarTraces1[j]*dc->squareMeterSolarPanels;
		amountOfRenewables->push_back(param);

		//CARBON COEFFICIENT
		param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd = (j+1) -1;
		param->value = tabFPCoeff1[j]; //Paris 0.001 ton/KWh, Finland 0.002, California 0.003, New York 0.004, Illinois 0.005, North Carolina 0.004
		carbonFPCoeff->push_back(param);

		//CARBON COST
		param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd = (j+1) -1;
		param->value = alphaGreen; //Same Carbon tax or price everywhere
		carbonFPUnitCost->push_back(param);
	}

	dc->amountOfRenewablesOverTime = amountOfRenewables;
	dc->carbonFootPrintCoefficientOverTime = carbonFPCoeff;
	dc->carbonFootPrintUnitCostOverTime = carbonFPUnitCost;
	dc->electrictyPriceOverTime = electricty;
	dc->pueOverTime = pue;

	//This is a real implemnantion, create a data center client for this one
	cout <<"Create a client for the data center "<<endl;
	DataCenterClient* clientDC = new DataCenterClient("127.0.0.1", DATA_CENTER_PORT_1);
	cout <<"Client created for the data center "<<endl;

	dc->remoteDataCenterClient = clientDC;
	//dc->remoteDataCenterClient = NULL;

	//This data center is implemented in remote SAVI
	dc->dataCenterImplementation = STUB_FOR_REMOTE; //SIMULATION REAL_DEPLOYMENT

	dataCenters->push_back(dc);


	//California

	double tabPUE2[24] = {1.22, 1.23, 1.25,1.27,1.26, 1.28, 1.29,1.31,1.32, 1.35,1.33,1.30,1.29, 1.27,1.27,1.26, 1.22, 1.21, 1.209, 1.2, 1.19, 1.19, 1.195, 1.199};
	double tabElectricty2[24] = {0.15, 0.137, 0.165, 0.10, 0.192, 0.19, 0.187, 0.1813, 0.18, 0.165, 0.142, 0.14, 0.141, 0.139, 0.143, 0.145, 0.165, 0.17, 0.18, 0.175, 0.159, 0.150, 0.142, 0.1361}; //Per KWh
	//double tabFPCoeff2[24] = {0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003, 0.0003}; //Ton/KWh

	//double tabElectricty2[24] = {0.05, 0.037, 0.065, 0.10, 0.092, 0.09, 0.087, 0.0813, 0.08, 0.065, 0.042, 0.04, 0.041, 0.039, 0.043, 0.045, 0.065, 0.07, 0.08, 0.075, 0.059, 0.050, 0.042, 0.0361}; //Per KWh
	//double tabFPCoeff2[24] = {0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009, 0.0009}; //Ton/KWh


	double tabSolarTraces2[24] = {0, 0, 0, 34, 42, 89, 82.21, 45.0, 0.0, 0.0, 0.0, 0.0, 0.0, 31.0, 150, 315, 440, 503, 530, 471, 369, 210, 55}; //Watt/squareMeter

	//We add some wind to California, during the first hours

	//double tabRenewables2[24] = {0.0, 0.0, 1000.0, 20000, 25000, 25500, 33000.0, 40000, 43000, 45000,41000, 43000, 36000, 21000, 5000, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

	//alphaGreen = 0.0127; //Carbon tax



	dc = new DataCenter();
	locat = new location_t;
	locat->maxDistance = -1;
	locat->xLocation = 0;
	locat->yLocation = 100;
	dc->location = locat;
	dc->nbIntervalsPerDay = 24;
	//dc->squareMeterSolarPanels = 105;
	dc->squareMeterSolarPanels = 650;

	pue = new vector<datacenter_parameter_t*>();
	electricty = new vector<datacenter_parameter_t*>();	//The electricity Price ($ per Kwh)
	carbonFPCoeff = new vector<datacenter_parameter_t*>(); //The coefficient that gives the amount of Carbon per unit of power (tonne per KWh)
	carbonFPUnitCost = new vector<datacenter_parameter_t*>();	//The cost per tonne of Carbon ($ per tonne)
	amountOfRenewables = new vector<datacenter_parameter_t*>(); //The amount of renewables
	for(int j = 0; j<24; j++){
		datacenter_parameter_t* param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd =  j + 0.999;
		param->value = tabPUE2[j];
		pue->push_back(param);

		//ELECTRICTY PRICE
		param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd =  j + 0.999;
		param->value = tabElectricty2[j];
		electricty->push_back(param);

		//AMOUNT OF RENEWABLES
		param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd =  j + 0.999;
		param->value = tabSolarTraces2[j]*dc->squareMeterSolarPanels;
		amountOfRenewables->push_back(param);

		//CARBON COEFFICIENT
		param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd =  j + 0.999;
		param->value = tabFPCoeff1[j]; //Paris 0.001 ton/KWh, Finland 0.002, California 0.003, New York 0.004, Illinois 0.005
		carbonFPCoeff->push_back(param);

		//CARBON COST
		param = new datacenter_parameter_t;
		param->tBegin = j;
		param->tEnd =  j + 0.999;
		param->value = alphaGreen; //Same Carbon tax or price everywhere
		carbonFPUnitCost->push_back(param);
	}

	dc->amountOfRenewablesOverTime = amountOfRenewables;
	dc->carbonFootPrintCoefficientOverTime = carbonFPCoeff;
	dc->carbonFootPrintUnitCostOverTime = carbonFPUnitCost;
	dc->electrictyPriceOverTime = electricty;
	dc->pueOverTime = pue;

	//This data center is simulated
	dc->dataCenterImplementation = SIMULATED;
	dc->remoteDataCenterClient = NULL;

	dataCenters->push_back(dc);


	//Add the datacenters to the list of datacenters

	substrateInf->dataCenters = dataCenters;


	//Generate the links between the nodes;
	vector<vector<double> *>* bw = new vector<vector<double> *>(numberOfDataCenters + numberOfNodes);
	vector<vector<double> *>* availablebw = new vector<vector<double> *>(numberOfDataCenters + numberOfNodes);
	vector<vector<double> *>* dl = new vector<vector<double> *>(numberOfDataCenters + numberOfNodes);
	vector<link_t*>* linksList = new vector<link_t*>();


	for(int i=0; i<numberOfDataCenters+numberOfNodes; i++){
		bw->at(i) = new vector<double>(numberOfDataCenters+numberOfNodes, 0);
		dl->at(i) = new vector<double>(numberOfDataCenters+numberOfNodes, INIFINITY);
		availablebw->at(i) = new vector<double>(numberOfDataCenters+numberOfNodes, 0);
	}

	//Connect Every data center to the neighbors in a full mesh manner

	for(int i=0; i<numberOfDataCenters; i++){
		for(int j=0; j<numberOfDataCenters; j++){
			//int i = ;//; Datacenter
			bw->at(i)->at(j) = centralControllerParams->accessBandwidthForDC;
			availablebw->at(i)->at(j) = centralControllerParams->accessBandwidthForDC;
			dl->at(i)->at(j) = 0.001;

			bw->at(j)->at(i) = centralControllerParams->accessBandwidthForDC;
			availablebw->at(j)->at(i) = centralControllerParams->accessBandwidthForDC;
			dl->at(j)->at(i) = 0.001;

			link_t* l1 = new link_t;
			l1->bw = centralControllerParams->accessBandwidthForDC;
			l1->delay = 0.001;
			l1->nodeFrom = i;
			l1->nodeTo = j;
			link_t* l2 = new link_t;
			l2->bw = centralControllerParams->accessBandwidthForDC;
			l2->delay = 0.001;
			l2->nodeFrom = j;
			l2->nodeTo = i;
			linksList->push_back(l2);
		}

	}

	substrateInf->links = linksList;
	substrateInf->delayBetweenNodes = dl;
	substrateInf->bandwidthBetweenNodes = bw;
	substrateInf->availableBandwidthBetweenNodes = availablebw;

	return substrateInf;

}


SubstrateInfrastructure* TopologyGenerator::generateTopologySecondVersion(central_controller_parameters_t* centralControllerParams){


	/*
	 * Remarks:
	 * 		tabElectrictyX: USD/KWh
	 * 		tabSolarTracesX: Watt/square meter
	 * 		tabFPCoeffX: Tons Of Carbon/Watt
	 */


	//Time is adjusted to be 0 in north Carolina, + 5 in California

	SubstrateInfrastructure* substrateInf = new SubstrateInfrastructure(centralControllerParams->numberOfNodesProvisionedNetwork, centralControllerParams->numberOfDatacenters);
	vector<DataCenter*>* dataCenters = new vector<DataCenter*>();
	double tabPUE1[24] = {1.19, 1.195, 1.199, 1.19, 1.2,1.209, 1.22, 1.23, 1.25,1.27,1.26, 1.28, 1.29,1.31,1.32, 1.35,1.33,1.30,1.29, 1.27,1.27,1.26, 1.22, 1.21};


	double tabElectricty1[24] = {0.15, 0.15, 0.18, 0.18, 0.18, 0.18, 0.21, 0.20, 0.15, 0.15, 0.18, 0.18, 0.18, 0.18, 0.21, 0.20, 0.15, 0.15, 0.18, 0.18, 0.18, 0.18, 0.21, 0.20}; //Per KWh
	//double tabFPCoeff[8] = {0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001}; //Ton/KWh (Adjust to 0.00001)
	double tabFPCoeff1[24] = {0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001}; //Ton/KWh
	double tabSolarTraces1[24] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 31.0, 150, 315, 440, 503, 530, 471, 369, 210, 55, 0, 0, 0, 0, 0, 0}; //Watt/squareMeter
	//Table of Solar traces gives the possible power per square meter, data from http://rredc.nrel.gov, January 4th 2012

	//double tabRenewables1[24] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1000.0, 20000, 25000, 25500, 33000.0, 40000, 43000, 45000,41000, 43000, 36000, 21000, 5000, 0.0, 0.0, 0.0};

	double alphaGreen = 0.0127; //Carbon tax


	//4 Datacenters

	//New York

	//North Carolina

	DataCenter* dc = new DataCenter();
	location_t* locat = new location_t;
	locat->maxDistance = -1;
	locat->xLocation = 0;
	locat->yLocation = 0;
	dc->location = locat;
	dc->nbIntervalsPerDay = 24;
	dc->squareMeterSolarPanels = 85;

	vector<datacenter_parameter_t*>* pue = new vector<datacenter_parameter_t*>();
	vector<datacenter_parameter_t*>* electricty = new vector<datacenter_parameter_t*>();	//The electricity Price ($ per Kwh)
	vector<datacenter_parameter_t*>* carbonFPCoeff = new vector<datacenter_parameter_t*>(); //The coefficient that gives the amount of Carbon per unit of power (tonne per KWh)
	vector<datacenter_parameter_t*>* carbonFPUnitCost = new vector<datacenter_parameter_t*>();	//The cost per tonne of Carbon ($ per tonne)
	vector<datacenter_parameter_t*>* amountOfRenewables = new vector<datacenter_parameter_t*>(); //The amount of renewables
	for(int j = 0; j<24; j++){
		datacenter_parameter_t* param = new datacenter_parameter_t;
		param->tBegin = j*60;
		param->tEnd = (j+1)*60 -1;
		param->value = tabPUE1[j];
		pue->push_back(param);

		//ELECTRICTY PRICE
		param = new datacenter_parameter_t;
		param->tBegin = j*60;
		param->tEnd = (j+1)*60 -1;
		param->value = tabElectricty1[j];
		electricty->push_back(param);

		//AMOUNT OF RENEWABLES
		param = new datacenter_parameter_t;
		param->tBegin = j*60;
		param->tEnd = (j+1)*60 -1;
		param->value = tabSolarTraces1[j]*dc->squareMeterSolarPanels;
		amountOfRenewables->push_back(param);

		//CARBON COEFFICIENT
		param = new datacenter_parameter_t;
		param->tBegin = j*60;
		param->tEnd = (j+1)*60 -1;
		param->value = tabFPCoeff1[j]*4; //Paris 0.001 ton/KWh, Finland 0.002, California 0.003, New York 0.004, Illinois 0.005, North Carolina 0.004
		carbonFPCoeff->push_back(param);

		//CARBON COST
		param = new datacenter_parameter_t;
		param->tBegin = j*60;
		param->tEnd = (j+1)*60 -1;
		param->value = alphaGreen; //Same Carbon tax or price everywhere
		carbonFPUnitCost->push_back(param);
	}

	dc->amountOfRenewablesOverTime = amountOfRenewables;
	dc->carbonFootPrintCoefficientOverTime = carbonFPCoeff;
	dc->carbonFootPrintUnitCostOverTime = carbonFPUnitCost;
	dc->electrictyPriceOverTime = electricty;
	dc->pueOverTime = pue;

	dataCenters->push_back(dc);


	//California

	double tabPUE2[24] = {1.22, 1.23, 1.25,1.27,1.26, 1.28, 1.29,1.31,1.32, 1.35,1.33,1.30,1.29, 1.27,1.27,1.26, 1.22, 1.21, 1.209, 1.2, 1.19, 1.19, 1.195, 1.199};
	double tabElectricty2[24] = {0.15, 0.15, 0.18, 0.18, 0.18, 0.18, 0.21, 0.20, 0.15, 0.15, 0.18, 0.18, 0.18, 0.18, 0.21, 0.20, 0.15, 0.15, 0.18, 0.18, 0.18, 0.18, 0.21, 0.20}; //Per KWh
	//double tabFPCoeff[8] = {0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001}; //Ton/KWh (Adjust to 0.00001)
	double tabFPCoeff2[24] = {0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001}; //Ton/KWh
	double tabSolarTraces2[24] = {0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 31.0, 150, 315, 440, 503, 530, 471, 369, 210, 55}; //Watt/squareMeter


	//double tabRenewables2[24] = {0.0, 0.0, 1000.0, 20000, 25000, 25500, 33000.0, 40000, 43000, 45000,41000, 43000, 36000, 21000, 5000, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

	alphaGreen = 0.0127; //Carbon tax



	dc = new DataCenter();
	locat = new location_t;
	locat->maxDistance = -1;
	locat->xLocation = 0;
	locat->yLocation = 100;
	dc->location = locat;
	dc->nbIntervalsPerDay = 24;
	dc->squareMeterSolarPanels = 85;

	pue = new vector<datacenter_parameter_t*>();
	electricty = new vector<datacenter_parameter_t*>();	//The electricity Price ($ per Kwh)
	carbonFPCoeff = new vector<datacenter_parameter_t*>(); //The coefficient that gives the amount of Carbon per unit of power (tonne per KWh)
	carbonFPUnitCost = new vector<datacenter_parameter_t*>();	//The cost per tonne of Carbon ($ per tonne)
	amountOfRenewables = new vector<datacenter_parameter_t*>(); //The amount of renewables
	for(int j = 0; j<24; j++){
		datacenter_parameter_t* param = new datacenter_parameter_t;
		param->tBegin = j*60;
		param->tEnd = (j+1)*60 -1;
		param->value = tabPUE2[j];
		pue->push_back(param);

		//ELECTRICTY PRICE
		param = new datacenter_parameter_t;
		param->tBegin = j*60;
		param->tEnd = (j+1)*60 -1;
		param->value = tabElectricty2[j];
		electricty->push_back(param);

		//AMOUNT OF RENEWABLES
		param = new datacenter_parameter_t;
		param->tBegin = j*60;
		param->tEnd = (j+1)*60 -1;
		param->value = tabSolarTraces2[j]*dc->squareMeterSolarPanels;
		amountOfRenewables->push_back(param);

		//CARBON COEFFICIENT
		param = new datacenter_parameter_t;
		param->tBegin = j*60;
		param->tEnd = (j+1)*60 -1;
		param->value = tabFPCoeff2[j]*3; //Paris 0.001 ton/KWh, Finland 0.002, California 0.003, New York 0.004, Illinois 0.005
		carbonFPCoeff->push_back(param);

		//CARBON COST
		param = new datacenter_parameter_t;
		param->tBegin = j*60;
		param->tEnd = (j+1)*60 -1;
		param->value = alphaGreen; //Same Carbon tax or price everywhere
		carbonFPUnitCost->push_back(param);
	}

	dc->amountOfRenewablesOverTime = amountOfRenewables;
	dc->carbonFootPrintCoefficientOverTime = carbonFPCoeff;
	dc->carbonFootPrintUnitCostOverTime = carbonFPUnitCost;
	dc->electrictyPriceOverTime = electricty;
	dc->pueOverTime = pue;


	dataCenters->push_back(dc);

	//Illinois

	double tabPUE3[24] = {1.19, 1.195, 1.199, 1.22, 1.23, 1.25,1.27,1.26, 1.28, 1.29,1.31,1.32, 1.35,1.33,1.30,1.29, 1.27,1.27,1.26, 1.22, 1.21, 1.209, 1.2, 1.19};
	double tabElectricty3[24] = {0.18, 0.21, 0.20, 0.15, 0.15, 0.18, 0.18, 0.18, 0.18, 0.21, 0.20, 0.15, 0.15, 0.18, 0.18, 0.18, 0.18, 0.21, 0.20, 0.15, 0.15, 0.18, 0.18, 0.18}; //Per KWh
	//double tabFPCoeff[8] = {0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001}; //Ton/KWh (Adjust to 0.00001)
	double tabFPCoeff3[24] = {0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001}; //Ton/KWh
	//double tabRenewables3[24] = {0.0, 0.0, 0.0, 0.0, 0.0, 1000.0, 20000, 25000, 25500, 33000.0, 40000, 43000, 45000,41000, 43000, 36000, 21000, 5000, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	double tabSolarTraces3[24] = {0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, 17, 140, 237, 304, 415, 234, 108, 81, 49, 19, 0, 0, 0, 0, 0}; //Renewables of Mississipi from January 2004 (http://rredc.nrel.gov)

	alphaGreen = 0.0127; //Carbon tax


	dc = new DataCenter();
	locat = new location_t;
	locat->maxDistance = -1;
	locat->xLocation = 100;
	locat->yLocation = 0;
	dc->location = locat;
	dc->nbIntervalsPerDay = 24;
	dc->squareMeterSolarPanels = 85;

	pue = new vector<datacenter_parameter_t*>();
	electricty = new vector<datacenter_parameter_t*>();	//The electricity Price ($ per Kwh)
	carbonFPCoeff = new vector<datacenter_parameter_t*>(); //The coefficient that gives the amount of Carbon per unit of power (tonne per KWh)
	carbonFPUnitCost = new vector<datacenter_parameter_t*>();	//The cost per tonne of Carbon ($ per tonne)
	amountOfRenewables = new vector<datacenter_parameter_t*>(); //The amount of renewables
	for(int j = 0; j<24; j++){
		datacenter_parameter_t* param = new datacenter_parameter_t;
		param->tBegin = j*60;
		param->tEnd = (j+1)*60 -1;
		param->value = tabPUE3[j];
		pue->push_back(param);

		//ELECTRICTY PRICE
		param = new datacenter_parameter_t;
		param->tBegin = j*60;
		param->tEnd = (j+1)*60 -1;
		param->value = tabElectricty3[j];
		electricty->push_back(param);

		//AMOUNT OF RENEWABLES
		param = new datacenter_parameter_t;
		param->tBegin = j*60;
		param->tEnd = (j+1)*60 -1;
		param->value = tabSolarTraces3[j]*dc->squareMeterSolarPanels;
		amountOfRenewables->push_back(param);

		//CARBON COEFFICIENT
		param = new datacenter_parameter_t;
		param->tBegin = j*60;
		param->tEnd = (j+1)*60 -1;
		param->value = tabFPCoeff3[j]*5; //Paris 0.001 ton/KWh, Finland 0.002, California 0.003, New York 0.004, Illinois 0.005
		carbonFPCoeff->push_back(param);

		//CARBON COST
		param = new datacenter_parameter_t;
		param->tBegin = j*60;
		param->tEnd = (j+1)*60 -1;
		param->value = alphaGreen; //Same Carbon tax or price everywhere
		carbonFPUnitCost->push_back(param);
	}

	dc->amountOfRenewablesOverTime = amountOfRenewables;
	dc->carbonFootPrintCoefficientOverTime = carbonFPCoeff;
	dc->carbonFootPrintUnitCostOverTime = carbonFPUnitCost;
	dc->electrictyPriceOverTime = electricty;
	dc->pueOverTime = pue;

	dataCenters->push_back(dc);



	//Paris

	double tabPUE4[24] = {1.30,1.29, 1.27,1.27,1.26, 1.22, 1.21, 1.209, 1.2, 1.19, 1.19, 1.195, 1.199, 1.22, 1.23, 1.25,1.27,1.26, 1.28, 1.29,1.31,1.32, 1.35,1.33,};
	double tabElectricty4[24] = {0.18, 0.18, 0.18, 0.21, 0.20, 0.15, 0.15, 0.18, 0.18, 0.18, 0.18, 0.21, 0.20, 0.15, 0.15, 0.18, 0.18, 0.18, 0.18, 0.21, 0.20, 0.15, 0.15, 0.18}; //Per KWh
	//double tabFPCoeff[8] = {0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001}; //Ton/KWh (Adjust to 0.00001)
	double tabFPCoeff4[24] = {0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001, 0.00001}; //Ton/KWh
	//double tabRenewables4[24] = {41000, 35000, 21300, 29900, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 100.0, 1100.0, 21000, 25500, 25500, 34000.0, 39000, 40000, 40700,40900};
	double tabSolarTraces4[24] = {0, 17, 140, 237, 304, 415, 234, 108, 81, 49, 19, 0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0}; //Renewables of Mississipi from



	alphaGreen = 0.0127; //Carbon tax



	dc = new DataCenter();
	locat = new location_t;
	locat->maxDistance = -1;
	locat->xLocation = 100;
	locat->yLocation = 100;
	dc->location = locat;
	dc->nbIntervalsPerDay = 24;
	dc->squareMeterSolarPanels = 85;

	pue = new vector<datacenter_parameter_t*>();
	electricty = new vector<datacenter_parameter_t*>();	//The electricity Price ($ per Kwh)
	carbonFPCoeff = new vector<datacenter_parameter_t*>(); //The coefficient that gives the amount of Carbon per unit of power (tonne per KWh)
	carbonFPUnitCost = new vector<datacenter_parameter_t*>();	//The cost per tonne of Carbon ($ per tonne)
	amountOfRenewables = new vector<datacenter_parameter_t*>(); //The amount of renewables
	for(int j = 0; j<24; j++){
		datacenter_parameter_t* param = new datacenter_parameter_t;
		param->tBegin = j*60;
		param->tEnd = (j+1)*60 -1;
		param->value = tabPUE4[j];
		pue->push_back(param);

		//ELECTRICTY PRICE
		param = new datacenter_parameter_t;
		param->tBegin = j*60;
		param->tEnd = (j+1)*60 -1;
		param->value = tabElectricty4[j];
		electricty->push_back(param);

		//AMOUNT OF RENEWABLES
		param = new datacenter_parameter_t;
		param->tBegin = j*60;
		param->tEnd = (j+1)*60 -1;
		param->value = tabSolarTraces4[j]*dc->squareMeterSolarPanels;
		amountOfRenewables->push_back(param);

		//CARBON COEFFICIENT
		param = new datacenter_parameter_t;
		param->tBegin = j*60;
		param->tEnd = (j+1)*60 -1;
		param->value = tabFPCoeff4[j]*1; //Paris 0.001 ton/KWh, Finland 0.002, California 0.003, New York 0.004, Illinois 0.005
		carbonFPCoeff->push_back(param);

		//CARBON COST
		param = new datacenter_parameter_t;
		param->tBegin = j*60;
		param->tEnd = (j+1)*60 -1;
		param->value = alphaGreen; //Same Carbon tax or price everywhere
		carbonFPUnitCost->push_back(param);
	}

	dc->amountOfRenewablesOverTime = amountOfRenewables;
	dc->carbonFootPrintCoefficientOverTime = carbonFPCoeff;
	dc->carbonFootPrintUnitCostOverTime = carbonFPUnitCost;
	dc->electrictyPriceOverTime = electricty;
	dc->pueOverTime = pue;

	dataCenters->push_back(dc);


	//Add the datacenters to the list of datacenters

	substrateInf->dataCenters = dataCenters;

	//Generate the links between the nodes;
	vector<vector<double> *>* bw = new vector<vector<double> *>(centralControllerParams->numberOfDatacenters + centralControllerParams->numberOfNodesProvisionedNetwork);
	vector<vector<double> *>* availablebw = new vector<vector<double> *>(centralControllerParams->numberOfDatacenters + centralControllerParams->numberOfNodesProvisionedNetwork);
	vector<vector<double> *>* dl = new vector<vector<double> *>(centralControllerParams->numberOfDatacenters + centralControllerParams->numberOfNodesProvisionedNetwork);
	vector<link_t*>* linksList = new vector<link_t*>();


	for(int i=0; i<centralControllerParams->numberOfDatacenters + centralControllerParams->numberOfNodesProvisionedNetwork; i++){
		bw->at(i) = new vector<double>(centralControllerParams->numberOfDatacenters + centralControllerParams->numberOfNodesProvisionedNetwork, 0);
		dl->at(i) = new vector<double>(centralControllerParams->numberOfDatacenters + centralControllerParams->numberOfNodesProvisionedNetwork, INIFINITY);
		availablebw->at(i) = new vector<double>(centralControllerParams->numberOfDatacenters + centralControllerParams->numberOfNodesProvisionedNetwork, 0);
	}

	//double DCToProvisionedBW = 10000.0;
	double r;
	//Connect Every datacenter to only one provisioned network node
	for(int i=0; i<centralControllerParams->numberOfDatacenters; i++){
		r = ((double)rand())/RAND_MAX;
		int j = r*centralControllerParams->numberOfNodesProvisionedNetwork;
		j += centralControllerParams->numberOfDatacenters;
		bw->at(i)->at(j) = centralControllerParams->accessBandwidthForDC;
		availablebw->at(i)->at(j) = centralControllerParams->accessBandwidthForDC;
		dl->at(i)->at(j) = 0.001;

		bw->at(j)->at(i) = centralControllerParams->accessBandwidthForDC;
		availablebw->at(j)->at(i) = centralControllerParams->accessBandwidthForDC;
		dl->at(j)->at(i) = 0.001;

		link_t* l1 = new link_t;
		l1->bw = centralControllerParams->accessBandwidthForDC;
		l1->delay = 0.001;
		l1->nodeFrom = i;
		l1->nodeTo = j;
		link_t* l2 = new link_t;
		l2->bw = centralControllerParams->accessBandwidthForDC;
		l2->delay = 0.001;
		l2->nodeFrom = j;
		l2->nodeTo = i;
		linksList->push_back(l2);
	}

	for(int i=centralControllerParams->numberOfDatacenters; i<centralControllerParams->numberOfDatacenters+centralControllerParams->numberOfNodesProvisionedNetwork; i++){
		for(int j=i+1; j<centralControllerParams->numberOfDatacenters+centralControllerParams->numberOfNodesProvisionedNetwork; j++){

			if(i>=centralControllerParams->numberOfDatacenters || j>=centralControllerParams->numberOfDatacenters){
				//No direct links between datacenters
				double r= ((double)rand())/RAND_MAX;
				if(r <= centralControllerParams->physicalLinkProbability){
					bw->at(i)->at(j) = centralControllerParams->provisionedNetworkLinkCapacity;
					availablebw->at(i)->at(j) = centralControllerParams->provisionedNetworkLinkCapacity;
					dl->at(i)->at(j) = 0.001;

					bw->at(j)->at(i) = centralControllerParams->provisionedNetworkLinkCapacity;
					availablebw->at(j)->at(i) = centralControllerParams->provisionedNetworkLinkCapacity;
					dl->at(j)->at(i) = 0.001;

					link_t* l1 = new link_t;
					l1->bw = centralControllerParams->provisionedNetworkLinkCapacity;
					l1->delay = 0.001;
					l1->nodeFrom = i;
					l1->nodeTo = j;
					link_t* l2 = new link_t;
					l2->bw = centralControllerParams->provisionedNetworkLinkCapacity;
					l2->delay = 0.001;
					l2->nodeFrom = j;
					l2->nodeTo = i;
					linksList->push_back(l2);
					//linksList->push_back(l2);
				}
			}
		}//END FOR J
	}//END FOR I

	substrateInf->links = linksList;
	substrateInf->delayBetweenNodes = dl;
	substrateInf->bandwidthBetweenNodes = bw;
	substrateInf->availableBandwidthBetweenNodes = availablebw;

	return substrateInf;

}

