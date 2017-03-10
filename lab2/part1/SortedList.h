struct SortedListElement {
		 	struct SortedListElement *prev;
				struct SortedListElement *next;
					const char *key;
	 };
typedef struct SortedListElement SortedList_t;
typedef struct SortedListElement SortedListElement_t;


void SortedList_insert(SortedList_t *list, SortedListElement_t *element);
int SortedList_delete( SortedListElement_t *element);
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key);
int SortedList_length(SortedList_t *list);

/**
 *  * variable to enable diagnositc yield calls
 *   */
extern int opt_yield;
#define	INSERT_YIELD	0x01	// yield in insert critical section
#define	DELETE_YIELD	0x02	// yield in delete critical section
#define	LOOKUP_YIELD	0x04	// yield in lookup/length critical esction

