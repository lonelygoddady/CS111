#include "SortedList.h"

void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
	//if there is no head in list, make this guy the head 	
	if (list == NULL) 
		{
			element->prev = NULL;
			element->next = NULL;
			list = element;
		}
	else 
		{
			//create a pointer to the head of the list
			SortedListElement_t *tmp = list;

			//iterate through the list
			while((tmp->next->key <= element->key) && (tmp->next != NULL))
					tmp = tmp->next;
			
			if (opt_yield & INSERT_YIELD)
				sched_yield();
			
			//insert the node
			element->next = tmp->next;
			element->prev = tmp;
			tmp->next->prev = element;
			tmp->next = element;
		}
}

int SortedList_delete(SortedListElement_t *element)
{
	if (element->prev != NULL)
		if (element->prev->next != element)
		{
			perror("Correupted prev node pointer!\n");
			return 1;
		}
	
	if (element->next != NULL)
		if (element->next->prev != element)
		{
			perror("Correupted next node pointer!\n");
			return 1;
		}

	//change the linkage
	element->prev->next = element->next;
	if (opt_yield & DELETE_YIELD)
		sched_yield();
	element->next->prev = element->prev;
	
	free(element);
	return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
	if (list == NULL)
		return NULL;
	else
		{
			SortedListElement_t *tmp = list;
			do{
				if (strcmp(tmp->key, key) == 0)
					return tmp;
				if (opt_yield & LOOKUP_YIELD)
					sched_yield();
				tmp = tmp->next;
			}
			while(tmp != NULL);
		}

	return NULL;
}

int SortedList_length(SortedList_t *list)
{
	if (list == NULL)
		return 0;
	
	//temporary node to iterate through list
	SortedListElement_t *element = list;
	int length = 0;
	
	while (element != NULL)
	{
		if (element->prev != NULL)
			if (element->prev->next != element)
			{
				perror("Correupted prev node pointer!\n");
				return -1;
			}
		
		if (element->next != NULL)
			if (element->next->prev != element)
			{
				perror("Correupted next node pointer!\n");
				return -1;
			}
		
		length ++;
		if (opt_yield & LOOKUP_YIELD)
				sched_yield();			
		element = element->next;
	}
	return length;
}

