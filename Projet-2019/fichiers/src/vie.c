
#include "compute.h"
#include "debug.h"
#include "global.h"
#include "graphics.h"
#include "ocl.h"
#include "scheduler.h"
#include "constants.h"

#include <stdbool.h>

static int isAlive(int x, int y);
static int willBeAlive(int x, int y);
static unsigned couleur = 0xFFFF00FF; // Yellow

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int isAlive(int x, int y){
  return cur_img(x,y) != 0;
}

int willBeAlive(int x, int y)
{
  int nbAlive=0;
  for(int i = MAX(x-1, 0); i <= MIN(x+1, DIM-1); i++){
    for(int j = MAX(y-1, 0); j <= MIN(y+1, DIM-1); j++){
      if((i != x || j != y) && isAlive(i,j)){
        nbAlive++;
      }
    }
  }
  if(!isAlive(x,y) && nbAlive==3)
    return couleur;
  else if(isAlive(x, y) && (nbAlive==2 || nbAlive==3))
    return couleur;
  else
    return 0x00;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int compute_new_state (int y, int x)
{
  unsigned n      = 0;
  unsigned change = 0;

  if (x > 0 && x < DIM - 1 && y > 0 && y < DIM - 1) {
    for (int i = y - 1; i <= y + 1; i++)
      for (int j = x - 1; j <= x + 1; j++)
        if (i != y || j != x)
          n += (cur_img (i, j) != 0);

    if (cur_img (y, x) != 0) {
      if (n == 2 || n == 3)
        n = 0xFFFF00FF;
      else {
        n      = 0;
        change = 1;
      }
    } else {
      if (n == 3) {
        n      = 0xFFFF00FF;
        change = 1;
      } else
        n = 0;
    }

    next_img (y, x) = n;
  }

  return change;
}

static int traiter_tuile (int i_d, int j_d, int i_f, int j_f)
{
  unsigned change = 0;

  PRINT_DEBUG ('c', "tuile [%d-%d][%d-%d] traitée\n", i_d, i_f, j_d, j_f);

  for (int i = i_d; i <= i_f; i++)
    for (int j = j_d; j <= j_f; j++)
      change |= compute_new_state (i, j);

  return change;
}

// Renvoie le nombre d'itérations effectuées avant stabilisation, ou 0
unsigned vie_compute_seq (unsigned nb_iter)
{
  for (unsigned it = 1; it <= nb_iter; it++) {

    // On traite toute l'image en un coup (oui, c'est une grosse tuile)
    unsigned change = traiter_tuile (0, 0, DIM - 1, DIM - 1);

    swap_images ();
  }

  return 0;
}


/*******************************************************************/
/*******************************************************************/
/*******************Versions OpenMP for*******************************/
/*******************************************************************/

unsigned vie_compute_base_ompfor (unsigned nb_iter)
{
  for (unsigned it = 1; it <= nb_iter; it++) {
    // On traite toute l'image en un coup (oui, c'est une grosse tuile)
    unsigned change=0;
    #pragma omp parallel for
    for (int i = 0; i <= DIM - 1; i++)
    for (int j = 0; j <= DIM - 1; j++)
        change |= compute_new_state (i, j);
    swap_images ();
  }
  return 0;
}
unsigned vie_compute_base (unsigned nb_iter)
{
  for (unsigned it = 1; it <= nb_iter; it++) {
    // On traite toute l'image en un coup (oui, c'est une grosse tuile)
    //unsigned change=0;
    #pragma omp parallel for
    for (int i = 0; i <= DIM - 1; i++)
    for (int j = 0; j <= DIM - 1; j++)
        next_img(i,j)= willBeAlive(i, j);
    swap_images ();
  }
  return 0;
}

unsigned static tranche =0;

unsigned vie_compute_tiled_ompfor (unsigned nb_iter)
{
  tranche = DIM / GRAIN;
  for (unsigned it = 1; it <= nb_iter; it++) {
     unsigned change=0;
    #pragma omp parallel for collapse(2)
    for (int i = 0; i <GRAIN ; i++)
    {
      for (int j = 0; j < GRAIN; j++)
      {
        for (int x = i * tranche; x <= (i + 1) * tranche - 1; x++)
        {
          for (int y = j * tranche; y <= (j + 1) * tranche - 1; y++)
          {
            //if(willBeAlive(i,j)==couleur)
              change |= compute_new_state (x, y);
          } 
        }
      }
    }
    swap_images ();
  }
  return 0;
}

unsigned vie_compute_opt_ompfor (unsigned nb_iter)
{
  return 0;
}
/*******************************************************************/
/*******************************************************************/
/********************fin Versions OpenMP for************************/
/*******************************************************************/


/*******************************************************************/
/*******************************************************************/
/******************Versions OpenMP task*****************************/
/*******************************************************************/
/*unsigned vie_compute_tiled_task (unsigned nb_iter)
{
  tranche = DIM / GRAIN;
  for (unsigned it = 1; it <= nb_iter; it++) {
     unsigned change=0;
    #pragma omp parallel for collapse(2)
    for (int i = 0; i <GRAIN ; i++)
    {
      for (int j = 0; j < GRAIN; j++)
      {
	#pragma omp task firstprivate(x,y)
        for (int x = i * tranche; x <= (i + 1) * tranche - 1; x++)
        {
          for (int y = j * tranche; y <= (j + 1) * tranche - 1; y++)
          {
            change |= compute_new_state (x, y);
          } 
        }
      }
    }
    swap_images ();
  }
  return 0;
}*/
/*******************************************************************/
/*******************************************************************/
/******************fin Versions OpenMP task*************************/
/*******************************************************************/


/*******************************************************************/
/*******************************************************************/
/******************Versions OpenCL *********************************/
/************************************************************static int isAlive(int x, int y);
static int willBeAlive(int x, int y);*******/

unsigned vie_compute_opencl (unsigned nb_iter)
{
  return 0;
}


/*******************************************************************/
/*******************************************************************/
/******************fin Versions OpenCL *****************************/
/*******************************************************************/


/*******************************************************************/
/*******************************************************************/
/******************Versions MPI ************************************/
/*******************************************************************/

unsigned vie_compute__mpi (unsigned nb_iter)
{
  return 0;
}
/*******************************************************************/
/*******************************************************************/
/******************fin Version  MPI ********************************/
/*******************************************************************/



///////////////////////////// Configuration initiale

void draw_stable (void);
void draw_guns (void);
void draw_random (void);
void draw_clown (void);
void draw_diehard (void);

void vie_draw (char *param)
{
  char func_name[1024];
  void (*f) (void) = NULL;

  if (param == NULL)
    f = draw_guns;
  else {
    sprintf (func_name, "draw_%s", param);
    f = dlsym (DLSYM_FLAG, func_name);

    if (f == NULL) {
      PRINT_DEBUG ('g', "Cannot resolve draw function: %s\n", func_name);
      f = draw_guns;
    }
  }

  f ();
}



static void gun (int x, int y, int version)
{
  bool glider_gun[11][38] = {
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
       0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0,
       0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0},
      {0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0,
       0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1,
       0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0,
       0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  };

  if (version == 0)
    for (int i = 0; i < 11; i++)
      for (int j = 0; j < 38; j++)
        if (glider_gun[i][j])
          cur_img (i + x, j + y) = couleur;

  if (version == 1)
    for (int i = 0; i < 11; i++)
      for (int j = 0; j < 38; j++)
        if (glider_gun[i][j])
          cur_img (x - i, j + y) = couleur;

  if (version == 2)
    for (int i = 0; i < 11; i++)
      for (int j = 0; j < 38; j++)
        if (glider_gun[i][j])
          cur_img (x - i, y - j) = couleur;

  if (version == 3)
    for (int i = 0; i < 11; i++)
      for (int j = 0; j < 38; j++)
        if (glider_gun[i][j])
          cur_img (i + x, y - j) = couleur;
}

void draw_stable (void)
{
  for (int i = 1; i < DIM - 2; i += 4)
    for (int j = 1; j < DIM - 2; j += 4)
      cur_img (i, j) = cur_img (i, (j + 1)) = cur_img ((i + 1), j) =
          cur_img ((i + 1), (j + 1))        = couleur;
}

void draw_guns (void)
{
  memset (&cur_img (0, 0), 0, DIM * DIM * sizeof (cur_img (0, 0)));

  gun (0, 0, 0);
  gun (0, DIM - 1, 3);
  gun (DIM - 1, DIM - 1, 2);
  gun (DIM - 1, 0, 1);
}

void draw_random (void)
{
  for (int i = 1; i < DIM - 1; i++)
    for (int j = 1; j < DIM - 1; j++)
      cur_img (i, j) = random () & 01;
}

void draw_clown (void)
{
  memset (&cur_img (0, 0), 0, DIM * DIM * sizeof (cur_img (0, 0)));

  int mid                = DIM / 2;
  cur_img (mid, mid - 1) = cur_img (mid, mid) = cur_img (mid, mid + 1) =
      couleur;
  cur_img (mid + 1, mid - 1) = cur_img (mid + 1, mid + 1) = couleur;
  cur_img (mid + 2, mid - 1) = cur_img (mid + 2, mid + 1) = couleur;
}

void draw_diehard (void)
{
  memset (&cur_img (0, 0), 0, DIM * DIM * sizeof (cur_img (0, 0)));

  int mid = DIM / 2;

  cur_img (mid, mid - 3) = cur_img (mid, mid - 2) = couleur;
  cur_img (mid + 1, mid - 2)                      = couleur;

  cur_img (mid - 1, mid + 3)     = couleur;
  cur_img (mid + 1, mid + 2)     = cur_img (mid + 1, mid + 3) =
      cur_img (mid + 1, mid + 4) = couleur;
}
