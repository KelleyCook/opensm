/*
 * Copyright (c) 2008      System Fabric Works, Inc.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

/*
 * Abstract:
 *      routines to analyze certain meshes
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif				/* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <opensm/osm_switch.h>
#include <opensm/osm_opensm.h>
#include <opensm/osm_log.h>
#include <opensm/osm_mesh.h>
#include <opensm/osm_ucast_lash.h>

#define MAX_DEGREE	(8)
#define MAX_DIMENSION	(8)

/*
 * characteristic polynomials for selected 1d through 8d tori
 */
struct _mesh_info {
	int dimension;			/* dimension of the torus */
	int size[MAX_DIMENSION];	/* size of the torus */
	int degree;			/* degree of polynomial */
	int poly[MAX_DEGREE+1];		/* polynomial */
} mesh_info[] = {
	{0, {0},       0, {0},					},

	{1, {2},       1, {0, -1},				},
	{1, {3},       2, {-1, 0, 1},				},
	{1, {5},       2, {-9, 0, 1},				},
	{1, {6},       2, {-36, 0, 1},				},

	{2, {2, 2},    2, {-4, 0, 1},				},
	{2, {3, 2},    3, {8, 9, 0, -1},			},
	{2, {5, 2},    3, {24, 17, 0, -1},			},
	{2, {6, 2},    3, {32, 24, 0, -1},			},
	{2, {3, 3},    4, {-15, -32, -18, 0, 1},		},
	{2, {5, 3},    4, {-39, -64, -26, 0, 1},		},
	{2, {6, 3},    4, {-48, -80, -33, 0, 1},		},
	{2, {5, 5},    4, {-63, -96, -34, 0, 1},		},
	{2, {6, 5},    4, {-48, -112, -41, 0, 1},		},
	{2, {6, 6},    4, {0, -128, -48, 0, 1},			},

	{3, {2, 2, 2}, 3, {16, 12, 0, -1},			},
	{3, {3, 2, 2}, 4, {-28, -48, -21, 0, 1},		},
	{3, {5, 2, 2}, 4, {-60, -80, -29, 0, 1},		},
	{3, {6, 2, 2}, 4, {-64, -96, -36, 0, 1},		},
	{3, {3, 3, 2}, 5, {48, 127, 112, 34, 0, -1},		},
	{3, {5, 3, 2}, 5, {96, 215, 160, 42, 0, -1},		},
	{3, {6, 3, 2}, 5, {96, 232, 184, 49, 0, -1},		},
	{3, {5, 5, 2}, 5, {144, 303, 208, 50, 0, -1},		},
	{3, {6, 5, 2}, 5, {96, 296, 232, 57, 0, -1},		},
	{3, {6, 6, 2}, 5, {0, 256, 256, 64, 0, -1},		},
	{3, {3, 3, 3}, 6, {-81, -288, -381, -224, -51, 0, 1},	},
	{3, {5, 3, 3}, 6, {-153, -480, -557, -288, -59, 0, 1},	},
	{3, {6, 3, 3}, 6, {-144, -480, -591, -320, -66, 0, 1},	},
	{3, {5, 5, 3}, 6, {-225, -672, -733, -352, -67, 0, 1},	},
	{3, {6, 5, 3}, 6, {-144, -576, -743, -384, -74, 0, 1},	},
	{3, {6, 6, 3}, 6, {0, -384, -720, -416, -81, 0, 1},	},
	{3, {5, 5, 5}, 6, {-297, -864, -909, -416, -75, 0, 1},	},
	{3, {6, 5, 5}, 6, {-144, -672, -895, -448, -82, 0, 1},	},
	{3, {6, 6, 5}, 6, {0, -384, -848, -480, -89, 0, 1},	},
	{3, {6, 6, 6}, 6, {0, 0, -768, -512, -96, 0, 1},	},

	{4, {2, 2, 2, 2},	4, {-48, -64, -24, 0, 1},	},
	{4, {3, 2, 2, 2},	5, {80, 180, 136, 37, 0, -1},	},
	{4, {5, 2, 2, 2},	5, {144, 276, 184, 45, 0, -1},	},
	{4, {6, 2, 2, 2},	5, {128, 288, 208, 52, 0, -1},	},
	{4, {3, 3, 2, 2},	6, {-132, -416, -487, -256, -54, 0, 1},	},
	{4, {5, 3, 2, 2},	6, {-228, -640, -671, -320, -62, 0, 1},	},
	{4, {6, 3, 2, 2},	6, {-192, -608, -700, -352, -69, 0, 1},	},
	{4, {5, 5, 2, 2},	6, {-324, -864, -855, -384, -70, 0, 1},	},
	{4, {6, 5, 2, 2},	6, {-192, -736, -860, -416, -77, 0, 1},	},
	{4, {6, 6, 2, 2},	6, {0, -512, -832, -448, -84, 0, 1},	},
	{4, {3, 3, 3, 2},	7, {216, 873, 1392, 1101, 440, 75, 0, -1},	},
	{4, {5, 3, 3, 2},	7, {360, 1329, 1936, 1405, 520, 83, 0, -1},	},
	{4, {6, 3, 3, 2},	7, {288, 1176, 1872, 1455, 560, 90, 0, -1},	},
	{4, {5, 5, 3, 2},	7, {504, 1785, 2480, 1709, 600, 91, 0, -1},	},
	{4, {6, 5, 3, 2},	7, {288, 1368, 2272, 1735, 640, 98, 0, -1},	},
	{4, {6, 6, 3, 2},	7, {0, 768, 1920, 1728, 680, 105, 0, -1},	},
	{4, {5, 5, 5, 2},	7, {648, 2241, 3024, 2013, 680, 99, 0, -1},	},
	{4, {6, 5, 5, 2},	7, {288, 1560, 2672, 2015, 720, 106, 0, -1},	},
	{4, {6, 6, 5, 2},	7, {0, 768, 2176, 1984, 760, 113, 0, -1},	},
	{4, {6, 6, 6, 2},	7, {0, 0, 1536, 1920, 800, 120, 0, -1},	},
	{4, {3, 3, 3, 3},	8, {-351, -1728, -3492, -3712, -2202, -704, -100, 0, 1},	},
	{4, {5, 3, 3, 3},	8, {-567, -2592, -4860, -4800, -2658, -800, -108, 0, 1},	},
	{4, {6, 3, 3, 3},	8, {-432, -2160, -4401, -4672, -2733, -848, -115, 0, 1},	},
	{4, {5, 5, 3, 3},	8, {-783, -3456, -6228, -5888, -3114, -896, -116, 0, 1},	},
	{4, {6, 5, 3, 3},	8, {-432, -2448, -5241, -5568, -3165, -944, -123, 0, 1},	},
	{4, {6, 6, 3, 3},	8, {0, -1152, -3888, -5056, -3183, -992, -130, 0, 1},	},
	{4, {5, 5, 5, 3},	8, {-999, -4320, -7596, -6976, -3570, -992, -124, 0, 1},	},
	{4, {6, 5, 5, 3},	8, {-432, -2736, -6081, -6464, -3597, -1040, -131, 0, 1},	},
	{4, {6, 6, 5, 3},	8, {0, -1152, -4272, -5760, -3591, -1088, -138, 0, 1},	},
	{4, {6, 6, 6, 3},	8, {0, 0, -2304, -4864, -3552, -1136, -145, 0, 1},	},

	{5, {2, 2, 2, 2, 2},	5, {128, 240, 160, 40, 0, -1},	},
	{5, {3, 2, 2, 2, 2},	6, {-208, -576, -600, -288, -57, 0, 1},	},
	{5, {5, 2, 2, 2, 2},	6, {-336, -832, -792, -352, -65, 0, 1},	},
	{5, {6, 2, 2, 2, 2},	6, {-256, -768, -816, -384, -72, 0, 1},	},
	{5, {3, 3, 2, 2, 2},	7, {336, 1228, 1776, 1287, 480, 78, 0, -1},	},
	{5, {5, 3, 2, 2, 2},	7, {528, 1772, 2368, 1599, 560, 86, 0, -1},	},
	{5, {6, 3, 2, 2, 2},	7, {384, 1504, 2256, 1644, 600, 93, 0, -1},	},
	{5, {5, 5, 2, 2, 2},	7, {720, 2316, 2960, 1911, 640, 94, 0, -1},	},
	{5, {6, 5, 2, 2, 2},	7, {384, 1760, 2704, 1932, 680, 101, 0, -1},	},
	{5, {6, 6, 2, 2, 2},	7, {0, 1024, 2304, 1920, 720, 108, 0, -1},	},
	{5, {3, 3, 3, 2, 2},	8, {-540, -2448, -4557, -4480, -2481, -752, -103, 0, 1},	},
	{5, {5, 3, 3, 2, 2},	8, {-828, -3504, -6101, -5632, -2945, -848, -111, 0, 1},	},
	{5, {6, 3, 3, 2, 2},	8, {-576, -2784, -5412, -5440, -3015, -896, -118, 0, 1},	},
	{5, {5, 5, 3, 2, 2},	8, {-1116, -4560, -7645, -6784, -3409, -944, -119, 0, 1},	},
	{5, {6, 5, 3, 2, 2},	8, {-576, -3168, -6404, -6400, -3455, -992, -126, 0, 1},	},
	{5, {6, 6, 3, 2, 2},	8, {0, -1536, -4800, -5824, -3468, -1040, -133, 0, 1},	},
	{5, {5, 5, 5, 2, 2},	8, {-1404, -5616, -9189, -7936, -3873, -1040, -127, 0, 1},	},
	{5, {6, 5, 5, 2, 2},	8, {-576, -3552, -7396, -7360, -3895, -1088, -134, 0, 1},	},
	{5, {6, 6, 5, 2, 2},	8, {0, -1536, -5312, -6592, -3884, -1136, -141, 0, 1},	},
	{5, {6, 6, 6, 2, 2},	8, {0, 0, -3072, -5632, -3840, -1184, -148, 0, 1},	},

	{6, {2, 2, 2, 2, 2, 2},	6, {-320, -768, -720, -320, -60, 0, 1},	},
	{6, {3, 2, 2, 2, 2, 2},	7, {512, 1680, 2208, 1480, 520, 81, 0, -1},	},
	{6, {5, 2, 2, 2, 2, 2},	7, {768, 2320, 2848, 1800, 600, 89, 0, -1},	},
	{6, {6, 2, 2, 2, 2, 2},	7, {512, 1920, 2688, 1840, 640, 96, 0, -1},	},
	{6, {3, 3, 2, 2, 2, 2},	8, {-816, -3392, -5816, -5312, -2767, -800, -106, 0, 1},	},
	{6, {5, 3, 2, 2, 2, 2},	8, {-1200, -4672, -7544, -6528, -3239, -896, -114, 0, 1},	},
	{6, {6, 3, 2, 2, 2, 2},	8, {-768, -3584, -6608, -6272, -3304, -944, -121, 0, 1},	},
	{6, {5, 5, 2, 2, 2, 2},	8, {-1584, -5952, -9272, -7744, -3711, -992, -122, 0, 1},	},
	{6, {6, 5, 2, 2, 2, 2},	8, {-768, -4096, -7760, -7296, -3752, -1040, -129, 0, 1},	},
	{6, {6, 6, 2, 2, 2, 2},	8, {0, -2048, -5888, -6656, -3760, -1088, -136, 0, 1},	},

	{7, {2, 2, 2, 2, 2, 2, 2},	7, {768, 2240, 2688, 1680, 560, 84, 0, -1},	},
	{7, {3, 2, 2, 2, 2, 2, 2},	8, {-1216, -4608, -7280, -6208, -3060, -848, -109, 0, 1},	},
	{7, {5, 2, 2, 2, 2, 2, 2},	8, {-1728, -6144, -9200, -7488, -3540, -944, -117, 0, 1},	},
	{7, {6, 2, 2, 2, 2, 2, 2},	8, {-1024, -4608, -8000, -7168, -3600, -992, -124, 0, 1},	},

	{8, {2, 2, 2, 2, 2, 2, 2, 2},	8, {-1792, -6144, -8960, -7168, -3360, -896, -112, 0, 1},	},

	{-1, {0,}, 0, {0, },					},
};

