/*
* BlockStreamAggregationIterator.cpp
*
* Created on: 2013-9-9
* Author: casa
*/

#include "BlockStreamAggregationIterator.h"
#include "../../Debug.h"
#include "../../rdtsc.h"
#include "../../Executor/ExpanderTracker.h"

BlockStreamAggregationIterator::BlockStreamAggregationIterator(State state)
:state_(state),open_finished_(false), open_finished_end_(false),hashtable_(0),hash_(0),bucket_cur_(0),ExpandableBlockStreamIteratorBase(3,2){
        sema_open_.set_value(1);
        sema_open_end_.set_value(1);
        initialize_expanded_status();
}

BlockStreamAggregationIterator::BlockStreamAggregationIterator()
:open_finished_(false), open_finished_end_(false),hashtable_(0),hash_(0),bucket_cur_(0),ExpandableBlockStreamIteratorBase(3,2){
        sema_open_.set_value(1);
        sema_open_end_.set_value(1);
        initialize_expanded_status();
}

BlockStreamAggregationIterator::~BlockStreamAggregationIterator() {

}

BlockStreamAggregationIterator::State::State(
                 Schema *input,
                 Schema *output,
                 BlockStreamIteratorBase *child,
                 std::vector<unsigned> groupByIndex,
                 std::vector<unsigned> aggregationIndex,
                 std::vector<State::aggregation> aggregations,
                 unsigned nbuckets,
                 unsigned bucketsize,
                 unsigned block_size)
                :input(input),
                 output(output),
                 child(child),
                 groupByIndex(groupByIndex),
         aggregationIndex(aggregationIndex),
         aggregations(aggregations),
         nbuckets(nbuckets),
         bucketsize(bucketsize),
         block_size(block_size){

        }

