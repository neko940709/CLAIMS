/*
 * set_up_environment.h
 *
 *  Created on: May 22, 2014
 *      Author: wangli
 */

#ifndef SET_UP_ENVIRONMENT_H_
#define SET_UP_ENVIRONMENT_H_
#include "../Environment.h"


static void startup_single_node_environment_of_poc(){
//	int master;
	int master=0;
//	printf("Master(0) or Slave(others)??\n");
//	scanf("%d",&master);
	if(master!=0){
		Environment::getInstance(false);
	}
	else{

		Environment::getInstance(true);

		ResourceManagerMaster *rmms=Environment::getInstance()->getResourceManagerMaster();
		Catalog* catalog=Environment::getInstance()->getCatalog();

		TableDescriptor* table_1=new TableDescriptor("cj",Environment::getInstance()->getCatalog()->allocate_unique_table_id());
		table_1->addAttribute("row_id",data_type(t_u_long));  				//0
		table_1->addAttribute("trade_date",data_type(t_int));
		table_1->addAttribute("order_no",data_type(t_u_long));
		table_1->addAttribute("sec_code",data_type(t_int));
		table_1->addAttribute("trade_dir",data_type(t_int));
		table_1->addAttribute("order_type",data_type(t_int));				//5
		table_1->addAttribute("trade_no",data_type(t_int));
		table_1->addAttribute("trade_time",data_type(t_int));
		table_1->addAttribute("trade_time_dec",data_type(t_u_long));
		table_1->addAttribute("order_time",data_type(t_int));
		table_1->addAttribute("order_time_dec",data_type(t_u_long));		//10
		table_1->addAttribute("trade_price",data_type(t_double));
		table_1->addAttribute("trade_amt",data_type(t_double));
		table_1->addAttribute("trade_vol",data_type(t_double));
		table_1->addAttribute("pbu_id",data_type(t_int));
		table_1->addAttribute("acct_id",data_type(t_int));					//15
		table_1->addAttribute("order_prtfil_code",data_type(t_int));
		table_1->addAttribute("tran_type",data_type(t_int));
		table_1->addAttribute("trade_type",data_type(t_int));
		table_1->addAttribute("proc_type",data_type(t_int));

		vector<ColumnOffset> cj_proj0_index;
		cj_proj0_index.push_back(0);
		cj_proj0_index.push_back(1);
		cj_proj0_index.push_back(2);
		cj_proj0_index.push_back(3);
		cj_proj0_index.push_back(4);
		cj_proj0_index.push_back(5);
		const int partition_key_index_1=2;
		table_1->createHashPartitionedProjection(cj_proj0_index,"row_id",1);	//G0
//		catalog->add_table(table_1);
		vector<ColumnOffset> cj_proj1_index;
		cj_proj1_index.push_back(0);
		cj_proj1_index.push_back(6);
		cj_proj1_index.push_back(7);
		cj_proj1_index.push_back(8);
		cj_proj1_index.push_back(9);
		cj_proj1_index.push_back(10);
		cj_proj1_index.push_back(11);
		cj_proj1_index.push_back(12);
		cj_proj1_index.push_back(13);
		cj_proj1_index.push_back(14);
		cj_proj1_index.push_back(15);
		cj_proj1_index.push_back(16);
		cj_proj1_index.push_back(17);
		cj_proj1_index.push_back(18);
		cj_proj1_index.push_back(18);

		table_1->createHashPartitionedProjection(cj_proj1_index,"row_id",1);	//G1


		catalog->add_table(table_1);

		////////////////////////////////////Create table right//////////////////////////
		TableDescriptor* table_2=new TableDescriptor("sb",Environment::getInstance()->getCatalog()->allocate_unique_table_id());
		table_2->addAttribute("row_id",data_type(t_u_long));
		table_2->addAttribute("order_no",data_type(t_u_long));
		table_2->addAttribute("entry_date",data_type(t_int));
		table_2->addAttribute("sec_code",data_type(t_int));
		table_2->addAttribute("order_type",data_type(t_int));
		table_2->addAttribute("entry_dir",data_type(t_int));			//5
		table_2->addAttribute("tran_maint_code",data_type(t_int));
		table_2->addAttribute("Last_upd_date",data_type(t_int));
		table_2->addAttribute("Last_upd_time",data_type(t_int));
		table_2->addAttribute("Last_upd_time_dec",data_type(t_u_long));
		table_2->addAttribute("entry_time",data_type(t_int));			//10
		table_2->addAttribute("entry_time_dec",data_type(t_double));
		table_2->addAttribute("order_price",data_type(t_double));
		table_2->addAttribute("order_exec_vol",data_type(t_double));
		table_2->addAttribute("order_vol",data_type(t_double));
		table_2->addAttribute("pbu_id",data_type(t_int));				//15
		table_2->addAttribute("acct_id",data_type(t_int));
		table_2->addAttribute("acct_attr",data_type(t_int));
		table_2->addAttribute("branch_id",data_type(t_int));
		table_2->addAttribute("pbu_inter_order_no",data_type(t_int));
		table_2->addAttribute("pub_inter_txt",data_type(t_int));		//20
		table_2->addAttribute("aud_type",data_type(t_int));
		table_2->addAttribute("trade_restr_type",data_type(t_int));
		table_2->addAttribute("order_star",data_type(t_int));
		table_2->addAttribute("order_restr_type",data_type(t_int));
		table_2->addAttribute("short_sell_flag",data_type(t_int));		//25















		vector<ColumnOffset> sb_proj0_index;
		sb_proj0_index.push_back(0);
		sb_proj0_index.push_back(1);
		sb_proj0_index.push_back(2);
		sb_proj0_index.push_back(3);
		sb_proj0_index.push_back(4);
		sb_proj0_index.push_back(5);

		table_2->createHashPartitionedProjection(sb_proj0_index,"row_id",1);	//G0



		vector<ColumnOffset> sb_proj1_index;
		sb_proj1_index.push_back(0);
		sb_proj1_index.push_back(6);
		sb_proj1_index.push_back(7);
		sb_proj1_index.push_back(8);
		sb_proj1_index.push_back(9);
		sb_proj1_index.push_back(10);
		sb_proj1_index.push_back(11);
		sb_proj1_index.push_back(12);
		sb_proj1_index.push_back(13);
		sb_proj1_index.push_back(14);
		sb_proj1_index.push_back(15);
		sb_proj1_index.push_back(16);
		sb_proj1_index.push_back(17);
		sb_proj1_index.push_back(18);
		sb_proj1_index.push_back(19);
		sb_proj1_index.push_back(20);
		sb_proj1_index.push_back(21);
		sb_proj1_index.push_back(22);
		sb_proj1_index.push_back(23);
		sb_proj1_index.push_back(24);
		sb_proj1_index.push_back(25);




		table_2->createHashPartitionedProjection(sb_proj1_index,"row_id",1);	//G1


		catalog->add_table(table_2);
		for(unsigned i=0;i<table_1->getProjectoin(0)->getPartitioner()->getNumberOfPartitions();i++){

			catalog->getTable(0)->getProjectoin(0)->getPartitioner()->RegisterPartition(i,2);
//			catalog->getTable(0)->getProjectoin(0)->getPartitioner()->RegisterPartition(i,0);
		}

		for(unsigned i=0;i<table_1->getProjectoin(1)->getPartitioner()->getNumberOfPartitions();i++){

			catalog->getTable(0)->getProjectoin(1)->getPartitioner()->RegisterPartition(i,6);
		}


		//sb_table
		for(unsigned i=0;i<table_2->getProjectoin(0)->getPartitioner()->getNumberOfPartitions();i++){

			catalog->getTable(1)->getProjectoin(0)->getPartitioner()->RegisterPartition(i,2);
		}

		for(unsigned i=0;i<table_2->getProjectoin(1)->getPartitioner()->getNumberOfPartitions();i++){

			catalog->getTable(1)->getProjectoin(1)->getPartitioner()->RegisterPartition(i,6);
		}
	}
}

#endif /* SET_UP_ENVIRONMENT_H_ */