/*
 * per fabric mesh info
 */
typedef struct _mesh {
	int num_class;			/* number of switch classes */
	int *class_type;		/* index of first switch found for each class */
	int *class_count;		/* population of each class */
	int dimension;			/* mesh dimension */
	int *size;			/* an array to hold size of mesh */
} mesh_t;

/*
 * poly_alloc
 *
 * allocate a polynomial of degree n
 */
static int *poly_alloc(lash_t *p_lash, int n)
{
	osm_log_t *p_log = &p_lash->p_osm->log;
	int *p;

	if (!(p = calloc(n+1, sizeof(int)))) {
		OSM_LOG(p_log, OSM_LOG_ERROR, "Failed allocating poly - out of memory\n");
	}

	return p;
}

/*
 * poly_diff
 *
 * return a nonzero value if polynomials differ else 0
 */
static int poly_diff(int n, int *p, switch_t *s)
{
	if (s->node->num_links != n)
		return 1;

	return memcmp(p, s->node->poly, n*sizeof(int));
}

/*
 * m_free
 *
 * free a square matrix of rank l
 */
static void m_free(int **m, int l)
{
	int i;

	if (m) {
		for (i = 0; i < l; i++) {
			if (m[i])
				free(m[i]);
		}
		free(m);
	}
}

