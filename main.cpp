#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/stat.h>

#include "Embed.h"
#include "DataCenter.h"
#include "CentralController.h"
#include "SubstrateInfrastructure.h"
#include "VDCRequestPartitioned.h"
#include "mainSimulator.h"

using namespace std;


int runSimVariateLocationConstraintProbabilityAndArrivalRateParametersFromArgs(int startRun, int endRun, double durationPerRun, int methodInf, int methodSup, int rateInf, int rateSup, int rateInterval, int locationInf, int locationSup, int locationInterval, int carbonCostInf, int carbonCostSup, int costInterval , double reconfigurationIntervalInf, double reconfigurationIntervalSup, double reconfigurationIntervalInterval, double reportingPeriodInf, double reportingPeriodSup, double reportingPeriodInterval);


int main2(int argc, char *argv[]){

	//mainSingleRun();

	//runSimVariateLocationConstraintProbability();

	//runSimVariateLocationConstraintProbabilityAndArrivalRate();

	if(argc != 21){
		cout<< "Oups, incorrect arguments "<< endl;
		cout<< "Usage : firstRun lastRun durationPerRun methodInf methodSup rateInf rateSup rateInterval locationInf locationSup locatInterval carbonCostInf carbonCostSup costInterval" << endl;
	}else{
		clock_t t1=clock();
		//Parse the arguments
		int runStart = atoi(argv[1]);
		int runEnd = atoi(argv[2]);
		double durationPerRun = atoi(argv[3]);
		int methodInf = atoi(argv[4]);
		int methodSup= atoi(argv[5]);

		int rateInf= atoi(argv[6]);
		int rateSup= atoi(argv[7]);
		int rateInterval= atoi(argv[8]);

		int locationInf= atoi(argv[9]);
		int locationSup= atoi(argv[10]);
		int locatInterval = atoi(argv[11]);

		int carbonCostInf= atoi(argv[12]);
		int carbonCostSup= atoi(argv[13]);
		int costInterval = atoi(argv[14]);

		double reconfigurationIntervalInf = atoi(argv[15]);
		double reconfigurationIntervalSup = atoi(argv[16]);
		double reconfigurationIntervalInterval = atoi(argv[17]);

		double reportingPeriodInf = atoi(argv[18]);
		double reportingPeriodSup = atoi(argv[19]);
		double reportingPeriodInterval = atoi(argv[20]);

		clock_t t2=clock();
		//printf("%.4lf seconds of processing\n", (t2-t1)/(double)CLOCKS_PER_SEC);

		runSimVariateLocationConstraintProbabilityAndArrivalRateParametersFromArgs(runStart, runEnd, durationPerRun, methodInf, methodSup, rateInf, rateSup, rateInterval, locationInf, locationSup, locatInterval, carbonCostInf, carbonCostSup, costInterval, reconfigurationIntervalInf, reconfigurationIntervalSup, reconfigurationIntervalInterval, reportingPeriodInf, reportingPeriodSup, reportingPeriodInterval);

		cout<< "The simulations ended successfully, have a look at the result and enjoy making money boss :) "<< endl;

		double dif =  (t2-t1)/(double)CLOCKS_PER_SEC;

		cout<< "The total running time on the server is "<< dif << " seconds" << endl;


	}
}