bool BlockStreamAggregationIterator::open(const PartitionOffset& partition_offset){
	barrier_.RegisterOneThread();
	RegisterNewThreadToAllBarriers();
	ExpanderTracker::getInstance()->addNewStageEndpoint(pthread_self(),LocalStageEndPoint(stage_desc,"Aggregation hash build",0));
	state_.child->open(partition_offset);
	if(ExpanderTracker::getInstance()->isExpandedThreadCallBack(pthread_self())){
//		printf("<<<<<<<<<<<<<<<<<Aggregation detected call back signal before constructing hash table!>>>>>>>>>>>>>>>>>\n");
		unregisterNewThreadToAllBarriers();
		return true;
	}

	AtomicPushFreeHtBlockStream(BlockStreamBase::createBlock(state_.input,state_.block_size));
	if(tryEntryIntoSerializedSection(0)){

		unsigned outputindex=0;
		for(unsigned i=0;i<state_.groupByIndex.size();i++)
		{
				inputGroupByToOutput_[i]=outputindex++;
		}
		for(unsigned i=0;i<state_.aggregationIndex.size();i++)
		{
				inputAggregationToOutput_[i]=outputindex++;
		}

		for(unsigned i=0;i<state_.aggregations.size();i++)
		{
			switch(state_.aggregations[i])
			{
				case BlockStreamAggregationIterator::State::count:
				{
					aggregationFunctions_.push_back(state_.output->getcolumn(inputAggregationToOutput_[i]).operate->GetIncreateByOneFunction());
					break;
				}
				case BlockStreamAggregationIterator::State::min:
				{
					aggregationFunctions_.push_back(state_.output->getcolumn(inputAggregationToOutput_[i]).operate->GetMINFunction());
					break;
				}
				case BlockStreamAggregationIterator::State::max:
				{
					aggregationFunctions_.push_back(state_.output->getcolumn(inputAggregationToOutput_[i]).operate->GetMAXFunction());
					break;
				}
				case BlockStreamAggregationIterator::State::sum:
				{
					aggregationFunctions_.push_back(state_.output->getcolumn(inputAggregationToOutput_[i]).operate->GetADDFunction());
					break;
				}
				default:
				{
					printf("invalid aggregation function!\n");
				}
			}
		}
		hash_=PartitionFunctionFactory::createGeneralModuloFunction(state_.nbuckets);
		hashtable_=new BasicHashTable(state_.nbuckets,state_.bucketsize,state_.output->getTupleMaxSize());
		open_finished_=true;

	}

	barrierArrive(0);


	void *cur=0;
	unsigned bn;
	bool key_exist;
	void* tuple_in_hashtable;
	void *key_in_input_tuple;
	void *key_in_hash_table;
	void *value_in_input_tuple;
	void *value_in_hash_table;
	void* new_tuple_in_hash_table;
	unsigned allocated_tuples_in_hashtable=0;
	BasicHashTable::Iterator ht_it=hashtable_->CreateIterator();

	unsigned long long one=1;
	BlockStreamBase *bsb=AtomicPopFreeHtBlockStream();
	bsb->setEmpty();

	unsigned consumed_tuples=0;
	unsigned matched_tuples=0;

		/*
		 * group-by aggregation
		 */
	if(!state_.groupByIndex.empty())
	while(state_.child->next(bsb)){
		BlockStreamBase::BlockStreamTraverseIterator *bsti=bsb->createIterator();
		bsti->reset();
		while(cur=bsti->currentTuple()){
			consumed_tuples++;
			bn=state_.input->getcolumn(state_.groupByIndex[0]).operate->getPartitionValue(state_.input->getColumnAddess(state_.groupByIndex[0],cur),state_.nbuckets);
			hashtable_->lockBlock(bn);
			hashtable_->placeIterator(ht_it,bn);
			key_exist=false;
			while((tuple_in_hashtable=ht_it.readCurrent())!=0){
				for(unsigned i=0;i<state_.groupByIndex.size();i++){
					key_in_input_tuple=state_.input->getColumnAddess(state_.groupByIndex[i],cur);
					key_in_hash_table=state_.output->getColumnAddess(inputGroupByToOutput_[i],tuple_in_hashtable);
					if(state_.input->getcolumn(state_.groupByIndex[i]).operate->equal(key_in_input_tuple,key_in_hash_table)){
						key_exist=true;
					}
					else{
						key_exist=false;
						break;
					}
				}
				if(key_exist){
					matched_tuples++;
					for(unsigned i=0;i<state_.aggregationIndex.size();i++){
						value_in_input_tuple=state_.input->getColumnAddess(state_.aggregationIndex[i],cur);
						value_in_hash_table=state_.output->getColumnAddess(inputAggregationToOutput_[i],tuple_in_hashtable);

						hashtable_->UpdateTuple(bn,value_in_hash_table,value_in_input_tuple,aggregationFunctions_[i]);
					}
					break;
				}
				else{
					ht_it.increase_cur_();
				}
			}
			if(key_exist){
				bsti->increase_cur_();
				hashtable_->unlockBlock(bn);
				continue;
			}
			new_tuple_in_hash_table=hashtable_->allocate(bn);
			hashtable_->unlockBlock(bn);
			allocated_tuples_in_hashtable++;

			for(unsigned i=0;i<state_.groupByIndex.size();i++){
				key_in_input_tuple=state_.input->getColumnAddess(state_.groupByIndex[i],cur);
				key_in_hash_table=state_.output->getColumnAddess(inputGroupByToOutput_[i],new_tuple_in_hash_table);
				state_.input->getcolumn(state_.groupByIndex[i]).operate->assignment(key_in_input_tuple,key_in_hash_table);
			}

			for(unsigned i=0;i<state_.aggregationIndex.size();i++){
				/**
				 * use if-else here is a kind of ugly.
				 * TODO: use a function which is initialized according to the aggregation function.
				 */
				if(state_.aggregations[i]==State::count){
						value_in_input_tuple=&one;
				}
				else{
						value_in_input_tuple=state_.input->getColumnAddess(state_.aggregationIndex[i],cur);
				}
				value_in_hash_table=state_.output->getColumnAddess(inputAggregationToOutput_[i],new_tuple_in_hash_table);
				state_.input->getcolumn(state_.aggregationIndex[i]).operate->assignment(value_in_input_tuple,value_in_hash_table);
			}
			bsti->increase_cur_();
		}

		bsb->setEmpty();
	}
	else{
		/**
		 * scalar aggregation, e.i., all tuples are in the same group.
		 */
		while(state_.child->next(bsb)){
			BlockStreamBase::BlockStreamTraverseIterator *bsti=bsb->createIterator();
			bsti->reset();
			while(cur=bsti->currentTuple()){
				consumed_tuples++;
				bn=0;
				hashtable_->placeIterator(ht_it,bn);
				key_exist=false;
				if((tuple_in_hashtable=ht_it.readCurrent())!=0){
					key_exist=true;
					matched_tuples++;
					for(unsigned i=0;i<state_.aggregationIndex.size();i++){
						value_in_input_tuple=state_.input->getColumnAddess(state_.aggregationIndex[i],cur);
						value_in_hash_table=state_.output->getColumnAddess(inputAggregationToOutput_[i],tuple_in_hashtable);
						hashtable_->atomicUpdateTuple(bn,value_in_hash_table,value_in_input_tuple,aggregationFunctions_[i]);
					}
					bsti->increase_cur_();
				}
				else{
					new_tuple_in_hash_table=hashtable_->atomicAllocate(bn);
					allocated_tuples_in_hashtable++;
					for(unsigned i=0;i<state_.aggregationIndex.size();i++){
						/**
						 * use if-else here is a kind of ugly.
						 * TODO: use a function which is initialized according to the aggregation function.
						 */
						if(state_.aggregations[i]==State::count){
							value_in_input_tuple=&one;
						}
						else{
							value_in_input_tuple=state_.input->getColumnAddess(state_.aggregationIndex[i],cur);
						}
						value_in_hash_table=state_.output->getColumnAddess(inputAggregationToOutput_[i],new_tuple_in_hash_table);
						state_.input->getcolumn(state_.aggregationIndex[i]).operate->assignment(value_in_input_tuple,value_in_hash_table);
					}
					bsti->increase_cur_();
				}
			}
			bsb->setEmpty();
		}
	}

//		if(ExpanderTracker::getInstance()->isExpandedThreadCallBack(pthread_self())){
//			unregisterNewThreadToAllBarriers(1);
//			return true;
//		}
		barrierArrive(1);

		if(tryEntryIntoSerializedSection(1)){
//			hashtable_->report_status();
				it_=hashtable_->CreateIterator();
				bucket_cur_=0;
				hashtable_->placeIterator(it_,bucket_cur_);
				open_finished_end_=true;
				ExpanderTracker::getInstance()->addNewStageEndpoint(pthread_self(),LocalStageEndPoint(stage_src,"Aggregation read",0));
		}
		barrierArrive(2);
}