/*
 * m_alloc
 *
 * allocate a square matrix of rank l
 */
static int **m_alloc(lash_t *p_lash, int l)
{
	osm_log_t *p_log = &p_lash->p_osm->log;
	int i;
	int **m = NULL;

	do {
		if (!(m = calloc(l, sizeof(int *))))
			break;

		for (i = 0; i < l; i++) {
			if (!(m[i] = calloc(l, sizeof(int))))
				break;
		}
		if (i != l)
			break;

		return m;
	} while(0);

	OSM_LOG(p_log, OSM_LOG_ERROR, "Failed allocating matrix - out of memory\n");

	m_free(m, l);
	return NULL;
}

/*
 * pm_free
 *
 * free a square matrix of rank l of polynomials
 */
static void pm_free(int ***m, int l)
{
	int i, j;

	if (m) {
		for (i = 0; i < l; i++) {
			if (m[i]) {
				for (j = 0; j < l; j++) {
					if (m[i][j])
						free(m[i][j]);
				}
				free(m[i]);
			}
		}
		free(m);
	}
}

/*
 * pm_alloc
 *
 * allocate a square matrix of rank l of polynomials of degree n
 */
static int ***pm_alloc(lash_t *p_lash, int l, int n)
{
	osm_log_t *p_log = &p_lash->p_osm->log;
	int i, j;
	int ***m = NULL;

	do {
		if (!(m = calloc(l, sizeof(int **))))
			break;

		for (i = 0; i < l; i++) {
			if (!(m[i] = calloc(l, sizeof(int *))))
				break;

			for (j = 0; j < l; j++) {
				if (!(m[i][j] = calloc(n+1, sizeof(int))))
					break;
			}
			if (j != l)
				break;
		}
		if (i != l)
			break;

		return m;
	} while(0);

	OSM_LOG(p_log, OSM_LOG_ERROR, "Failed allocating matrix - out of memory\n");

	pm_free(m, l);
	return NULL;
}

