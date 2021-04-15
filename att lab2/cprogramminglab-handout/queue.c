/* 
 * Code for basic C skills diagnostic.
 * Developed for courses 15-213/18-213/15-513 by R. E. Bryant, 2017
 * Modified to store strings, 2018
 */

/*
 * This program implements a queue supporting both FIFO and LIFO
 * operations.
 *
 * It uses a singly-linked list to represent the set of queue elements
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/*
  Create empty queue.
  Return NULL if could not allocate space.
*/
queue_t *q_new()
{
    queue_t *q = malloc(sizeof(queue_t));
    /* What if malloc returned NULL? */
    if (q == NULL) {
      return NULL;
    }
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    return q;
}

/* Free all storage used by queue */
void q_free(queue_t *q)
{
    /* How about freeing the list elements and the strings? */
    /* Free queue structure */
    if (!q) {
      return;
    }
    list_ele_t *freePointer;
    freePointer = q->head;
    while (freePointer != NULL) {
      list_ele_t *extraPointer;
      extraPointer = freePointer;
      freePointer = freePointer->next;
      free(extraPointer->value);
      free(extraPointer);
    }
    free(q);
}

/*
  Attempt to insert element at head of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(queue_t *q, char *s)
{
    list_ele_t *newh;
    /* What should you do if the q is NULL? */
    if (q == NULL) {
      return false;
    }
    newh = malloc(sizeof(list_ele_t));
    if (newh == NULL) {
      return false;
    }
    memset(newh, 0, sizeof(list_ele_t));
    /* Don't forget to allocate space for the string and copy it */
    /* What if either call to malloc returns NULL? */
    if (s) {
      newh->value = malloc(strlen(s) + 1);
      if (newh->value) {
        strcpy(newh->value, s);
      } else {
        free(newh);
        return false;
      }
    } else {
      newh->value = NULL; 
    }
    newh->next = q->head;
    q->head = newh;
    if (newh ->next == NULL) {
      q->tail = newh;
    }
    q->size++;
    return true;
}


/*
  Attempt to insert element at tail of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(queue_t *q, char *s)
{
    /* You need to write the complete code for this function */
    /* Remember: It should operate in O(1) time */
    
    if (q == NULL) {
      return false;
    }
    if (!q->head) {
      return q_insert_head(q,s);
    }
    list_ele_t *newh;
    newh = malloc(sizeof(list_ele_t));
    if (newh == NULL) {
      return false;
    }
    memset(newh, 0, sizeof(list_ele_t));
    if (s) {
      char *copy = malloc(strlen(s) + 1);
      if (copy != NULL) {
        strcpy(copy, s);
      } else {
        free(newh);
        return false;
      }
    newh->value = copy;
    } else {
      newh->value = NULL; 
    }
    q->tail->next = newh;
    q->tail = newh;
    q->size++;
    return true;
}

/*
  Attempt to remove element from head of queue.
  Return true if successful.
  Return false if queue is NULL or empty.
  If sp is non-NULL and an element is removed, copy the removed string to *sp
  (up to a maximum of bufsize-1 characters, plus a null terminator.)
  The space used by the list element and the string should be freed.
*/
bool q_remove_head(queue_t *q, char *sp, size_t bufsize)
{
    if(!q||!q->head) {
    	return false;
    }
    list_ele_t* extraPointer = q->head;

    if (sp){
      strncpy(sp,extraPointer->value,bufsize);
      sp[bufsize - 1] = '\0';
    } 
    q->head = q->head->next;
    free(extraPointer->value);
    free(extraPointer);
    q->size--;
    if(q->size == 0){
      q->tail = NULL;
    }
    return true;
}   


/*
  Return number of elements in queue.
  Return 0 if q is NULL or empty
 */
int q_size(queue_t *q)
{
  if (!q||!q->head) {
    return 0;
  } else {
    /* code */
  
  
    /* You need to write the code for this function */
    /* Remember: It should operate in O(1) time */
    return q->size;
  }
}

/*
  Reverse elements in queue
  No effect if q is NULL or empty
  This function should not allocate or free any list elements
  (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
  It should rearrange the existing ones.
 */
void q_reverse(queue_t *q)
{
    if (!q||!q->head) {
      return;
    }
    list_ele_t* preP = NULL;
    list_ele_t* current = q->head;
    list_ele_t* nextP = NULL;
    list_ele_t* oriHead = q->head;
    while (current != NULL) {
      nextP = current->next;
      current->next = preP;
      preP = current;
      current = nextP;
    }
    q->head = preP;
    q->tail = oriHead;
    /* You need to write the code for this function */
}