int runSimVariateLocationConstraintProbabilityAndArrivalRateParametersFromArgs(int startRun, int endRun, double durationPerRun, int methodInf, int methodSup, int rateInf, int rateSup, int rateInterval, int locationInf, int locationSup, int locationInterval, int carbonCostInf, int carbonCostSup, int costInterval, double reconfigurationIntervalInf, double reconfigurationIntervalSup, double reconfigurationIntervalInterval, double reportingPeriodInf, double reportingPeriodSup, double reportingPeriodInterval){


	//int nbRuns = 30;

	//double simPerRun = 3000;

	//Usage nbRuns metInf metSup


	for(int currentRun=startRun; currentRun<endRun; currentRun++){

		for(double rateCount=rateInf; rateCount<=rateSup; rateCount+=rateInterval){//Arrival Rate

			//double arrivaleRate = ((double)rateCount)/(double)100;
			double arrivaleRate = (double)rateCount;

			for(int pLocation=locationInf; pLocation<=locationSup; pLocation+=locationInterval){//Location probability

				double probaLocation = ((double)pLocation)/(double)100;

				for(double costOfCarbon=carbonCostInf; costOfCarbon<=carbonCostSup; costOfCarbon+=costInterval){

					for(double reconfigurationInterval=reconfigurationIntervalInf; reconfigurationInterval<=reconfigurationIntervalSup; reconfigurationInterval+=reconfigurationIntervalInterval){

						for(double reportingPeriod=reportingPeriodInf; reportingPeriod<=reportingPeriodSup; reportingPeriod+=reportingPeriodInterval){

							//Generate the list of events for the different methods

							mainSimulator* simulator = new mainSimulator();

							//General Settings
							simulator->simulationParameters->simDuration = durationPerRun; //In hours
							simulator->seedInitialValue = currentRun;
							simulator->simulationParameters->currentRun = currentRun;

							//Physical Infrastructure
							simulator->centralControllerParmaeters->numberOfDatacenters = 4;
							simulator->centralControllerParmaeters->numberOfNodesProvisionedNetwork = 10;
							simulator->centralControllerParmaeters->physicalLinkProbability = 0.6;
							simulator->centralControllerParmaeters->accessBandwidthForDC = 100000;
							simulator->centralControllerParmaeters->provisionedNetworkLinkCapacity = 100000;
							simulator->centralControllerParmaeters->provisionedNetworkTopology = TEST_TOPOLOGY_SAVI; //NSFNET_TOPOLOGY  RANDOM_TOPOLOGY TEST_TOPOLOGY_SAVI

							//VDC requests setting
							simulator->simulationParameters->arrivalRate = arrivaleRate; //rate gives the requests per hour
							simulator->simulationParameters->meanLifetime = 24; //Life time of the request in hours
							simulator->simulationParameters->lifeTimeDistribution = EXPONENTIAL; //CONSTANT EXPONENTIAL
							simulator->simulationParameters->requestGeneration = RANDOM; //RANDOM, GOOGLE, TEST_TEST
							simulator->simulationParameters->virtualLinkProbability = 0.5;

							simulator->simulationParameters->carbonSLAConstraintDistribution = UNIFORM;
							simulator->simulationParameters->carbonSLAConstraintLimitInf = 0.005; //Tons per day (0.016 ton/12 hours)
							simulator->simulationParameters->carbonSLAConstraintLimitSup = 0.030; //Tons per day

							simulator->simulationParameters->carbonSLAConstraintLimitInf = simulator->simulationParameters->carbonSLAConstraintLimitInf*reportingPeriod/24;
							simulator->simulationParameters->carbonSLAConstraintLimitSup = simulator->simulationParameters->carbonSLAConstraintLimitSup*reportingPeriod/24;

							simulator->simulationParameters->numberOfVirtualNodesDistribution = UNIFORM; //CONSTANT POISSON UNIFORM
							simulator->simulationParameters->minNumberOfVirtualNodes = 5;
							simulator->simulationParameters->maxNumberOfVirtualNodes = 50;

							simulator->simulationParameters->locationConstraintProbability = probaLocation;
							simulator->simulationParameters->carbonCost = costOfCarbon; //30$ per ton of carbon

							simulator->simulationParameters->bandwidthInf = 10;
							simulator->simulationParameters->bandwidthSup = 50;


							simulator->simulationParameters->revenuePerUnitOfCPU = 0.06; //Amazon old generation instances
							simulator->simulationParameters->revenuPerUnitOfBandwidth = 0.000001; //Estimation to align with the cost per hour
							simulator->simulationParameters->revenuPerUnitOfMemory = 0.00;


							//Optimization and reconfiguration
							simulator->centralControllerParmaeters->lookAheadTimeForForcasting = 2; //For hours to make a decision to migrate or not
							simulator->centralControllerParmaeters->reoptimizationInterval = reconfigurationInterval; //For hours to make a decision to migrate or not


							//The Green SLA guarantee time
							simulator->centralControllerParmaeters->performanceEvaluationReportingInterval = (double)5/(double)60; //In hours
							simulator->centralControllerParmaeters->parameterUpdateInterval = 0.33; //In hours, 20 minutes
							simulator->centralControllerParmaeters->SLAServiceGuaranteePeriod = reportingPeriod; // 24 hours
							simulator->centralControllerParmaeters->timeSlotDuration = (double)1/(double)6; //in hours
							//simulator->SLAViolationCostPerUnitOfCarbon = 50; //Violation penalty per unit of Carbon Emission
							simulator->centralControllerParmaeters->SLAViolationProportionOfRevenueToRefund = 0.5; //Proportion of the the revenue of the request (bill) to be given back by the InP



							//The costs parameters
							simulator->centralControllerParmaeters->costPerUnitOfBandwidthInBackboneNetwork = 0.00015; //0.00015 $/Mbps/hour, 0.15 $/Gbps/hour
							simulator->centralControllerParmaeters->costPerOnNodeInProvisionedNetwork = 0.05; //PowerPerNode * Electricty Price
							simulator->centralControllerParmaeters->carbonPerUnitOfBandwidthInBackbone = 0.0000005; // 0.5*e-06
							simulator->centralControllerParmaeters->costPerTonOfCarbon = costOfCarbon; //30$ per ton of carbon

							simulator->centralControllerParmaeters->numberOfAlternativePaths = 5;


							event_t** listEvents = simulator->generateListEventsForSimulation();


							for(int met=methodInf;met<=methodSup; met++){ //1=LOUVAIN , 2=NO_PARTITIONING , 3=LOAD_BALANCING , 4=OPTIMAL

								cout << "*************************************************************************************************************************" << endl;
								cout << "Simulation for method " << met << " Arrival Rate " << arrivaleRate << " Requests/hour, Location probability " << probaLocation << " Run " << currentRun << " The cost per ton of carbon is "<< costOfCarbon<< endl;
								cout << "*************************************************************************************************************************" << endl;

								//cout << "Partitioning the request" << endl;

								cout << "Starting Simulations" << endl;

								switch(met){
								case 1:
									simulator->centralControllerParmaeters->partitioningMethod =  PARTITIONING_LOUVAIN;
									break;

								case 2:
									simulator->centralControllerParmaeters->partitioningMethod =  PARTITIONING_NODE_IS_PARTITION;
									break;

								case 3:
									simulator->centralControllerParmaeters->partitioningMethod = PARTITIONING_EQUAL_PARTITIONS_SIZE;
									break;

								case 4:
									simulator->centralControllerParmaeters->partitioningMethod = PARTITIONING_LOUVAIN_WITH_DYNAMIC_MIGRATION;
									break;

								case 5:
									simulator->centralControllerParmaeters->partitioningMethod = PARTITIONING_LOUVAIN_WITH_DYNAMIC_MIGRATION_AND_ADMISSION_CONTROL;
									break;

								case 6:
									simulator->centralControllerParmaeters->partitioningMethod =  PARTITIONING_TO_SOLVE_TO_OPTIMAL_SOLUTION;
									simulator->centralControllerParmaeters->accessBandwidthForDC = 99999999; //Ignore the capacities of the links in the backbone network
									simulator->centralControllerParmaeters->provisionedNetworkLinkCapacity = 99999999;
									break;

								default:
									cout << "Oooops, there a problem, No method has been provided to solve the problem " << endl;
									break;
								}


								infrastructure_utilization_t* res = new infrastructure_utilization_t;

								cout << "Simulation for method " << met << " Arrival Rate " << simulator->simulationParameters->arrivalRate << " Location probability " << simulator->simulationParameters->locationConstraintProbability << endl;

								res = simulator->startSimulationFromListOfEvents(*listEvents);

								cout << "End Of Simulations at time " << endl;
								cout << "The total cost is " << res->costDatacenters+res->costForUsedNodes << endl;
								cout << "The acceptance ratio is "<< res->acceptanceRatio << endl;


								simulator->cn->writeResultsInFiles(*(simulator->simulationParameters));

								/*
								//Write the results into files
								cout << "Writing the results into files " << endl;
								string base("Results");
								mkdir(base.c_str(), 0777);

								string baseDirectory("Results/ResultsArrivalRate");
								stringstream ss1;
								ss1 << rateCount;
								baseDirectory.append(ss1.str());

								baseDirectory.append("LocationProbability");
								stringstream ss2;
								ss2 << pLocation;//add number to the stream
								baseDirectory.append(ss2.str());

								baseDirectory.append("CarbonCost");
								stringstream ss3;
								ss3 << costOfCarbon;//add number to the stream
								baseDirectory.append(ss3.str());

								baseDirectory.append("ReconfigurationInterval");
								stringstream ss4;
								ss4 << reconfigurationInterval;//add number to the stream
								baseDirectory.append(ss4.str());

								baseDirectory.append("ReportingPeriod");
								stringstream ss5;
								ss5 << reportingPeriod;//add number to the stream
								baseDirectory.append(ss5.str());


								mkdir(baseDirectory.c_str(), 0777);

								baseDirectory.append("/Run");

								stringstream ssss;
								ssss << currentRun;
								baseDirectory.append(ssss.str());

								//CreateDirectory(baseDirectory.c_str(), NULL);
								mkdir(baseDirectory.c_str(), 0777);

								baseDirectory.append("/");



								switch(simulator->centralControllerParmaeters->partitioningMethod){
								case PARTITIONING_NODE_IS_PARTITION :{
									//Every node in the VDC is considered as a partition
									baseDirectory.append("ResultsNoPartitioning/");
								}
								break;

								case PARTITIONING_TO_SOLVE_TO_OPTIMAL_SOLUTION :{
									//Every node in the VDC is considered as a partition

									baseDirectory.append("ResultsOptimalSolution/");
								}
								break;

								case PARTITIONING_LOUVAIN :{
									//Use the LOUVAIN Algorithm
									baseDirectory.append("ResultsLouvain/");
								}
								break;
								case PARTITIONING_LOUVAIN_WITH_DYNAMIC_MIGRATION :{
									//Use the LOUVAIN Algorithm, dynamic migration over time
									baseDirectory.append("ResultsLouvainWithDynamicMigration/");
								}
								break;


								case PARTITIONING_EQUAL_PARTITIONS_SIZE :{
									//Partition into equal size partitions
									baseDirectory.append("ResultsLoadBalancing/");
								}
								break;
								case PARTITIONING_LOUVAIN_WITH_DYNAMIC_MIGRATION_AND_ADMISSION_CONTROL :{
									//Partition into equal size partitions
									baseDirectory.append("ResultsLouvainWithDynamicMigrationAndAdmissionControl/");
								}
								break;



								default:{
									//There is a problem, no partitioning method is specified.
									//Should rise an exception, no partitioning specified
								}
								break;
								}



								mkdir(baseDirectory.c_str(), 0777);

								//Writing the overall parameters
								ofstream outFileAll[27]; //0: Cost in DCs, 1: Cost in Provisioned Network, 2: Acceptance ratio; 3: Average Link Utilization in Provisioned Network
								//mkdir("Results");
								string s = baseDirectory + string("CostInAllDCs");
								outFileAll[0].open(s.c_str());
								//outFileAll[0].open ("Results/CostInAllDCs");

								s = baseDirectory + string("CostForUsedNodesInProvisionedNetwork");
								outFileAll[1].open(s.c_str());

								s = baseDirectory + string("AcceptanceRatio");
								outFileAll[2].open(s.c_str());;

								s = baseDirectory + string("AverageLinkUtilizationInProvisionedNetwork");
								outFileAll[3].open(s.c_str());

								s = baseDirectory + string("CDFLinkUtilizationInProvisionedNetwork");
								outFileAll[4].open(s.c_str());

								s = baseDirectory + string("SumUsedBandwidthInProvisionedNetwork");
								outFileAll[5].open(s.c_str());

								s = baseDirectory + string("CostForUsedBandwidthInProvisionedNetwork");
								outFileAll[6].open(s.c_str());

								s = baseDirectory + string("InstantaneousRevenue");
								outFileAll[7].open(s.c_str());

								s = baseDirectory + string("AmountOfCarbonAllDatacenters");
								outFileAll[8].open(s.c_str());

								s = baseDirectory + string("AmountOfGeneratedRenewablesAllDatacenters");
								outFileAll[9].open(s.c_str());

								s = baseDirectory + string("AveragePartitioningTimeComplexity");
								outFileAll[10].open(s.c_str());

								s = baseDirectory + string("AverageEmbeddingTimeComplexity");
								outFileAll[11].open(s.c_str());

								s = baseDirectory + string("InstantaneousNumberOfEmbeddedRequests");
								outFileAll[12].open(s.c_str());

								s = baseDirectory + string("SoFarNumberOfEmbeddedRequests");
								outFileAll[13].open(s.c_str());

								s = baseDirectory + string("SoFarNumberOfTotalRequests");
								outFileAll[14].open(s.c_str());

								s = baseDirectory + string("InstantaneousCarbonEmissionPerUnitOfVM");
								outFileAll[15].open(s.c_str());

								s = baseDirectory + string("InstantaneousCarbonEmissionPerUnitOfBW");
								outFileAll[16].open(s.c_str());


								s = baseDirectory + string("InstantaneousSLALimitsCarbonEmission");
								outFileAll[17].open(s.c_str());

								s = baseDirectory + string("InstantaneousSLALimitsCarbonEmissionPerUnitOfVM");
								outFileAll[18].open(s.c_str());

								s = baseDirectory + string("InstantaneousSLALimitsCarbonEmissionPerUnitOfBW");
								outFileAll[19].open(s.c_str());

								s = baseDirectory + string("SLAViolationAmountOfCarbon");
								outFileAll[20].open(s.c_str());

								s = baseDirectory + string("SLAViolationCost");
								outFileAll[21].open(s.c_str());

								s = baseDirectory + string("SLAViolationNumberOfRequests");
								outFileAll[22].open(s.c_str());

								s = baseDirectory + string("ReoptimizationSummaryNumberOfRequests");
								outFileAll[23].open(s.c_str());

								s = baseDirectory + string("ReoptimizationSummaryNumberOfMigratedVMs");
								outFileAll[24].open(s.c_str());

								s = baseDirectory + string("ReoptimizationSummaryNumberOfMigratedPartitions");
								outFileAll[25].open(s.c_str());

								s = baseDirectory + string("ReoptimizationSummaryMigrationCost");
								outFileAll[26].open(s.c_str());



								for(unsigned int i=0; i<simulator->cn->wholeInfrastructureOverTime->size(); i++){
									//
									infrastructure_utilization_t* resStruct = simulator->cn->wholeInfrastructureOverTime->at(i);
									outFileAll[0] <<resStruct->tBegin << "\t" << resStruct->costDatacenters << endl;
									outFileAll[1] <<resStruct->tBegin << "\t" << resStruct->costForUsedNodes*simulator->centralControllerParmaeters->costPerOnNodeInProvisionedNetwork << endl;
									outFileAll[2] <<resStruct->tBegin << "\t" << resStruct->acceptanceRatio << endl;
									outFileAll[3] <<resStruct->tBegin << "\t" << resStruct->averageLinkUtilizationInBackbone << endl;
									for(int j=0; j < NUMBER_OF_INTERVALS_CDF ; j++){
										double amount = (j*100)/CDF_STEP_INTERVAL;
										double percentage = resStruct->cdfLinkUtilization[j];
										outFileAll[4] <<resStruct->tBegin << "\t" << amount << "\t" << percentage << endl;
									}
									outFileAll[5] <<resStruct->tBegin << "\t" << resStruct->sumUsedBandwidthInBackbone << endl;

									outFileAll[6] <<resStruct->tBegin << "\t" << resStruct->sumUsedBandwidthInBackbone*simulator->centralControllerParmaeters->costPerUnitOfBandwidthInBackboneNetwork<< endl;
									outFileAll[7] <<resStruct->tBegin << "\t" << resStruct->revenue << endl;
									outFileAll[8] <<resStruct->tBegin << "\t" << resStruct->amountOfCarbon << endl;
									outFileAll[9] <<resStruct->tBegin << "\t" << resStruct->generatedRenewables << endl;

									outFileAll[10] <<resStruct->tBegin << "\t" << resStruct->partitioningTimeComplexityPerRequest << endl;

									outFileAll[11] <<resStruct->tBegin << "\t" << resStruct->embeddingTimeComplexityPerRequest << endl;

									outFileAll[12] <<resStruct->tBegin << "\t" << resStruct->instantaneousNumberOfEmbeddedRequests << endl;

									outFileAll[13] <<resStruct->tBegin << "\t" << resStruct->numberOfEmbeddedRequestsSoFar << endl;

									outFileAll[14] <<resStruct->tBegin << "\t" << resStruct->numberOfTotalRequestsSoFar << endl;

									outFileAll[15] <<resStruct->tBegin << "\t" << resStruct->carbonEmissionPerUnitOfVM << endl;
									outFileAll[16] <<resStruct->tBegin << "\t" << resStruct->carbonEmissionPerUnitOfBandwidth << endl;

									outFileAll[17] <<resStruct->tBegin << "\t" << resStruct->carbonEmissionLimitByAllRequests << endl;
									outFileAll[18] <<resStruct->tBegin << "\t" << resStruct->carbonEmissionLimitPerUnitOfVM << endl;
									outFileAll[19] <<resStruct->tBegin << "\t" << resStruct->carbonEmissionLimitPerUnitOfBandwidth << endl;

								}


								for(unsigned int i=0; i<simulator->cn->SLAviolatioOverTime->size(); i++){
									sla_violation_summary_t* resStruct = simulator->cn->SLAviolatioOverTime->at(i);
									outFileAll[20] <<resStruct->tEnd << "\t" << resStruct->totalCarbonViolation << endl;
									outFileAll[21] <<resStruct->tEnd << "\t" << resStruct->totalPenaltyCostToPay << endl;
									outFileAll[22] <<resStruct->tEnd << "\t" << resStruct->numberOfRequestViolated << endl;
								}

								for(unsigned int i=0; i<simulator->cn->reoptimazationSummariesOverTime->size(); i++){
									reoptimization_summary_t* resStruct = simulator->cn->reoptimazationSummariesOverTime->at(i);
									outFileAll[23] <<resStruct->tBegin << "\t" << resStruct->numberOfDistinctRequests << endl;
									outFileAll[24] <<resStruct->tBegin << "\t" << resStruct->numberOfMigratedVMs << endl;
									outFileAll[25] <<resStruct->tBegin << "\t" << resStruct->numberOfMigratedPartitions << endl;
									outFileAll[26] <<resStruct->tBegin << "\t" << resStruct->migrationCost << endl;
								}


								for(int i=0; i<27; i++){
									outFileAll[i].close();
								}


								//For the different datacenters;
								ofstream outFileDCs[simulator->centralControllerParmaeters->numberOfDatacenters][11]; //[i][] for datacenter i, 0:InstantaneousCost, 1: PUE, 2: ElectricityPrice: 3:CarbonFootprintPerUnitOfPower, 4:CarbonCostPerTon, 5:availableRenawables, numberOfVMs

								for(int i=1; i <= simulator->centralControllerParmaeters->numberOfDatacenters; i++){


									string s0 = baseDirectory + string("InstantaneousCostDataCenter");
									stringstream ss;//create a stringstream
									ss << i;//add number to the stream
									s0.append(ss.str());
									outFileDCs[i-1][0].open(s0.c_str());

									string s1 = baseDirectory + string("PUEDataCenter");
									s1.append(ss.str());
									outFileDCs[i-1][1].open(s1.c_str());

									string s2 = baseDirectory + string("ElectricityPriceDataCenter");
									s2.append(ss.str());
									outFileDCs[i-1][2].open(s2.c_str());

									string s3 = baseDirectory + string("CarbonFootprintPerUnitOfPowerDataCenter");
									s3.append(ss.str());
									outFileDCs[i-1][3].open(s3.c_str());

									string s4 = baseDirectory + string("CarbonCostPerTonDataCenter");
									s4.append(ss.str());
									outFileDCs[i-1][4].open(s4.c_str());

									string s5 = baseDirectory + string("AvailableRenawablesDataCenter");
									s5.append(ss.str());
									outFileDCs[i-1][5].open(s5.c_str());

									string s6 = baseDirectory + string("NumberOfVMsDataCenter");
									s6.append(ss.str());
									outFileDCs[i-1][6].open(s6.c_str());

									string s7 = baseDirectory + string("CostPerUnitOfPowerDataCenter");
									s7.append(ss.str());
									outFileDCs[i-1][7].open(s7.c_str());

									string s8 = baseDirectory + string("UsedPowerDataCenter");
									s8.append(ss.str());
									outFileDCs[i-1][8].open(s8.c_str());

									string s9 = baseDirectory + string("UsedPowerFromTheGridDataCenter");
									s9.append(ss.str());
									outFileDCs[i-1][9].open(s9.c_str());

									string s10 = baseDirectory + string("AmountOfCarbonDataCenter");
									s10.append(ss.str());
									outFileDCs[i-1][10].open(s10.c_str());


								}


								for(unsigned int i=0; i<simulator->cn->datacenterParameterOverTime->size(); i++){
									vector<datacenter_utilization_t*>* DCsForThisInsatant = simulator->cn->datacenterParameterOverTime->at(i);
									//For every datcenter
									for(unsigned int j=0; j<DCsForThisInsatant->size(); j++){

										datacenter_utilization_t* ut = DCsForThisInsatant->at(j);
										outFileDCs[j][0] <<ut->tBegin << "\t" << ut->instantaneousCost << endl;
										outFileDCs[j][1] <<ut->tBegin << "\t" << ut->PUE << endl;
										outFileDCs[j][2] <<ut->tBegin << "\t" << ut->electricityPrice << endl;
										outFileDCs[j][3] <<ut->tBegin << "\t" << ut->carbonCoefficient << endl;
										outFileDCs[j][4] <<ut->tBegin << "\t" << ut->carbonCost << endl;
										outFileDCs[j][5] <<ut->tBegin << "\t" << ut->availableRenwables << endl;
										outFileDCs[j][6] <<ut->tBegin << "\t" << ut->numberOfVMs << endl;
										outFileDCs[j][7] <<ut->tBegin << "\t" << ut->costPerUnitOfPower << endl;
										outFileDCs[j][8] <<ut->tBegin << "\t" << ut->usedPowerIT*ut->PUE << endl;

										double usedFromGrid = ut->usedPowerIT*ut->PUE;
										if(ut->availableRenwables > usedFromGrid){
											usedFromGrid = 0;
										}else{
											usedFromGrid -= ut->availableRenwables;
										}
										outFileDCs[j][9] <<ut->tBegin << "\t" << usedFromGrid << endl;
										outFileDCs[j][10] <<ut->tBegin << "\t" << ut->amountOfCarbon << endl;

									}
								}


								for(int i=0; i<simulator->centralControllerParmaeters->numberOfDatacenters; i++){
									for(int j=0; j<11; j++){
										outFileDCs[i][j].close();
									}
								}

								//End of the test
								cout << "Results were written into files, end of the simulation " << endl;
								//delete simulator;

								*/


							}//End For method
							delete simulator;

						}//End reportingPeriod

					}//End ReconfigurationInterval
				}//End For CarbonCost
			}//End For Location Probability

		}//End For Arrival Rate

	}//END FOR NHMBER of Runs

	return 1;
}


