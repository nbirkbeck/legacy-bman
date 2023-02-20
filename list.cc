#include "list.h"

void initList(List * list)
{
	list->head=NULL;
	list->tail=NULL;
}

void printList(List * list,FILE * file)
{
	Node * finger = list->head;
	fprintf(file,"<");
	while(finger!=NULL)
	{
		fprintf(file,"%p ",finger->value);
		finger=finger->next;
	}
	fprintf(file,">\n");
}

void insert(List * list, void * value)
{
	Node * newNode =(Node *) malloc(sizeof(Node));
	Node * finger = list->head;
	newNode->next=NULL;
	newNode->value=value;

	if(list->head==NULL)
	{
		list->head=newNode;
		list->tail=newNode;
	}
	else
	{
		list->tail->next = newNode;
		list->tail = newNode;
	}
}
void remAll(List * list)
{
	Node * finger;
	finger = list->head;
	while(finger!=NULL)
	{
		Node * prev = finger;
		finger = finger->next;
		free(prev);
	}
	list->head=NULL;
	list->tail=NULL;
}

void rem(List * list,void * value)
{
	Node * finger;
	Node * previous = NULL;
	
	finger=list->head;
	while(finger!=NULL)
	{
		if(finger->value==value)
		{
			/*
				we are removing the first element of the list
			*/
			if(finger==list->head)
			{
				list->head = (list->head)->next;
				/*
					if the tail == finger then there is only
					one element in the list
				*/
				if(list->tail==finger)
				{
					list->tail = NULL;
				}
				free(finger);
				finger=NULL;
				break;
			}
			else
			{
				previous->next=finger->next;
				if(list->tail == finger)
				{
					list->tail = previous;
				}
				finger->value=NULL;
				free(finger);
				finger=NULL;
				break;
			}
		}
		previous=finger;
		finger=finger->next;
	}		
}
