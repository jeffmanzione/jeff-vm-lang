/*
 * hashtable.c
 *
 *  Created on: Jan 4, 2016
 *      Author: Jeff
 */

#include "hashtable.h"

#include <stdlib.h>
#include <string.h>

static unsigned int  hash(const Hashtable *hashtable, const char *str);

Hashtable *hashtable_create(int size) {
  Hashtable *new_table;
  int i;

  if (size < 1)
    return NULL; /* invalid size for table */

//  printf("A\n");
//  fflush(stdout);

  /* Attempt to allocate memory for the table structure */
  new_table = NEW(new_table, Hashtable)

//  printf("%p %d %d\n", new_table, size, sizeof(H_List *));
//  fflush(stdout);

  /* Attempt to allocate memory for the table itself */
  new_table->table = NEW_ARRAY(new_table->table, size, H_List *)

  /* Initialize the elements of the table */
  for (i = 0; i < size; i++)
    new_table->table[i] = NULL;

  /* Set the table's size */
  new_table->size = size;

  return new_table;
}

unsigned int hash(const Hashtable *hashtable, const char *str) {
  unsigned int hashval;

  /* we start our hash out at 0 */
  hashval = 0;

  /* for each character, we multiply the old hash by 31 and add the current
   * character.  Remember that shifting a number left is equivalent to
   * multiplying it by 2 raised to the number of places shifted.  So we
   * are in effect multiplying hashval by 32 and then subtracting hashval.
   * Why do we do this?  Because shifting and subtraction are much more
   * efficient operations than multiplication.
   */
  for (; *str != '\0'; str++)
    hashval = *str + (hashval << 5) - hashval;

  /* we then return the hash value mod the hashtable size so that it will
   * fit into the necessary range
   */
  return hashval % hashtable->size;
}

H_List *lookup_id(const Hashtable *hashtable, const char *str) {
  H_List *list;
  unsigned int hashval = hash(hashtable, str);

  /* Go to the correct list based on the hash value and see if str is
   * in the list.  If it is, return return a pointer to the list element.
   * If it isn't, the item isn't in the table, so return NULL.
   */
  for (list = hashtable->table[hashval]; list != NULL; list = list->next) {
    if (strcmp(str, list->id) == 0)
      return list;
  }
  return NULL;
}

void *hashtable_lookup(const Hashtable *hashtable, const char *str) {
  H_List *list = lookup_id(hashtable, str);
  if (NULL == list) {
    return NULL;
  }

  return list->obj;
}

int hashtable_insert(Hashtable *hashtable, const char *id, void *obj) {
  H_List *new_list;
  //H_List *current_list;
  unsigned int hashval = hash(hashtable, id);

  /* Attempt to allocate memory for list */
  if ((new_list = malloc(sizeof(H_List))) == NULL)
    return 1;

  /* Does item already exist? */
  //current_list = lookup_id(hashtable, id);
  /* item already exists, don't insert it again. */
  /*if (current_list != NULL)
   return 2;*/
  /* Insert into list */
  new_list->id = strdup(id);
  new_list->obj = obj;
  new_list->next = hashtable->table[hashval];
  hashtable->table[hashval] = new_list;

  return 0;
}

void hashtable_free(Hashtable *hashtable, Deleter del) {
  int i;
  H_List *list, *temp;

  if (hashtable == NULL)
    return;

  /* Free the memory for every item in the table, including the
   * strings themselves.
   */
  for (i = 0; i < hashtable->size; i++) {
    list = hashtable->table[i];
    while (list != NULL) {
      temp = list;
      list = list->next;
      free(temp->id);
      del(temp->obj);
      free(temp);
    }
  }

  /* Free the table itself */
  free(hashtable->table);
  free(hashtable);
}

void hashtable_iterate(Hashtable *hashtable, HT_Action act) {
  int i;
  H_List *list, *temp;

  if (hashtable == NULL)
    return;

  /* Free the memory for every item in the table, including the
   * strings themselves.
   */
  for (i = 0; i < hashtable->size; i++) {
    list = hashtable->table[i];
    while (list != NULL) {
      char *id = list->id;
      temp = list;
      list = list->next;
      act(id, temp->obj);
    }
  }
}