static int determinant(lash_t *p_lash, int n, int rank, int ***m, int *p);

/*
 * sub_determinant
 *
 * compute the determinant of a submatrix of matrix of rank l of polynomials of degree n
 * with row and col removed in poly. caller must free poly
 */
static int sub_determinant(lash_t *p_lash, int n, int l, int row, int col, int ***matrix, int **poly)
{
	int ret = -1;
	int ***m = NULL;
	int *p = NULL;
	int i, j, k, x, y;
	int rank = l - 1;

	do {
		if (!(p = poly_alloc(p_lash, n))) {
			break;
		}

		if (rank <= 0) {
			p[0] = 1;
			ret = 0;
			break;
		}

		if (!(m = pm_alloc(p_lash, rank, n))) {
			free(p);
			p = NULL;
			break;
		}

		x = 0;
		for (i = 0; i < l; i++) {
			if (i == row)
				continue;

			y = 0;
			for (j = 0; j < l; j++) {
				if (j == col)
					continue;

				for (k = 0; k <= n; k++)
					m[x][y][k] = matrix[i][j][k];

				y++;
			}
			x++;
		}

		if (determinant(p_lash, n, rank, m, p)) {
			free(p);
			p = NULL;
			break;
		}

		ret = 0;
	} while(0);

	pm_free(m, rank);
	*poly = p;
	return ret;
}

