struct request{
	int trader_id;
	int price;
	int quantity;
};

struct trade{
	int item_id;
	int seller_id;
	int buyer_id;
	int price;
	int quantity;
};

struct trades{
	int locked;
	int cur_size;
	struct trade array[101];
};

struct prio_queue{
  int locked;
  int cur_size;
  struct request heap[101];
};

void initialise_t(struct trades *t){
	t->locked=0;
	t->cur_size=0;
}

void initialise(struct prio_queue *pq){
	pq->locked=0;
	pq->cur_size=0;
}

int isFull_t(struct trades *t){
	if(t->locked==1)
		return -1;
	t->locked=1;
	if(t->cur_size==100){
		t->locked=0;
		return 1;
	}
	else{
		t->locked=0;
		return 0;
	}
}

int isFull(struct prio_queue *pq){
	if(pq->locked==1)
		return -1;
	pq->locked=1;
	if(pq->cur_size==100){
		pq->locked=0;
		return 1;
	}
	else{
		pq->locked=0;
		return 0;
	}
}

int getSize(struct trades *t){
	if(t->locked==1)
		return -1;
	t->locked=1;
	int siz = t->cur_size;
	t->locked=0;
	return siz;
}

int getTradeByIndex(struct trades *t, struct trade *current, int index){
	if(t->locked==1)
		return -1;
	t->locked=1;
	*current = t->array[index];
	t->locked=0;
	return 0;
}

int isEmpty_t(struct trades *t){
	if(t->locked==1)
		return -1;
	t->locked=1;
	if(t->cur_size == 0){
		t->locked=0;
		return 1;
	}
	else{
		t->locked=0;
		return 0;
	}
}

int isEmpty(struct prio_queue *pq){
	if(pq->locked==1)
		return -1;
	pq->locked=1;
	if(pq->cur_size == 0){
		pq->locked=0;
		return 1;
	}
	else{
		pq->locked=0;
		return 0;
	}
}

int insertIntoArray(struct trade newEntry, struct trades *t){
	if(isFull_t(t))
		return -2;

	if(t->locked==1)
		return -1;
	t->locked=1;
	t->cur_size++;
	t->array[t->cur_size]=newEntry;
	t->locked=0;
	return 0;
}

int insertIntoMinHeap(struct request newEntry, struct prio_queue *pq){
	if(isFull(pq))
		return -2;

	if(pq->locked==1)
		return -1;
	pq->locked=1;
	pq->cur_size++;
	pq->heap[pq->cur_size]=newEntry;
	int curr=pq->cur_size;
	while(curr>1 && ((pq->heap[curr].price)<(pq->heap[curr/2].price))){
		struct request temp=pq->heap[curr];
		pq->heap[curr]=pq->heap[curr/2];
		pq->heap[curr/2]=temp;
		curr/=2;
	}
	pq->locked=0;
	return 0;
}

int insertIntoMaxHeap(struct request newEntry, struct prio_queue *pq){
	if(isFull(pq))
		return -2;

	if(pq->locked==1)
		return -1;
	pq->locked=1;
	pq->cur_size++;
	pq->heap[pq->cur_size]=newEntry;
	int curr=pq->cur_size;
	while(curr>1 && ((pq->heap[curr].price)>(pq->heap[curr/2].price))){
		struct request temp=pq->heap[curr];
		pq->heap[curr]=pq->heap[curr/2];
		pq->heap[curr/2]=temp;
		curr/=2;
	}
	pq->locked=0;
	return 0;
}


int fix_min(struct prio_queue *pq){
	if(pq->locked==1)
		return -1;
	pq->locked=1;
	int curr=1;
	while(curr*2<=pq->cur_size){
		if(curr*2+1<=pq->cur_size){
			if((pq->heap[curr].price)<=(pq->heap[curr*2].price)&&(pq->heap[curr].price)<=(pq->heap[curr*2+1].price))
				break;
			else{
				if((pq->heap[curr*2].price)<=(pq->heap[curr*2+1].price)){
					struct request temp=pq->heap[curr*2];
					pq->heap[curr*2]=pq->heap[curr];
					pq->heap[curr]=temp;
					curr*=2;
				} else {
					struct request temp=pq->heap[curr*2+1];
					pq->heap[curr*2+1]=pq->heap[curr];
					pq->heap[curr]=temp;
					curr*=2;
					curr++;
				}
			}
		} else {
			if((pq->heap[curr].price)<=(pq->heap[curr*2].price))
				break;
			else{
				struct request temp=pq->heap[curr*2];
				pq->heap[curr*2]=pq->heap[curr];
				pq->heap[curr]=temp;
				curr*=2;
			}
		}
	}
	pq->locked=0;
	return 0;
}

