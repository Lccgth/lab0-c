#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "queue.h"

#define MAX_STRING_LENGTH 256

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head)
        return NULL;

    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;

    struct list_head *cur, *temp;

    list_for_each_safe (cur, temp, head) {
        element_t *entry = list_entry(cur, element_t, list);
        list_del(cur);
        q_release_element(entry);
    }
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head || !s)
        return false;

    element_t *element = malloc(sizeof(element_t));
    if (!element)
        return false;

    int len = strlen(s);
    element->value = malloc((len + 1) * sizeof(char));
    if (!element->value) {
        free(element);
        return false;
    }

    strncpy(element->value, s, len + 1);
    list_add(&element->list, head);

    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head || !s)
        return false;

    element_t *element = malloc(sizeof(element_t));
    if (!element)
        return false;

    int len = strlen(s);
    element->value = malloc((len + 1) * sizeof(char));
    if (!element->value) {
        free(element);
        return false;
    }

    strncpy(element->value, s, len + 1);
    list_add_tail(&element->list, head);

    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *entry = list_first_entry(head, element_t, list);
    if (sp) {
        strncpy(sp, entry->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    list_del_init(&entry->list);
    return entry;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *entry = list_last_entry(head, element_t, list);
    if (sp) {
        strncpy(sp, entry->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    list_del_init(&entry->list);
    return entry;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    struct list_head *fast = head->next, *slow = head->next;

    while (fast->next != head && fast->next->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }

    element_t *entry = list_entry(slow, element_t, list);
    list_del(slow);
    q_release_element(entry);

    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    struct list_head *node, *safe;
    bool dup = false;

    list_for_each_safe (node, safe, head) {
        element_t *node_entry = list_entry(node, element_t, list);
        element_t *safe_entry = list_entry(safe, element_t, list);

        if (safe != head && !strcmp(node_entry->value, safe_entry->value)) {
            dup = true;
            list_del(node);
            q_release_element(node_entry);
        } else if (dup) {
            dup = false;
            list_del(node);
            q_release_element(node_entry);
        }
    }

    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *cur = head->next;

    while (cur != head && cur->next != head) {
        list_move(cur, cur->next);
        cur = cur->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *cur, *safe;

    list_for_each_safe (cur, safe, head)
        list_move(cur, head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    int size = q_size(head);

    if (!head || list_empty(head) || size < k)
        return;

    struct list_head *prev = head, *start = head->next, *cur = head->next,
                     *temp;

    do {
        int count = k;
        size -= k;

        do {
            count--;
            temp = cur->next;
            cur->next = cur->prev;
            cur->prev = temp;
            cur = temp;
        } while (cur != head && count > 0);

        cur->prev->prev = prev;
        prev->next = cur->prev;
        start->next = cur;
        prev = start;
        start = cur;
    } while (size > k);
}

void merge(struct list_head *head,
           struct list_head *left,
           struct list_head *right,
           bool descend)
{
    struct list_head *cur = head;

    while (!list_empty(left) && !list_empty(right)) {
        element_t *left_entry = list_entry(left->next, element_t, list);
        element_t *right_entry = list_entry(right->next, element_t, list);

        if (descend ? strcmp(left_entry->value, right_entry->value) >= 0
                    : strcmp(left_entry->value, right_entry->value) <= 0)
            list_move(left->next, cur);
        else
            list_move(right->next, cur);

        cur = cur->next;
    }
    struct list_head *remaining = list_empty(left) ? right : left;
    list_splice_tail(remaining, cur->next);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *slow = head->next, *fast = head->next->next;
    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }

    LIST_HEAD(left);
    LIST_HEAD(right);

    list_cut_position(&left, head, slow);
    list_splice_init(head, &right);

    q_sort(&left, descend);
    q_sort(&right, descend);

    merge(head, &left, &right, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    int size = 0;
    struct list_head *cur = head->prev->prev;
    element_t *min = list_entry(head->prev, element_t, list);

    while (cur != head) {
        element_t *entry = list_entry(cur, element_t, list);
        struct list_head *prev = cur->prev;

        if (strcmp(entry->value, min->value) >= 0) {
            list_del_init(cur);
            q_release_element(entry);
        } else {
            size++;
            min = entry;
        }
        cur = prev;
    }

    return size;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    int size = 0;
    struct list_head *cur = head->prev->prev;
    element_t *max = list_entry(head->prev, element_t, list);

    while (cur != head) {
        element_t *entry = list_entry(cur, element_t, list);
        struct list_head *prev = cur->prev;

        if (strcmp(entry->value, max->value) <= 0) {
            list_del_init(cur);
            q_release_element(entry);
        } else {
            size++;
            max = entry;
        }
        cur = prev;
    }

    return size;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
