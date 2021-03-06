/* Enumerate clique by Bron-Kerbosch Algorithms 
 * Author: Yun Zhang
 * Date: September 2006
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"
#include "graph.h"
#include "bk.h"


/* Global Variables */
int LB, UB;
int VERSION;
int PRINT;


/* ------------------------------------------------------------- *
 * Function: clique_out()                                        *
 * ------------------------------------------------------------- */
void clique_out(FILE *fp, Graph *G, vid_t *clique, int len)
{
  int i;
  for (i = 0; i < len-1; i++)
	fprintf(fp, "%s\t", G->_label[clique[i]]);
  fprintf(fp, "%s\n", G->_label[clique[i]]);
  return;
}


/* ------------------------------------------------------------- *
 * Function: clique_profile_out()                                *
 * ------------------------------------------------------------- */
void clique_profile_out(FILE *fp, u64 *nclique, Graph *G)
{
  unsigned int n = G->_num_vertices;
  u64 sum=0;
  int max=0;
  int i;
  fprintf(fp, "Size\tNumber\n");
  for (i = LB; i <= n; i++) {
	if (nclique[i]) {
	  fprintf(fp, "%d\t%lu\n", i, nclique[i]);
	  sum += nclique[i];
	  max = i;
	}
  }
  fprintf(fp, "\n");
  fprintf(fp, "No. of vertices : %d\n", n);
  fprintf(fp, "No. of edges    : %d\n", G->_num_edges);
  fprintf(fp, "No. of cliques  : %lu\n", sum);
  fprintf(fp, "Max clique size : %d\n", max);
}





/* ------------------------------------------------------------- *
 * Function: clique_find_v1()                                    *
 *   Bron-Kerbosch version 1 : numerical order                   *
 *   Recursive function to find cliques                          *
 * ------------------------------------------------------------- */
void clique_find_v1(FILE *fp, u64 *nclique, Graph *G, \
		vid_t *clique, vid_t *old, int lc, int ne, int ce)
{
  vid_t new[ce];
  int new_ne, new_ce;
  vid_t u;
  int i, j;

  while (ne < ce) {

#ifdef DEBUG
  for (i = 0; i < ne; i++) printf(" %s", G->_label[old[i]]);
  printf("\t|");
  for (i = 0; i < lc; i++) printf(" %s", G->_label[clique[i]]);
  printf("\t|");
  for (i = ne; i < ce; i++) printf(" %s", G->_label[old[i]]);
  printf("\n");
#endif

	u = old[ne];

	/* Set new cand and not */
	memset(new, -1, ce*sizeof(vid_t));
    new_ne = 0;
	for (j = 0; j < ne; j++)
	  if (edge_exists(G, u, old[j])) new[new_ne++] = old[j];
	new_ce = new_ne;
	for (j = ne+1; j < ce; j++)
	  if (edge_exists(G, u, old[j])) new[new_ce++] = old[j];
	
	/* Output clique or extend */
	clique[lc] = u;
	if (new_ce == 0 && lc+1 >= LB) {
	  nclique[lc+1]++;
	  if (PRINT) clique_out(fp, G, clique, lc+1);
	}
	else { 
	  if (new_ne < new_ce)
	    clique_find_v1(fp, nclique, G, clique, new, lc+1, new_ne, new_ce);
	}

	
	/* Move u to not */
	ne++;

	/* Bound condition: Stop if any not is a neighbor of all candidates */ 
    for (i = 0; i < ne; i++) {
	  for (j = ne; j < ce; j++) {
	    if (!edge_exists(G, old[i], old[j])) break;
	  }
	  if (j == ce) return;
	}

  }

}


/* ------------------------------------------------------------- *
 * Function: clique_find_v2()                                    *
 *   Bron-Kerbosch version 2                                     *
 *   Recursive function to find cliques                          *
 * ------------------------------------------------------------- */
void clique_find_v2(FILE *fp, u64 *nclique, Graph *G, \
		vid_t *clique, vid_t *old, int lc, int ne, int ce)
{
  vid_t new[ce];
  int new_ne, new_ce;
  vid_t fixp=0, p, u;
  int s=0, pos=0, nod, minnod, count;
  int i, j, k;

#ifdef DEBUG
  for (i = 0; i < ne; i++) printf(" %s", G->_label[old[i]]);
  printf("\t|");
  for (i = 0; i < lc; i++) printf(" %s", G->_label[clique[i]]);
  printf("\t|");
  for (i = ne; i < ce; i++) printf(" %s", G->_label[old[i]]);
  printf("\n");
#endif

  /* Choose a vertex, fixp, in old (both not and cand) that
	 has lowest number of non-adjacent vertices in old cand */
  minnod = ce + 1;
  nod = 0;
  for (i = 0; i < ce; i++) {
	count = 0;
	p = old[i];
	for (j = ne; j < ce; j++) {
	  if (!edge_exists(G, p, old[j])) {
		count++;
		pos = j;
	  }
	}
	if (count < minnod) {
	  fixp = p;
	  minnod = count;
	  if (i < ne) { s = pos; }    // if p in not
	  else { s = i; nod = 1; }    // if p in cand
	}
  }
  
  /* Recursively extend clique */
  for (k = minnod+nod; k > 0; k--) {

	/* Swap this candidate to be the next one */
	p = old[s];
	old[s] = old[ne];
	old[ne] = p;

	u = old[ne];

	/* Set new cand and not */
	memset(new, -1, ce*sizeof(vid_t));
    new_ne = 0;
	for (j = 0; j < ne; j++)
	  if (edge_exists(G, u, old[j])) new[new_ne++] = old[j];
	new_ce = new_ne;
	for (j = ne+1; j < ce; j++)
	  if (edge_exists(G, u, old[j])) new[new_ce++] = old[j];
	
	/* Output clique or extend */
	clique[lc] = u;
	if (lc+1 <= UB) {
	  if (new_ce == 0 && lc+1 >= LB) {
	    nclique[lc+1]++;
	    if (PRINT) clique_out(fp, G, clique, lc+1);
	  }
	  else if (new_ne < new_ce) {
	    clique_find_v2(fp, nclique, G, clique, new, lc+1, new_ne, new_ce);
	  }
	}
	
	/* Move u to not */
	ne++;

	/* Bound condition: Stop if fixp is a neighbor of all candidates */ 
    if (k > 1) {
	  for (s = ne; s < ce; s++) {
	    if (!edge_exists(G, fixp, old[s])) break;
	  }
	  if (s == ce) return;
	}

  }

  return;
}