/*
 * determinant
 *
 * compute the determinant of matrix m of rank of polynomials of degree deg
 * and add the result to polynomial p allocated by caller
 */
static int determinant(lash_t *p_lash, int deg, int rank, int ***m, int *p)
{
	int i, j, k;
	int *q;
	int sign = 1;

	/*
	 * handle simple case of 1x1 matrix
	 */
	if (rank == 1) {
		for (i = 0; i <= deg; i++)
			p[i] += m[0][0][i];
	}

	/*
	 * handle simple case of 2x2 matrix
	 */
	else if (rank == 2) {
		for (i = 0; i <= deg; i++) {
			if (m[0][0][i] == 0)
				continue;

			for (j = 0; j <= deg; j++) {
				if (m[1][1][j] == 0)
					continue;

				p[i+j] += m[0][0][i]*m[1][1][j];
			}
		}

		for (i = 0; i <= deg; i++) {
			if (m[0][1][i] == 0)
				continue;

			for (j = 0; j <= deg; j++) {
				if (m[1][0][j] == 0)
					continue;

				p[i+j] -= m[0][1][i]*m[1][0][j];
			}
		}
	}

	/*
	 * handle the general case
	 */
	else {
		for (i = 0; i < rank; i++) {
			if (sub_determinant(p_lash, deg, rank, 0, i, m, &q))
				return -1;

			for (j = 0; j <= deg; j++) {
				if (m[0][i][j] == 0)
					continue;

				for (k = 0; k <= deg; k++) {
					if (q[k] == 0)
						continue;

					p[j+k] += sign*m[0][i][j]*q[k];
				}
			}

			free(q);
			sign = -sign;
		}
	}

	return 0;
}

/*
 * char_poly
 *
 * compute the characteristic polynomial of matrix of rank
 * by computing the determinant of m-x*I and return in poly
 * as an array. caller must free poly
 */
static int char_poly(lash_t *p_lash, int rank, int **matrix, int **poly)
{
	osm_log_t *p_log = &p_lash->p_osm->log;
	int ret = -1;
	int i, j;
	int ***m = NULL;
	int *p = NULL;
	int deg = rank;

	OSM_LOG_ENTER(p_log);

	do {
		if (!(p = poly_alloc(p_lash, deg))) {
			break;
		}

		if (!(m = pm_alloc(p_lash, rank, deg))) {
			free(p);
			p = NULL;
			break;
		}

		for (i = 0; i < rank; i++) {
			for (j = 0; j < rank; j++) {
				m[i][j][0] = matrix[i][j];
			}
			m[i][i][1] = -1;
		}

		if (determinant(p_lash, deg, rank, m, p)) {
			free(p);
			p = NULL;
			break;
		}

		ret = 0;
	} while(0);

	pm_free(m, rank);
	*poly = p;

	OSM_LOG_EXIT(p_log);
	return ret;
}

/*
 * get_switch_metric
 *
 * compute the matrix of minimum distances between each of
 * the adjacent switch nodes to sw along paths
 * that do not go through sw. do calculation by
 * relaxation method
 * allocate space for the matrix and save in node_t structure
 */
