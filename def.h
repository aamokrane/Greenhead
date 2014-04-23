#ifndef DEF_H
#define DEF_H

//#define OPTION_ALLOW_MIGRATION

#define MAX_REQUESTS 15000

#define MAX_REQ_NODES 50
#define MAX_REQ_LINKS 1500
//100

#define MAX_REQ_PER_NODE 1000
#define MAX_REQ_PER_LINK 1000 //15000

#define MAX_SUB_NODES 450
#define MAX_SUB_LINKS 450


#define LEN_FILENAME 300

#define MAX_CPU 100
#define MAX_BW 100

#define MAX_NECK_COUNT 1000

#define TOPO_GENERAL 0
#define TOPO_STAR 1
#define TOPO_TREE 2
#define TOPO_HUBS 3
#define TOPO_FULL 4

#define STATE_MAP_NODE_FAIL -1  //tried but failed
#define STATE_NEW 0   //first state
#define STATE_MAP_NODE  1  //success in mapping node (only nodes have been mapped)
#define STATE_MAP_LINK  2  // links were sucessfully mapped too
#define STATE_DONE  3  //done
#define STATE_EXPIRE 4

#define NO_MIGRATION 0
#define ROUTE_MIGRATION 1
//#define NODE_MIGRATION 2

#define LINK_SPLITTABLE 0
#define LINK_UNSPLITTABLE 1

#define TIMES_TRY 1

//ECVENTS
#define LEAVE_EVENT 0
#define EXPIRE_EVENT 1
#define ARRIVAL_EVENT 2
#define PUE_CHANGE_EVENT 3
#define RECONFIGURATION_EVENT 4
#define REPORT_INSTANTNEOUS_PARAMETERS 5 //REport the PUE, Energy cost, utilization, Used to monitor and get the results
#define SERVICE_GUARANTEE_TIME_PERIOD_EXPIRED 6 //Verifying that the SLAs in terms of Carbon Emission are guaranteed for every VDC
#define MIGRATION_EVENT 7 //Verifying that the SLAs in terms of Carbon Emission are guaranteed for every VDC


#define PARTITIONING_LOUVAIN 10
#define PARTITIONING_NODE_IS_PARTITION 11
#define PARTITIONING_EQUAL_PARTITIONS_SIZE 12
#define PARTITIONING_LOUVAIN_WITH_DYNAMIC_MIGRATION 13
#define PARTITIONING_LOUVAIN_WITH_DYNAMIC_MIGRATION_AND_ADMISSION_CONTROL 14
#define PARTITIONING_TO_SOLVE_TO_OPTIMAL_SOLUTION 18

#define POISSON 0

#define GOOGLE 1
#define TEST_TEST 6
#define RANDOM 7
#define WEB 2
#define XML 3
#define POISSON 0

#define CONSTANT 2
#define EXPONENTIAL 3
#define UNIFORM 4

#define INIFINITY 99999999

#define FAILURE_PARTITION_EMBED -1
#define FAILURE_LINK_EMBED -2
#define FAILURE_MIGRATE -3
#define FAILURE_PARTITION_LOCATION_CONSTRAINT_VIOLATION -4

#define POWER_CONSUMPTION_INTERMEDIARY_NODE 100 //The power consumption of an intermediary node
#define BANDWIDTH_CAPACITY_PROVIIONED_NETWORK_NODE 1000 //The capacity of the intermediary nodes

#define POWER_PER_SERVER_IN_DATACENTERS 80

#define MAX_MAPPING_HISORY 100

#define SUCCESS_OPERATION 1

#define MAX_NUMBER_DATACENTERS 5

#define RANDOM_TOPOLOGY 1
#define NSFNET_TOPOLOGY 2
#define TEST_TOPOLOGY_SAVI 3



#define NUMBER_OF_INTERVALS_CDF 11
#define CDF_STEP_INTERVAL 10 //For the CDF in the provisioned network utilization


#define SIMULATED 1
#define STUB_FOR_REMOTE 2
#define	DEPLOYED_WITH_VDCPLANNER_IN_SAVI 3


#define MAX_USER_NAME_LENGTH 32
#define MAX_VDC_NAME_LENGTH 32
#define MAX_SESSION_NAME_LENGTH 32
#define MAX_APPLICATION_NAME_LENGTH 32



#define TOPO_GENERAL 0
#define TOPO_STAR 1
#define TOPO_TREE 2
#define TOPO_HUBS 3
#define TOPO_FULL 4

#define STATE_MAP_NODE_FAIL -1  //tried but failed
#define STATE_NEW 0   //first state
#define STATE_MAP_NODE  1  //success in mapping node (only nodes have been mapped)
#define STATE_MAP_LINK  2  // links were successfully mapped too
#define STATE_DONE  3  //done
#define STATE_EXPIRE 4
#define STATE_MAP_LINK_FAIL  5 //nodes fine but links failed

#define STATE_RECEIVED 0   //has just arrived
#define STATE_RUNNING  1  //success in mapping node and links
#define STATE_WAITING  2  // waiting for free resources
#define STATE_EXPIRED 3  //after duration
#define STATE_ABORTED 4  //after duration
#define STATE_REJECTED 5  //after reaching max queueing delay

#define NO_MIGRATION 0
#define ROUTE_MIGRATION 1
//#define NODE_MIGRATION 2

#define LINK_SPLITTABLE 0
#define LINK_UNSPLITTABLE 1

#define TIMES_TRY 1

#define LEAVE_EVENT 0
#define EXPIRE_EVENT 1


#define OFF 0
#define ON 1

#define QUEING_DELAY 60

#endif//DEF_H