/* ------------------------------------------------------------- *
 * Function: maxclique_find()                                    *
 *   Bron-Kerbosch version 2                                     *
 *   Recursive function to find one of the maximum cliques       *
 * ------------------------------------------------------------- */
void maxclique_find(vid_t *maxclique, int *maxclique_size, Graph *G, \
		vid_t *clique, vid_t *old, int lc, int ne, int ce)
{
  vid_t new[ce];
  int new_ne, new_ce;
  vid_t fixp=0, p, u;
  int s=0, pos=0, nod, minnod, count;
  int i, j, k;

#ifdef DEBUG
  for (i = 0; i < ne; i++) printf(" %d", old[i]);
  printf("\t|");
  for (i = 0; i < lc; i++) printf(" %d", clique[i]);
  printf("\t|");
  for (i = ne; i < ce; i++) printf(" %d", old[i]);
  printf("\n");
#endif

  /* Choose a vertex, fixp, in old (both not and cand) that
	 has lowest number of non-adjacent vertices in old cand */
  minnod = ce + 1;
  nod = 0;
  for (i = 0; i < ce; i++) {
	count = 0;
	p = old[i];
	for (j = ne; j < ce; j++) {
	  if (!edge_exists(G, p, old[j])) {
		count++;
		pos = j;
	  }
	}
	if (count < minnod) {
	  fixp = p;
	  minnod = count;
	  if (i < ne) { s = pos; }    // if p in not
	  else { s = i; nod = 1; }    // if p in cand
	}
  }
  
  /* Recursively extend clique */
  for (k = minnod+nod; k > 0; k--) {

	/* Swap this candidate to be the next one */
	p = old[s];
	old[s] = old[ne];
	old[ne] = p;

	u = old[ne];

	/* Set new cand and not */
	memset(new, -1, ce*sizeof(vid_t));
    new_ne = 0;
	for (j = 0; j < ne; j++)
	  if (edge_exists(G, u, old[j])) new[new_ne++] = old[j];
	new_ce = new_ne;
	for (j = ne+1; j < ce; j++)
	  if (edge_exists(G, u, old[j])) new[new_ce++] = old[j];
	
	/* Record clique or extend */
	clique[lc] = u;
	if (new_ce == 0 && lc+1 >= *maxclique_size) {
	  *maxclique_size = lc + 1;
	  memcpy(maxclique, clique, (*maxclique_size)*sizeof(vid_t));
	  if (PRINT) {
		printf("max size %d\n", lc+1);
		clique_out(stdout, G, clique, lc+1);
	  }
	}
	else if (new_ce-new_ne+lc+1 >= *maxclique_size) {
	  maxclique_find(maxclique, maxclique_size, G, clique, new, lc+1, new_ne, new_ce);
	}
	
	/* Move u to not */
	ne++;

	/* Bound condition: Stop if fixp is a neighbor of all candidates */ 
    if (k > 1) {
	  for (s = ne; s < ce; s++) {
	    if (!edge_exists(G, fixp, old[s])) break;
	  }
	  if (s == ce) return;
	}

  }

  return;
}


/* ------------------------------------------------------------- *
 * Function: clique_enumerate()                                  
 *   For mpiclique version 1, 2
 * ------------------------------------------------------------- */

void clique_enumerate(FILE *fp, u64 *nclique, Graph *G, vid_t *cand, int lcand)
{
  unsigned int n = num_vertices(G);
  vid_t clique[n];
  vid_t vertices[n];
  int ne, ce;
  vid_t u, i, j;
#ifdef DEBUG 
  double utime;
#endif
  
  memset(clique, -1, n*sizeof(vid_t));

  for (i = 0; i < lcand; i++) {

#ifdef DEBUG
    utime = get_cur_time();
#endif
	
	u = cand[i];

	/* Prepare cand and not array */
	ne = 0;
	for (j = 0; j < u; j++)
	  if (edge_exists(G, j, u)) vertices[ne++] = j;
	ce = ne;
	for (j = u+1; j < n; j++)
	  if (edge_exists(G, u, j)) vertices[ce++] = j;

	/* Recursively find cliques containing u if enough candidates */
	if (ce - ne >= LB - 1) {
	  clique[0] = u;
	  if (VERSION == 1)
        clique_find_v1(fp, nclique, G, clique, vertices, 1, ne, ce);
	  else if (VERSION == 2)
        clique_find_v2(fp, nclique, G, clique, vertices, 1, ne, ce);
    }

#ifdef DEBUG
	utime = get_cur_time() - utime;
	printf("task %4d : %d subtasks, %f seconds\n", u, ce-ne, utime);
#endif
  }
  
  return;
}