static int get_switch_metric(lash_t *p_lash, int sw)
{
	osm_log_t *p_log = &p_lash->p_osm->log;
	int ret = -1;
	int i, j, change;
	int sw1, sw2, sw3;
	switch_t *s = p_lash->switches[sw];
	switch_t *s1, *s2, *s3;
	int **m;
	mesh_node_t *node = s->node;
	int num_links = node->num_links;

	OSM_LOG_ENTER(p_log);

	do {
		if (!(m = m_alloc(p_lash, num_links)))
			break;

		for (i = 0; i < num_links; i++) {
			sw1 = node->links[i]->switch_id;
			s1 = p_lash->switches[sw1];

			/* make all distances big except s1 to itself */
			for (sw2 = 0; sw2 < p_lash->num_switches; sw2++)
				p_lash->switches[sw2]->node->temp = 0x7fffffff;

			s1->node->temp = 0;

			do {
				change = 0;

				for (sw2 = 0; sw2 < p_lash->num_switches; sw2++) {
					s2 = p_lash->switches[sw2];
					if (s2->node->temp == 0x7fffffff)
						continue;
					for (j = 0; j < s2->node->num_links; j++) {
						sw3 = s2->node->links[j]->switch_id;
						s3 = p_lash->switches[sw3];

						if (sw3 == sw)
							continue;

						if ((s2->node->temp + 1) < s3->node->temp) {
							s3->node->temp = s2->node->temp + 1;
							change++;
						}
					}
				}
			} while(change);

			for (j = 0; j < num_links; j++) {
				sw2 = node->links[j]->switch_id;
				s2 = p_lash->switches[sw2];
				m[i][j] = s2->node->temp;
			}
		}

		if (char_poly(p_lash, num_links, m, &node->poly)) {
			m_free(m, num_links);
			m = NULL;
			break;
		}

		ret = 0;
	} while(0);

	node->matrix = m;

	OSM_LOG_EXIT(p_log);
	return ret;
}

/*
 * classify_switch
 *
 * add switch to histogram of switch types
 * we keep a reference to the first switch
 * found of each type as an exemplar
 */
static void classify_switch(lash_t *p_lash, mesh_t *mesh, int sw)
{
	osm_log_t *p_log = &p_lash->p_osm->log;
	int i;
	switch_t *s = p_lash->switches[sw];
	switch_t *s1;

	OSM_LOG_ENTER(p_log);

	for (i = 0; i < mesh->num_class; i++) {
		s1 = p_lash->switches[mesh->class_type[i]];

		if (poly_diff(s->node->num_links, s->node->poly, s1))
			continue;

		mesh->class_count[i]++;
		OSM_LOG_EXIT(p_log);
		return;
	}

	mesh->class_type[mesh->num_class] = sw;
	mesh->class_count[mesh->num_class] = 1;
	mesh->num_class++;

	OSM_LOG_EXIT(p_log);
	return;
}

/*
 * classify_mesh_type
 *
 * try to look up node polynomial in table
 */
static void classify_mesh_type(lash_t *p_lash, int sw)
{
	int i;
	switch_t *s = p_lash->switches[sw];
	struct _mesh_info *t;

	for (i = 1; (t = &mesh_info[i])->dimension != -1; i++) {
		if (poly_diff(t->degree, t->poly, s))
			continue;

		s->node->type = i;
		s->node->dimension = t->dimension;
		return;
	}

	s->node->type = 0;
	return;
}

/*
 * get_local_geometry
 *
 * analyze the local geometry around each switch
 */
static int get_local_geometry(lash_t *p_lash, mesh_t *mesh)
{
	osm_log_t *p_log = &p_lash->p_osm->log;
	int sw;

	OSM_LOG_ENTER(p_log);

	for (sw = 0; sw < p_lash->num_switches; sw++) {
		/*
		 * skip switches with more links than MAX_DEGREE
		 * since they will never match a known case
		 */
		if (p_lash->switches[sw]->node->num_links > MAX_DEGREE)
			continue;

		if (get_switch_metric(p_lash, sw)) {
			OSM_LOG_EXIT(p_log);
			return -1;
		}
		classify_switch(p_lash, mesh, sw);
		classify_mesh_type(p_lash, sw);
	}

	OSM_LOG_EXIT(p_log);
	return 0;
}

/*
 * osm_mesh_delete - free per mesh resources
 */