/*
 * In the current implementation, the lock is used based on the entire
 * hash table, which will definitely reduce the degree of parallelism.
 * But it is for now, assuming that the aggregated results are small.
 *
 */
bool BlockStreamAggregationIterator::next(BlockStreamBase *block){
	if(ExpanderTracker::getInstance()->isExpandedThreadCallBack(pthread_self())){
		unregisterNewThreadToAllBarriers(3);
		printf("<<<<<<<<<<<<<<<<<Aggregation next detected call back signal!>>>>>>>>>>>>>>>>>\n");
		return false;
	}
	void *cur_in_ht;
	void *tuple;
	ht_cur_lock_.acquire();
	while(it_.readCurrent()!=0||(hashtable_->placeIterator(it_,bucket_cur_))!=false){
		while((cur_in_ht=it_.readCurrent())!=0){

			if((tuple=block->allocateTuple(hashtable_->getHashTableTupleSize()))!=0){
				memcpy(tuple,cur_in_ht,hashtable_->getHashTableTupleSize());
				it_.increase_cur_();
			}
			else{
				ht_cur_lock_.release();
				return true;
			}
		}
		bucket_cur_++;
	}
	ht_cur_lock_.release();
	if(block->Empty()){
		   return false;
	}
	else{
		return true;
	}
}

bool BlockStreamAggregationIterator::close(){

    initialize_expanded_status();
	sema_open_.post();
	sema_open_end_.post();

	open_finished_=false;
	open_finished_end_=false;

	hashtable_->~BasicHashTable();
	ht_free_block_stream_list_.clear();
	aggregationFunctions_.clear();
	inputAggregationToOutput_.clear();
	inputGroupByToOutput_.clear();

	state_.child->close();
	return true;
}
void BlockStreamAggregationIterator::print(){
	printf("Aggregation:  %d buckets in hash table\n",state_.nbuckets);
	printf("---------------\n");
	state_.child->print();
}
BlockStreamBase* BlockStreamAggregationIterator::AtomicPopFreeHtBlockStream(){
	assert(!ht_free_block_stream_list_.empty());
	lock_.acquire();
	BlockStreamBase *block=ht_free_block_stream_list_.front();
	ht_free_block_stream_list_.pop_front();
	lock_.release();
	return block;
}

void BlockStreamAggregationIterator::AtomicPushFreeHtBlockStream(BlockStreamBase* block){
	lock_.acquire();
	ht_free_block_stream_list_.push_back(block);
	lock_.release();
}