int fix_max(struct prio_queue *pq){
	if(pq->locked==1)
		return -1;
	pq->locked=1;
	int curr=1;
	while(curr*2<=pq->cur_size){
		if(curr*2+1<=pq->cur_size){
			if((pq->heap[curr].price)>=(pq->heap[curr*2].price)&&(pq->heap[curr].price)>=(pq->heap[curr*2+1].price))
				break;
			else{
				if((pq->heap[curr*2].price)>=(pq->heap[curr*2+1].price)){
					struct request temp=pq->heap[curr*2];
					pq->heap[curr*2]=pq->heap[curr];
					pq->heap[curr]=temp;
					curr*=2;
				} else {
					struct request temp=pq->heap[curr*2+1];
					pq->heap[curr*2+1]=pq->heap[curr];
					pq->heap[curr]=temp;
					curr*=2;
					curr++;
				}
			}
		} else {
			if((pq->heap[curr].price)>=(pq->heap[curr*2].price))
				break;
			else{
				struct request temp=pq->heap[curr*2];
				pq->heap[curr*2]=pq->heap[curr];
				pq->heap[curr]=temp;
				curr*=2;
			}
		}
	}
	pq->locked=0;
	return 0;
}

int extractMin(struct prio_queue *pq, struct request *minimum){
	if(isEmpty(pq))
		return -2;
	if(pq->locked==1)
		return -1;
	pq->locked=1;
	*minimum=pq->heap[1];
	if(pq->cur_size==1)
	{
		pq->cur_size=0;
		pq->locked=0;
	}
	else{
		pq->heap[1] = pq->heap[pq->cur_size];
		pq->cur_size--;
		pq->locked=0;

		fix_min(pq);
	}
	return 0;
}

int extractMax(struct prio_queue *pq, struct request *maximum){
	if(isEmpty(pq))
		return -2;
	if(pq->locked==1)
		return -1;
	pq->locked=1;
	*maximum=pq->heap[1];
	if(pq->cur_size==1)
	{
		pq->cur_size=0;
		pq->locked=0;
	}
	else{
		pq->heap[1] = pq->heap[pq->cur_size];
		pq->cur_size--;
		pq->locked=0;

		fix_max(pq);
	}
	return 0;
}

int min_price_min_heap(struct prio_queue *pq){
	if(pq->locked==1)
		return -1;
	pq->locked=1;
	int mini=pq->heap[1].price;
	pq->locked=0;
	return mini;
}

int max_price_min_heap(struct prio_queue *pq){
	if(pq->locked==1)
		return -1;
	pq->locked=1;
	int maxi=pq->heap[pq->cur_size].price;
	pq->locked=0;
	return maxi;
}

int max_price_max_heap(struct prio_queue *pq){
	if(pq->locked==1)
		return -1;
	pq->locked=1;
	int maxi=pq->heap[1].price;
	pq->locked=0;
	return maxi;
}

int min_price_max_heap(struct prio_queue *pq){
	if(pq->locked==1)
		return -1;
	pq->locked=1;
	int mini=pq->heap[pq->cur_size].price;
	pq->locked=0;
	return mini;
}

int quantity_of_min_price_min_heap(struct prio_queue *pq){
	if(pq->locked==1)
		return -1;
	pq->locked=1;
	int quant=pq->heap[1].quantity;
	pq->locked=0;
	return quant;
}

int quantity_of_max_price_max_heap(struct prio_queue *pq){
	if(pq->locked==1)
		return -1;
	pq->locked=1;
	int quant=pq->heap[1].quantity;
	pq->locked=0;
	return quant;
}

int changeMinQuantity_min_heap(struct prio_queue *pq, int new_value){
	if(pq->locked==1)
		return -1;
	pq->locked=1;
	pq->heap[1].quantity = new_value;
	pq->locked=0;
	return 0;
}

int changeMaxQuantity_max_heap(struct prio_queue *pq, int new_value){
	if(pq->locked==1)
		return -1;
	pq->locked=1;
	pq->heap[1].quantity = new_value;
	pq->locked=0;
	return 0;
}

int showMin_min_heap(struct prio_queue *pq, struct request *minimum){
	if(pq->locked==1)
		return -1;
	pq->locked=1;
	*minimum=pq->heap[1];
	pq->locked=0;
	return 0;
}

int showMax_max_heap(struct prio_queue *pq, struct request *maximum){
	if(pq->locked==1)
		return -1;
	pq->locked=1;
	*maximum=pq->heap[1];
	pq->locked=0;
	return 0;
}