static void mesh_delete(mesh_t *mesh)
{
	if (mesh) {
		if (mesh->class_type)
			free(mesh->class_type);

		if (mesh->class_count)
			free(mesh->class_count);

		free(mesh);
	}
}

/*
 * osm_mesh_create - allocate per mesh resources
 */
static mesh_t *mesh_create(lash_t *p_lash)
{
	osm_log_t *p_log = &p_lash->p_osm->log;
	mesh_t *mesh;

	if(!(mesh = calloc(1, sizeof(mesh_t))))
		goto err;

	if (!(mesh->class_type = calloc(p_lash->num_switches, sizeof(int))))
		goto err;

	if (!(mesh->class_count = calloc(p_lash->num_switches, sizeof(int))))
		goto err;

	return mesh;

err:
	mesh_delete(mesh);
	OSM_LOG(p_log, OSM_LOG_ERROR, "Failed allocating mesh - out of memory\n");
	return NULL;
}

/*
 * osm_mesh_node_delete - cleanup per switch resources
 */
void osm_mesh_node_delete(lash_t *p_lash, switch_t *sw)
{
	osm_log_t *p_log = &p_lash->p_osm->log;
	int i;
	mesh_node_t *node = sw->node;
	unsigned num_ports = sw->p_sw->num_ports;

	OSM_LOG_ENTER(p_log);

	if (node) {
		if (node->links) {
			for (i = 0; i < num_ports; i++) {
				if (node->links[i]) {
					if (node->links[i]->ports)
						free(node->links[i]->ports);
					free(node->links[i]);
				}
			}
			free(node->links);
		}

		if (node->poly)
			free(node->poly);

		if (node->matrix) {
			for (i = 0; i < node->num_links; i++) {
				if (node->matrix[i])
					free(node->matrix[i]);
			}
			free(node->matrix);
		}

		if (node->axes)
			free(node->axes);

		free(node);

		sw->node = NULL;
	}

	OSM_LOG_EXIT(p_log);
}

/*
 * osm_mesh_node_create - allocate per switch resources
 */
int osm_mesh_node_create(lash_t *p_lash, switch_t *sw)
{
	osm_log_t *p_log = &p_lash->p_osm->log;
	int i;
	mesh_node_t *node;
	unsigned num_ports = sw->p_sw->num_ports;

	OSM_LOG_ENTER(p_log);

	if (!(node = sw->node = calloc(1, sizeof(mesh_node_t))))
		goto err;

	if (!(node->links = calloc(num_ports, sizeof(link_t *))))
		goto err;

	for (i = 0; i < num_ports; i++) {
		if (!(node->links[i] = calloc(1, sizeof(link_t))) ||
		    !(node->links[i]->ports = calloc(num_ports, sizeof(int))))
			goto err;
	}

	if (!(node->axes = calloc(num_ports, sizeof(int))))
		goto err;

	for (i = 0; i < num_ports; i++) {
		node->links[i]->switch_id = NONE;
	}

	OSM_LOG_EXIT(p_log);
	return 0;

err:
	osm_mesh_node_delete(p_lash, sw);
	OSM_LOG(p_log, OSM_LOG_ERROR, "Failed allocating mesh node - out of memory\n");
	OSM_LOG_EXIT(p_log);
	return -1;
}

/*
 * osm_do_mesh_analysis
 */
int osm_do_mesh_analysis(lash_t *p_lash)
{
	osm_log_t *p_log = &p_lash->p_osm->log;
	mesh_t *mesh;

	OSM_LOG_ENTER(p_log);

	mesh = mesh_create(p_lash);
	if (!mesh)
		goto err;

	if (get_local_geometry(p_lash, mesh))
		goto err;

	if (mesh->num_class == 0) {
		OSM_LOG(p_log, OSM_LOG_INFO, "found no likely mesh nodes - done\n");
		goto done;
	}

done:
	mesh_delete(mesh);
	OSM_LOG_EXIT(p_log);
	return 0;

err:
	mesh_delete(mesh);
	OSM_LOG_EXIT(p_log);
	return -1;
}